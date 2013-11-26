

#include "server/eserver.h"
#include "server/user.h"
#include "server/us_simple.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



int test1(int argc, char **argv) {
  EBS_SERVER *es;
  EBS_USERSTORE *us;
  EBS_USER *u;
  const char *userId;

  if (argc<3) {
    fprintf(stderr, "User id needed.\n");
    return 1;
  }
  userId=argv[2];

  es=EBS_Server_new("/tmp/ebicstest");
  us=EBS_UsSimple_new(EBS_Server_GetHome(es));
  EBS_Server_SetUserStore(es, us);

  u=EBS_UserStore_CreateUser(us, userId, 0);
  if (u==0) {
    fprintf(stderr, "Could not create user \"%s\"\n", userId);
    return 2;
  }
  return 0;
}



int test2(int argc, char **argv) {
  EBS_SERVER *es;
  EBS_USERSTORE *us;
  EBS_USER *u;

  es=EBS_Server_new("/tmp/ebicstest");
  us=EBS_UsSimple_new(EBS_Server_GetHome(es));
  EBS_Server_SetUserStore(es, us);

  u=EBS_UserStore_GetUser(us, "martin", "partnerId");
  if (u==0) {
    fprintf(stderr, "Could not load user\n");
    return 2;
  }
  return 0;
}



int test3(int argc, char **argv) {
  EBS_SERVER *es;
  EBS_USERSTORE *us;
  EBS_USER *u;
  int rv;

  es=EBS_Server_new("/tmp/ebicstest");
  us=EBS_UsSimple_new(EBS_Server_GetHome(es));
  EBS_Server_SetUserStore(es, us);

  u=EBS_UserStore_GetUser(us, "martin", "partnerId");
  if (u==0) {
    fprintf(stderr, "Could not load user\n");
    return 2;
  }

  rv=EBS_UserStore_DelUser(us, u);
  if (rv) {
    fprintf(stderr, "Could not delete user\n");
    return 2;
  }

  return 0;
}



int readFile(const char *fname, GWEN_BUFFER *buf) {
  FILE *f;

  f=fopen(fname, "r");
  if (!f)
    return -1;
  while(!feof(f)) {
    char tmpbuf[512];
    size_t size;

    size=fread(tmpbuf, 1, sizeof(tmpbuf), f);
    if (!size) {
      fprintf(stderr, "fread(%s): %s\n", fname, strerror(errno));
      fclose(f);
      return -1;
    }
    GWEN_Buffer_AppendBytes(buf, tmpbuf, size);
  }

  fclose(f);

  return 0;
}



int test4(int argc, char **argv) {
  int rv;
  GWEN_BUFFER *rqbuf;
  GWEN_BUFFER *rspbuf;
  EBS_SERVER *es;
  EBS_USERSTORE *us;

  es=EBS_Server_new("/tmp/ebicstest");
  us=EBS_UsSimple_new(EBS_Server_GetHome(es));
  EBS_Server_SetUserStore(es, us);

  rqbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rspbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  if (readFile("test-ini.xml", rqbuf)) {
    fprintf(stderr, "Error reading file.\n");
    return 3;
  }
  rv=EBS_Server_HandleMessage(es,
                              GWEN_Buffer_GetStart(rqbuf),
                              GWEN_Buffer_GetUsedBytes(rqbuf),
                              rspbuf);
  if (rv) {
    fprintf(stderr, "ERROR: Could not create response.\n");
    return 3;
  }

  GWEN_Buffer_Dump(rspbuf, stderr, 2);

  return 0;
}



int main(int argc, char **argv) {
  const char *cmd;
  int rv;

  if (argc<2) {
    fprintf(stderr, "Command missing.\n");
    return 1;
  }

  cmd=argv[1];
  if (strcasecmp(cmd, "test1")==0)
    rv=test1(argc, argv);
  else if (strcasecmp(cmd, "test2")==0)
    rv=test2(argc, argv);
  else if (strcasecmp(cmd, "test3")==0)
    rv=test3(argc, argv);
  else if (strcasecmp(cmd, "test4")==0)
    rv=test4(argc, argv);
  else {
    fprintf(stderr, "Unknown test \"%s\"\n", cmd);
    return 1;
  }

  return rv;
}

