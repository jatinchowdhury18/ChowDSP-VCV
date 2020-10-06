SOURCES = $(wildcard *.tex)
TEX_FLAGS = 
PNG_PRE_FLAGS = -density 300
PNG_POST_FLAGS = -quality 90

PDFS = $(patsubst %.tex, %.pdf, $(SOURCES))
PNGS = $(patsubst %.tex, %.png, $(SOURCES))

.PHONY: clean clean-artifacts

clean: clean-artifacts
	rm -f $(PDFS)
	rm -f $(PNGS)

clean-artifacts:
	rm -f *.{log,aux,fls,fdb*,sync*}

%.pdf: %.tex
	pdflatex $(TEX_FLAGS) $<
	
%.png: %.pdf
	magick convert $(PNG_PRE_FLAGS) $< $(PNG_POST_FLAGS) $@

pdf: $(PDFS)
png: $(PNGS) $(PDFS)
