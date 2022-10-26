
VERSION_PRODUCT		= $(VERSION_PRODUCT_EDP)
VERSION_MAJOR		= $(VERSION_MAJOR_EDP)
VERSION_MINOR		= $(VERSION_MINOR_EDP)
VERSION_MAINTENANCE = $(VERSION_MAINTENANCE_EDP)
VERSION_PATCH		= $(VERSION_PATCH_EDP)

COMPILER=VS2008
PLATFORMSDK=7.0

# Need to specify RCSRC here so that top-level Makefile does not try to create src/use_default.rc
# and find src does not exist. This also cause vibe_adapter.rc to be updated.
RCSRC = vibe_adapter.rc

CUSTOM = vibe_adapter.vcproj
	