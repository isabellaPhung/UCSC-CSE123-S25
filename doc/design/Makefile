.PHONY: pdf clean

MAIN_TEX := design.tex
MAIN_TEX += design.sty
TARGET := CSE123_Design_Document_Group2.pdf

include includes.mk

CONVERT_MD := $(notdir $(INSERT_MD))
CONVERT_TEX := $(CONVERT_MD:%.md=build/%_md.tex)

pdf: build/$(TARGET)

vpath %.md ../common ../common/personas ../common/tests ../common/tests ../common/startManual 
build/%_md.tex: %.md
	@mkdir -p build
	pandoc -f markdown-implicit_figures -t latex $< -o $@

build/$(TARGET): $(MAIN_TEX) $(INSERT_TEX) $(CONVERT_TEX)
	@mkdir -p build
	latexmk -pdf -output-directory=build -jobname=$(basename $(TARGET)) $<

clean:
	rm -rf build
