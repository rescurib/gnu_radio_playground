#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <pmt/pmt.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <chrono>

int main(int argc, char** argv) {

    // Parámetros de la señal
    const float fs = 44000.0f; // Frecuencia de muestreo
    const int tiempo_final = 40; // Tiempo final en ms
    const int num_muestras = static_cast<int>(fs * tiempo_final / 1000.0f); // Número total de muestras
    float f1 = 200.0f;   // Frecuencia baja
    float f2 = 2000.0f;  // Frecuencia alta 1
    float f3 = 5000.0f;  // Frecuencia alta 2

    auto tb = gr::make_top_block("generador");

    // Fuentes de señal
    auto src1 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f1, 1.0, 0.0);
    auto src2 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f2, 0.7, 0.0);
    auto src3 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f3, 0.25, 0.0);

    // Suma src1 + src2
    auto adder1 = gr::blocks::add_ff::make();
    // Suma (src1+src2) + src3
    auto adder2 = gr::blocks::add_ff::make();

    // Escribir header personalizado en signal.dat
    std::ofstream outfile("signal.dat", std::ios::binary | std::ios::trunc);
    if (outfile.is_open()) {
        outfile << "fs=" << fs << "\n";
        outfile << "datatype=float\n";
        outfile << "datasize=" << sizeof(float)*1 << "\n";
        auto now = std::chrono::system_clock::now();
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        outfile << "timestamp=" << now_sec << "\n";
        outfile.close();
    } else {
        std::cerr << "No se pudo abrir signal.dat para escribir el header." << std::endl;
        return 1;
    }

    // Sink de archivo para una señal
    auto sink = gr::blocks::file_sink::make(sizeof(float)*1, "signal.dat", true); // true: append mode

    // Limitador de muestras para cada señal
    auto head = gr::blocks::head::make(sizeof(float), num_muestras);

    // Conexiones
    tb->connect(src1, 0, adder1, 0);
    tb->connect(src2, 0, adder1, 1);
    tb->connect(adder1, 0, adder2, 0);
    tb->connect(src3, 0, adder2, 1);
    tb->connect(adder2, 0, head, 0);
    tb->connect(head, 0, sink, 0);

      // Ejecutar flujo
    tb->start();
    tb->wait();
    tb->stop();

}