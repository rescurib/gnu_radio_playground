# Filtro Pasa-Bajas IIR

Para esta implemntación de un filtro *Infinite Impulse Response* (IIR) vamos a reutilizar la suma de fuentes senoidales del ejemplo FIR y también el sumidero de archivos con cabecera.

Los filtros IIR de orden bajo pueden tener un desempeño similar al un filtro FIR de orden grande, reduciendo bastante la carga de cómputo. Pero nada es gratis es la vida. ¿Cuál es el precio que hay que pagar?:

* La estabilidad no está garantizada
* La señal filtrada no estará libre de distorsión. 

Es por eso que si se van a usar en aplicaciones de instrumentación cientifica se debe tener una modelación clara para evaluar su uso, y tener un diseño adecuado en caso de implementarlo.

GNU Radio, al menos hasta última version que estoy usando (3.10.9), no tiene una función de diseño de filtros IIR. Pero podemos utilizar Octave. Para instalarlo en sistemas basados en Debian junto con las librerías que vamos a ocupar:

```Bash
sudo apt install octave octave-dev octave-control octave-signal
```
El filtro FIR del ejemplo anterior tenía un orden de 212. Veamos si podemos reducir el orden manteniendo el despeño:

```Matlab
pkg load signal;

% Especificaciones del filtro
fs = 44000;           % Frecuencia de muestreo (Hz)
fc = 1000;            % Frecuencia de corte (Hz)
transition_bw = 500;  % Ancho de banda de transicion (Hz)
rp = 1;               % Rizado en banda pasante (dB)
rs = 60;              % Atenuacion en banda de rechazo (dB)

% Calculo de frecuencias normalizadas (respecto a Nyquist)
Wp = fc / (fs/2);     % Frecuencia de paso normalizada
Ws = (fc + transition_bw) / (fs/2); % Frecuencia de rechazo normalizada

% Diseno del filtro Chebyshev Tipo I
% n: orden del filtro, Wn: frecuencia de corte ajustada
[n, Wn] = cheb1ord(Wp, Ws, rp, rs);
% b: coeficientes feedforward, a: coeficientes feedback
[b, a] = cheby1(n, rp, Wn);

disp(["Orden del filtro IIR: ",num2str(n)]);
```
¡Este filtro IIR resultó de solo 9no orden! ¿Pero que tal nos fue en su respuesta?

<p align="center">
<img src="https://github.com/rescurib/gnu_radio_playground/blob/main/Filtros/iir_respuesta.png" width="730">
<p>

El **retardo de grupo** (*group delay*) mide cuánto se retrasa, en promedio, cada componente de frecuencia de una señal al pasar por un filtro. Se define como la derivada negativa de la fase respecto a la frecuencia:

```math
\tau_g(\omega) = -\frac{d\phi(\omega)}{d\omega}
```

dónde $\phi(\omega)$ es la respuesta de fase (desenvuelta) del filtro. En Octave podemos obtener el retardo de grupo con la función grpdelay(). Aunque en este ejemplo no parece estar desenvolviendo la fase correctamente, asi que podemos calcularla a partir de su definición y de la respuesta compleja del filtro (obtenida con [freqz()](https://octave.sourceforge.io/octave/function/freqz.html)):

```Matlab
[H,f]  = freqz(b,a,[],fs);
[gd,f] = grpdelay(b,a,[],fs);

phi = unwrap(angle(H));
group_delay = -diff(phi) ./ diff(2*pi*f/fs);

plot(f(2:end), group_delay*1000/fs);
title('Retardo de grupo del filtro IIR Chebyshev');
xlabel('Frecuencia [Hz]'); ylabel('Retardo de grupo [ms]');
```

Para encontrar el retardo en una frecuencia en particular, por ejemplo en la componente de 200Hz:
```Matlab
% Obtener índice del arreglo f del valor más cercano a 200Hz
[~, idx_200Hz] = min(abs(f(1:end-1) - 200));

% Obtener retardo a partir del índice
gd_muestras = group_delay(idx_200Hz);
gd_ms       = gd_muestras * 1000 / fs;

fprintf('Retardo en 200 Hz: %.3f ms\n', gd_ms);
```
Para nuestro filtro, el retardo en 200Hz es de 1.247ms.

>[!NOTE]
> **Tratamiento Digital de señales (4ed)**, Proakis-Manolakis
> * Respuesta en frecuencia de los sistemas LTI, pag. 281

## Bloque de filtro 

**Nota**: Los programas de este ejemplo ya están completos (iir_pasa_bajas.cpp, iir_lpf_design.m). Queda pendiente terminar este markdown.