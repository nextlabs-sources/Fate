ifeq ($(NLBUILDROOT),)
  $(error NLBUILDROOT undefined.)
endif

ifeq ($(NLEXTERNALDIR),)
  $(error NLEXTERNALDIR undefined.)
endif

include $(NLBUILDROOT)/build/Makefile.java
include $(NLBUILDROOT)/build/Makefile.ver

XLIB_JAVA_DIR=$(NLBUILDROOT)/xlib/java


#
# Create build.properties content
#

define BUILD_PROPERTIES_CONTENT
# ---------------------------------------------------------------------------
# Configurable build properties
#
# Description:
#	The properties in this file are configurable properties. They should be updated 
#	according to the environment and release version. The properties should be used
#	by build_compile.xml. This property file should contain all properties need to 
#	run Ant script build_compile.xml. If any configurable property is missing, it 
#	should be added to this script.
#
#	Note that this script should not contain any property that acts a local variable
#	to a build_compile.xml file. Local variables should be specified within 
#	build_compile.xml file using <properties> tag.
#
#	For developer:
#		The values specified in this property file need to be adjusted in order for 
#		you to compile code in you development environment using build_compile.xml. 
#		Since this property file contains all configurable properties needed to
#		build a source tree, you should be able to build after updating this file.
#		You may compile using runit_compile.bat or run Ant at a command prompt.
#
#	For build machine:
#		To perform scripted build, we generate this build.properties file on-the-fly. 
#		Alternatively, we can use Ant commandline to override the properties. Ant 
#		commandline can be specified through environment varibale ANT_ARGS. For
#		commandline and ANT_ARGS, specify -D<name>=<value> to override a property.
# ---------------------------------------------------------------------------

# Source and libraries
nlbuildroot=$(NLBUILDROOT)

# Build variables
product_name=$(VERSION_PRODUCT_PC)
version_str=$(VERSION_MAJOR_PC).$(VERSION_MINOR_PC).$(VERSION_MAINTENANCE_PC).$(VERSION_PATCH_PC)
build_number=$(BUILD_NUMBER)
endef

export BUILD_PROPERTIES_CONTENT

#
# Targets
#

.PHONY: all
all: compile

.PHONY: compile
compile:
	rm -f build_compile.properties
	echo "$$BUILD_PROPERTIES_CONTENT" > build_compile.properties
	$(ANT) -f build_compile.xml -v compile

.PHONY: clean
clean:
	$(ANT) -f build_compile.xml clean
