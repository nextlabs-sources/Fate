ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile

includemanifest:
		cp -f $(BINDIR)/nltag.exe.manifest ${BUILDOUTPUTDIR}/$(BINDIR);
		$(WINSDK_ROOT)/bin/mt.exe -manifest ${BUILDOUTPUTDIR}/$(BINDIR)/nltag.exe.manifest -outputresource:${BUILDOUTPUTDIR}/$(BINDIR)/nltag.exe\;1;

