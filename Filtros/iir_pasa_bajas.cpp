#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/stream_mux.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <iostream>
#include <vector>

int main() {
    // Crear el bloque principal
    auto tb = gr::make_top_block("LPS_IIR_Filter");

    // Parámetros de la señal
    const float fs = 44000.0f; // Frecuencia de muestreo
    const int tiempo_final = 40; // Tiempo final en ms
    const int num_muestras = static_cast<int>(fs * tiempo_final / 1000.0f); // Número total de muestras
    float f1 = 200.0f;   // Frecuencia baja
    float f2 = 2000.0f;  // Frecuencia alta 1
    float f3 = 5000.0f;  // Frecuencia alta 2

    // Fuentes de señal
    auto src1 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f1, 1.0, 0.0);
    auto src2 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f2, 0.7, 0.0);
    auto src3 = gr::analog::sig_source_f::make(fs, gr::analog::GR_SIN_WAVE, f3, 0.5, 0.0);

    // Suma src1 + src2
    auto adder1 = gr::blocks::add_ff::make();
    // Suma (src1+src2) + src3
    auto adder2 = gr::blocks::add_ff::make();

    /***********************************************************/
    //                  Cabecera de datos                              
    /***********************************************************/
    const char* archivo_datos = "iir_lpf_signal.dat";
    std::ofstream outfile(archivo_datos, std::ios::binary | std::ios::trunc);
    if (outfile.is_open()) {
        outfile << "fs=" << fs << "\n";
        outfile << "datatype=float\n";
        outfile << "datasize=" << sizeof(float) << "\n"; // Tamaño de dato de cada señal
        outfile << "num_streams=2\n"; // Número de streams multiplexados
        outfile << "mux_format=1,1\n"; // Formato de multiplexado: 1 elemento de cada stream
        auto now = std::chrono::system_clock::now();
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        outfile << "timestamp=" << now_sec << "\n";
        outfile.close();
    } else {
        std::cerr << "No se pudo abrir iir_lpf_signal.dat para escribir el header." << std::endl;
        return 1;
    }

    // Sink de archivo
    auto sink = gr::blocks::file_sink::make(sizeof(float), archivo_datos, true); // true: append mode

    // Limitador de muestras para cada señal
    auto head_unfiltered = gr::blocks::head::make(sizeof(float), num_muestras);
    auto head_filtered   = gr::blocks::head::make(sizeof(float), num_muestras);

    // Mux para combinar dos flujos en uno
    std::vector<int> input_lengths = {1, 1}; // Intercalado de 1 float cada uno
    auto mux = gr::blocks::stream_mux::make(sizeof(float), input_lengths);

    /***********************************************************/
    //           Diseño del filtro IIR pasa-bajas                            
    /***********************************************************/
    // Coeficientes generados en Octave (iir_coeficientes.txt)
    std::vector<double> feedforward = {0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f};
    std::vector<double> feedback = {1.00000000f, -8.82357023f, 34.64828281f, -79.47088791f, 117.33243477f, -115.63875002f, 76.07845294f, -32.21785965f, 7.96909649f, -0.87719917f};

    std::cout << "Orden del filtro IIR: " << feedback.size() - 1 << std::endl;

    auto iir = gr::filter::iir_filter_ffd::make(feedforward, feedback);

    // Conexiones
    tb->connect(src1, 0, adder1, 0);
    tb->connect(src2, 0, adder1, 1);
    tb->connect(adder1, 0, adder2, 0);
    tb->connect(src3, 0, adder2, 1);
    tb->connect(adder2, 0, iir, 0);
    tb->connect(adder2, 0, head_unfiltered, 0); // suma sin filtrar
    tb->connect(iir, 0, head_filtered, 0);      // suma filtrada
    tb->connect(head_unfiltered, 0, mux, 0);    // primer señal
    tb->connect(head_filtered, 0, mux, 1);      // segunda señal
    tb->connect(mux, 0, sink, 0);

    // Ejecutar flujo
    tb->start();
    tb->wait();
    tb->stop();
}
