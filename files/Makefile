# Directories
DEST_DIR := ../src/views/

# Get all PNG files in the current directory
PNG_FILES := $(wildcard *.png)
BIN_FILES := $(PNG_FILES:.png=.bin)
H_FILES   := $(addprefix $(DEST_DIR), $(BIN_FILES:.bin=.h))

# ensure applications we need are available
ifeq (, $(shell command -v png2asset &>/dev/null))
$(error "Error: png2asset is not installed. Get it from https://github.com/gbdk-2020/gbdk-2020")
endif
ifeq (, $(shell command -v xxd &>/dev/null))
$(error "Error: xxd is not installed.")
endif

# Default target
all: $(H_FILES)


.PHONY: all clean

# Rule to create .bin files from .png
%.bin: %.png
	png2asset $< -bin -map -tiles_only -sprite_no_optimize >/dev/null
	@rm $(patsubst %.png, %.h, $<)
	mv $(patsubst %.png, %, $<)_tiles.bin $@

# Rule to generate .c and .h files from .bin and place them in the DEST_DIR
$(DEST_DIR)%.h: %.bin | $(DEST_DIR)
	xxd -i $< > $@

# Clean up generated files
clean:
	rm -f $(H_FILES)

