# Notas sueltas y experimentos con la API de GNU Radiode de C++

## Instalación en Linux Mint / Ubuntu

```Bash
sudo apt update
sudo apt upgrade
sudo apt-get install gnuradio
sudo apt install gnuradio-dev
```
Modificar archivo de fuentes de APT
```Bash
sudo nano /etc/apt/sources.list
```
```Bash
deb http://packages.linuxmint.com una main upstream import backport 
deb http://archive.ubuntu.com/ubuntu focal main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu focal-updates main restricted universe multiverse
deb http://security.ubuntu.com/ubuntu focal-security main restricted universe multiverse
```

Después de guardar, ejecutar:
```Bash
sudo apt update
sudo apt build-dep gnuradio
```

## Verificación de instalación

Revisar versión de API:
```bash
pkg-config --modversion gnuradio-runtime
```

En mi caso estoy usando la versión 3.10.9. La documentación puede encontrarse [acá](https://www.gnuradio.org/doc/doxygen/index.html).

Programa de verificación
```C++
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

    // Crear e iniciar el hilo del flowgraph
    tb->start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Detener el flowgraph
    tb->stop();

    // Esperar a que el hilo del flowgraph termine
    tb->wait();

    std::cout << "Flowgraph ejecutado exitosamente." << std::endl;
    return 0;
}
```