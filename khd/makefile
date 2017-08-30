FRAMEWORKS     = -framework Carbon -framework Cocoa -framework IOKit
BUILD_PATH     = ./bin
BUILD_FLAGS    = -std=gnu89 -Wall -g
KHD_SRC        = ./src/khd.m
BINS           = $(BUILD_PATH)/khd

.PHONY: all clean install

all: clean $(BINS)

install: BUILD_FLAGS=-std=gnu89 -O3
install: clean $(BINS)

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/khd: $(KHD_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORKS) -o $@
