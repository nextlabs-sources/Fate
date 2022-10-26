TARGETS = \
	Shared \
	IPCJNI \
	IPCProxy \
	IPCTest \
	IPCStub \
	IPCStubTest \
	LoggedInUserTest

.PHONY: all clean

all:
	@for i in $(TARGETS); \
	do \
		if \
			$(MAKE) TARGETENV=$$TARGETENV BUILDTYPE=$$BUILDTYPE -C $$i; \
		then \
			echo "$(TARGET) build completed"; \
		else \
			echo "$(TARGET) build failed."; \
			exit 2; \
		fi; \
	done

clean:
	@for i in $(TARGETS); \
	do \
		$(MAKE) TARGETENV=$$TARGETENV BUILDTYPE=$$BUILDTYPE clean -C $$i; \
	done
