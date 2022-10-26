################################################################################
#
# Content Analysis
#
################################################################################

Files:

analyze/	Content Analysis interface for file/text scanning.
docs/		Functional spec. and design doc.
libs/           BOOST Serialization library (debug|release)
plugin/		Policy Controller Plug-in for NLCA Service
framework/      Support framework for NLCA service and clients of NLCA service.
service/	NLCA Service
configuration/  Configuration for all related NLCA services.

                Files:
                  attributes.xml
                  obligations.xml
               
                These contain attribute and redaction expression definitions.
                Changes to those values *must* be verified against many file
                types due to encoding.

test/           Testing/Regression for Content Analysis via nlcrack.
