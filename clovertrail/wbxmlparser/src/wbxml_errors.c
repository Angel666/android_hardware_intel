/*
 * WBXML Lib, the WBXML Library.
 * Copyright (C) 2002-2003  Aymerick Jéhanne
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License (version 2.1) as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * LGPL v2.1: http://www.gnu.org/licenses/lgpl.txt
 *
 * Author Contact: libwbxml@jehanne.org
 * WBXML Lib home: http://libwbxml.jehanne.org
 */

/*
 * Copyright (C) 2003 Motorola Inc.
 * 
 * Date       Author      Comments
 * -------------------------------------
 * 10/15/2003 Motorola    Add more strict data type 
 */

/**
 * @file wbxml_errors.c
 * @ingroup wbxml_errors
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/11/18
 *
 * @brief WBXML Errors Handling
 */

#include <stdlib.h>
#include <string.h>

#include "wbxml.h"


/**
 * @brief Error Code item
 */
typedef struct errCodeItem_s {
    WB_TINY code;           /**< Error Code */
    const WB_UTINY *string; /**< Error Description */
} errCodeItem;


/**
 * @brief Error Code table
 */
static const errCodeItem error_table [] = {

    /* Generic Errors */
    { WBXML_OK,                                 (const WB_UTINY *)"No Error" },
    { WBXML_NOT_ENCODED,                        (const WB_UTINY *)"Nor Encoded" },
    { WBXML_ERROR_ATTR_TABLE_UNDEFINED,         (const WB_UTINY *)"Attribute Table Undefined" },
    { WBXML_ERROR_BAD_DATETIME,                 (const WB_UTINY *)"Bad %Datetime Format" },
    { WBXML_ERROR_BAD_PARAMETER,                (const WB_UTINY *)"Bad Parameter" },
    { WBXML_ERROR_INTERNAL,                     (const WB_UTINY *)"Internal Error" },
    { WBXML_ERROR_LANG_TABLE_UNDEFINED,         (const WB_UTINY *)"Languages Table Undefined" },
    { WBXML_ERROR_NOT_ENOUGH_MEMORY,            (const WB_UTINY *)"Not Enough Memory" },
    { WBXML_ERROR_NOT_IMPLEMENTED,              (const WB_UTINY *)"Not Implemented" },
    { WBXML_ERROR_TAG_TABLE_UNDEFINED,          (const WB_UTINY *)"Tag Table Undefined" },
    { WBXML_ERROR_WV_DATETIME_FORMAT,           (const WB_UTINY *)"Bad Wireless-Village Date and Time Format" },
    /* WBXML Parser Errors */
    { WBXML_ERROR_ATTR_VALUE_TABLE_UNDEFINED,   (const WB_UTINY *)"Attribute Value Table Undefined" },
    { WBXML_ERROR_BAD_LITERAL_INDEX,            (const WB_UTINY *)"Bad Literal Index" },
    { WBXML_ERROR_BAD_NULL_TERMINATED_STRING_IN_STRING_TABLE,    (const WB_UTINY *)"Not NULL Terminated String in String Table" },
    { WBXML_ERROR_BAD_OPAQUE_LENGTH,            (const WB_UTINY *)"Bad Opaque Length" },
    { WBXML_ERROR_EMPTY_WBXML,                  (const WB_UTINY *)"Empty WBXML" },    
    { WBXML_ERROR_END_OF_BUFFER,                (const WB_UTINY *)"Unexpected End Of WBXML Buffer" },
    { WBXML_ERROR_ENTITY_CODE_OVERFLOW,         (const WB_UTINY *)"Entity Code Overflow" },
    { WBXML_ERROR_EXT_VALUE_TABLE_UNDEFINED,    (const WB_UTINY *)"Extension Value Table Undefined" },
    { WBXML_ERROR_INVALID_STRTBL_INDEX,         (const WB_UTINY *)"Invalid String Table Index" },
    { WBXML_ERROR_LITERAL_NOT_NULL_TERMINATED_IN_STRING_TABLE,    (const WB_UTINY *)"Literal Not NULL Terminated in String Table" },
    { WBXML_ERROR_NOT_NULL_TERMINATED_INLINE_STRING,             (const WB_UTINY *)"Not NULL Terminated Inline String" },
    { WBXML_ERROR_NULL_PARSER,                  (const WB_UTINY *)"Null Parser" },
    { WBXML_ERROR_NULL_STRING_TABLE,            (const WB_UTINY *)"No String Table In Document" },
    { WBXML_ERROR_STRING_EXPECTED,              (const WB_UTINY *)"String Expected" },
    { WBXML_ERROR_STRTBL_LENGTH,                (const WB_UTINY *)"Bad String Table Length" },
    { WBXML_ERROR_UNKNOWN_ATTR,                 (const WB_UTINY *)"Unknown Attribute" },
    { WBXML_ERROR_UNKNOWN_ATTR_VALUE,           (const WB_UTINY *)"Unknown Attribute Value" },
    { WBXML_ERROR_UNKNOWN_EXTENSION_TOKEN,      (const WB_UTINY *)"Unknown Extension Token" },
    { WBXML_ERROR_UNKNOWN_EXTENSION_VALUE,      (const WB_UTINY *)"Unknown Extension Value token" },
    { WBXML_ERROR_UNKNOWN_PUBLIC_ID,            (const WB_UTINY *)"Unknown Public ID" },    
    { WBXML_ERROR_UNKNOWN_TAG,                  (const WB_UTINY *)"Unknown Tag" },
    { WBXML_ERROR_UNVALID_MBUINT32,             (const WB_UTINY *)"Unvalid MultiByte UINT32" },
    { WBXML_ERROR_WV_INTEGER_OVERFLOW,          (const WB_UTINY *)"Wireless-Village Integer Overflow" },
    /* WBXML Encoder Errors */
    { WBXML_ERROR_ENCODER_APPEND_DATA,          (const WB_UTINY *)"Can't append data to output buffer" },
    { WBXML_ERROR_STRTBL_DISABLED,              (const WB_UTINY *)"String Table generation disabled: can't encode Literal" },
    { WBXML_ERROR_XML_NODE_NOT_ALLOWED,         (const WB_UTINY *)"XML Node Type not allowed" },
    { WBXML_ERROR_XML_NULL_ATTR_NAME,           (const WB_UTINY *)"NULL XML Attribute Name" },
    { WBXML_ERROR_XML_PARSING_FAILED,           (const WB_UTINY *)"Parsing of XML Document Failed" },
};

#define ERROR_TABLE_SIZE ((WB_ULONG) (sizeof(error_table) / sizeof(error_table[0])))


/***************************************************
 *    Public Functions
 */

WBXML_DECLARE(const WB_UTINY *) wbxml_errors_string(WBXMLError error_code)
{
    WB_ULONG i;

    for (i=0; i < ERROR_TABLE_SIZE; i++) {
        if (error_table[i].code == error_code)
            return error_table[i].string;
    }

    return (const WB_UTINY *)"Unknown Error Code";
}
