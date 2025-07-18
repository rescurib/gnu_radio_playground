# Notas sueltas y experimentos con la API de GNU Radio para C++

He comenzado aprender a usar esta API para utilizarla en proyectos en la banda VLF con aplicaciones para radio-astronomía y ciencias espaciales. En particular, con instalaciones de [antenas de loop](https://physicsopenlab.org/2020/05/03/loop-antenna-for-very-low-frequency/) y tarjetas de sonido como dispositivos de digitalización. Este será mi repo para notas y pruebas.  

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

### Compilación con make

La API de GNU Radio esta dividida en muchos paquetes y sería un dolor de cabeza añadir manualmente las banderas de compilación que vamos a necesitar. Pero tenemos a nuestra herramienta [pkt-config](https://linux.die.net/man/1/pkg-config). Podemos automatizar esta tarea directamente en el archivo Makefile:
```Makefile
PKG_CONFIG = pkg-config
GNURADIO_PKGS = $(shell $(PKG_CONFIG) --list-all | grep '^gnuradio-' | cut -d ' ' -f 1)
CXXFLAGS += $(shell $(PKG_CONFIG) --cflags $(GNURADIO_PKGS))
LDFLAGS += $(shell $(PKG_CONFIG) --libs $(GNURADIO_PKGS)) -lfmt
```
Los argumentos de grep y cut son los siguientes:
* `^` significa "inicio línea".
* `cut` es un comando que extrae campos de lineas de texto.
* `-d ' '`: espacio como delimitador.
* `-f 1`: primer campo de cada línea.

GNU Radio también usa la librería [fmt](https://fmt.dev/11.1/) y para eso es el flag `-lfmt`.

Para compilar la mayoria de los ejemplos de estas notas, debes cambiar el nombre del proyecto en el Makefile al nombre del archivo fuente que necesites. En el caso de este programa de verificación de instalación:

```Makefile
# Nombre del proyecto
PROJECT_NAME = verify_gnu_radio
```

y hacer el build en el directorio del archivo fuente con:

```Bash
make
```