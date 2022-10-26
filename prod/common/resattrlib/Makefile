ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile

# My custom task, here we copy the resattrmgr.h
copyheaders:
	@if [ ! -d ${BUILDOUTPUTDIR}/$(INCDIR) ]; then \
	  mkdir -p ${BUILDOUTPUTDIR}/$(INCDIR);        \
	fi
	cp -f  include/resattrlib.h ${BUILDOUTPUTDIR}/$(INCDIR);