// PathFix.cpp: routine to fix the path dependent of architecture
//
//////////////////////////////////////////////////////////////////////
#include "PathFix.h"

#include <string.h>

char *pathfix( const char *path )
{
  long ii;
  char *npath;

  npath = strdup(path);

#ifdef _WIN32
  for (ii=0; ii<strlen(npath); ii++)
  {
    if (npath[ii]=='/') npath[ii]='\\';
  }
#else
  for (ii=0; ii<strlen(npath); ii++)
  {
    if (npath[ii]=='\\') npath[ii]='/';
  }
#endif // !defined WIN_COMPILE

  return npath;
}
