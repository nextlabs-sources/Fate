ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

# Add this target to avoid invoking default target "all"
.PHONY: all_override
all_override: service

include $(NLBUILDROOT)/build/Makefile

# Even if we can build x64 pcs_rpc.exe, we cannot run it on a x86 build machine.
ifeq ($(BUILDTYPE), release)
	BINDIRX86	= release_$(TARGETENVOS)_x86
else
	BUILDTYPE	= debug
	BINDIRX86	= debug_$(TARGETENVOS)_x86
endif

#PCS_RPC_IDL = "$(BUILDOUTPUTDIR)/$(BINDIR)/pcs_rpc.exe"
PCS_RPC_IDL = "$(BUILDOUTPUTDIR)/$(BINDIRX86)/pcs_rpc.exe"

# chmod does not work reliablely on Windows, use rm instead
.PHONY: service
service:
	rm -f server/src/se_server.cpp
	rm -f $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/src/se_client.hpp
	rm -f $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib/src/se_client.cpp
	$(PCS_RPC_IDL) --prefix=se --service=NL_SE_CLIENT < se_service
	cp se_server.* server/src
	cp se_client.* ../nl_sysenc_lib/src
	cd server/service ; make -B
	cd server ; make -B

.PHONY: idl
idl:
	cd ../pcs_rpc ; make -B -f Makefile.pcs_rpc
