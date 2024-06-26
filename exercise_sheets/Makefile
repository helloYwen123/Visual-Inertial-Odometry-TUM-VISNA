.PHONY: all clean wipe once

all:
	latexmk -pdf

clean:
	latexmk -c
	rm -f *.run.xml *-blx.bib *.bbl

wipe: clean
	rm *.pdf

once: all
	$(MAKE) clean
