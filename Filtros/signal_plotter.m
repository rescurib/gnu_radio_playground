% signal_plotter.m
% Este script lee una señal almacenada en un archivo binario con cabecera y la grafica en funcion del tiempo.

function signal_plotter()
    % Nombre del archivo de datos
    filename = 'signal.dat';
    
    % Leer la cabecera y la posicion donde inicia la señal
    [header, data_start] = parse_header(filename);
    disp('Cabecera leída del archivo:');
    disp(header);

    % Extraer frecuencia de muestreo y tamaño de datos desde la cabecera
    fs = str2double(header.fs);             % Frecuencia de muestreo [Hz]
    datasize = str2double(header.datasize); % Tamaño de los datos (no usado directamente)

    % Abrir el archivo y leer la señal en formato float32
    fid = fopen(filename, 'rb');        % rb: leer en modo binario
    fseek(fid, data_start, 'bof');      % Saltar la cabecera. bof: beginning of file
    signal = fread(fid, inf, 'float32');% Leer todos los datos como flotantes de 32 bits
    fclose(fid);

    % Crear vector de tiempo en milisegundos
    t = (0:length(signal)-1) / fs * 1000; % Tiempo [ms]

    % Graficar la señal
    figure('Color', 'w', 'Position', [100 100 1100 500]); % Ventana grande, fondo blanco
    plot(t, signal, 'b-', 'LineWidth', 1.8);             % Línea azul, más gruesa
    title('Señal desde signal.dat');
    xlabel('Tiempo [ms]', 'FontSize', 12);
    ylabel('Amplitud', 'FontSize', 12);
    grid on;                                             % Mostrar rejilla
    box on;                                              % Mostrar marco
    set(gca, 'FontSize', 11, 'LineWidth', 1);            % Mejorar aspecto de ejes
    ylim([min(signal) max(signal)] + [-0.05 0.05]*range(signal)); % Margen automático en Y
    
    disp('Presiona una tecla para cerrar la gráfica...');
    waitforbuttonpress;                                  % Espera interacción del usuario
end

% -------------------------------------------------------------------------
% Función para leer la cabecera del archivo y encontrar donde inicia la señal
% -------------------------------------------------------------------------
function [header, data_start] = parse_header(filename)
    header = struct();                 % Estructura para guardar los datos de la cabecera
    fid = fopen(filename, 'rb');       % Abrir archivo en modo binario
    data_start = 0;                    % Inicializar posición de inicio de datos
    while true
        line = fgetl(fid);             % Leer línea como texto
        if ~ischar(line) || isempty(line)
            break;                     % Salir si se llega al final o línea vacía
        end
        equal_sym_idx = strfind(line, '=');   % Buscar el símbolo '='
        if ~isempty(equal_sym_idx)
            key   = strtrim(line(1:equal_sym_idx-1));   % Extraer nombre del campo
            value = strtrim(line(equal_sym_idx+1:end)); % Extraer valor
            header.(key) = value;              % Guardar en la estructura de header
        end
        if strncmp(line, 'timestamp=', 10)
            break;                     % Parar al encontrar 'timestamp='
        end
    end
    data_start = ftell(fid);           % Guardar posición de inicio de datos binarios
    fclose(fid);                       % Cerrar archivo
end

% Ejecutar la función principal al correr el script
signal_plotter();