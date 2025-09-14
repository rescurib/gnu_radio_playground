#include <iostream>
#include <thread>
#include <chrono>

#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/audio/source.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter.h>
#include <gnuradio/blocks/multiply.h>
#include <complex>
#include <gnuradio/fft/goertzel_fc.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/qtgui/time_sink_c.h>
#include <QWidget>
#include <QApplication>

// Bloque personalizado para imprimir la amplitud y fase de la señal
// Hereda de gr::sync_block para integrarse en el flujo de GNU Radio
class print_block : public gr::sync_block {
public:
    // Definición de tipo para shared_ptr
    typedef std::shared_ptr<print_block> sptr;
    // Método estático para crear instancias del bloque
    static sptr make() {
        return gnuradio::get_initial_sptr(new print_block());
    }

    // Constructor: define nombre y firmas de entrada/salida
    print_block()
        : gr::sync_block("print_block",
                         gr::io_signature::make(1, 1, sizeof(gr_complex)),
                         gr::io_signature::make(0, 0, 0)) {}

    // Método principal de procesamiento del bloque
    // Imprime amplitud y fase de cada muestra recibida
    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &) override {
        const gr_complex *in = (const gr_complex*) input_items[0];
        for (int i = 0; i < noutput_items; i++) {
            float amplitude = 2*std::abs(in[i]);
            float phase_rad = std::arg(in[i]);
            float phase_deg = phase_rad * 180.0f / M_PI; // Conversión a grados
            std::cout << "Amplitud: " << amplitude << ", Fase: " << phase_deg << " grados" << std::endl;
        }
        return noutput_items;
    }
};

int main(int argc, char** argv) {

     // Inicializar Qt GUI
     QApplication app(argc, argv);
     auto tb = gr::make_top_block("MSK en banda base");

    // Fuente de Tarjeta de Sonido (Sound Card)
    const int samp_rate = 48000; // Tasa de muestreo en Hz

    // Nota: obtener nombres de dispositivos con "arecord -l"
    auto soundcard = gr::audio::source::make(samp_rate, "plughw:1,0", true);

    // Convertidor de componentes FI a IQ
    auto c2ff   = gr::blocks::complex_to_float::make();
    const int decimation = 8;

    // Bloque Goertzel para obtención de fase
    const float goertzel_freq = 100.0f; // Frecuencia de interés
    const int batch_samples = static_cast<int>(samp_rate/decimation * 1); // n segundo(n)
    auto goertzel = gr::fft::goertzel_fc::make(samp_rate/decimation, batch_samples, goertzel_freq);

    // IQ demodulator
    auto ff2c = gr::blocks::float_to_complex::make();
    auto zero_src  = gr::analog::sig_source_f::make(samp_rate, gr::analog::GR_CONST_WAVE, 0, 0.0, 0.0);

    // Multiplicador para cuadrado de la señal
    auto mult = gr::blocks::multiply_cc::make();

    // Bloque a la medida para imprimir la fase
    auto printer = print_block::make(); 

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

    // Convertidor de frecuencia y filtrado en un solo bloque
    const float fc = 809; // Frecuencia de la portadora (Hz)
    auto freq_xlating = gr::filter::freq_xlating_fir_filter_ccc::make(
        decimation, complex_taps, fc, samp_rate
    );

    /*************************************************/
    /*              Sumidero  GUI                    */
    /*************************************************/

    // Crear visualizador en tiempo (QT GUI Time Sink)
    const int size = 1024; // Muestras para mostrar
    const std::string name = "MSK en Banda Base";
    const unsigned int nconnections = 1;

    auto time_sink = gr::qtgui::time_sink_c::make(size, samp_rate/decimation, name, nconnections, nullptr);
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
    tb->connect(soundcard, 0, ff2c, 0);
    tb->connect(zero_src,  0, ff2c, 1); // Parte imaginaria en cero
    tb->connect(ff2c, 0, freq_xlating, 0);
    tb->connect(freq_xlating,0,mult,0);
    tb->connect(freq_xlating,0,mult,1);
    tb->connect(mult, 0, c2ff, 0);
    tb->connect(c2ff, 0, goertzel, 0);
    tb->connect(goertzel, 0, printer, 0);
    tb->connect(mult, 0, time_sink, 0);

//    tb->connect(soundcard, 0, time_sink, 0);
   
    // Iniciar flujo
    tb->start();

    // Correr loop de Qt
    app.exec();

    // Detener flujo cuando se cierre la ventana de Qt
    tb->stop();
    tb->wait();

    return 0;
}