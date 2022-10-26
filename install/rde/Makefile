ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

.PHONY: all
all: assembly installer publish

.PHONY: assembly
assembly:
	make -f Makefile.x86 assembly

.PHONY: installer
installer:
	make -f Makefile.x86 installer
	make -f Makefile.x64 installer

.PHONY: publish
publish:
	make -f Makefile.x86 publish
	make -f Makefile.x64 publish

.PHONY: clean
clean:
	make -f Makefile.x86 clean
	make -f Makefile.x64 clean
