# The majority of targets are under prod/pc, although there are some
# prod/common (and a couple of endpoint) targets as well

.PHONY: brain
brain:
	cd $(NLBUILDROOT)/prod/common/brain; $(MAKE)

.PHONY: celog
celog:
	cd $(NLBUILDROOT)/prod/common/celog; $(MAKE)

.PHONY: resattrlib
resattrlib:
	cd $(NLBUILDROOT)/prod/common/resattrlib; $(MAKE)

.PHONY: resattrmgr
resattrmgr: resattrlib celog nextlabstagginglib nl_sysenc_lib
	cd $(NLBUILDROOT)/prod/common/resattrmgr; $(MAKE)

.PHONY: nextlabstagginglib
nextlabstagginglib: nl_sysenc_lib
	cd $(NLBUILDROOT)/prod/common/nextlabstagginglib; $(MAKE)
	
.PHONY: nl_sysenc_lib
nl_sysenc_lib: brain
	cd $(NLBUILDROOT)/prod/pep/endpoint/se/nl_sysenc_lib ; $(MAKE)

.PHONY: cem
cem: brain
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/cem; $(MAKE)
	
.PHONY: ceprivate
ceprivate: cesdk
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/ceprivate; $(MAKE)

.PHONY: conn
conn: brain cem marshal pepman transport
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/conn; $(MAKE)
	
.PHONY: marshal
marshal: brain cem
	cd $(NLBUILDROOT)/prod/pc/marshal; $(MAKE)

.PHONY: pepman
pepman: cem brain marshal transport
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/pepman; $(MAKE)

.PHONY: eval
eval: brain cem marshal transport pepman celog conn
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/EVAL; $(MAKE)

.PHONY: loggin
logging: brain cem marshal transport pepman
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/LOG; $(MAKE)
	
.PHONY: protect
protect: brain cem marshal pepman transport
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/Protect; $(MAKE)

.PHONY: sec
sec: brain cem marshal pepman transport
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SEC; $(MAKE)

.PHONY: service
service: brain cem marshal pepman transport
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/Service; $(MAKE)

.PHONY: cesdk
cesdk: brain cem conn eval logging marshal pepman transport sec service
	cd $(NLBUILDROOT)/prod/pc/cesdk/client/stub/SingleDLL; $(MAKE)

.PHONY: transport
transport: brain
	cd $(NLBUILDROOT)/prod/pc/transport; $(MAKE)

.PHONY: pdpgeneric
pdpgeneric: cesdk celog
	cd $(NLBUILDROOT)/prod/pc/service/generic; $(MAKE)

.PHONY: pdpconn
pdpconn: cesdk celog
	cd $(NLBUILDROOT)/prod/pc/service/conn; $(MAKE)

.PHONY: pdpeval
pdpeval: cesdk celog tamperconfig
	cd $(NLBUILDROOT)/prod/pc/service/eval; $(MAKE)

.PHONY: pdpsec
pdpsec: cesdk celog
	cd $(NLBUILDROOT)/prod/pc/service/SEC; $(MAKE)

.PHONY: pdplog
pdplog: cesdk celog
	cd $(NLBUILDROOT)/prod/pc/service/LOG; $(MAKE)

.PHONY: pdpprivate
pdpprivate: cesdk celog
	cd $(NLBUILDROOT)/prod/pc/service/CEPrivate; $(MAKE)

.PHONY: pdpprotect
pdpprotect: cesdk celog tamperconfig
	cd $(NLBUILDROOT)/prod/pc/tamper/CEProtect; $(MAKE)

.PHONY: tamperconfig
tamperconfig: cesdk celog brain
	cd $(NLBUILDROOT)/prod/pc/tamper/TamperproofConfig; $(MAKE)
	
.PHONY: pdpjni
pdpjni: brain cesdk kif pdpgeneric
	cd $(NLBUILDROOT)/prod/pc/service/pdpjni; $(MAKE)

.PHONY: quenchhash
quenchhash: 
	cd $(NLBUILDROOT)/prod/pc/quenchhash; $(MAKE)

.PHONY: kif
kif: brain
	cd $(NLBUILDROOT)/prod/pc/kif; $(MAKE)

.PHONY: transctrl
transctrl: cesdk kif
	cd $(NLBUILDROOT)/prod/pc/TransCtrl; $(MAKE)

.PHONY: ipcproxy
ipcproxy:
	cd $(NLBUILDROOT)/prod/pc/IPCProxy; $(MAKE)

.PHONY: shared
ipcshared: 
	cd $(NLBUILDROOT)/prod/pc/service/ipc/shared; $(MAKE)

.PHONY: oswrapper
oswrapper: ipcshared
	cd $(NLBUILDROOT)/prod/pc/service/ipc/ipcjni; $(MAKE)

.PHONY: nlca
nlca: nlca_analyze nlca_client nlca_framework nlca_plugin nlca_service

.PHONY: nlca_analyze
nlca_analyze:
	cd $(NLBUILDROOT)/prod/pc/content/analyze; $(MAKE);

.PHONY: nlca_client
nlca_client: brain
	cd $(NLBUILDROOT)/prod/pc/content/client; $(MAKE);

.PHONY: nlca_framework
nlca_framework: brain celog
	cd $(NLBUILDROOT)/prod/pc/content/framework; $(MAKE);

.PHONY: nlca_plugin
nlca_plugin: nlca_client
	cd $(NLBUILDROOT)/prod/pc/content/plugin; $(MAKE);

.PHONY: nlca_service
nlca_service: brain celog nlca_framework
	cd $(NLBUILDROOT)/prod/pc/content/service; $(MAKE);

.PHONY: cepdpman
cepdpman: cesdk celog pdpconn pdpeval pdplog pdpjni oswrapper pdpprotect pdpprivate transctrl kif pdpgeneric pdpsec quenchhash ipcproxy
	cd $(NLBUILDROOT)/prod/pc/service/pdpman; $(MAKE)
