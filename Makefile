# Makefile for GNU Radio C++ apps

CXX = g++
CXXFLAGS = -Wall -std=c++17
PKG_CONFIG = pkg-config

# Ask pkg-config to get the compiler and linker flags
PKG_MODULES = gnuradio-runtime gnuradio-blocks gnuradio-analog
CXXFLAGS += $(shell $(PKG_CONFIG) --cflags $(PKG_MODULES))
LDFLAGS += $(shell $(PKG_CONFIG) --libs $(PKG_MODULES)) -lfmt

# Output binary name
TARGET = verify_gnu_radio
SRCS = verify_gnu_radio.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

