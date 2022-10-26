ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif


.PHONY: all
.PHONY: adobe_reader
.PHONY: adobe_acrobat
.PHONY: clean

all: adobe_reader adobe_acrobat

adobe_reader:
	@echo "--------------------------------------------------------------------------"
	@echo "Compiling target for Adobe Reader"
	@echo "--------------------------------------------------------------------------"
	make -f Makefile.forapp ADOBEAPP=reader

adobe_acrobat:
	@echo "--------------------------------------------------------------------------"
	@echo "Compiling target for Adobe Acrobat"
	@echo "--------------------------------------------------------------------------"
	make -f Makefile.forapp ADOBEAPP=acrobat

clean:
	make -f Makefile.forapp ADOBEAPP=reader clean
	make -f Makefile.forapp ADOBEAPP=acrobat clean
