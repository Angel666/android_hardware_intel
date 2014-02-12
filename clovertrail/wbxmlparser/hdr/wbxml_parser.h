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
 * @file wbxml_parser.h
 * @ingroup wbxml_parser
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/03/12
 *
 * @brief WBXML Parser
 */

#ifndef WBXML_PARSER_H
#define WBXML_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_parser 
 *  @{ 
 */

typedef struct WBXMLParser_s WBXMLParser;

/**
 * @brief Create a WBXML Parser
 * @result Return the newly created WBXMLParser, or NULL if not enough memory
 */
WBXML_DECLARE(WBXMLParser *) wbxml_parser_create(void);

/**
 * @brief Destroy a WBXML Parser
 * @param parser The WBXMLParser to destroy
 */
WBXML_DECLARE(void) wbxml_parser_destroy(WBXMLParser *parser);

/**
 * @brief Parse a WBXML document, using User Defined callbacks
 * @param parser The WBXML Parser to use for parsing 
 * @param wbxml The WBXML document to parse
 * @param wbxml_len The WBXML document length
 * @result Return WBXML_OK if no error, an error code otherwise
 */
WBXML_DECLARE(WBXMLError) wbxml_parser_parse(WBXMLParser *parser, WB_UTINY *wbxml, WB_ULONG wbxml_len);

/**
 * @brief Parse a WBXML document, using internal callbacks, and construct a WBXML Tree
 * @param wbxml The WBXML document to parse
 * @param wbxml_len The WBXML document length
 * @param tree [out] The resulting WBXML Tree
 * @result Return WBXML_OK if no error, an error code otherwise
 */
WBXML_DECLARE(WBXMLError) wbxml_parser_parse_to_tree(WB_UTINY *wbxml, WB_ULONG wbxml_len, WBXMLTree **tree);

/**
 * @brief Set User Data for a WBXML Parser
 * @param parser The WBXML Parser
 * @param user_data User data (returned as a parameter in every Content Handler callbacks)
 */
WBXML_DECLARE(void) wbxml_parser_set_user_data(WBXMLParser *parser, void *user_data);

/**
 * @brief Set Content Handler for a WBXML Parser
 * @param parser The WBXML Parser
 * @param content_handler The Content Handler structure
 */
WBXML_DECLARE(void) wbxml_parser_set_content_handler(WBXMLParser *parser, WBXMLContentHandler *content_handler);

/**
 * @brief Set Main WBXML Languages Table
 * @param parser The WBXML Parser
 * @param main_table The Main WBXML Languages Table to set
 */
WBXML_DECLARE(void) wbxml_parser_set_main_table(WBXMLParser *parser, const WBXMLLangEntry *main_table);

/**
 * @brief Force to parse the Document with a given WBXML Public ID
 * @param parser The WBXML Parser
 * @param public_id The WBXML Public ID
 * @return TRUE is WBXML Public ID is set, FALSE otherwise
 * @note This permits to force the WBXML Parser to parse a WBXML Document with a given Public ID.
 *       If this fonction is used, the internal Public ID of the WBXML Document is ignored.
 */
WBXML_DECLARE(WB_BOOL) wbxml_parser_set_wbxml_public_id(WBXMLParser *parser, WB_LONG public_id);

/**
 * @brief Get WBXML Public ID
 * @param parser The WBXML Parser
 * @return The WBXML Public ID of current parsing document
 */
WBXML_DECLARE(WB_ULONG) wbxml_parser_get_wbxml_public_id(WBXMLParser *parser);

/**
 * @brief Get XML Public ID
 * @param parser The WBXML Parser
 * @return The XML Public ID of current parsing document, or NULL if not found
 */
WBXML_DECLARE(const WB_UTINY *) wbxml_parser_get_xml_public_id(WBXMLParser *parser);

/**
 * @brief Get WBXML Version
 * @param parser The WBXML Parser
 * @return The WBXML Version of current parsing document
 */
WBXML_DECLARE(WB_UTINY) wbxml_parser_get_wbxml_version(WBXMLParser *parser);

/**
 * @brief Return current parsing position in WBXML
 * @param parser The WBXML Parser
 * @return The parsing position in WBXML
 */
WBXML_DECLARE(WB_LONG) wbxml_parser_get_current_byte_index(WBXMLParser *parser);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_PARSER_H */
