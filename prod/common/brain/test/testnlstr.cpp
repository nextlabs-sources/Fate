
#include <stdio.h>


#define NLMODULE mynlstrtest

#include "cetype.h"
#include "brain.h"
#include "nlstrings.h"

void testnlprintf ()
{
  int   i = 1;
  long  l = 2;
  nlchar * s = _T("mystring");

  // Use calloc to initialize the memory 
  nlchar * str_buffer = (nlchar *) calloc (1000, sizeof(nlchar));
  if( NULL == str_buffer )
  {
	  nlfprintf(stderr, L"Error: calloc failed to allocate memory");
	  return;
  }

  nlprintf (_T("12345 %ld\t%d %s %p 0x%x\n"), l, i, s, s, i);

  nlsprintf (str_buffer, 1000 * sizeof(nlchar), _T("This is our test string: \"%s\":%d\n"), s, i);

  nlfprintf (stderr, str_buffer);
  free (str_buffer);
}

void testnlstring()
{
  // Simple test to showcase how to use the nl-string library

  nlchar s1[NL_PATH_MAX];
  nlchar s2[NL_PATH_MAX];
  nlchar *sp1, *sp2;
  nlchar *sp3, *sp4;

  memset (s1, 0x0, sizeof(s1));
  memset (s2, 0x0, sizeof(s2));

  TRACE (1, _T("sizeof nlchar %d\n"), sizeof (nlchar));

  nlstrcpy  (s1, _T("1234567890"));
  nlstrncpy (s2, _T("1234567890"), 5);

  TRACE (1, _T("cpy: Expect: 1234567890\t\t value: %s\n"), s1);
  TRACE (1, _T("ncpy: Expect: 12345\t\t value %s\n"), s2);

  /* Strcat */
  nlstrcat  (s1, s2);
  TRACE (1, _T("cat: Expect: 123456789012345\t\t value: %s\n"), s1);

  nlstrncat (s1, _T("abcdef"), 3);
  TRACE (1, _T("ncat: Expect: 123456789012345abc\t\t value %s\n"), s1);

  sp1= nlstrdup (s2);
  TRACE (1, _T("dup: Expect: 12345\t\t value %s\n"), sp1);
  if (sp1 == NULL)
  {
    return;
  }

  nluint32 len =static_cast<nluint32> (nlstrlen (sp1));
  TRACE (1, _T("len: Expect: 5\t\t value %d\n"), len);

  nluint32 blen =static_cast<nluint32> (nlstrblen (sp1));
  TRACE (1, _T("len: Expect: 5/10\t\t value %d\n"), blen);

  sp2 = nlstrstr (s1, _T("890"));
  if (sp2 == NULL)
  {
    return;
  }

  TRACE (1, _T("str: Expect: 89012345abc\t\t value %s\n"), sp2);

  TRACE (1, _T("cmp: Expect: 0\t\t value %d\n"), nlstrcmp (sp1, s2));

  TRACE (1, _T("cmp: Expect: nonZero\t\t value %d\n"), nlstrncmp (sp1, sp2, 3));

  sp3 = nlstrchr (s1, _T('3'));

  TRACE (1, _T("chr: Expect: 3456789012345abc\t\t value %s\n"), sp3);

  sp4 = nlstrrchr (s1, '4');

  TRACE (1, _T("chr: Expect: 45abc\t\t value %s\n"), sp4);

  free (sp1);

  char asc_buf[100];
  if(nlstrtoascii(_T("Test nlstrtoascii"), asc_buf, sizeof asc_buf)) {
    if(strcmp(asc_buf, "Test nlstrtoascii"))
      printf("nlstrtoascii failed: '%s'.\n", asc_buf);
    else
      TRACE(0, _T("Test nlstrtoascii succeed.\n"));
  } else
    TRACE(0, _T("nlstrtoascii failed\n"));
}
