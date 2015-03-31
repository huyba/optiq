all:
	pdflatex optiq.tex
	bibtex optiq
	pdflatex optiq.tex
	pdflatex optiq.tex

	rm optiq.aux
	rm optiq.log
	rm optiq.bbl
	rm optiq.blg

	open optiq.pdf
