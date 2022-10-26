#include "brain.h"
#include "nlstrings.h"
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "nlTamperproofConfig.h"


int main()
{
  NLTamperproofMap *o;
  NLTamperproofMap::iterator it;
  nlchar *t = NULL;

#if defined(_DEBUG) || defined(DEBUG)
  CELogS::Instance()->SetLevel(CELOG_DEBUG);
#else
  CELogS::Instance()->SetLevel(CELOG_WARNING);
#endif
  CELogS::Instance()->SetPolicy( new CELogPolicy_WinDbg());

  bool r=NLTamperproofConfiguration_Load(NL_TAMPERPROOF_TYPE_FILE, &o);
  if(r && o) {
    for(it=o->begin(); it!=o->end(); it++) {
      if(it->second.type==NL_TAMPERPROOF_TYPE_REGKEY) {
	TRACE(CELOG_DEBUG, _T("1REG: %s\n%s----\n"),  it->second.root.c_str(),
	      it->first.c_str());
	t=_T("REG");
      } else if(it->second.type==NL_TAMPERPROOF_TYPE_FILE)
	t=_T("FILE");
      else if(it->second.type==NL_TAMPERPROOF_TYPE_PROCESS)
	t=_T("PROCESS");
      TRACE(CELOG_DEBUG, _T("%s: %s------\n"), t, it->first.c_str());
    }
  }

  NLTamperproofConfiguration_Free(o);  

  return 0;
}
