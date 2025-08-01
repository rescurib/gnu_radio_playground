% Prueba de filtro IIR Chebyshev con señal compuesta y visualización

pkg load signal;

% Especificaciones del filtro
fs = 44000;           % Frecuencia de muestreo (Hz)
fc = 1000;            % Frecuencia de corte (Hz)
transition_bw = 500;  % Ancho de banda de transicion (Hz)
rp = 1;               % Rizado en banda pasante (dB)
rs = 60;              % Atenuacion en banda de rechazo (dB)

% Calculo de frecuencias normalizadas
Wp = fc / (fs/2);
Ws = (fc + transition_bw) / (fs/2);

% Diseño del filtro Chebyshev Tipo I
[n, Wn] = cheb1ord(Wp, Ws, rp, rs);
[b, a] = cheby1(n, rp, Wn);

% Mostrar orden del filtro
disp(["Orden del filtro IIR: ", num2str(n)]);

% Verificación de estabilidad
poles = roots(a);
if all(abs(poles) < 1)
    disp("✅ El filtro es ESTABLE.");
else
    disp("❌ El filtro es INESTABLE.");
endif

% Generar señal compuesta (igual que en iir_pasa_bajas.cpp)
tiempo_final = 40; % ms
num_muestras = round(fs * tiempo_final / 1000);
t = (0:num_muestras-1) / fs;
f1 = 200;   % Frecuencia baja
f2 = 2000;  % Frecuencia alta 1
f3 = 5000;  % Frecuencia alta 2

x = sin(2*pi*f1*t) + 0.5*sin(2*pi*f2*t) + 0.25*sin(2*pi*f3*t);

% Filtrar la señal
y = filter(b, a, x);

% Graficar señal de entrada y salida
figure;
plot(t*1000, x, 'b', 'LineWidth', 1.2); hold on;
plot(t*1000, y, 'r', 'LineWidth', 1.2);
legend('Señal original', 'Señal filtrada');
xlabel('Tiempo [ms]'); ylabel('Amplitud');
title('Respuesta del filtro IIR Chebyshev sobre señal compuesta');
grid on;

% Graficar retardo de grupo en figura aparte

[H,f]  = freqz(b,a,[],fs);
[gd,f] = grpdelay(b,a,[],fs);

phi = unwrap(angle(H));
gd_manual = -diff(phi) ./ diff(2*pi*f/fs);

% Obtener índice del arreglo f del valor más cercano a 200Hz
[~, idx_200Hz] = min(abs(f(1:end-1) - 200));

% Obtener retardo a partir del índice
gd_muestras = gd_manual(idx_200Hz);
gd_ms       = gd_muestras * 1000 / fs;

fprintf('Retardo en 200 Hz: %.3f ms\n', gd_ms);

figure;
subplot(211);
plot(f,abs(H));
title('Respuesta en frecuencia del filtro IIR Chebyshev');
xlabel('Frecuencia [Hz]'); ylabel('Magnitud');
xlim([0 2*fc]);
grid on;

subplot(212);
plot(f(2:end), gd_manual*1000/fs, 'LineWidth', 1.2);
title('Retardo de grupo del filtro IIR Chebyshev');
xlabel('Frecuencia [Hz]'); ylabel('Retardo de grupo [ms]');
xlim([0 2*fc]);
ylim([0 max(gd_manual)*1000/fs]);
grid on;

disp('Presiona una tecla para cerrar la gráfica...');
waitforbuttonpress;
