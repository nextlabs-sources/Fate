ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile

includemanifest:
		cp -f src/$(BUILDTYPE).manifest ${BUILDOUTPUTDIR}/$(BINDIR)/cepdpman.exe.manifest
		$(WINSDK_ROOT)/bin/mt.exe -manifest ${BUILDOUTPUTDIR}/$(BINDIR)/cepdpman.exe.manifest -outputresource:${BUILDOUTPUTDIR}/$(BINDIR)/cepdpman.exe\;1;
