all: bitthunder.pdf


bitthunder.pdf: *.tex
	@pdflatex bitthunder.tex > /dev/null
	@echo "[TEX] panorama.tex"

clean:
	rm -rf *.log *.aux *.toc *.pdf
