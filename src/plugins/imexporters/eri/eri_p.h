#ifndef AQHBCI_IMEX_ERI_P_H
#define AQHBCI_IMEX_ERI_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_ERI AH_IMEXPORTER_ERI;
struct AH_IMEXPORTER_ERI {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};


AB_IMEXPORTER* eri_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void AH_ImExporterERI_FreeData(void *bp, void *p);

int AH_ImExporterERI_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);
int AH_ImExporterERI_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);

int AH_ImExporterERI_CheckFile(AB_IMEXPORTER *ie, const char *fname);


int AH_ImExporterERI__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
                                      GWEN_DB_NODE *dbParams);


#define CHECKBUF_LENGTH 128

#endif /* AQHBCI_IMEX_ERI_P_H */
