# Makefile para aplicaciones de GNU Radio en C++

CXX = g++
CXXFLAGS = -Wall -std=c++17
PKG_CONFIG = pkg-config

# Obtención de banderas con pkg-config
GNURADIO_PKGS = $(shell $(PKG_CONFIG) --list-all | grep '^gnuradio-' | cut -d ' ' -f 1)
CXXFLAGS += $(shell $(PKG_CONFIG) --cflags $(GNURADIO_PKGS))
LDFLAGS += $(shell $(PKG_CONFIG) --libs $(GNURADIO_PKGS)) -lfmt

# Nombre del proyecto
PROJECT_NAME = generador

SRC = $(PROJECT_NAME).cpp
TARGET = $(PROJECT_NAME)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

