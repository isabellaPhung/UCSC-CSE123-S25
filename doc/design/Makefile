.PHONY: pdf clean

TMPDIR := build

MAIN_TEX := design.tex
TARGET := CSE123_Design_Document_Group2.pdf

INSERT_TEX := ../common/need_statement.tex
INSERT_TEX += ../common/goal_statement.tex

INSERT_MD := ../common/personas/camillia.md
INSERT_MD += ../common/personas/elijah.md

CONVERT_MD := $(notdir $(INSERT_MD))
CONVERT_TEX := $(CONVERT_MD:%.md=$(TMPDIR)/%_md.tex)

pdf: build/$(TARGET)

vpath %.md ../common ../common/personas
$(TMPDIR)/%_md.tex: %.md
	@mkdir -p build
	pandoc -f markdown -t latex $< -o $@

build/$(TARGET): $(MAIN_TEX) $(INSERT_TEX) $(CONVERT_TEX)
	@mkdir -p build
	pdflatex -output-directory=build -jobname=$(basename $(TARGET)) $^
	pdflatex -output-directory=build -jobname=$(basename $(TARGET)) $^

clean:
	rm -rf build
