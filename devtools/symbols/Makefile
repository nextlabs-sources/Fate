#########################################################################################
#
# Publish symbols
#
#########################################################################################

WINDDK   = c:/winddk/7600.16385.1
SYMSTORE = $(WINDDK)/debuggers/symstore.exe
SYMSTORE_OPTIONS = /p

BUILDS_RELEASES = \\\\nextlabs.com\share\data\releases
BUILDS_NIGHTLY  = \\\\nextlabs.com\share\data\build\nightly
BUILDS_PCV      = \\\\nextlabs.com\share\data\build\pcv

BUILDS_SYMBOLS_RELEASES = \\\\nextlabs.com\share\data\releases\symbols
BUILDS_SYMBOLS_NIGHTLY  = \\\\nextlabs.com\share\data\build\nightly\symbols
BUILDS_SYMBOLS_PCV      = \\\\nextlabs.com\share\data\build\pcv\symbols

PRODUCT = NextLabs

SPATHS   = //nextlabs.com/share/data/build/nightly
RELEASES = //nextlabs.com/share/data/releases

all: releases nightly pcv

releases:
	@$(SYMSTORE) add /r /t $(PRODUCT) /f "$(BUILDS_RELEASES)\*.*" /s "$(BUILDS_SYMBOLS_RELEASES)" $(SYMSTORE_OPTIONS)

nightly:
	@$(SYMSTORE) add /r /t $(PRODUCT) /f "$(BUILDS_NIGHTLY)\*.*" /s "$(BUILDS_SYMBOLS_NIGHTLY)" $(SYMSTORE_OPTIONS)

pcv:
	@$(SYMSTORE) add /r /t $(PRODUCT) /f "$(BUILDS_PCV)\*.*" /s "$(BUILDS_SYMBOLS_PCV)" $(SYMSTORE_OPTIONS)

clean:
	@rm -rf $(BUILDS_SYMBOLS_RELEASES)/*
	@rm -rf $(BUILDS_SYMBOLS_NIGHTLY)/*
	@rm -rf $(BUILDS_SYMBOLS_PCV)/*
