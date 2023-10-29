/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "merchant_p.h"

AG_MERCHANT * AG_MERCHANT_new ( )
{
    AG_MERCHANT *m = malloc ( sizeof ( AG_MERCHANT ) );
    m->name = NULL;
    return m;
}

void AG_MERCHANT_SetName ( AG_MERCHANT* m, const char* name )
{
    if ( m ) {
        if ( m->name ) {
            free ( m->name );
        }

        m->name = strdup ( name );
    }
}

void AG_MERCHANT_free ( AG_MERCHANT* m )
{
    if ( m ) {
        if ( m->name ) {
            free ( m->name );
        }

        free ( m );
    }
}

const char * AG_MERCHANT_GetName ( AG_MERCHANT* m )
{
    char * name = NULL;
    if ( m ) {
        name = m->name;
    }
    return name;
}

AG_MERCHANT * AG_MERCHANT_FromJsonElem ( GWEN_JSON_ELEM* meta_elem )
{
    AG_MERCHANT *m = NULL;

    if ( !meta_elem ) {
        return NULL;
    }

    int type = GWEN_JsonElement_GetType ( meta_elem );
    const char *val = GWEN_JsonElement_GetData ( meta_elem );


    if ( ( type == GWEN_JSON_ELEMTYPE_STRING ) && ( strcmp ( val, "null" ) == 0 ) ) {
        return NULL;
    }

    if ( type == GWEN_JSON_ELEMTYPE_OBJECT ) {

        m = AG_MERCHANT_new();

        GWEN_JSON_ELEM *json_name = GWEN_JsonElement_GetElementByPath ( meta_elem, "name", 0 );
        if ( json_name ) {
            AG_MERCHANT_SetName ( m, GWEN_JsonElement_GetData ( json_name ) );
        }
    }
    return m;
}
