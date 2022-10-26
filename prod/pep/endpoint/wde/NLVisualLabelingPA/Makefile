ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif


.PHONY: all
.PHONY: office2003
.PHONY: office2007
.PHONY: clean

all: office2003 office2007

office2003:
	@echo "--------------------------------------------------------------------------"
	@echo "Compiling target for Office 2003"
	@echo "--------------------------------------------------------------------------"
	make -f Makefile.forapp OFFICEVERSION=2003

office2007:
	@echo "--------------------------------------------------------------------------"
	@echo "Compiling target for Office 2007"
	@echo "--------------------------------------------------------------------------"
	make -f Makefile.forapp OFFICEVERSION=2007

clean:
	make -f Makefile.forapp OFFICEVERSION=2003 clean
	make -f Makefile.forapp OFFICEVERSION=2007 clean
