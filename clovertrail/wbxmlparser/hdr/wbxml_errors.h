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
 
/**
 * @file wbxml_errors.h
 * @ingroup wbxml_errors
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/11/18
 *
 * @brief WBXML Error Codes Handling
 *
 * @todo Replace the Ugly negative values in the WBXMLError enum
 */

#ifndef WBXML_ERRORS_H
#define WBXML_ERRORS_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_errors
 *  @{ 
 */

/**
 * @brief The WBXML Parser Errors enumeration
 */
typedef enum WBXMLError_e {
    /* Generic Errors */
    WBXML_OK = 0,       /**< No Error */
    WBXML_NOT_ENCODED,  /**< Not an error; just a special internal return code */
    WBXML_ERROR_ATTR_TABLE_UNDEFINED,
    WBXML_ERROR_BAD_DATETIME,
    WBXML_ERROR_BAD_PARAMETER,    
    WBXML_ERROR_INTERNAL,
    WBXML_ERROR_LANG_TABLE_UNDEFINED,
    WBXML_ERROR_NOT_ENOUGH_MEMORY,
    WBXML_ERROR_NOT_IMPLEMENTED,
    WBXML_ERROR_TAG_TABLE_UNDEFINED,
    WBXML_ERROR_WV_DATETIME_FORMAT,
    /* WBXML Parser Errors */    
    WBXML_ERROR_ATTR_VALUE_TABLE_UNDEFINED,
    WBXML_ERROR_BAD_LITERAL_INDEX,
    WBXML_ERROR_BAD_NULL_TERMINATED_STRING_IN_STRING_TABLE,
    WBXML_ERROR_BAD_OPAQUE_LENGTH,
    WBXML_ERROR_EMPTY_WBXML,
    WBXML_ERROR_END_OF_BUFFER,
    WBXML_ERROR_ENTITY_CODE_OVERFLOW,
    WBXML_ERROR_EXT_VALUE_TABLE_UNDEFINED,
    WBXML_ERROR_INVALID_STRTBL_INDEX,
    WBXML_ERROR_LITERAL_NOT_NULL_TERMINATED_IN_STRING_TABLE,
    WBXML_ERROR_NOT_NULL_TERMINATED_INLINE_STRING,
    WBXML_ERROR_NULL_PARSER,
    WBXML_ERROR_NULL_STRING_TABLE,
    WBXML_ERROR_STRING_EXPECTED,
    WBXML_ERROR_STRTBL_LENGTH,   
    WBXML_ERROR_UNKNOWN_ATTR,
    WBXML_ERROR_UNKNOWN_ATTR_VALUE,
    WBXML_ERROR_UNKNOWN_EXTENSION_TOKEN,
    WBXML_ERROR_UNKNOWN_EXTENSION_VALUE,
    WBXML_ERROR_UNKNOWN_PUBLIC_ID,
    WBXML_ERROR_UNKNOWN_TAG,
    WBXML_ERROR_UNVALID_MBUINT32,
    WBXML_ERROR_WV_INTEGER_OVERFLOW,    
    /* WBXML Encoder Errors */
    WBXML_ERROR_ENCODER_APPEND_DATA,
    WBXML_ERROR_STRTBL_DISABLED,
    WBXML_ERROR_UNKNOWN_XML_LANGUAGE,
    WBXML_ERROR_XML_NODE_NOT_ALLOWED,
    WBXML_ERROR_XML_NULL_ATTR_NAME,
    WBXML_ERROR_XML_PARSING_FAILED    
} WBXMLError;


/**
 * @brief Return a String describing an Error Code
 * @param error_code The error code returned by wbxml_parser_parse()
 * @return The error description
 */
WBXML_DECLARE(const WB_UTINY *) wbxml_errors_string(WBXMLError error_code);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_ERRORS_H */
