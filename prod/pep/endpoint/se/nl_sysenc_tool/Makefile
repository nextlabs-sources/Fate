##########################################################################
#
# This makefile is a front-end for building nl_sysenc using two options.
#
# Targets:
#
#   console - builds a console version of the tool (it has a window).
#   windows - builds a windows (i.e., non-console) version which
#             has no console.
#
##########################################################################

SE_OPTIONS =

# Set PRODUCT if defined
ifneq ($(PRODUCT), )
	SE_OPTIONS += PRODUCT=$(PRODUCT)
endif

# Set BUILDTYPE if defined
ifneq ($(BUILDTYPE), )
	SE_OPTIONS += BUILDTYPE=$(BUILDTYPE)
endif

main: console windows
	@echo Complete

console:
	@echo Building CONSOLE nlSysEncryption
	@cd ../nl_sysenc ; make $(SE_OPTIONS) -B SE_LD=CONSOLE

windows:
	@echo Building WINDOWS nlSysEncryption
	@cd ../nl_sysenc ; make $(SE_OPTIONS) -B SE_LD=WINDOWS

clean:
	@echo Cleaning CONSOLE nlSysEncryption
	@cd ../nl_sysenc ; make $(SE_OPTIONS) -B SE_LD=CONSOLE clean
	@echo Cleaning WINDOWS nlSysEncryption
	@cd ../nl_sysenc ; make $(SE_OPTIONS) -B SE_LD=WINDOWS clean



