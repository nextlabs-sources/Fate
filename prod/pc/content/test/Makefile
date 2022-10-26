####################################################################################
#
# Basic test for Content Analysis using NLCRACK
#
####################################################################################

NLCRACK = ../analyze/bin_win32/nlcrack.exe
DATA = \
	data/grail.txt        \
	data/random_200k.txt  \
	data/random_200k.rtf  \
	data/random_200k.doc  \
	data/random_200k.docx \
	data/random_200k.pdf  \
	data/6130-h.html

all: $(DATA)
	@echo Complete

data/%:
	@echo Scanning $@ using IFilter
	@$(NLCRACK) $@ --regex="coconuts",0 > output
	@$(NLCRACK) $@ --regex="Shrub\w+",0 --regex="Roger",0 > output
	@$(NLCRACK) $@ --regex="Scene \d+",0 --regex="\d{3}\w{3}",0 > output
	@echo Scanning $@ using Raw
	@$(NLCRACK) $@ --regex="coconuts",0 > output
	@$(NLCRACK) $@ --regex="Shrub\w+",0 --regex="Roger",0 > output
	@$(NLCRACK) $@ --regex="Scene \d+",0 --regex="\d{3}\w{3}",0 > output