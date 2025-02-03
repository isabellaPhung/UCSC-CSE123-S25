.PHONY: pdf clean

PANDOC_FLAGS:= --metadata-file ../misc/metadata.md --lua-filter=../misc/include-files/include-files.lua

TARGET := $(MAIN_MD:%.md=%.pdf)

pdf: build/$(TARGET)
build/$(TARGET): $(MAIN_MD) $(INSERT_TEX) $(INSERT_MD)
	@mkdir -p build
	pandoc -f markdown -t pdf $(PANDOC_FLAGS) $< -o $@

clean:
	rm -rf build
