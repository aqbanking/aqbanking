/* A program that "make installcheck" builds against the installed
 * AqBanking. It fails if the installed library and headers do not match.
 */

#include <aqbanking/banking.h>
#include <aqbanking/version.h>

#include <stdio.h>



int main(void)
{
  int major, minor, patchlevel, build;

  AB_Banking_GetVersion(&major, &minor, &patchlevel, &build);
  printf("library %d.%d.%d.%d, headers %d.%d.%d.%d\n",
         major, minor, patchlevel, build,
         AQBANKING_VERSION_MAJOR, AQBANKING_VERSION_MINOR,
         AQBANKING_VERSION_PATCHLEVEL, AQBANKING_VERSION_BUILD);
  if (major!=AQBANKING_VERSION_MAJOR ||
      minor!=AQBANKING_VERSION_MINOR ||
      patchlevel!=AQBANKING_VERSION_PATCHLEVEL ||
      build!=AQBANKING_VERSION_BUILD) {
    fprintf(stderr, "library and header versions differ\n");
    return 1;
  }
  return 0;
}
