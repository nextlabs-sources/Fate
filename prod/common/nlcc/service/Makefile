ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

# Check if JNI file is up-to-date
.PHONY: checked_all
checked_all: jnicheck all

.PHONY: jnicheck
jnicheck:
	@if [ java/NLCCService.java -nt java/com_bluejungle_NLCC_NLCCServiceDispatcher.h ]; then \
		echo "ERROR: java/com_bluejungle_NLCC_NLCCServiceDispatcher.h is not up-to-date. You must compile project prod/common/nlcc/service/java/ to update the JNI header file." ; \
		exit 1 ; \
	fi

include $(NLBUILDROOT)/build/Makefile
