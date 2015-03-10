all:
	pdflatex main.tex
	bibtex main
	pdflatex main.tex
	pdflatex main.tex

	rm main.aux
	rm main.log
	rm main.bbl
	rm main.blg

	open main.pdf
