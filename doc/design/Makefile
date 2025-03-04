.PHONY: pdf clean

TMPDIR := build

MAIN_TEX := design.tex
MAIN_TEX += design.sty
TARGET := CSE123_Design_Document_Group2.pdf

INSERT_MD := ../common/ethics_statement.md

INSERT_MD += ../common/personas/camillia.md
INSERT_MD += ../common/personas/elijah.md
INSERT_MD += ../common/personas/linda.md
INSERT_MD += ../common/personas/mary.md
INSERT_MD += ../common/personas/peyton.md

INSERT_MD += ../common/need_statement.md
INSERT_MD += ../common/goal_statement.md

INSERT_MD += ../common/design_objective.md
INSERT_MD += ../common/design_objective_table.md

INSERT_MD += ../common/morphological_chart.md
INSERT_MD += ../common/brainstorm.md

INSERT_MD += ../common/design_for_manufacture_and_assembly.md
INSERT_MD += ../common/lifecycle_assessment.md


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
