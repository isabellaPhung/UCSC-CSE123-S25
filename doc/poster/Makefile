.PHONY: requirements pdf clean

MAIN_TEX := poster.tex
TARGET := CSE123_Poster_Group2.pdf

include includes.mk

CONVERT_MD := $(notdir $(INSERT_MD))
CONVERT_TEX := $(CONVERT_MD:%.md=build/%_md.tex)

GENERATE_PY := $(notdir $(INSERT_DIAGRAM))
GENERATE_PNG := $(GENERATE_PY:%.py=build/%.png)

pdf: build/$(TARGET)

DIAGRAMS_DIR := ../common/aws_chart

$(DIAGRAMS_DIR)/.venv:
	cd $(DIAGRAMS_DIR) && python3 -m venv $@
$(DIAGRAMS_DIR)/.venv/bin/diagrams: $(DIAGRAMS_DIR)/.venv
	cd $(DIAGRAMS_DIR) && . .venv/bin/activate && pip3 install -r requirements.txt

vpath %.py $(DIAGRAMS_DIR)
build/%.png: %.py $(DIAGRAMS_DIR)/.venv/bin/diagrams
	@mkdir -p build
	cd $(dir $<) && . .venv/bin/activate && python3 ../$<
	cp $(DIAGRAMS_DIR)/$(notdir $@) build/
	convert $@ -trim +repage $@

vpath %.md ../common ../common/personas ../common/tests
build/%_md.tex: %.md
	@mkdir -p build
	pandoc -f markdown -t latex $< -o $@

build/$(TARGET): $(MAIN_TEX) $(INSERT_TEX) $(CONVERT_TEX) $(GENERATE_PNG)
	@mkdir -p build
	latexmk -pdf -jobname=$(basename $(TARGET)) $<

clean:
	rm -rf build
