ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

ifeq ($(VERSION_BUILD_SHORT),)
	VERSION_BUILD_SHORT=1
endif


INSTALL_DIR=$(NLBUILDROOT)/install/updater
INSTALL_BUILD_DIR=$(INSTALL_DIR)/build
ASSEMBLY_DIR=$(INSTALL_BUILD_DIR)/data
OUTPUT_DIR=$(INSTALL_BUILD_DIR)/output

MAKE_ASSEMBLY_TARGETS=prepare_archive


.PHONY: all
all: assembly installer publish

.PHONY: assembly
assembly:
	@echo "INFO: Prepare assembly"
	@if [ -d $(ASSEMBLY_DIR) ] || [ -d $(OUTPUT_DIR) ] ; then	\
		echo "INFO:   Removing old build directory" ;			\
		rm -rf $(INSTALL_BUILD_DIR) ;							\
	fi
	make -f Makefile.assembly $(MAKE_ASSEMBLY_TARGETS)

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
