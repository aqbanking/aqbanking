/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_OFXGROUP_L_H
#define AIO_OFX_OFXGROUP_L_H

#include <aqbanking/imexporter.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xmlctx.h>



typedef struct AIO_OFX_GROUP AIO_OFX_GROUP;
GWEN_INHERIT_FUNCTION_DEFS(AIO_OFX_GROUP)


typedef int (*AIO_OFX_GROUP_STARTTAG_FN)(AIO_OFX_GROUP *g,
					 const char *tagName);
typedef int (*AIO_OFX_GROUP_ENDTAG_FN)(AIO_OFX_GROUP *g,
				       const char *tagName);
typedef int (*AIO_OFX_GROUP_ADDDATA_FN)(AIO_OFX_GROUP *g,
					const char *data);

typedef int (*AIO_OFX_GROUP_ENDSUBGROUP_FN)(AIO_OFX_GROUP *g,
                                            AIO_OFX_GROUP *sg);




AIO_OFX_GROUP *AIO_OfxGroup_new(const char *groupName,
				AIO_OFX_GROUP *parent,
				GWEN_XML_CONTEXT *ctx);
void AIO_OfxGroup_free(AIO_OFX_GROUP *g);


AIO_OFX_GROUP *AIO_OfxGroup_GetParent(const AIO_OFX_GROUP *g);
GWEN_XML_CONTEXT *AIO_OfxGroup_GetXmlContext(const AIO_OFX_GROUP *g);
const char *AIO_OfxGroup_GetGroupName(const AIO_OFX_GROUP *g);



AIO_OFX_GROUP_STARTTAG_FN
  AIO_OfxGroup_SetStartTagFn(AIO_OFX_GROUP *g,
			     AIO_OFX_GROUP_STARTTAG_FN f);

AIO_OFX_GROUP_ENDTAG_FN
  AIO_OfxGroup_SetEndTagFn(AIO_OFX_GROUP *g,
			   AIO_OFX_GROUP_ENDTAG_FN f);

AIO_OFX_GROUP_ADDDATA_FN
  AIO_OfxGroup_SetAddDataFn(AIO_OFX_GROUP *g,
			    AIO_OFX_GROUP_ADDDATA_FN f);

AIO_OFX_GROUP_ENDSUBGROUP_FN
  AIO_OfxGroup_SetEndSubGroupFn(AIO_OFX_GROUP *g,
				AIO_OFX_GROUP_ENDSUBGROUP_FN f);



int AIO_OfxGroup_StartTag(AIO_OFX_GROUP *g, const char *tagName);

/**
 * @return 1 if this tag ends the current group, 0 otherwise (<0 on error)
 */
int AIO_OfxGroup_EndTag(AIO_OFX_GROUP *g, const char *tagName);
int AIO_OfxGroup_AddData(AIO_OFX_GROUP *g,
			 const char *data);

int AIO_OfxGroup_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);

#endif
