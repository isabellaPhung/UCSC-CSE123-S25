.PHONY: pdf clean

TMPDIR := build

MAIN_TEX := presentation.tex
MAIN_TEX += presentation.sty
TARGET := Demo_Presentation.pdf

include includes.mk

CONVERT_MD := $(notdir $(INSERT_MD))
CONVERT_TEX := $(CONVERT_MD:%.md=$(TMPDIR)/%_md.tex)

pdf: build/$(TARGET)

vpath %.md ../common ../common/personas ../common/tests
$(TMPDIR)/%_md.tex: %.md
	@mkdir -p build
	pandoc -f markdown -t latex $< -o $@

build/$(TARGET): $(MAIN_TEX) $(INSERT_TEX) $(CONVERT_TEX)
	@mkdir -p build
	pdflatex -output-directory=build -jobname=$(basename $(TARGET)) $^
	pdflatex -output-directory=build -jobname=$(basename $(TARGET)) $^

clean:
	rm -rf build
