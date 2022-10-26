
#include "nextlabs_feature_manager.hpp"

int main(void)
{

  nextlabs::feature feat;
  DWORD result;
  BOOL enabled;

  result = feat.open(L"features.cfg");
  if( result != ERROR_SUCCESS )
  {
    fprintf(stderr, "open failed\n");
    return 1;
  }

  result = feat.query(L"NEXTLABS_FEATURE_FAKE0",&enabled);
  fprintf(stdout, "feat.query : %d : enabled %d\n", result, enabled);
  if( result == ERROR_SUCCESS )
  {
    fprintf(stdout, "NEXTLABS_FEATURE_FAKE = TRUE\n");
  }

  feat.close();

  return 0;
}/* main */
