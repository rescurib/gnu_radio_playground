% multi_signal_plotter.m
% Lee un archivo binario con cabecera y grafica todas las señales contenidas en cada vector.

function multi_signal_plotter(filename)
    if nargin < 1
        filename = 'fir_lpf_signal.dat';
    end

    % Leer la cabecera y la posición donde inicia la señal
    [header, data_start] = parse_header(filename);
    disp('Cabecera leída del archivo:');
    disp(header);

    % Extraer frecuencia de muestreo, tamaño de datos, número de streams y formato de mux desde la cabecera
    fs = str2double(header.fs);             % Frecuencia de muestreo [Hz]
    datasize = str2double(header.datasize); % Tamaño de los datos en bytes
    num_streams = str2double(header.num_streams); % Número de streams multiplexados
    mux_format = str2num(header.mux_format); % Formato de multiplexado (vector)
    if isempty(mux_format)
        mux_format = ones(1, num_streams); % Por defecto, 1 elemento por stream
    end
    items_per_cycle = sum(mux_format);

    % Abrir el archivo y leer la señal en formato float32
    fid = fopen(filename, 'rb');
    fseek(fid, data_start, 'bof');
    raw_data = fread(fid, inf, 'float32');
    fclose(fid);

    printf('Datos leídos: %d elementos\n', length(raw_data));

    % Validar datos suficientes
    num_cycles = floor(length(raw_data) / items_per_cycle);
    if num_cycles < 1 || isempty(raw_data)
        error('No se encontraron datos para graficar.');
    end
    signals = zeros(num_cycles, num_streams);
    for i = 1:num_cycles
        idx = (i-1)*items_per_cycle + 1;
        for s = 1:num_streams
            signals(i, s) = mean(raw_data(idx:idx + mux_format(s) - 1));
            idx = idx + mux_format(s);
        end
    end

    % Crear vector de tiempo en milisegundos
    t = (0:num_cycles-1) / fs * 1000; % Tiempo [ms]

    % Graficar todas las señales
    figure('Color', 'w', 'Position', [100 100 1100 500]);
    if isempty(signals)
        error('No se encontraron datos para graficar.');
    elseif num_streams == 1
        plot(t, signals, 'LineWidth', 1.5);
        legend('Señal 1');
    else
        plot(t, signals, 'LineWidth', 1.5);
        legend(arrayfun(@(i) sprintf('Señal %d', i), 1:num_streams, 'UniformOutput', false));
    end
    title('Lectura de señales multiplexadas desde archivo');
    xlabel('Tiempo [ms]', 'FontSize', 12);
    ylabel('Amplitud', 'FontSize', 12);
    grid on;
    box on;
    set(gca, 'FontSize', 11, 'LineWidth', 1);

    disp('Presiona una tecla para cerrar la gráfica...');
    waitforbuttonpress;
end

% -------------------------------------------------------------------------
% Función para leer la cabecera del archivo y encontrar inicio de datos
% -------------------------------------------------------------------------
function [header, data_start] = parse_header(filename)
    header = struct();
    fid = fopen(filename, 'rb');
    data_start = 0;
    while true
        line = fgetl(fid);
        if ~ischar(line) || isempty(line)
            break;
        end
        equal_sym_idx = strfind(line, '=');
        if ~isempty(equal_sym_idx)
            key   = strtrim(line(1:equal_sym_idx-1));
            value = strtrim(line(equal_sym_idx+1:end));
            header.(key) = value;
        end
        if strncmp(line, 'timestamp=', 10)
            break;
        end
    end
    % Avanzar hasta el primer byte binario (descartar salto de línea final)
    fseek(fid, 0, 'cof');
    data_start = ftell(fid);
    fclose(fid);
end

% Ejecutar la función principal si se llama el script directamente
if ~isdeployed
    multi_signal_plotter();
end