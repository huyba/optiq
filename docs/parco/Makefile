all: create clean open

create:
	pdflatex parco.tex
	bibtex parco
	pdflatex parco.tex
	pdflatex parco.tex

clean:
	rm parco.aux
	rm parco.bbl
	rm parco.spl
	rm parco.blg
	rm parco.log

open:

	open parco.pdf
