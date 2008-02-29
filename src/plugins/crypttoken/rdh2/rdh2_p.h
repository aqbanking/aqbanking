/***************************************************************************
    begin       : Mon Feb 25 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef AQBANKING_PLUGIN_CT_RDH2_RDH2_P_H
#define AQBANKING_PLUGIN_CT_RDH2_RDH2_P_H

#define RDH2_PINMINLENGTH 4

#define RDH2_DISK_HEADER      0x564e
#define RDH2_USER_DATA        0x4b56
# define RDH2_USER_DATA_INFO  0x4b44
# define RDH2_USER_DATA_KEY   0x5345
#define RDH2_BANK_KEY         0xd653
#define RDH2_DATE             0x5244
#define RDH2_MAC              0x444d



GWEN_PLUGIN *GWEN_Crypt_TokenRDH2_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					     const char *modName,
					     const char *fileName);
GWEN_CRYPT_TOKEN* GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_Plugin_CreateToken(GWEN_PLUGIN *pl, const char *name);

int GWENHYWFAR_CB
  GWEN_Crypt_TokenRDH2_Plugin_CheckToken(GWEN_PLUGIN *pl,
					 GWEN_BUFFER *name);




typedef struct GWEN_CRYPT_TOKEN_RDH2 GWEN_CRYPT_TOKEN_RDH2;
struct GWEN_CRYPT_TOKEN_RDH2 {
  GWEN_CRYPT_TOKEN_OPEN_FN openFn;
  GWEN_CRYPT_TOKEN_CREATE_FN createFn;
  GWEN_CRYPT_TOKEN_CLOSE_FN closeFn;


  char password[24];
  int passWordIsSet;

  uint16_t version;
  uint8_t salt[20];
  uint32_t iterations;

  int justCreated;
};

void GWENHYWFAR_CB GWEN_Crypt_TokenRDH2_FreeData(void *bp, void *p);


int GWENHYWFAR_CB
  GWEN_Crypt_TokenRDH2_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_Open(GWEN_CRYPT_TOKEN *ct, int manage, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid);

int GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_Write(GWEN_CRYPT_TOKEN *ct, int fd, int cre, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_Read(GWEN_CRYPT_TOKEN *ct, int fd, uint32_t gid);


int GWENHYWFAR_CB 
  GWEN_Crypt_TokenRDH2_ChangePin(GWEN_CRYPT_TOKEN *ct,
				 int admin,
				 uint32_t gid);



#endif
