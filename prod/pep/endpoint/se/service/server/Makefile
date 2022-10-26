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
	@if [ service/SystemEncryptionService.java -nt service/com_bluejungle_EDP_SystemEncryptionService.h ]; then \
		echo "ERROR: service/com_bluejungle_EDP_SystemEncryptionService.h is not up-to-date. You must compile project prod/pep/endpoint/se/service/server/service/ to update the JNI header file." ; \
		exit 1 ; \
	fi

include $(NLBUILDROOT)/build/Makefile
