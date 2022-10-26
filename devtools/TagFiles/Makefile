ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile

includemanifest:
		cp -f $(BINDIR)/TagFiles.exe.manifest ${BUILDOUTPUTDIR}/$(BINDIR);
		$(WINSDK_ROOT)/bin/mt.exe -manifest ${BUILDOUTPUTDIR}/$(BINDIR)/TagFiles.exe.manifest -outputresource:${BUILDOUTPUTDIR}/$(BINDIR)/TagFiles.exe\;1;

