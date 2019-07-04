/***************************************************************************
 begin       : Sun Jun 23 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_p.h"


/*

  TODO:
  - ReadXml
  - WriteXml

  - ReadFints
  - WriteFints

  - TREE=encode(DEFS, DB)
  - DB  =decode(TREE, DEFS)


  - Getting data:
    - DEFS     DATA
      DEG      DEG    WORK-DATA = DEG
      DE       DEG    WORK-DATA = first DE below DEG

    - context:
      - current DB path
      - current DEF node
      - current DATA node

 */



AQFINTS_PARSER *AQFINTS_Parser_new()
{
  AQFINTS_PARSER *parser;

  GWEN_NEW_OBJECT(AQFINTS_PARSER, parser);

  parser->segmentList=AQFINTS_Segment_List_new();
  parser->groupTree=AQFINTS_Element_new();

  return parser;
}



void AQFINTS_Parser_free(AQFINTS_PARSER *parser)
{
  if (parser) {
    AQFINTS_Element_Tree2_free(parser->groupTree);
    AQFINTS_Segment_List_free(parser->segmentList);

    GWEN_FREE_OBJECT(parser);
  }
}



AQFINTS_SEGMENT_LIST *AQFINTS_Parser_GetSegmentList(const AQFINTS_PARSER *parser)
{
  assert(parser);
  return parser->segmentList;
}



AQFINTS_ELEMENT *AQFINTS_Parser_GetGroupTree(const AQFINTS_PARSER *parser)
{
  assert(parser);
  return parser->groupTree;
}



AQFINTS_SEGMENT *AQFINTS_Parser_FindSegment(const AQFINTS_PARSER *parser, const char *id, int segmentVersion, int protocolVersion)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(parser->segmentList);
  while(segment) {
    if ((segmentVersion==0 || segmentVersion==AQFINTS_Segment_GetSegmentVersion(segment)) &&
        (protocolVersion==0 || protocolVersion==AQFINTS_Segment_GetProtocolVersion(segment))){
      if (!(id && *id))
        return segment;
      else {
        const char *s;

        s=AQFINTS_Segment_GetId(segment);
        if (s && *s && strcasecmp(s, id)==0)
          return segment;
      }
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return NULL;
}



AQFINTS_ELEMENT *AQFINTS_Parser_FindGroupInTree(AQFINTS_ELEMENT *groupTree, const char *id, int version)
{
  AQFINTS_ELEMENT *group;

  group=AQFINTS_Element_Tree2_GetFirstChild(groupTree);
  while(group) {
    if (version==0 || version==AQFINTS_Element_GetVersion(group)) {
      if (!(id && *id))
        return group;
      else {
        const char *s;

        s=AQFINTS_Element_GetId(group);
        if (s && *s && strcasecmp(s, id)==0)
          return group;
      }
    }
    group=AQFINTS_Element_Tree2_GetNext(group);
  }

  return NULL;
}






