% Diseño de filtro IIR Chebyshev y exportar coeficientes como vectores C++

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

% Verificacion de estabilidad (todos los polos dentro del circulo unitario)
poles = roots(a);
if all(abs(poles) < 1)
    disp("✅ El filtro es ESTABLE.");
else
    disp("❌ El filtro es INESTABLE.");
endif

% Escritura de coeficientes en archivo de texto como vectores C++
fid = fopen("iir_coeficientes.txt", "w");

fprintf(fid, "// Coeficientes del filtro IIR pasa bajas (Chebyshev Tipo I)\n");
fprintf(fid, "// Frecuencia de muestreo: %d Hz, Corte: %d Hz, BW transicion: %d Hz\n\n", fs, fc, transition_bw);

% Escribir coeficientes feedforward (numerador)
fprintf(fid, "std::vector<float> feedforward = {");
for i = 1:length(b)
    fprintf(fid, "%.8ff", b(i));
    if i < length(b)
        fprintf(fid, ", ");
    endif
end
fprintf(fid, "};\n");

% Escribir coeficientes feedback (denominador)
fprintf(fid, "std::vector<double> feedback = {");
for i = 1:length(a)
    fprintf(fid, "%.8ff", a(i));
    if i < length(a)
        fprintf(fid, ", ");
    endif
end

fprintf(fid, "};\n");
fclose(fid);
disp("📄 Declaraciones de vectores en C++: 'iir_coeficientes.txt'");
