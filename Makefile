##########################################################################################
#
# This file is the top-level makefile for building product, custom application and devtools 
# targets.  The specific targets are defined in a file called "targets" in the top-level directory
# //depot/dev/D_SiriusR2/.  For example, Desktop Enforcer would have targets file 
# 'prod/pep/endpoint/wde/targets'.
#
# Variables defined as a parameter to this makefile are passed to the product target
# makefiles.
#
# Variables:
#
#   TARGETSFILE  Directory of the product to build.  For example, building the Desktop
#                Enforcer would use TARGETSFILE=prod/pep/endpoint/wde/targets.
#
# Examples:
#	make TARGETSFILE=prod/pep/endpoint/wde/targets
#	make TARGETSFILE=custom/KLA/Blackbox_64_5.1.0/targets
#	make TARGETSFILE=devtools/targets
#
##########################################################################################

ifeq ($(NLBUILDROOT),)
  $(error ERROR: NLBUILDROOT undefined)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error ERROR: NLEXTERNALDIR undefined)
endif

ifeq ($(NLEXTERNALDIR2),)
  $(error ERROR: NLEXTERNALDIR2 undefined)
endif

ifneq ($(shell [ -e $(TARGETSFILE) ] && echo 0),0)
  $(error ERROR: TARGETSFILE $(TARGETSFILE) does not exist)
endif

include $(TARGETSFILE)

ifeq ($(TARGETS),)
  $(error ERROR: TARGETS undefined)
endif

$(info )
$(info ==========================================================================)
$(info BUILDTYPE=$(BUILDTYPE))
$(info BUILDOUTPUTDIR=$(BUILDOUTPUTDIR))
$(info NLBUILDROOT=$(NLBUILDROOT))
$(info NLEXTERNALDIR=$(NLEXTERNALDIR))
$(info NLENFORCERSDIR=$(NLENFORCERSDIR) (legacy))
$(info OFFICIALCERT=$(OFFICIALCERT))
$(info TARGETENVARCH=$(TARGETENVARCH))
$(info TARGETENVOS=$(TARGETENVOS))
$(info ==========================================================================)

.PHONY: all
.PHONY: $(TARGETS)
.PHONY: clean

all: $(TARGETS)

$(TARGETS):
	@echo --------------------------------------------------------------------
	@echo Building $@
	@echo --------------------------------------------------------------------
	@cd $(NLBUILDROOT)/$@ ; $(MAKE)

clean:
	@echo --------------------------------------------------------------------
	@echo Cleaning targets
	@echo --------------------------------------------------------------------
	@for target in $(TARGETS); \
		do cd $(NLBUILDROOT)/$$target ; \
		$(MAKE) clean ; \
	done
