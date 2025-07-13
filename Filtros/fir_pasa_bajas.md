# Filtro Pasa-Bajas FIR

Vamos a reutilizar la suma de fuentes senoidales del primer ejemplo y también el sumidero de archivos, aunque habrán algunos cambios importantes.

## Bloque de filtro FIR

Para generar los coeficientes FIR usamos la plantilla de clase `fir_filter_blk`[[doc](https://www.gnuradio.org/doc/doxygen/classgr_1_1filter_1_1fir__filter__blk.html)] donde tenemos diferentes combinaciones de tipos de datos para:
* Señal de entrada 
* Señal de salida
* Coeficientes
```C++
 typedef fir_filter_blk<gr_complex, gr_complex, gr_complex> fir_filter_ccc;
 typedef fir_filter_blk<gr_complex, gr_complex, float> fir_filter_ccf;
 typedef fir_filter_blk<float, gr_complex, gr_complex> fir_filter_fcc;
 typedef fir_filter_blk<float, float, float> fir_filter_fff;
 typedef fir_filter_blk<float, std::int16_t, float> fir_filter_fsf;
 typedef fir_filter_blk<std::int16_t, gr_complex, gr_complex> fir_filter_scc;
```

En este ejemplo usarémos un bloque con entrada, salida y coeficientes tipo *float*: `fir_filter_fff`.

## Diseño y creación de filtro

Para generar los coeficientes utilizamos:

```C++
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/filter/firdes.h>

const float lpf_cutoff = 1000.0f;
const float lpf_trans  = 500.0f;  
auto taps = gr::filter::firdes::low_pass(
                                  1.0,        /* Ganancia */
                                  fs,         /* Frec. de muestreo */
                                  lpf_cutoff, /* Frec. de corte */
                                  lpf_trans,  /* Ancho de banda de transición*/
                                  gr::fft::window::win_type::WIN_HAMMING
                                );

auto lpf = gr::filter::fir_filter_fff::make(1, taps); // (decimación, coeficientes FIR)
```

## Multiplexor de flujo

Hasta aquí ya tenemos todo para conectar nuestro filtro al flujo, pero para guardar multiples señales en el archivo binario necesitamos de el bloque `stream_mux`[[doc](https://www.gnuradio.org/doc/doxygen/classgr_1_1blocks_1_1stream__mux.html)]. Este bloque toma varias señales de entrada y las combina en una sola secuencia, intercalando sus muestras. Si tenemos $N$ señales de entrada $x_1[n], x_2[n], \ldots, x_N[n]$, la salida será:

$$
y[m] = x_{k}[n]
$$

Donde:
- $k = m \bmod N$ (el índice de la señal)
- $n = \left\lfloor \frac{m}{N} \right\rfloor$ (el índice de la muestra dentro de cada señal)

Por ejemplo, para $N=2$:
- Entrada 1: $x_1 = [a_1, a_2, a_3, \ldots]$
- Entrada 2: $x_2 = [b_1, b_2, b_3, \ldots]$
- Salida: $y = [a_1, b_1, a_2, b_2, a_3, b_3, \ldots]$

Esto permite guardar varias señales en un solo archivo, manteniendo el orden de las muestras de cada una.

Creamos y configuramos el bloque multiplexor con:

```C++
std::vector<int> input_lengths = {1, 1}; // Intercalado de 1 float para cada señal
auto mux = gr::blocks::stream_mux::make(sizeof(float), input_lengths);
```
Tambien necesitaremos añadir dos campos mas a la cabecera de archivo: *"num_streams=2"* y *"mux_format=1,1"*.

## Conexiones y ejecución de flujo
```C++
tb->connect(src1, 0, adder1, 0);
tb->connect(src2, 0, adder1, 1);
tb->connect(adder1, 0, adder2, 0);
tb->connect(src3, 0, adder2, 1);
tb->connect(adder2, 0, lpf, 0);
tb->connect(adder2, 0, head_unfiltered, 0); // suma sin filtrar
tb->connect(lpf, 0, head_filtered, 0);      // suma filtrada
tb->connect(head_unfiltered, 0, mux, 0);    // primer señal
tb->connect(head_filtered, 0, mux, 1);      // segunda señal
tb->connect(mux, 0, sink, 0);

// Ejecutar flujo
tb->start();
tb->wait();
tb->stop();
```

## Compilación y gráficas

Ajustar el Makefile con el nombre del proyecto (mismo que el archivo fuente .cpp) y correr:

```Bash
make
./fir_pasa_bajas
octave multi_signal_plotter.m fir_lpf_signal.dat
```

<p align="center">
<img src="https://github.com/rescurib/gnu_radio_playground/blob/main/Filtros/gr_lpf.png" width="730">
<p>