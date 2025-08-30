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
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/digital/cpmmod_bc.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/wavfile_sink.h>



int main(int argc, char** argv) {

    /*************************************************/
    /*   Parámetros por línea de comandos (CLI)      */
    /*************************************************/
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <duración_segundos> <archivo_wav>" << std::endl;
        return 1;
    }
    // Duración en segundos y nombre de archivo WAV
    int duracion_segundos = std::stoi(argv[1]);
    const char*  archivo_wav = argv[2];

    /*************************************************/
    /*     Generación de flujo de bits aleatorios    */
    /*************************************************/
    auto tb = gr::make_top_block("Stream de bits aleatorios");

    // Crear fuente de bits aleatorios (uint8_t)
    auto rand_src = gr::analog::random_uniform_source_b::make(0, 2, 0); // (min, max, seed)

    // Convertir enteros a flotantes: (+/-1)
    auto uchar_to_float = gr::blocks::uchar_to_float::make();
    auto map_to_bipolar = gr::blocks::add_const_ff::make(-0.5);     // 0/1 -> -0.5/0.5

    // Escalamiento adecuado para el modulador de fase
    auto scale_to_pm = gr::blocks::multiply_const_ff::make(2.0); // -0.5/0.5 -> -1/+1
    auto bb_pm       = gr::blocks::float_to_char::make();

    // Convertidor de componentes IQ a FI
    auto c2ff   = gr::blocks::complex_to_float::make();

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
    /*        Limitador de muestras y sumidero       */
    /*************************************************/
    // Calcular número total de muestras
    int num_muestras = static_cast<int>(samp_rate * duracion_segundos);

    // Limitador de muestras
    auto head = gr::blocks::head::make(sizeof(float), num_muestras);

    // Sink WAV para la señal
    auto wav_sink = gr::blocks::wavfile_sink::make(archivo_wav,
                                                   1, /* Canales*/
                                                   static_cast<int>(samp_rate),
                                                   gr::blocks::FORMAT_WAV,
                                                   gr::blocks::FORMAT_PCM_16 
                                                ); 

    // Conectar bloques
    tb->connect(rand_src, 0, uchar_to_float, 0);
    tb->connect(uchar_to_float, 0, map_to_bipolar, 0);
    tb->connect(map_to_bipolar, 0, scale_to_pm, 0);
    tb->connect(scale_to_pm, 0, bb_pm, 0);
    tb->connect(bb_pm, 0, msk_mod, 0);
    tb->connect(msk_mod, 0, mixer, 0);
    tb->connect(mixer_osc, 0, mixer, 1);
    tb->connect(mixer, 0, c2ff, 0); // Tomar parte real: y = I*cos(th) - Q*sin(th)
    tb->connect(c2ff, 0, head, 0);
    tb->connect(head, 0, wav_sink, 0);

    // Ejecutar flujo
    tb->start();
    tb->wait();
    tb->stop();

    std::cout << "Archivo WAV generado: " << archivo_wav << " con " << num_muestras << " muestras." << std::endl;
    return 0;
}