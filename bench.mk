include $(RACK_DIR)/arch.mk

MY_RACK = ~/Rack

BENCH_SOURCES = $(wildcard bench/*.cpp)
BENCH_SOURCES += $(SOURCES)
BENCH_SOURCES += $(MY_RACK)/dep/nanovg/src/nanovg.c
BENCH_SOURCES += $(MY_RACK)/dep/osdialog/osdialog.c
BENCH_SOURCES += $(wildcard $(MY_RACK)/dep/pffft-*/pffft.c) $(wildcard $(MY_RACK)/dep/pffft-*/fftpack.c)
BENCH_SOURCES += $(MY_RACK)/src/Quantity.cpp $(MY_RACK)/src/app.cpp $(MY_RACK)/src/asset.cpp $(MY_RACK)/src/color.cpp \
  $(MY_RACK)/src/history.cpp $(MY_RACK)/src/logger.cpp $(MY_RACK)/src/network.cpp $(MY_RACK)/src/patch.cpp $(MY_RACK)/src/plugin.cpp \
  $(MY_RACK)/src/random.cpp $(MY_RACK)/src/settings.cpp $(MY_RACK)/src/string.cpp $(MY_RACK)/src/svg.cpp $(MY_RACK)/src/system.cpp \
  $(MY_RACK)/src/tag.cpp $(MY_RACK)/src/ui.cpp $(MY_RACK)/src/window.cpp $(MY_RACK)/src/event.cpp $(MY_RACK)/src/gamepad.cpp \
  $(MY_RACK)/src/dep.cpp $(MY_RACK)/src/keyboard.cpp $(MY_RACK)/src/midi.cpp $(MY_RACK)/src/audio.cpp $(MY_RACK)/src/updater.cpp \
  $(MY_RACK)/src/bridge.cpp
BENCH_SOURCES += $(wildcard $(MY_RACK)/src/*/*.cpp)

BENCH_SOURCES := $(filter-out $(MY_RACK)/src/main.cpp, $(BENCH_SOURCES))
LDFLAGS = 

ifdef ARCH_LIN
  BENCH_SOURCES += $(MY_RACK)/dep/osdialog/osdialog_gtk2.c
build_bench/$(MY_RACK)/dep/osdialog/osdialog_gtk2.c.o: CXXFLAGS += $(shell pkg-config --cflags gtk+-2.0)
  
  CXXFLAGS += -fpermissive
  LDFLAGS += -rdynamic \
    $(RACK_DIR)/dep/lib/libGLEW.a $(RACK_DIR)/dep/lib/libglfw3.a $(RACK_DIR)/dep/lib/libjansson.a $(RACK_DIR)/dep/lib/libcurl.a $(RACK_DIR)/dep/lib/libssl.a $(RACK_DIR)/dep/lib/libcrypto.a $(RACK_DIR)/dep/lib/libzip.a $(RACK_DIR)/dep/lib/libz.a $(RACK_DIR)/dep/lib/libspeexdsp.a $(RACK_DIR)/dep/lib/librtaudio.a \
	-lpthread -lGL -ldl -lX11 -lasound -ljack \
 	$(shell pkg-config --libs gtk+-2.0)
endif

BENCH_OBJECTS = $(patsubst %, build_bench/%.o, $(BENCH_SOURCES))

build_bench/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build_bench/%.c.o: %.c
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bench : bench.exe

cleanbench :
	rm -rfv build_bench
	rm -fv bench.exe

bench.exe : $(BENCH_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)
