ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

ifeq ($(VERSION_BUILD_SHORT),)
	VERSION_BUILD_SHORT=1
endif


INSTALL_DIR=$(NLBUILDROOT)/install/pc
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
ASSEMBLY_DIR=$(INSTALL_BUILD_DIR)/data
OUTPUT_DIR=$(INSTALL_BUILD_DIR)/output


.PHONY: all
all: assembly installer publish

.PHONY: assembly
assembly:
	make -f Makefile.assembly clean
	make -f Makefile.assembly

.PHONY: installer
installer:
	# make -f Makefile.edp-x86 installer
	make -f Makefile.edp-x64 installer
	# make -f Makefile.ce-x86 installer
	make -f Makefile.ce-x64 installer
	# make -f Makefile.wfse-x86 installer
	make -f Makefile.wfse-x64 installer

.PHONY: publish
publish:
	# make -f Makefile.edp-x86 publish
	make -f Makefile.edp-x64 publish
	# make -f Makefile.ce-x86 publish
	make -f Makefile.ce-x64 publish
	# make -f Makefile.wfse-x86 publish
	make -f Makefile.wfse-x64 publish
	
.PHONY: clean
clean:
	make -f Makefile.assembly clean
