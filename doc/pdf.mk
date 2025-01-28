.PHONY: pdf clean

TARGET := $(MAIN_MD:%.md=%.pdf)

pdf: $(TARGET)
$(TARGET): $(MAIN_MD) $(INSERT_TEX)
	pandoc -f markdown -t pdf --metadata-file ../misc/metadata.md $< -o $@

clean:
	rm -f ./$(TARGET)
