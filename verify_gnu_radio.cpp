#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/blocks/null_sink.h>

#include <chrono>
#include <thread>
#include <iostream>

int main() {
    std::cout << "Iniciando flowgraph de GNU Radio..." << std::endl;

    double samp_rate = 32000;

    // Fuente de señal: genera una onda senoidal de 1 kHz y amplitud 1.0
    auto src = gr::analog::sig_source_f::make(
        samp_rate,
        gr::analog::GR_SIN_WAVE,
        1000.0,   // frecuencia
        1.0       // amplitud
    );

    // Bloque throttle: limita la velocidad de procesamiento a la frecuencia de muestreo
    auto throttle = gr::blocks::throttle::make(sizeof(float), samp_rate);

    // Sumidero nulo: descarta los datos, solo para verificar el flujo
    auto sink = gr::blocks::null_sink::make(sizeof(float));

    // Crea el top block (flujo principal de bloques)
    auto tb = gr::make_top_block("verify_gnuradio");

    // Conecta los bloques: src -> throttle -> sink
    // (0 es el puerto de salida, 0 es el puerto de entrada)
    tb->connect(src, 0, throttle, 0);
    tb->connect(throttle, 0, sink, 0);

    tb->start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    tb->stop();
    tb->wait();

    std::cout << "Flowgraph ejecutado exitosamente." << std::endl;
    return 0;
}
