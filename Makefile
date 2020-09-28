# Use full Rack repo as RACK_DIR (needed for osdialog)
RACK_DIR = ../..

include $(RACK_DIR)/arch.mk

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Ilib -DUSE_EIGEN
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
SOURCES += $(wildcard lib/r8lib/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# Eigen library has a ton of deprecated copy warnings
ifndef_any_of = $(filter undefined,$(foreach v,$(1),$(origin $(v))))
ifdef_any_of = $(filter-out undefined,$(foreach v,$(1),$(origin $(v))))
ifneq ($(call ifdef_any_of,ARCH_WIN ARCH_LIN),)
  CXXFLAGS += -Wno-deprecated-copy
endif

ifdef ARCH_WIN
# extra dist target for Azure CI Windows build, as there is only 7zip available and no zip command
azure-win-dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
	@# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	@# Create ZIP package
	cd dist && 7z a -tzip -mx=9 $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
endif

include bench.mk
