#include <iostream>
#include <thread>
#include <chrono>

#include <gnuradio/random.h>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/random_uniform_source.h>
#include <gnuradio/blocks/uchar_to_float.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/multiply_const.h>

#include <gnuradio/qtgui/time_sink_f.h>
#include <QWidget>
#include <QApplication>


int main(int argc, char** argv) {
    /*********************************************/
    /*          Generación de escalares          */
    /*********************************************/
    // Crear objeto de generación de números aleatorios
    gr::random rng(12345);  // time(0) o 0 para semilla de tiempo de sistema

    // Generar flotante aleatorio entre [0.0, 1.0)
    float r = rng.ran1();

    std::cout << " Número aleatorio: " << r << std::endl;

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

    // Escalamiento adacuado para el modulador de fase del próximo programa
    auto scale_to_pm = gr::blocks::multiply_const_ff::make(2.0); // -0.5/0.5 -> -1/+1

    // Crear visualizador en tiempo (QT GUI Time Sink)
    const int size = 1024; // Muestras para mostrar
    const double samp_rate = 1000.0; 
    const std::string name = "Flujo de bits aleatorios";
    const unsigned int nconnections = 1;

    auto time_sink = gr::qtgui::time_sink_f::make(size, samp_rate, name, nconnections, nullptr);
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
    tb->connect(rand_src, 0, uchar_to_float, 0);
    tb->connect(uchar_to_float, 0, map_to_bipolar, 0);
    tb->connect(map_to_bipolar, 0, scale_to_pm, 0);
    tb->connect(scale_to_pm, 0, time_sink, 0);

    // Iniciar flowgraph
    tb->start();

    // Correr loop de Qt
    app.exec();

    // Detener flujo cuando se cierre la ventana de Qt
    tb->stop();
    tb->wait();

    return 0;
}
