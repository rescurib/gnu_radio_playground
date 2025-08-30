#include <iostream>
#include <thread>
#include <chrono>

#include <gnuradio/random.h>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/analog/random_uniform_source.h>
#include <gnuradio/blocks/uchar_to_float.h>
#include <gnuradio/blocks/float_to_char.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/digital/cpmmod_bc.h>

#include <gnuradio/blocks/multiply.h>

#include <gnuradio/qtgui/time_sink_f.h>
#include <QWidget>
#include <QApplication>


int main(int argc, char** argv) {

    /*************************************************/
    /*     Generación de flujo de bits aleatorios    */
    /*************************************************/

     // Inicializar Qt GUI
     QApplication app(argc, argv);
     auto tb = gr::make_top_block("Stream de bits aleatorios");

    // Crear fuente de bits aleatorios (uint8_t)
    auto rand_src = gr::analog::random_uniform_source_b::make(0, 2, 0); // (min, max, seed)

    // Convert enteros a flotantes: (+/-1)
    auto uchar_to_float = gr::blocks::uchar_to_float::make();
    auto map_to_bipolar = gr::blocks::add_const_ff::make(-0.5);     // 0/1 -> -0.5/0.5

    // Escalamiento adecuado para el modulador de fase
    auto scale_to_pm = gr::blocks::multiply_const_ff::make(2.0); // -0.5/0.5 -> -1/+1
    auto bb_pm       = gr::blocks::float_to_char::make();

    // Sumador y convertidores de componentes IQ a FI
    auto c2ff   = gr::blocks::complex_to_float::make();
    auto iq_sum = gr::blocks::sub_ff::make();

    /*************************************************/
    /*              Modulador MSK                    */
    /*************************************************/
    const double bit_rate = 200.0;
    const int samples_per_sym = 32;
    const double samp_rate = bit_rate * samples_per_sym;

    auto msk_mod = gr::digital::cpmmod_bc::make(
        gr::analog::cpm::LREC, /* tipo de pulso rectangular -> MSK */
        0.5,                   // h = 0.5 para MSK
        samples_per_sym,       // muestras por símbolo
        1                      // L=1 -> full response
                               // beta no se usa en LREC
    );

    // Conversión Banda Base a Frecuencia Intermedia (IF)
    const float fc = 800; // Frecuencia de la portadora (Hz)
    auto mixer_osc = gr::analog::sig_source_c::make(samp_rate, gr::analog::GR_COS_WAVE, fc, 1.0, 0.0);

    // Multiplicador complejo para mezclar la señal MSK con el oscilador
    auto mixer = gr::blocks::multiply_cc::make();

    /*************************************************/
    /*              Sumidero  GUI                    */
    /*************************************************/

    // Crear visualizador en tiempo (QT GUI Time Sink)
    const int size = 1024; // Muestras para mostrar
    const std::string name = "Modulación MSK de tren de bits aleatorios";
    const unsigned int nconnections = 1;

    auto time_sink = gr::qtgui::time_sink_f::make(size, samp_rate, name, nconnections, nullptr);
    time_sink->set_update_time(0.10);    
    time_sink->set_y_axis(-1.5, 1.5);   
    time_sink->enable_autoscale(false);  // Mantener rango de ejes fijos
    time_sink->enable_grid(true);
    time_sink->set_y_label("Amplitud", "");
    time_sink->set_line_label(0, "Señal MSK");
    time_sink->set_line_color(0, "blue");
    time_sink->enable_control_panel(true);
    time_sink->enable_tags(0, false);

    // Mostrar GUI
    time_sink->qwidget()->show();

    // Conectar bloques
    tb->connect(rand_src, 0, uchar_to_float, 0);
    tb->connect(uchar_to_float, 0, map_to_bipolar, 0);
    tb->connect(map_to_bipolar, 0, scale_to_pm, 0);
    tb->connect(scale_to_pm, 0, bb_pm, 0);
    tb->connect(bb_pm, 0, msk_mod, 0);

    // Mezclar la señal MSK con el oscilador complejo para subir a banda IF
    tb->connect(msk_mod, 0, mixer, 0);
    tb->connect(mixer_osc, 0, mixer, 1);
    tb->connect(mixer, 0, c2ff, 0); // Tomar parte real: y = I*cos(th) - Q*sin(th)
    tb->connect(c2ff, 0, time_sink, 0);

    // Iniciar flowgraph
    tb->start();

    // Correr loop de Qt
    app.exec();

    // Detener flujo cuando se cierre la ventana de Qt
    tb->stop();
    tb->wait();

    return 0;
}