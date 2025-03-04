.PHONY: pdf clean

PANDOC_FLAGS := --metadata-file ../misc/metadata.md --lua-filter=../misc/include-files/include-files.lua
ifdef BIB_FILE
PANDOC_CITE = --citeproc --bibliography $(BIB_FILE)
endif
METADATA := ../misc/metadata.md

TARGET := $(MAIN_MD:%.md=%.pdf)

pdf: build/$(TARGET)
build/$(TARGET): $(MAIN_MD) $(INSERT_TEX) $(INSERT_MD) $(METADATA) $(BIB_FILE)
	@mkdir -p build
	pandoc -f markdown -t pdf $(PANDOC_FLAGS) $(PANDOC_CITE) $< -o $@

clean:
	rm -rf build
