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


<p align="center">
<img src="https://github.com/rescurib/gnu_radio_playground/blob/main/Filtros/gr_lpf.png" width="730">
<p>