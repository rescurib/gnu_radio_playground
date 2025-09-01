#include <iostream>
#include <thread>
#include <chrono>

#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/digital/cpmmod_bc.h>

#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/filter/firdes.h>

#include <gnuradio/blocks/multiply.h>
#include <complex>
#include <gnuradio/fft/goertzel_fc.h>

#include <gnuradio/qtgui/time_sink_c.h>
#include <QWidget>
#include <QApplication>


int main(int argc, char** argv) {

    /*************************************************/
    /*     Generación de flujo de bits aleatorios    */
    /*************************************************/

     // Inicializar Qt GUI
     QApplication app(argc, argv);
     auto tb = gr::make_top_block("MSK en banda base");

    // Fuente WAV
    auto wav_source = gr::blocks::wavfile_source::make("msk_800_Hz_200_bps.wav", true); // true: repetir

    // Leer tasa de muestreo del archivo
    // (nota: los objetos de bloque son shared_ptr's)
    const int samp_rate = wav_source->sample_rate();

    // Sumador y convertidores de componentes IQ a FI
    auto c2ff   = gr::blocks::complex_to_float::make();

    // Demodulador de cuadratura
    auto qdemod = gr::analog::quadrature_demod_cf::make(1.0);

    // Conversión de Frecuencia Intermedia (IF) a Banda Base
    const float fc = 800; // Frecuencia de la portadora (Hz)
    auto mixer_osc = gr::analog::sig_source_c::make(samp_rate, gr::analog::GR_COS_WAVE, fc, 1.0, 0.0);

    // Multiplicador complejo para mezclar la señal MSK con el oscilador
    auto mixer = gr::blocks::multiply_cc::make();

    // IQ demodulator
    auto ff2c = gr::blocks::float_to_complex::make();
    auto zero_src  = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_CONST_WAVE, 0, 0.0, 0.0);

    /************************************************/
    /*          Filtro para demodulador             */
    /************************************************/

    // Filtro pasa bajas de 400 Hz
    // Diseño de filtro FIR
    const float lpf_cutoff = 400.0f;  // Frecuencia de corte
    const float lpf_trans  = 200.0f;  // Ancho de transición
    auto taps = gr::filter::firdes::low_pass(
                                     1.0, 
                                     samp_rate,
                                     lpf_cutoff, 
                                     lpf_trans, 
                                     gr::fft::window::win_type::WIN_HAMMING
                                   );
    // Convert to complex taps
    std::vector<gr_complex> complex_taps;
    for (auto t : taps) {
        complex_taps.push_back(gr_complex(t, 0.0f));
    }

    std::cout << "Orden del filtro FIR: " << complex_taps.size() - 1 << std::endl;
    auto lpf = gr::filter::fir_filter_ccc::make(1, complex_taps);

    /*************************************************/
    /*              Sumidero  GUI                    */
    /*************************************************/

    // Crear visualizador en tiempo (QT GUI Time Sink)
    const int size = 1024; // Muestras para mostrar
    //const double samp_rate = 1000.0; 
    const std::string name = "MSK en Banda Base";
    const unsigned int nconnections = 1;

    auto time_sink = gr::qtgui::time_sink_c::make(size, samp_rate, name, nconnections, nullptr);
    time_sink->set_update_time(0.10);    
    time_sink->set_y_axis(-1.5, 1.5);   
    time_sink->enable_autoscale(false);  // Mantener rango de ejes fijos
    time_sink->enable_grid(true);
    time_sink->set_y_label("Amplitud", "");
    time_sink->set_line_label(0, "Bitstream");
    time_sink->set_line_color(0, "blue");
    time_sink->enable_control_panel(true);
    time_sink->enable_tags(0, false);

    // Mostrar GUI
    time_sink->qwidget()->show();

    // Conectar bloques
  
    // Mezclar la señal MSK con el oscilador complejo
    tb->connect(wav_source, 0, ff2c, 0);
    tb->connect(zero_src,   0, ff2c, 1); // Parte imaginaria en cero
    tb->connect(ff2c, 0, mixer, 0);
    tb->connect(mixer_osc, 0, mixer, 1);
    tb->connect(mixer, 0, lpf,0);
   // tb->connect(lpf, 0, qdemod, 0);
    tb->connect(lpf, 0, time_sink, 0);
   
    // Iniciar flujo
    tb->start();

    // Correr loop de Qt
    app.exec();

    // Detener flujo cuando se cierre la ventana de Qt
    tb->stop();
    tb->wait();

    return 0;
}