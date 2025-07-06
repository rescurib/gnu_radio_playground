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

