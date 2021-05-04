/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_PARSER_H
#define AQFINTS_PARSER_H


#include "libaqfints/parser/element.h"
#include "libaqfints/parser/segment.h"
#include "libaqfints/parser/jobdef.h"

#include <gwenhywfar/xml.h>


typedef struct AQFINTS_PARSER AQFINTS_PARSER;



/** @name Constructor, Destructor
 *
 */
/*@{*/
/**
 * Constructor.
 */
AQFINTS_PARSER *AQFINTS_Parser_new();


/**
 * Destructor.
 */
void AQFINTS_Parser_free(AQFINTS_PARSER *parser);
/*@}*/



/** @name Read HBCI/FinTS Segments
 *
 */
/*@{*/

/**
 * Read a HBCI message from a buffer into a GWEN_DB_NODE.
 *
 * First reads a HBCI message into a segment list and retrieves data from that list
 * by looking up definitions for the segments contained in the message.
 * Unknown segments are skipped.
 *
 * @return 0 if okay (i.e. at least one segment could be parsed), error code otherwise
 * @param parser parser object
 * @param ptrBuf pointer to the buffer containing the HBCI message
 * @param lenBuf size of the HBCI message
 * @param dbData GWEN_DB_NODE tree to receive the parsed data
 */
int AQFINTS_Parser_ReadIntoDb(AQFINTS_PARSER *parser,
                              const uint8_t *ptrBuf,
                              uint32_t lenBuf,
                              GWEN_DB_NODE *dbData);


/**
 * Read a HBCI message into a segment list.
 *
 * This function only parses HBCI messages into segment lists without interpreting the data.
 *
 * @return 0 if okay, errorcode otherwise
 * @param parser parser object
 * @param targetSegmentList segment list to receive the HBCI segments read
 * @param ptrBuf pointer to the buffer containing the HBCI message
 * @param lenBuf size of the HBCI message
 */
int AQFINTS_Parser_ReadIntoSegmentList(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *targetSegmentList,
                                       const uint8_t *ptrBuf,
                                       uint32_t lenBuf);


/**
 * Parses a segment list into a GWEN_DB_NODE.
 *
 * The segment list should have been read via @ref AQFINTS_Parser_ReadIntoSegmentList().
 *
 * @return 0 if okay, errorcode otherwise
 * @param parser parser object
 * @param segmentList segment list to read data from
 */
int AQFINTS_Parser_ReadSegmentListToDb(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *segmentList);
/*@}*/



/** @name Write HBCI/FinTS Segments
 *
 */
/*@{*/

int AQFINTS_Parser_WriteSegment(AQFINTS_PARSER *parser, AQFINTS_SEGMENT *segment);
/*@}*/




/** @name Load Definition Files
 *
 */
/*@{*/

/**
 *  Add a path containing *.fints files to the parser,
 *
 * The paths added here are used by @ref AQFINTS_Parser_ReadFiles().
 *
 * @param parser parser object
 * @param path folder to add
 */
void AQFINTS_Parser_AddPath(AQFINTS_PARSER *parser, const char *path);


/**
 * Read files from the folders specified via @ref AQFINTS_Parser_AddPath().
 *
 * Adding paths after calling this function here has no effect.
 * Only returns an error if no file could be loaded (either because of errors or because there was no file to load).
 * This function is used to load segment and job definitions for the parser.
 *
 * @return 0 if okay, errorcode otherwise
 * @param parser parser object
 */
int AQFINTS_Parser_ReadFiles(AQFINTS_PARSER *parser);
/*@}*/



/** @name Find Segments
 *
 */
/*@{*/

/**
 * Find segment by id.
 *
 * @return segment found (NULL otherwise)
 * @param parser parser object
 * @param id segment id (see @ref AQFINTS_Segment_GetId)
 * @param segmentVersion segment version (0 matches any)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_SEGMENT *AQFINTS_Parser_FindSegmentById(const AQFINTS_PARSER *parser, const char *id, int segmentVersion,
                                                int protocolVersion);


/**
 * Find segment by code.
 *
 * @return segment found (NULL otherwise)
 * @param parser parser object
 * @param code HBCI segment code (like "HNSHK", see @ref AQFINTS_Segment_GetCode)
 * @param segmentVersion segment version (0 matches any)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_SEGMENT *AQFINTS_Parser_FindSegmentByCode(const AQFINTS_PARSER *parser, const char *code, int segmentVersion,
                                                  int protocolVersion);


/**
 *  Find segment with highest version for the given protocol.
 *
 * @return segment found (NULL otherwise)
 * @param parser parser object
 * @param code HBCI segment code (like "HNSHK", see @ref AQFINTS_Segment_GetCode)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_SEGMENT *AQFINTS_Parser_FindSegmentHighestVersionForProto(const AQFINTS_PARSER *parser,
                                                                  const char *code,
                                                                  int protocolVersion);


/**
 * Creates segment ready to be used by the caller.
 *
 * This function looks up the segment definition using given id and segment version and makes a copy of that
 * definition. It also creates and sets a GWEN_DB_NODE for the segment and stores it with the new segment.
 *
 * @return prepared segment
 * @param parser parser object
 * @param code HBCI segment code (like "HNSHK", see @ref AQFINTS_Segment_GetCode)
 * @param segmentVersion segment version (0 matches any)
 */
AQFINTS_SEGMENT *AQFINTS_Parser_CreateSegmentByCode(const AQFINTS_PARSER *parser, const char *code, int segmentVersion);

/*@}*/



/** @name Find Job Definitions
 *
 */
/*@{*/

/**
 * Find job definition by id.
 *
 * @return object found (NULL otherwise)
 * @param parser parser object
 * @param id job id (see @ref AQFINTS_JobDef_GetId)
 * @param jobVersion segment version (0 matches any)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_JOBDEF *AQFINTS_Parser_FindJobDefByCode(const AQFINTS_PARSER *parser, const char *id, int jobVersion,
                                                int protocolVersion);


/**
 * Find job definition by code.
 *
 * @return object  found (NULL otherwise)
 * @param parser parser object
 * @param code HBCI segment code (like "HNSHK", see @ref AQFINTS_JobDef_GetCode)
 * @param jobVersion segment version (0 matches any)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_JOBDEF *AQFINTS_Parser_FindJobDefById(const AQFINTS_PARSER *parser, const char *id, int jobVersion,
                                              int protocolVersion);


/**
 * Find job definition by name of the PARAMs segment.
 *
 * @return object  found (NULL otherwise)
 * @param parser parser object
 * @param code HBCI segment code (like "HNSHK", see @ref AQFINTS_JobDef_GetParams)
 * @param jobVersion segment version (0 matches any)
 * @param protocolVersion protocol version (0 matches any)
 */
AQFINTS_JOBDEF *AQFINTS_Parser_FindJobDefByParams(const AQFINTS_PARSER *parser, const char *params, int jobVersion,
                                                  int protocolVersion);

/*@}*/



void AQFINTS_Parser_DumpDefinitions(AQFINTS_PARSER *parser, int indent);



#endif

