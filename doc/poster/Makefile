.PHONY: requirements pdf clean

MAIN_TEX := poster.tex
TARGET := CSE123_Poster_Group2.pdf

include includes.mk

CONVERT_MD := $(notdir $(INSERT_MD))
CONVERT_TEX := $(CONVERT_MD:%.md=build/%_md.tex)

pdf: build/$(TARGET)

vpath %.md ../common ../common/personas ../common/tests
build/%_md.tex: %.md
	@mkdir -p build
	pandoc -f markdown -t latex $< -o $@

build/$(TARGET): $(MAIN_TEX) $(INSERT_TEX) $(CONVERT_TEX) $(GENERATE_PNG)
	@mkdir -p build
	latexmk -pdf -jobname=$(basename $(TARGET)) $<

clean:
	rm -rf build
