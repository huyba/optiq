all: paper clean

paper:
	pdflatex hipc.tex
	bibtex hipc
	pdflatex hipc.tex
	pdflatex hipc.tex

clean:
	rm hipc.aux
	rm hipc.log
	rm hipc.bbl
	rm hipc.blg

	open hipc.pdf

report:
	pdflatex report.tex
	pdflatex report.tex
	open report.pdf

	rm *.aux
	rm *.log
	rm *.bbl
	rm *.blg
