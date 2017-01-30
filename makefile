FRAMEWORKS     = -framework Carbon -framework Cocoa
BUILD_PATH     = ./bin
BUILD_FLAGS    = -std=gnu89 -Wall -g
KHD_SRC        = ./src/khd.m
BINS           = $(BUILD_PATH)/khd

all: $(BINS)

.PHONY: all clean install

install: BUILD_FLAGS=-std=gnu89 -O3
install: clean $(BINS)

$(BINS): | $(BUILD_PATH)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/khd: $(KHD_SRC)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORKS) -o $@
