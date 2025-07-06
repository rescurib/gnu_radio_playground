// audio_recorder.cpp
// Programa de línea de comandos para grabar audio usando GNU Radio
// Uso: ./audio_recorder <duracion_segundos> <archivo_salida.wav> <dispositivo_entrada>
// Ejemplo: ./audio_recorder 5 grabacion.wav hw:0,0

#include <gnuradio/top_block.h>
#include <gnuradio/audio/source.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <duracion_segundos> <archivo_salida.wav> <dispositivo_entrada>" << std::endl;
        return 1;
    }

    int duracion = std::stoi(argv[1]);
    // archivo_salida debe ser char* para wavfile_sink::make
    char* archivo_salida = argv[2];
    std::string dispositivo = argv[3];

    // Parámetros por defecto
    double samp_rate = 44100; // Frecuencia de muestreo estándar
    int nchan = 1; // Mono

    std::cout << "Grabando " << duracion << " segundos de audio desde '" << dispositivo << "' en '" << archivo_salida << "'..." << std::endl;

    // Crear bloques de GNU Radio
    auto src = gr::audio::source::make(samp_rate, dispositivo, nchan);
    // wavfile_sink::make espera un const char* como primer argumento
    auto sink = gr::blocks::wavfile_sink::make(static_cast<const char*>(archivo_salida),
                                                1, 
                                                samp_rate,
                                                gr::blocks::FORMAT_WAV,
                                                gr::blocks::FORMAT_PCM_16,
                                                false);

    // Crear top block
    auto tb = gr::make_top_block("audio_recorder");
    tb->connect(src, 0, sink, 0);

    // Iniciar grabación
    tb->start();
    std::this_thread::sleep_for(std::chrono::seconds(duracion));
    tb->stop();
    tb->wait();

    std::cout << "Grabación finalizada." << std::endl;
    return 0;
}
