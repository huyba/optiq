all: paper clean open

paper:
	pdflatex hipineb.tex
	bibtex hipineb
	pdflatex hipineb.tex
	pdflatex hipineb.tex

clean:
	rm hipineb.aux
	rm hipineb.log
	rm hipineb.bbl
	rm hipineb.blg
	rm hipineb.out

open:
	open hipineb.pdf
