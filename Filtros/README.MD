# Implementación de filtros digitales con GNU Radio

En esta carpeta voy a colocar notas de implementación de diferentes filtros digitales.

## Bloque de generación de señal.

La API de GNU Radio usa constantemente una característica de C++ llamada [Plantillas de Clase](https://learn.microsoft.com/es-es/cpp/cpp/templates-cpp). Es muy importante entender este concepto o será muy confuso utilizarla. [Aquí](https://www.youtube.com/watch?v=mQqzP9EWu58) hay un video conciso y bueno.

`sig_source`[[doc](https://www.gnuradio.org/doc/doxygen/classgr_1_1analog_1_1sig__source.html)] es una platilla de clase que nos permite instanciar generadores de señales de diferentes tipos de datos. La manera en la que la documentación oficial presenta las diferetes plantillas me parece confusa así que suelo referir directamente al archivo de header [sig_source.h](https://www.gnuradio.org/doc/doxygen/sig__source_8h_source.html) en donde podemos ver los siguientes:

```C++
 typedef sig_source<std::int8_t> sig_source_b;
 typedef sig_source<std::int16_t> sig_source_s;
 typedef sig_source<std::int32_t> sig_source_i;
 typedef sig_source<float> sig_source_f;
 typedef sig_source<gr_complex> sig_source_c;
```
Por ejemplo, para crear una fuente senoidal de tipo *float*:

```C++
#include <gnuradio/analog/sig_source.h>

const float fs = 44000.0f; 
float f1 = 200.0f;   
auto src_sin = gr::analog::sig_source_f::make(
                 fs, /* Frec. de muestreo*/
                 gr::analog::GR_SIN_WAVE, 
                 f1,  /* Frec. de señal */
                 1.0, /* Amplitud*/
                 0.0,  /* Offset de amplitud */
                 0.0, /* Fase inicial */
               );
```

## Bloque Head

Si queremos que el flujo se detenga después de un número definido de muestras, necesitamos el bloque `head`[[doc](https://www.gnuradio.org/doc/doxygen/classgr_1_1blocks_1_1head.html)]. 

Por ejemplo, para procesar solo los primeros 20 ms de la señal del ejemplo anterior:

```C++
#include <gnuradio/blocks/head.h>

const float fs = 44000.0f; // Frecuencia de muestreo
const int tiempo_final = 40; // Tiempo final en ms
const int num_muestras = static_cast<int>(fs * tiempo_final / 1000.0f);

// Limitador de muestras
auto head = gr::blocks::head::make(sizeof(float), num_muestras);
```