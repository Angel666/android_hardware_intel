/*
 * WBXML Lib, the WBXML Library.
 * Copyright (C) 2002-2003  Aymerick J�hanne
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
 * 11/14/2003 Motorola    Add  (END, END) indication for each characteristic
 * 12/04/2003 Motorola    Increase the size for multiple sessions
 */

/**
 * @file wbxml_parser.c
 * @ingroup wbxml_parser
 *
 * @author Aymerick J�hanne <libwbxml@jehanne.org>
 * @date 02/03/12
 *
 * @brief WBXML Parser
 *
 * @todo Handle correctly charset
 *
 * @note WBXML Versions Differences:
 *            - WBXML 1.2: - No differences with WBXML 1.3, except a clarification in BNF for 'LITERAL' handling
 *            - WBXML 1.1: - No Switch Page mecanism
 *                         - An Attribute value can't be "opaque"
 *            - WBXML 1.0: - No 'charset' handling
 *                         - No 'opaque' support
 *                         - A strange 'body' rule description in BNF (just forget it).
 *
 * @todo For Wireless-Village CSP :
 *              - Decpde "Date and Time" from OPAQUE (OMA-WV-CSP_WBXML-V1_1-20021001-A.pdf - 6.6)
 */

#include <stdlib.h>
#include <string.h>

#include "wbxml.h"

#ifdef JUIX_ATTR_VECTORS
extern WB_UTINY *attr_name_global[];
extern WB_UTINY *attr_value_global[];
extern WB_ULONG index_global;
#endif

/* Memory management related defines */
#define WBXML_PARSER_MALLOC_BLOCK 5000
#define WBXML_PARSER_STRING_TABLE_MALLOC_BLOCK 200
#define WBXML_PARSER_ATTR_VALUE_MALLOC_BLOCK 100

/** Supported WBXML version */
#define WBXML_SUPPORTED_VERSION "1.3"

/** Set it to '1' for Best Effort mode */
#define WBXML_PARSER_BEST_EFFORT 1

/** For unknown Tag Name or Attribute Name (in Best Effort Mode) */
#define WBXML_PARSER_UNKNOWN_STRING "unknown"

/** If you want to modify this define, change the 'entcode' variable length in parse_entity() too please */
#define MAX_ENTITY_CODE 999999


/**
 * @brief The WBXML Application Token types
 */
typedef enum WBXMLTokenType_e {
    WBXML_TAG_TOKEN,        /**< Tag token */
    WBXML_ATTR_TOKEN        /**< Attribute token */
} WBXMLTokenType;


/**
 * @brief The WBXML Parser
 * @warning For now 'current_tag' field is only used for WV Content Parsing. And for this use, it works.
 *          But this field is reset after End Tag, and as there is no Linked List mecanism, this is bad for
 *          cascading elements: we don't fill this field with parent Tag when parsing End Tag.
 */
struct WBXMLParser_s {
    void *user_data;			      /**< User Data */
    WBXMLContentHandler *content_hdl; /**< Content Handlers Callbacks */
    WBXMLBuffer *wbxml;               /**< The wbxml we are parsing */
    WBXMLBuffer *strstbl;             /**< String Table specified in WBXML document */
    const WBXMLLangEntry *langTable;  /**< Current document Language Table */
    const WBXMLLangEntry *mainTable;  /**< Main WBXML Languages Table */
    const WBXMLTagEntry *current_tag; /**< Current Tag */

    WB_LONG public_id;          /**< Public ID specified in WBXML document */
    WB_LONG public_id_forced;   /**< Public ID forced by User */
    WB_LONG public_id_index;    /**< If Public ID is a String Table reference, this is the index defined in the strtbl */
    WB_LONG charset;            /**< Charset specified in WBXML document */
    WB_ULONG pos;               /**< Position of parsing curser in wbxml */
    WB_UTINY version;           /**< WBXML Version field specified in WBXML document */
    WB_UTINY tagCodePage;       /**< Current Tag Code Page */
    WB_UTINY attrCodePage;      /**< Current Attribute Code Page */
};



/***************************************************
 *    Private Functions prototypes
 */

/* WBXML Parser functions */
static void wbxml_parser_reinit(WBXMLParser *parser);

/* Check functions */
static WB_BOOL is_token(WBXMLParser *parser, WB_UTINY token);
static WB_BOOL is_literal(WBXMLParser *parser);
static WB_BOOL is_attr_value(WBXMLParser *parser);
static WB_BOOL is_string(WBXMLParser *parser);
static WB_BOOL is_extension(WBXMLParser *parser);
static WB_BOOL check_public_id(WBXMLParser *parser);

/* Parse functions */
static WBXMLError parse_version(WBXMLParser *parser);
static WBXMLError parse_publicid(WBXMLParser *parser);
static WBXMLError parse_charset(WBXMLParser *parser);
static WBXMLError parse_strtbl(WBXMLParser *parser);
static WBXMLError parse_body(WBXMLParser *parser);

static WBXMLError parse_pi(WBXMLParser *parser);
static WBXMLError parse_element(WBXMLParser *parser);
static void free_attrs_table(WBXMLAttribute **attrs);

static WBXMLError parse_switch_page(WBXMLParser *parser, WBXMLTokenType code_space);
static WBXMLError parse_stag(WBXMLParser *parser, WB_UTINY *tag, WBXMLTag **element);
static WBXMLError parse_tag(WBXMLParser *parser, WB_UTINY *tag, WBXMLTag **element);
static WBXMLError parse_attribute(WBXMLParser *parser, WBXMLAttribute **attr);
static WBXMLError parse_content(WBXMLParser *parser, WB_UTINY **content, WB_LONG *len, WB_BOOL *static_content);

static WBXMLError parse_string(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len);
static WBXMLError parse_extension(WBXMLParser *parser, WBXMLTokenType code_space, WB_UTINY **ext, WB_LONG *len);
static WBXMLError parse_entity(WBXMLParser *parser, WB_UTINY **entity, WB_LONG *len);
static WBXMLError parse_opaque(WBXMLParser *parser, WB_UTINY **data, WB_LONG *len);

static WBXMLError parse_literal(WBXMLParser *parser, WB_UTINY *tag, WB_UTINY **result);

static WBXMLError parse_attr_start(WBXMLParser *parser, WBXMLAttributeName **name, WB_UTINY **value);
static WBXMLError parse_attr_value(WBXMLParser *parser, WB_UTINY **value, WB_LONG *len, WB_BOOL *static_value);

static WBXMLError parse_termstr(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len);
static WBXMLError parse_inline(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len);
static WBXMLError parse_tableref(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len);
static WBXMLError parse_entcode(WBXMLParser *parser, WB_ULONG *result);

static WBXMLError get_strtbl_reference(WBXMLParser *parser, WB_ULONG index, WB_UTINY **str, WB_LONG *len);

/* Basic Types Parse functions */
static WBXMLError parse_uint8(WBXMLParser *parser, WB_UTINY *result);
static WBXMLError parse_mb_uint32(WBXMLParser *parser, WB_ULONG *result);

/* Language Specific Decoding Functions */
static WBXMLError decode_datetime(WBXMLBuffer *buff);

static WBXMLError decode_opaque_content(WBXMLParser *parser, WB_UTINY **data, WB_LONG *len, WB_BOOL *static_content);
static WBXMLError decode_wv_integer(WB_UTINY **data, WB_LONG *len);
static WBXMLError decode_wv_datetime(WB_UTINY **data, WB_LONG *len);

/* Macro for error handling */
#define CHECK_ERROR if (ret != WBXML_OK) return ret;


/***************************************************
 *    Public Functions
 */

WBXML_DECLARE(WBXMLParser *) wbxml_parser_create(void)
{
    WBXMLParser *parser = NULL;

    parser = (WBXMLParser *) wbxml_malloc(sizeof(WBXMLParser));
    if (parser == NULL) {
        return NULL;
    }

    parser->wbxml = NULL;
    parser->user_data = NULL;
    parser->content_hdl = NULL;
    parser->strstbl = NULL;
    parser->langTable = NULL;

    /* Default Main WBXML Languages Table */
    parser->mainTable = wbxml_tables_get_main();

    parser->current_tag = NULL;

    parser->public_id = -1;
    parser->public_id_forced = -1;
    parser->public_id_index = -1;
    parser->charset = -1;
    parser->version = -1;

    parser->pos = 0;
    parser->tagCodePage = 0;
    parser->attrCodePage = 0;

    return parser;
}


WBXML_DECLARE(void) wbxml_parser_destroy(WBXMLParser *parser)
{
    if (parser == NULL)
        return;

    wbxml_buffer_destroy(parser->wbxml);
    wbxml_buffer_destroy(parser->strstbl);

    wbxml_free(parser);
}


WBXML_DECLARE(WBXMLError) wbxml_parser_parse(WBXMLParser *parser, WB_UTINY *wbxml, WB_ULONG wbxml_len)
{
    WBXMLError ret = WBXML_OK;

    if (parser == NULL)
        return WBXML_ERROR_NULL_PARSER;

    if ((wbxml == NULL) || (wbxml_len <= 0))
        return WBXML_ERROR_EMPTY_WBXML;

    /* Reinitialize WBXML Parser */
    wbxml_parser_reinit(parser);

    parser->wbxml = wbxml_buffer_create(wbxml, wbxml_len, WBXML_PARSER_MALLOC_BLOCK);
    if (parser->wbxml == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* WBXML Version */
    ret = parse_version(parser);
    CHECK_ERROR

    if (parser->version > 0x03) {
        WBXML_WARNING((WBXML_PARSER, "This library only supports WBXML " WBXML_SUPPORTED_VERSION));
    }

    /* WBXML Public ID */
    ret = parse_publicid(parser);
    CHECK_ERROR

    /* Ignore Document Public ID if user has forced use of another Public ID */
    if (parser->public_id_forced != -1)
        parser->public_id = parser->public_id_forced;

    /* No charset in WBXML 1.0 */
    if (parser->version != 0x00) {
        ret = parse_charset(parser);
        CHECK_ERROR

        /** @todo check charset ! */
    }

    /* WBXML String Table */
    ret = parse_strtbl(parser);
    CHECK_ERROR

    /* Now that we have parsed String Table, we can check Public ID */
    if (!check_public_id(parser)) {
        WBXML_ERROR((WBXML_PARSER, "PublicID not found"));
        return WBXML_ERROR_UNKNOWN_PUBLIC_ID;
    }

    /* Call to WBXMLStartDocumentHandler */
    if ((parser->content_hdl != NULL) && (parser->content_hdl->start_document_clb != NULL))
        parser->content_hdl->start_document_clb(parser->user_data, parser->charset, parser->langTable);

    /* WBXML Body */
    ret = parse_body(parser);
    CHECK_ERROR

    /* Call to WBXMLEndDocumentHandler */
    if ((parser->content_hdl != NULL) && (parser->content_hdl->end_document_clb != NULL))
        parser->content_hdl->end_document_clb(parser->user_data);

    return ret;
}


WBXML_DECLARE(WBXMLError) wbxml_parser_parse_to_tree(WB_UTINY *wbxml, WB_ULONG wbxml_len, WBXMLTree **tree)
{
    WBXMLParser *wbxml_parser = NULL;
    WB_LONG error_index = 0;
    WBXMLTreeClbCtx wbxml_tree_clb_ctx;
    WBXMLError ret = WBXML_OK;
    WBXMLContentHandler wbxml_tree_content_handler =
        {
            wbxml_tree_clb_start_document,
            wbxml_tree_clb_end_document,
            wbxml_tree_clb_start_element,
            wbxml_tree_clb_end_element,
            wbxml_tree_clb_characters,
            wbxml_tree_clb_pi
        };

    /** @todo Test wbxml_parser_parse_to_tree() */

    /* Create WBXML Parser */
    if((wbxml_parser = wbxml_parser_create()) == NULL) {
        WBXML_ERROR((WBXML_PARSER, "Can't create WBXML Parser"));
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Init context */
    wbxml_tree_clb_ctx.error = WBXML_OK;
    wbxml_tree_clb_ctx.current = NULL;
    if ((wbxml_tree_clb_ctx.tree = wbxml_tree_create()) == NULL) {
        wbxml_parser_destroy(wbxml_parser);
        WBXML_ERROR((WBXML_PARSER, "Can't create WBXML Tree"));
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Initialize WBXML Parser */
    wbxml_parser_set_user_data(wbxml_parser, &wbxml_tree_clb_ctx);
    wbxml_parser_set_content_handler(wbxml_parser, &wbxml_tree_content_handler);

    /* Parse WBXML document */
    ret = wbxml_parser_parse(wbxml_parser, wbxml, wbxml_len);
    if ((ret != WBXML_OK) || (wbxml_tree_clb_ctx.error != WBXML_OK))
    {
        error_index = wbxml_parser_get_current_byte_index(wbxml_parser);
        WBXML_ERROR((WBXML_PARSER, "WBXML Parser failed at %ld - token: %x (%s)",
                                   error_index,
                                   wbxml[error_index],
                                   ret != WBXML_OK ? wbxml_errors_string(ret) : wbxml_errors_string(wbxml_tree_clb_ctx.error)));

        *tree = NULL;
        wbxml_tree_destroy(wbxml_tree_clb_ctx.tree);
    }
    else {
        *tree = wbxml_tree_clb_ctx.tree;
    }

    /* Clean-up */
    wbxml_parser_destroy(wbxml_parser);

    if (ret != WBXML_OK)
        return ret;
    else
        return wbxml_tree_clb_ctx.error;
}


WBXML_DECLARE(void) wbxml_parser_set_user_data(WBXMLParser *parser, void *user_data)
{
    if (parser != NULL)
        parser->user_data = user_data;
}


WBXML_DECLARE(void) wbxml_parser_set_content_handler(WBXMLParser *parser, WBXMLContentHandler *content_handler)
{
    if (parser != NULL)
        parser->content_hdl = content_handler;
}


WBXML_DECLARE(void) wbxml_parser_set_main_table(WBXMLParser *parser, const WBXMLLangEntry *main_table)
{
    if (parser != NULL)
        parser->mainTable = main_table;
}


WBXML_DECLARE(WB_BOOL) wbxml_parser_set_wbxml_public_id(WBXMLParser *parser, WB_LONG public_id)
{
    if (parser != NULL) {
        /* Just check if Public ID is >= 0x01 */
        if (public_id >= WBXML_PUBLIC_ID_UNKNOWN) {
            parser->public_id_forced = public_id;
            return TRUE;
        }
        else
            return FALSE;
    }
    else return FALSE;
}


WBXML_DECLARE(WB_ULONG) wbxml_parser_get_wbxml_public_id(WBXMLParser *parser)
{
    if (parser != NULL)
        return (WB_ULONG) parser->public_id;
    else
        return WBXML_PUBLIC_ID_UNKNOWN;
}


WBXML_DECLARE(const WB_UTINY *) wbxml_parser_get_xml_public_id(WBXMLParser *parser)
{
    WB_LONG index = 0;

    if (parser == NULL)
        return NULL;

    while (parser->mainTable[index].publicID != NULL) {
        if (parser->mainTable[index].publicID->wbxmlPublicID == (WB_ULONG) parser->public_id)
            return parser->mainTable[index].publicID->xmlPublicID;

        index++;
    }

    return NULL;
}


WBXML_DECLARE(WB_UTINY) wbxml_parser_get_wbxml_version(WBXMLParser *parser)
{
    if (parser != NULL)
        return parser->version;
    else
        return -1;
}


WBXML_DECLARE(WB_LONG) wbxml_parser_get_current_byte_index(WBXMLParser *parser)
{
    if (parser != NULL)
        return parser->pos - 1;
    else
        return 0;
}


/***************************************************
 *    Private Functions
 */

/**************************
 * WBXML Parser functions
 */

/**
 * @brief Reinitialize a WBXML Parser
 * @param parser The WBXMLParser to reinitialize
 * @note Only reinitialize internal fields of parser, and so keep User Data
 *         and Content Handler pointers.
 */
static void wbxml_parser_reinit(WBXMLParser *parser)
{
    if (parser == NULL)
        return;

    wbxml_buffer_destroy(parser->wbxml);
    parser->wbxml = NULL;

    wbxml_buffer_destroy(parser->strstbl);
    parser->strstbl = NULL;

    parser->langTable = NULL;

    parser->current_tag = NULL;

    parser->public_id = -1;
    parser->public_id_forced = -1;
    parser->public_id_index = -1;
    parser->charset = -1;
    parser->version = -1;

    parser->pos = 0;
    parser->tagCodePage = 0;
    parser->attrCodePage = 0;
}


/******************
 * Check functions
 */

/**
 * @brief Check if current byte a specified WBXML token
 * @param parser The WBXML Parser
 * @param token The WBXML token
 * @return TRUE is current byte is the specified token, FALSE otherwise
 */
static WB_BOOL is_token(WBXMLParser *parser, WB_UTINY token)
{
    WB_UTINY result;

    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &result))
        return FALSE;

    return (result == token);
}


/**
 * @brief Check if current byte is a WBXML literalTag token
 * @param parser The WBXML Parser
 * @return TRUE is current byte is a literalTag token, FALSE otherwise
 */
static WB_BOOL is_literal(WBXMLParser *parser)
{
    WB_UTINY result;

    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &result))
        return FALSE;

    return ((result == WBXML_LITERAL) || (result == WBXML_LITERAL_A) || (result == WBXML_LITERAL_C) || (result == WBXML_LITERAL_AC));
}


/**
 * @brief Check if next token to parse is an Attribute Value
 * @param parser The WBXML Parser
 * @return TRUE if next token to parse is an Attribute Value, FALSE otherwise
 * @note attrValue    = ([switchPage] ATTRVALUE | string | extension | entity | opaque)
 */
static WB_BOOL is_attr_value(WBXMLParser *parser)
{
    WB_UTINY cur_byte, next_byte;

    /* Get current byte */
    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &cur_byte))
        return FALSE;

    /* If current byte is a switch page, check that following token is an Attribute Value token */
    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        if (!wbxml_buffer_get_char(parser->wbxml, parser->pos + 2, &next_byte))
            return FALSE;

        /* Attribute Value is greater than or equal to 128 */
        if ((next_byte & 0x80) == 0x80)
            return TRUE;
    }

    /* Else, check current byte is an Attribute Value, a string, an extension, an entity or an opaque */
    if (((cur_byte & 0x80) == 0x80) ||
        (is_string(parser)) ||
        (is_extension(parser)) ||
        (is_token(parser, WBXML_ENTITY)) ||
        (is_token(parser, WBXML_OPAQUE)))
        return TRUE;

    return FALSE;
}


/**
 * @brief Check if current byte is a string
 * @param parser The WBXML Parser
 * @return TRUE if current byte is a string, FALSE otherwise
 */
static WB_BOOL is_string(WBXMLParser *parser)
{
    return (is_token(parser, WBXML_STR_I) || is_token(parser, WBXML_STR_T));
}


/**
 * @brief Check if current byte is an extension
 * @param parser The WBXML Parser
 * @return TRUE if current byte is an extension, FALSE otherwise
 */
static WB_BOOL is_extension(WBXMLParser *parser)
{
    WB_UTINY cur_byte;

    /* If current byte is a switch page, check the following token */
    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        if (!wbxml_buffer_get_char(parser->wbxml, parser->pos + 2, &cur_byte))
            return FALSE;
    }
    else {
        if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &cur_byte))
            return FALSE;
    }

    return ((cur_byte == WBXML_EXT_I_0) || (cur_byte == WBXML_EXT_I_1) || (cur_byte == WBXML_EXT_I_2) ||
            (cur_byte == WBXML_EXT_T_0) || (cur_byte == WBXML_EXT_T_1) || (cur_byte == WBXML_EXT_T_2) ||
            (cur_byte == WBXML_EXT_0)      || (cur_byte == WBXML_EXT_1)   || (cur_byte == WBXML_EXT_2));
}


/**
 * @brief Check the Public ID
 * @param parser The WBXML Parser
 * @return TRUE if Public ID is found, FALSE otherwise
 */
static WB_BOOL check_public_id(WBXMLParser *parser)
{
    WB_LONG index = 0, len = 0;
    WB_UTINY *public_id = NULL;

    WBXML_DEBUG((WBXML_PARSER, "\t  Checking PublicID"));

    /************************************************
     * Case 1: Public ID referenced in String Table
     */
    if (parser->public_id_index != -1) {
        WBXML_DEBUG((WBXML_PARSER, "\t  PublicID is in String Table (index: 0x%X)", parser->public_id_index));

        if (get_strtbl_reference(parser, parser->public_id_index, &public_id, &len) != WBXML_OK) {
            WBXML_ERROR((WBXML_PARSER, "Bad publicID reference in string table"));
            return FALSE;
        }

        WBXML_DEBUG((WBXML_PARSER, "\t  PublicID : '%s'", public_id));

        /* Search Public ID Table */
        while (parser->mainTable[index].publicID != NULL) {
            if (strcmp((const char *)(parser->mainTable[index].publicID->xmlPublicID), (const char *)public_id) == 0) {
                parser->langTable = &(parser->mainTable[index]);
                parser->public_id = parser->mainTable[index].publicID->wbxmlPublicID;
                return TRUE;
            }

            index++;
        }
    }


    /************************************************
     * Case 1: Public ID was a normal token
     */

    /* Oups, no Public ID parsed at all ? */
    if (parser->public_id == -1)
        return FALSE;

    WBXML_DEBUG((WBXML_PARSER, "\t  PublicID token: 0x%X", parser->public_id));

    /* Search Public ID Table */
    while (parser->mainTable[index].publicID != NULL) {
        if (parser->mainTable[index].publicID->wbxmlPublicID == (WB_ULONG) parser->public_id) {
            parser->langTable = &(parser->mainTable[index]);

            WBXML_DEBUG((WBXML_PARSER, "\t  PublicID : '%s'", parser->mainTable[index].publicID->xmlPublicID));

            return TRUE;
        }

        index++;
    }

    return FALSE;
}



/***************************
 *    WBXML Parse functions
 */

/**
 * @brief Parse WBXML version
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note version = u_int8
 */
static WBXMLError parse_version(WBXMLParser *parser)
{
    WBXMLError ret = parse_uint8(parser, &parser->version);

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsed version: '0x%X'", parser->pos - 1, parser->version));

    return ret;
}


/**
 * @brief Parse WBXML public id
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note publicid = mb_u_int32 | ( zero index )
 * @note index = mb_u_int32
 */
static WBXMLError parse_publicid(WBXMLParser *parser)
{
    WB_UTINY public_id;

    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &public_id))
        return WBXML_ERROR_END_OF_BUFFER;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsed publicid: '0x%X'", parser->pos, public_id));

    if (public_id == 0x00) {
        parser->pos++;

        /* Get index (we will retreive the Public ID latter) */
        return parse_mb_uint32(parser, (WB_ULONG *)&parser->public_id_index);
    }
    else {
        /* Get Public ID */
        return parse_mb_uint32(parser, (WB_ULONG *)&parser->public_id);
    }
}


/**
 * @brief Parse WBXML charset
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note charset = mb_u_int32
 * @note "The binary XML format contains a representation of the XML document character encoding.
 *          This is the WBXML equivalent of the XML document format encoding attribute,
 *          which is specified in the ?xml processing instruction. The character set is encoded as
 *          a multi-byte positive integer value, representing the IANA-assigned MIB number for
 *          a character set. A value of zero indicates an unknown document encoding. In the case of
 *          an unknown encoding, transport meta-information should be used to determine the character
 *          encoding. If transport meta-information is unavailable, the default encoding of UTF-8
 *          should be assumed."
 */
static WBXMLError parse_charset(WBXMLParser *parser)
{
    WB_ULONG startpos = parser->pos;
    WBXMLError ret = parse_mb_uint32(parser, (WB_ULONG *)&parser->charset);

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsed charset: '0x%X'", startpos, parser->charset));

    return ret;
}


/**
 * @brief Parse WBXML string table
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note strtbl = length *byte
 * @note length = mb_u_int32
 */
static WBXMLError parse_strtbl(WBXMLParser *parser)
{
    WB_UTINY *data = NULL;
    WB_ULONG strtbl_len = 0;
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing strtbl", parser->pos));

    /* Get String Table Length */
    ret = parse_mb_uint32(parser, &strtbl_len);
    if (ret != WBXML_OK)
        return WBXML_ERROR_END_OF_BUFFER;

    if (strtbl_len > 0) {
        /* Check this string table length */
        if (parser->pos + strtbl_len > wbxml_buffer_len(parser->wbxml))
            return WBXML_ERROR_STRTBL_LENGTH;

        /* Get String Table */
        data = wbxml_buffer_get_cstr(parser->wbxml);
        parser->strstbl = wbxml_buffer_create(data + parser->pos, strtbl_len, WBXML_PARSER_STRING_TABLE_MALLOC_BLOCK);
        if (parser->strstbl == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

        parser->pos = parser->pos + strtbl_len;
    }

    return WBXML_OK;
}


/**
 * @brief Parse WBXML body
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note body = *pi element *pi
 */
static WBXMLError parse_body(WBXMLParser *parser)
{
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing body", parser->pos));

    while (is_token(parser, WBXML_PI)) {
        if ((ret = parse_pi(parser)) != WBXML_OK)
            return ret;
    }

    if ((ret = parse_element(parser)) != WBXML_OK)
        return ret;

    while (is_token(parser, WBXML_PI)) {
        if ((ret = parse_pi(parser)) != WBXML_OK)
            return ret;
    }

    return WBXML_OK;
}


/**
 * @brief Parse WBXML pi
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note pi = PI attrStart *attrValue END
 */
static WBXMLError parse_pi(WBXMLParser *parser)
{
    WBXMLAttributeName *name = NULL;
    WB_UTINY *value = NULL;
    WBXMLBuffer *attr_value = NULL;
    WB_LONG len = 0;
    WBXMLError ret = WBXML_OK;
    WB_BOOL static_value;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing PI", parser->pos));

    /* Skip PI */
    parser->pos++;

    /* Parse attrStart */
    ret = parse_attr_start(parser, &name, &value);
    if (ret != WBXML_OK)
        return ret;

    if (value != NULL)
        attr_value = wbxml_buffer_create(value, WBXML_STRLEN(value), WBXML_PARSER_ATTR_VALUE_MALLOC_BLOCK);
    else
        attr_value = wbxml_buffer_create(NULL, 0, WBXML_PARSER_ATTR_VALUE_MALLOC_BLOCK);

    if (attr_value == NULL) {
        wbxml_attribute_name_destroy(name);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    while (!is_token(parser, WBXML_END))
    {
        static_value = TRUE;

        /* Parse attrValue */
        ret = parse_attr_value(parser, &value, &len, &static_value);
        if (ret != WBXML_OK) {
            wbxml_attribute_name_destroy(name);
            wbxml_buffer_destroy(attr_value);
            return ret;
        }

        if (!wbxml_buffer_append_data(attr_value, value, len)) {
            wbxml_attribute_name_destroy(name);
            wbxml_buffer_destroy(attr_value);
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!static_value) {
            wbxml_free(value);
            value = NULL;
        }
    }

    /* Skip END */
    parser->pos++;

    /* Append NULL char to attr value */
    if (wbxml_buffer_len(attr_value) > 0)
        wbxml_buffer_append_char(attr_value, '\0');

    /* Call to WBXMLProcessingInstructionHandler */
    if ((parser->content_hdl != NULL) && (parser->content_hdl->pi_clb != NULL))
        parser->content_hdl->pi_clb(parser->user_data, wbxml_attribute_name_get_xml_name(name), wbxml_buffer_get_cstr(attr_value));

    wbxml_attribute_name_destroy(name);
    wbxml_buffer_destroy(attr_value);

    return WBXML_OK;
}


/**
 * @brief Parse WBXML element
 * @param parser The WBXML Parser
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note element = ([switchPage] stag) [ 1*attribute END ] [ *content END ]
 */
static WBXMLError parse_element(WBXMLParser *parser)
{
	WB_UTINY *str = NULL;

    WBXMLTag *element = NULL;
    WBXMLAttribute *attr = NULL;
    WBXMLAttribute **attrs = NULL;
    WB_UTINY *content = NULL;

    WB_ULONG attrs_nb = 0;
    WB_LONG len;
    WBXMLError ret = WBXML_OK;
    WB_UTINY tag;
    WB_BOOL static_content, is_empty;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing element", parser->pos));

    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        ret = parse_switch_page(parser, WBXML_TAG_TOKEN);
        if (ret != WBXML_OK)
            return ret;
    }

    /* Parse Tag */
    ret = parse_stag(parser, &tag, &element);
    if (ret != WBXML_OK)
        return ret;

    /* Set Current Tag */
    if (element->type == WBXML_VALUE_TOKEN)
        parser->current_tag = element->u.token;

	if (parser->public_id == 0x05)  { // [JUIX] for SI only, overwritten by the last tag name.
	    str = (WB_UTINY *)parser->current_tag->xmlName;
#ifdef JUIX_ATTR_VECTORS
	    attr_name_global[0] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
            if (attr_name_global[0] != NULL){
	        memcpy(attr_name_global[0], str, strlen((char *)str)+1);
            }
#endif
	}

    /* Parse Attributes */
    if (tag & WBXML_TOKEN_WITH_ATTRS) {
        /* There must be at least one attribute */
        do {
            /* Parse attribute */
            ret = parse_attribute(parser, &attr);
            if (ret != WBXML_OK) {
                wbxml_tag_destroy(element);
                free_attrs_table(attrs);
                return ret;
            }

            /* Append this attribute in WBXMLAttribute **attrs table */
            attrs_nb++;

            attrs = (WBXMLAttribute **)wbxml_realloc(attrs, (attrs_nb + 1) * sizeof(*attrs));
            if (attrs == NULL) {
                /* Clean-up */
                wbxml_tag_destroy(element);
                wbxml_attribute_destroy(attr);
                free_attrs_table(attrs);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            attrs[(attrs_nb - 1)] = attr;
            attrs[attrs_nb] = NULL;
        } while (!is_token(parser, WBXML_END));

        /* Skip END */
        parser->pos++;
    }

    /* Is it an empty element ? */
    is_empty = !(tag & WBXML_TOKEN_WITH_CONTENT);

    /* Call WBXMLStartElementHandler */
    if ((parser->content_hdl != NULL) && (parser->content_hdl->start_element_clb != NULL))
        parser->content_hdl->start_element_clb(parser->user_data, element, attrs, is_empty);

    /* Free Attributes */
    free_attrs_table(attrs);

    /* Parse Content */
    if (!is_empty) {
        /* There can be NO content */
        while (!is_token(parser, WBXML_END))
        {
            static_content = TRUE;

            ret = parse_content(parser, &content, &len, &static_content);
            if (ret != WBXML_OK) {
                wbxml_tag_destroy(element);
                return ret;
            }

            /* Call WBXMLCharactersHandler if content is not NULL */
            if ((content != NULL) && (parser->content_hdl != NULL) && (parser->content_hdl->characters_clb != NULL))
                parser->content_hdl->characters_clb(parser->user_data, content, 0, len);

            /* Free content if it is not static content */
            if (!static_content)
                wbxml_free(content);

            content = NULL;
        }

        /* Skip END */
        parser->pos++;

#ifdef JUIX_ATTR_VECTORS
        /* to insert (END, END) pair for current characteristic group */
	if (parser->public_id == 0x0B)  // [JUIX] for Provisioning only,
	{
            if ( index_global >= DOC_ENTRY_SIZE) {
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }
	    attr_name_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)"END")+1);
            if (attr_name_global[index_global] != NULL) {
                memcpy(&attr_name_global[index_global][0], "END", strlen((char *)"END")+1);
            }

	    attr_value_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)"END")+1);
            if (attr_value_global[index_global] != NULL) {
	        memcpy(&attr_value_global[index_global][0], "END", strlen((char *)"END")+1);
            }
	    index_global++;
        }
#endif
    }

    /* Call WBXMLEndElementHandler */
    if ((parser->content_hdl != NULL) && (parser->content_hdl->end_element_clb != NULL))
        parser->content_hdl->end_element_clb(parser->user_data, element, is_empty);

    /* Free Tag */
    wbxml_tag_destroy(element);

    /* Reset Current Tag */
    parser->current_tag = NULL;

    return WBXML_OK;
}


/**
 * @brief Free a (WBXMLAttribute *) table
 * @param attrs The table to ree
 */
static void free_attrs_table(WBXMLAttribute **attrs)
{
    WB_LONG i = 0;

    if (attrs != NULL) {
        while (attrs[i] != NULL) {
            /* Free attribute */
            wbxml_attribute_destroy(attrs[i++]);
        }
        wbxml_free(attrs);
    }
}


/**
 * @brief Parse WBXML switchPage
 * @param parser The WBXML Parser
 * @param code_space The token code space
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note switchPage = SWITCH_PAGE pageindex
 */
static WBXMLError parse_switch_page(WBXMLParser *parser, WBXMLTokenType code_space)
{
    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing switchPage", parser->pos));

    if (parser->version < 0x02) {
        WBXML_WARNING((WBXML_PARSER, "No Switch Page mecanism possible in WBXML < 1.2"));
    }

    /* Skip SWITCH_PAGE token */
    parser->pos++;

    /* Change Code Page in correct Code Space */
    if (code_space == WBXML_TAG_TOKEN)
        return parse_uint8(parser, &parser->tagCodePage);
    else
        if (code_space == WBXML_ATTR_TOKEN)
            return parse_uint8(parser, &parser->attrCodePage);
        else
            return WBXML_ERROR_INTERNAL;
}


/**
 * @brief Parse WBXML stag
 * @param parser The WBXML Parser
 * @param tag The parsed tag token
 * @param element The parsed element corresponding to token
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note stag = TAG | (literalTag index)
 */
static WBXMLError parse_stag(WBXMLParser *parser, WB_UTINY *tag, WBXMLTag **element)
{
    WB_UTINY *name = NULL;
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing stag", parser->pos));

    if (is_literal(parser)) {
        if ((ret = parse_literal(parser, tag, &name)) != WBXML_OK)
            return ret;

        /* Create Element Tag */
        if ((*element = wbxml_tag_create_literal(name)) == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

        return WBXML_OK;
    }

    return parse_tag(parser, tag, element);
}


/**
 * @brief Parse WBXML Application Token (tag)
 * @param parser The WBXML Parser
 * @param tag The parsed token tag
 * @param element The parsed element (the text element corresponding to token)
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_tag(WBXMLParser *parser, WB_UTINY *tag, WBXMLTag **element)
{
    WB_ULONG index = 0;
    WB_UTINY token;
    WBXMLError ret = WBXML_OK;

    /* Parse UINT8 */
    ret = parse_uint8(parser, tag);
    if (ret != WBXML_OK)
        return ret;

    /* Remove ATTR and CONTENT bits */
    token = *tag & WBXML_TOKEN_MASK;

    /* Search tag in Tags Table */
    if (parser->langTable == NULL)
        return WBXML_ERROR_LANG_TABLE_UNDEFINED;

    if (parser->langTable->tagTable == NULL)
        return WBXML_ERROR_TAG_TABLE_UNDEFINED;


    while ((parser->langTable->tagTable[index].xmlName != NULL) &&
           ((parser->langTable->tagTable[index].wbxmlToken != token) ||
            (parser->langTable->tagTable[index].wbxmlCodePage != parser->tagCodePage))) {
        index++;
    }


	if (parser->langTable->tagTable[index].xmlName == NULL) {
#if WBXML_PARSER_BEST_EFFORT
        /* Create "unknown" Tag Element */
		if ((*element = wbxml_tag_create_literal((WB_UTINY *)WBXML_PARSER_UNKNOWN_STRING)) == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

		return WBXML_OK;
#else
		return WBXML_ERROR_UNKNOWN_TAG;
#endif /* WBXML_PARSER_BEST_EFFORT */
	}

	if ((*element = wbxml_tag_create(WBXML_VALUE_TOKEN)) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    (*element)->u.token = &(parser->langTable->tagTable[index]);

    WBXML_DEBUG((WBXML_PARSER, "(%d) Token: 0x%X", parser->pos - 1, token));

    return WBXML_OK;
}


/**
 * @brief Parse WBXML attribute
 * @param parser The WBXML Parser
 * @param attr The resulting attribute parsed
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note attribute = attrStart *attrValue
 * @warning The attr_value parameter MUST be freed by caller
 */
static WBXMLError parse_attribute(WBXMLParser *parser, WBXMLAttribute **attr)
{
    WBXMLAttributeName *attr_name = NULL;
    WBXMLBuffer *attr_value = NULL;
    WB_UTINY *value = NULL;
    WB_LONG len;
    WBXMLError ret = WBXML_OK;
    WB_BOOL static_value;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing attribute", parser->pos));

    /* Parse attrStart */
    ret = parse_attr_start(parser, &attr_name, &value);
    if (ret != WBXML_OK)
        return ret;

    if (value != NULL)
        attr_value = wbxml_buffer_create(value, WBXML_STRLEN(value), WBXML_PARSER_ATTR_VALUE_MALLOC_BLOCK);
    else
        attr_value = wbxml_buffer_create(NULL, 0, WBXML_PARSER_ATTR_VALUE_MALLOC_BLOCK);

    if (attr_value == NULL) {
        wbxml_attribute_name_destroy(attr_name);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Construct Attribute Value */
    while (is_attr_value(parser))
    {
        static_value = TRUE;

        /* Parse attrValue */
        ret = parse_attr_value(parser, &value, &len, &static_value);
        if (ret != WBXML_OK) {
            wbxml_attribute_name_destroy(attr_name);
            wbxml_buffer_destroy(attr_value);
            return ret;
        }

        if (len > 0) {
            if (!wbxml_buffer_append_data(attr_value, value, len)) {
                wbxml_attribute_name_destroy(attr_name);
                wbxml_buffer_destroy(attr_value);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if (!static_value) {
            wbxml_free(value);
            value = NULL;
        }
    }

    if ((wbxml_buffer_len(attr_value) > 0) && (attr_name->type == WBXML_VALUE_TOKEN))
    {
        /* Handle Language Specific Attribute Values */
        switch (parser->langTable->publicID->wbxmlPublicID) {
        case WBXML_PUBLIC_ID_SI10:
            /* SI 1.0: Decode date for 'created' and 'si-expires' attributes */
            if ((attr_name->u.token->wbxmlCodePage == 0x00) &&
                ((attr_name->u.token->wbxmlToken == 0x0a) || (attr_name->u.token->wbxmlToken == 0x10)))
            {
                if ((ret = decode_datetime(attr_value)) != WBXML_OK) {
                    wbxml_attribute_name_destroy(attr_name);
                    wbxml_buffer_destroy(attr_value);
                    return ret;
                }
            }
            break;

        case WBXML_PUBLIC_ID_EMN10:
            /* EMN 1.0: Decode date for 'timestamp' attribute */
            if ((attr_name->u.token->wbxmlCodePage == 0x00) && (attr_name->u.token->wbxmlToken == 0x05))
            {
                if ((ret = decode_datetime(attr_value)) != WBXML_OK) {
                    wbxml_attribute_name_destroy(attr_name);
                    wbxml_buffer_destroy(attr_value);
                    return ret;
                }
            }
            break;

        default:
            break;
        }
    }

    /* Append NULL char to attr value */
    if (wbxml_buffer_len(attr_value) > 0)
        wbxml_buffer_append_char(attr_value, '\0');

    if ((*attr = wbxml_attribute_create()) == NULL) {
        wbxml_attribute_name_destroy(attr_name);
        wbxml_buffer_destroy(attr_value);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    (*attr)->name = attr_name;
    (*attr)->value = attr_value;

    return WBXML_OK;
}


/**
 * @brief Parse WBXML content
 * @param parser The WBXML Parser
 * @param content Resulting parsed content, if content is not an Element
 * @param len Resulting parsed content length
 * @param static_content This boolen is set to TRUE if the resulting content is static
 *          and doesn't need to be freed by caller
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note content = element | string | extension | entity | pi | opaque
 */
static WBXMLError parse_content(WBXMLParser *parser, WB_UTINY **content, WB_LONG *len, WB_BOOL *static_content)
{
    WB_UTINY cur_byte;
    WBXMLError ret = WBXML_OK;

    /* Debug */
    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &cur_byte))
        return WBXML_ERROR_END_OF_BUFFER;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing content: '0x%X'", parser->pos, cur_byte));

    *static_content = FALSE;

    /* extension */
    if (is_extension(parser))
        return parse_extension(parser, WBXML_TAG_TOKEN, content, len);

    /* entity */
    if (is_token(parser, WBXML_ENTITY))
        return parse_entity(parser, content, len);

    *static_content = TRUE;

    /* string */
    if (is_string(parser))
        return parse_string(parser, content, len);

    /* opaque */
    if (is_token(parser, WBXML_OPAQUE)) {
        if ((ret = parse_opaque(parser, content, len)) != WBXML_OK)
            return ret;

        return decode_opaque_content(parser, content, len, static_content);
    }

    /* pi */
    if (is_token(parser, WBXML_PI))
        return parse_pi(parser);

    /** @note We have recurrency here ! */
    return parse_element(parser);
}


/**
 * @brief Parse WBXML string
 * @param parser [in] The WBXML Parser
 * @param str [out] The resulting parsed string
 * @param len [out] The resulting parsed string length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note string = inline | tableref
 */
static WBXMLError parse_string(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len)
{
    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing string", parser->pos));

    if (is_token(parser, WBXML_STR_I))
        return parse_inline(parser, str, len);

    if (is_token(parser, WBXML_STR_T))
        return parse_tableref(parser, str, len);

    return WBXML_ERROR_STRING_EXPECTED;
}


/**
 * @brief Parse WBXML extension
 * @param parser The WBXML Parser
 * @param code_space The token code space
 * @param ext Resulting parsed extension
 * @param len Resulting parsed extension length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note extension = [switchPage] (( EXT_I termstr ) | ( EXT_T index ) | EXT)
 * @note 5.8.4.2 - The effect of a switchPage preceding an extension will depend upon where the extension appears.
 *                   If switchPage appears in content, it will change the tag code page. Is switchPage appears in
 *                   an attribute list, it will change the attribute code page.
 * @note Extensions tokens are explained in WML Specifications (WAP-191-WML-20000219-a.pdf - 14.1.1 & 14.3)
 * @warning The resulting ext paramater MUST be freed by caller !
 */
static WBXMLError parse_extension(WBXMLParser *parser, WBXMLTokenType code_space, WB_UTINY **ext, WB_LONG *len)
{
    WB_UTINY var_begin[3] = "$(";
    WB_UTINY var_end[2] = ")";
    WB_UTINY escape[8] = ":escape";
    WB_UTINY unesc[7] = ":unesc";
    WB_UTINY noesc[7] = ":noesc";
    WB_UTINY *var = NULL;

    WB_LONG var_len = 0;
    WB_ULONG index, ext_value;
    WBXMLError ret = WBXML_OK;
    WB_UTINY token, tab_index;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing extension", parser->pos));

    /* Parse switchPage */
    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        ret = parse_switch_page(parser, code_space);
        if (ret != WBXML_OK)
            return ret;
    }

    /* Get Extension Token */
    ret = parse_uint8(parser, &token);
    if (ret != WBXML_OK)
        return ret;

    *len = 0;

    switch(parser->public_id) {
    case WBXML_PUBLIC_ID_WML10:
    case WBXML_PUBLIC_ID_WML11:
    case WBXML_PUBLIC_ID_WML12:
    case WBXML_PUBLIC_ID_WML13:
    case WBXML_PUBLIC_ID_WTAWML12:
        /*****************************
         * WML Variable Substitution
         */

        switch(token) {
        case WBXML_EXT_0:
        case WBXML_EXT_1:
        case WBXML_EXT_2:
            WBXML_WARNING((WBXML_PARSER, "This extension token is reserved for futur use (ignoring)"));
            return WBXML_OK;
        case WBXML_EXT_I_0:
        case WBXML_EXT_I_1:
        case WBXML_EXT_I_2:
            /* Inline variable */
            if ((ret = parse_termstr(parser, &var, &var_len)) != WBXML_OK) {
                WBXML_ERROR((WBXML_PARSER, "Bad Inline Extension"));
                return ret;
            }
            break;
        case WBXML_EXT_T_0:
        case WBXML_EXT_T_1:
        case WBXML_EXT_T_2:
            /* Index in String Table */
            if ((ret = parse_mb_uint32(parser, &index)) != WBXML_OK)
                return ret;

            if ((ret = get_strtbl_reference(parser, index, &var, &var_len)) != WBXML_OK) {
                WBXML_ERROR((WBXML_PARSER, "Bad Extension reference in string table"));
                return ret;
            }
            break;
        default:
            return WBXML_ERROR_UNKNOWN_EXTENSION_TOKEN;
        }

        /* Build Variable */
        *ext = (WB_UTINY*) wbxml_malloc(strlen((const char *)var_begin) + var_len + WBXML_STRLEN(escape) + WBXML_STRLEN(var_end) + 1);
        if (*ext == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

        /* Generate "$(" */
        memcpy(*ext + *len, var_begin, WBXML_STRLEN(var_begin));
        *len += WBXML_STRLEN(var_begin);

        /* Generate 'variable' */
        memcpy(*ext + *len, var, var_len);
        *len += WBXML_STRLEN(var);

        switch(token) {
        case WBXML_EXT_I_0:
        case WBXML_EXT_T_0:
            /* Generate ":escape" */
            memcpy(*ext + *len, escape, WBXML_STRLEN(escape));
            *len += WBXML_STRLEN(escape);
            break;
        case WBXML_EXT_I_1:
        case WBXML_EXT_T_1:
            /* Generate ":unesc" */
            memcpy(*ext + *len, unesc, WBXML_STRLEN(unesc));
            *len += WBXML_STRLEN(unesc);
            break;
        case WBXML_EXT_I_2:
        case WBXML_EXT_T_2:
            /* Generate ":noesc" */
            memcpy(*ext + *len, noesc, WBXML_STRLEN(noesc));
            *len += WBXML_STRLEN(noesc);
            break;
        }

        /* Generate ")" */
        memcpy(*ext + *len, var_end, WBXML_STRLEN(var_end));
        *len += WBXML_STRLEN(var_end);
        break;

    case WBXML_PUBLIC_ID_WV_CSP11:
        /** @todo There is no WBXML Public ID for Wireless-Village for now... if one day it happens, remove this ! */
        if ((WBXML_STRCMP(parser->langTable->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP11) == 0) ||
            (WBXML_STRCMP(parser->langTable->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP12) == 0))
        {
            if (token != WBXML_EXT_T_0) {
                WBXML_ERROR((WBXML_PARSER, "Only EXT_T_0 extensions authorized with Wireless Village CSP 1.1"));
                return WBXML_OK;
            }

            /* Get Extension Value Token */
            if ((ret = parse_mb_uint32(parser, &ext_value)) != WBXML_OK)
                return ret;

            /* Search Token in Extension Value Table */
            if (parser->langTable == NULL)
                return WBXML_ERROR_LANG_TABLE_UNDEFINED;

            if (parser->langTable->extValueTable == NULL)
                return WBXML_ERROR_EXT_VALUE_TABLE_UNDEFINED;

            tab_index = 0;

            while ((parser->langTable->extValueTable[tab_index].xmlName != NULL) &&
                   (parser->langTable->extValueTable[tab_index].wbxmlToken != ext_value)) {
                tab_index++;
            }

	        if (parser->langTable->extValueTable[tab_index].xmlName == NULL) {
#if WBXML_PARSER_BEST_EFFORT
		        *ext = (WB_UTINY *)WBXML_STRDUP(WBXML_PARSER_UNKNOWN_STRING);
                *len = WBXML_STRLEN(WBXML_PARSER_UNKNOWN_STRING);
		        return WBXML_OK;
#else
		        return WBXML_ERROR_UNKNOWN_EXTENSION_VALUE;
#endif /* WBXML_PARSER_BEST_EFFORT */
	        }

            *ext = (WB_UTINY *)WBXML_STRDUP(parser->langTable->extValueTable[tab_index].xmlName);
            *len = WBXML_STRLEN(parser->langTable->extValueTable[tab_index].xmlName);
        }
        else {
            WBXML_ERROR((WBXML_PARSER, "Extension tokens not allowed with this Document !"));
        }
        break;

    default:
        WBXML_ERROR((WBXML_PARSER, "Extension tokens not allowed with this Document !"));
        return WBXML_ERROR_UNKNOWN_EXTENSION_VALUE;
    }

    return WBXML_OK;
}


/**
 * @brief Parse WBXML entity
 * @param parser The WBXML Parser
 * @param entity The resulting parsed entity (MUST BE FREED BY CALLER !)
 * @param len The resulting parsed entity length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note entity = ENTITY entcode
 * @note http://www.w3.org/TR/wbxml/ :
 *         "The character entity token (ENTITY) encodes a numeric character entity. This has the same semantics
 *          as an XML numeric character entity (eg, &#32;). The mb_u_int32 refers to a character in the UCS-4
 *          character encoding. All entities in the source XML document must be represented using either a string
 *          token (eg, STR_I or the ENTITY token."
 * @warning The resulting entity paramater MUST be freed by caller !
 */
static WBXMLError parse_entity(WBXMLParser *parser, WB_UTINY **entity, WB_LONG *len)
{
    WB_ULONG code;
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing entity", parser->pos));

    /* Skip ENTITY */
    parser->pos++;

    if ((ret = parse_entcode(parser, &code)) != WBXML_OK)
        return ret;

    /* Build Entity */
    if (code > MAX_ENTITY_CODE)
        return WBXML_ERROR_ENTITY_CODE_OVERFLOW;

    /* WARNING: If you change this variable length (10 chars), change too 'MAX_ENTITY_CODE'
     *            defined in this file !
     */
    *entity = (WB_UTINY*) wbxml_malloc(10 * sizeof(WB_UTINY));
    if ((*entity) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    *len = sprintf((char *)(*entity), "&#%u;", code);

    return WBXML_OK;
}


/**
 * @brief Parse WBXML opaque
 * @param parser The WBXML Parser
 * @param data Resulting opaque data parsed
 * @param len Length of opaque data parsed
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note opaque = OPAQUE length *byte
 * @note length = mb_u_int32
 */
static WBXMLError parse_opaque(WBXMLParser *parser, WB_UTINY **data, WB_LONG *len)
{
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing opaque", parser->pos));

    if (parser->version < 0x01) {
        WBXML_WARNING((WBXML_PARSER, "No 'opaque' support in WBXML < 1.1"));
    }

    /* Skip OPAQUE */
    parser->pos++;

    ret = parse_mb_uint32(parser, (WB_ULONG *)len);
    if (ret != WBXML_OK)
        return ret;

    *data = wbxml_buffer_get_cstr(parser->wbxml) + parser->pos;

    /* Check that length specified in OPAQUE doesn't overflow wbxml length */
    if (parser->pos + (WB_ULONG) *len > wbxml_buffer_len(parser->wbxml))
        return WBXML_ERROR_BAD_OPAQUE_LENGTH;

    parser->pos+= *len;

    return ret;
}


/**
 * @brief Parse WBXML literalTag
 * @param parser The WBXML Parser
 * @param mask Resulting tag mask (WBXML_TOKEN_MASK | WBXML_TOKEN_WITH_CONTENT | WBXML_TOKEN_WITH_ATTRS | (WBXML_TOKEN_WITH_CONTENT || WBXML_TOKEN_WITH_ATTRS))
 * @param result The resulting parsed literal
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note    result = ( literalTag index )
 *            literalTag = LITERAL | LITERAL_A | LITERAL_C | LITERAL_AC
 */
static WBXMLError parse_literal(WBXMLParser *parser, WB_UTINY *mask, WB_UTINY **result)
{
    WBXMLError ret = WBXML_OK;
    WB_UTINY token;
    WB_ULONG index;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing literalTag", parser->pos));

    /* Parse literalTag */
    ret = parse_uint8(parser, &token);
    if (ret != WBXML_OK)
        return ret;

    /* Parse index */
    ret = parse_mb_uint32(parser, &index);
    if (ret != WBXML_OK)
        return ret;

    /* Check that index specified is 'normal' */
    if (index > wbxml_buffer_len(parser->strstbl))
        return WBXML_ERROR_BAD_LITERAL_INDEX;

    /** @bug "String representation is dependent on the character document encoding and should not
     *        be presumed to include NULL termination" */
    *result = wbxml_buffer_get_cstr(parser->strstbl) + index;

    /* Check that element length is 'normal' */
    if ((index + WBXML_STRLEN(*result)) > wbxml_buffer_len(parser->strstbl))
        return WBXML_ERROR_LITERAL_NOT_NULL_TERMINATED_IN_STRING_TABLE;

    switch(token) {
    case WBXML_LITERAL:
        *mask = WBXML_TOKEN_MASK;
        break;
    case WBXML_LITERAL_C:
        *mask = WBXML_TOKEN_WITH_CONTENT;
        break;
    case WBXML_LITERAL_A:
        *mask = WBXML_TOKEN_WITH_ATTRS;
        break;
    case WBXML_LITERAL_AC:
        *mask = (WBXML_TOKEN_WITH_CONTENT | WBXML_TOKEN_WITH_ATTRS);
        break;
    default:
        return WBXML_ERROR_INTERNAL;
    }

    return WBXML_OK;
}


/**
 * @brief Parse WBXML attrStart
 * @param parser The WBXML Parser
 * @param name The Attribute Name parsed
 * @param value The Attribute Value associated, if any
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note attrStart = ([switchPage] ATTRSTART) | ( LITERAL index )
 */
static WBXMLError parse_attr_start(WBXMLParser *parser, WBXMLAttributeName **name, WB_UTINY **value)
{
    WB_UTINY *literal_str = NULL;
    WB_UTINY literal;
    WB_UTINY tag;
    WBXMLError ret = WBXML_OK;
    WB_ULONG index = 0;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing attrStart", parser->pos));


    /**************************
     * Case: ( LITERAL index )
     */

    if (is_token(parser, WBXML_LITERAL)) {
        ret = parse_literal(parser, &literal, &literal_str);
        if (ret != WBXML_OK)
            return ret;

        if ((*name = wbxml_attribute_name_create_literal(literal_str)) == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

        /** @todo Return Warning if 'literal' is different from 'WBML_TOKEN_MASK' (because it MUST be a 'LITERAL' token, not
         *          LITERAL_A, nor LITERAL_C, nor LITERAL_AC */

        return WBXML_OK;
    }


    /***********************************
     * Case: ( [switchPage] ATTRSTART )
     */

    /* Parse switchPage */
    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        ret = parse_switch_page(parser, WBXML_ATTR_TOKEN);
        if (ret != WBXML_OK)
            return ret;
    }

    /* Parse UINT8 */
    ret = parse_uint8(parser, &tag);
    if (ret != WBXML_OK)
        return ret;

    WBXML_DEBUG((WBXML_PARSER, "\tToken: 0x%X", tag));

    /* Search tag in Tags Table */
    if (parser->langTable == NULL)
        return WBXML_ERROR_LANG_TABLE_UNDEFINED;

    if (parser->langTable->attrTable == NULL)
        return WBXML_ERROR_ATTR_TABLE_UNDEFINED;

    while ((parser->langTable->attrTable[index].xmlName != NULL) &&
           ((parser->langTable->attrTable[index].wbxmlToken != tag) ||
            (parser->langTable->attrTable[index].wbxmlCodePage != parser->attrCodePage))) {
        index++;
    }

	if (parser->langTable->attrTable[index].xmlName == NULL) {
#if WBXML_PARSER_BEST_EFFORT
        /* Create "unknown" Attribute Name */
		if ((*name = wbxml_attribute_name_create_literal((WB_UTINY *)WBXML_PARSER_UNKNOWN_STRING)) == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

		return WBXML_OK;
#else
		return WBXML_ERROR_UNKNOWN_ATTR;
#endif /* WBXML_PARSER_BEST_EFFORT */
	}

    /* Create Token Attribute Name */
    if ((*name = wbxml_attribute_name_create(WBXML_VALUE_TOKEN)) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    (*name)->u.token = &(parser->langTable->attrTable[index]);

    /* Get Attribute start value (if any) */
    if (parser->langTable->attrTable[index].xmlValue != NULL)
        *value = parser->langTable->attrTable[index].xmlValue;

    return WBXML_OK;
}


/**
 * @brief Parse WBXML attrValue
 * @param parser [in] The WBXML Parser
 * @param value [out] The resulting Value parsed
 * @param len [out] Resulting Value length
 * @param static_value [out] Is this a static string ?
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note attrValue = ([switchPage] ATTRVALUE) | string | extension | entity | opaque
 */
static WBXMLError parse_attr_value(WBXMLParser *parser, WB_UTINY **value, WB_LONG *len, WB_BOOL *static_value)
{
    WB_ULONG index = 0;
    WB_UTINY tag;
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing attrValue", parser->pos));

    *static_value = FALSE;

    /* Parse extension */
    if (is_extension(parser))
        return parse_extension(parser, WBXML_ATTR_TOKEN, value, len);

    /* Parse entity */
    if (is_token(parser, WBXML_ENTITY))
        return parse_entity(parser, value, len);

    *static_value = TRUE;

    /* Parse string */
    if (is_string(parser))
        return parse_string(parser, value, len);

    /* Parse opaque */
    if (is_token(parser, WBXML_OPAQUE)) {
        if (parser->version < 0x02) {
            WBXML_ERROR((WBXML_PARSER, "An Attribute value can't be 'opaque' in WBXML version < 1.2"));
        }

        return parse_opaque(parser, value, len);
    }


    /*****************************
     *    ([switchPage] ATTRVALUE)
     */

    /* Parse switchPage */
    if (is_token(parser, WBXML_SWITCH_PAGE)) {
        ret = parse_switch_page(parser, WBXML_ATTR_TOKEN);
        if (ret != WBXML_OK)
            return ret;
    }

    /* Parse UINT8 */
    ret = parse_uint8(parser, &tag);
    if (ret != WBXML_OK)
        return ret;

    /* Search tag in Tags Table */
    if (parser->langTable == NULL)
        return WBXML_ERROR_LANG_TABLE_UNDEFINED;

    if (parser->langTable->attrValueTable == NULL)
        return WBXML_ERROR_ATTR_VALUE_TABLE_UNDEFINED;

    while ((parser->langTable->attrValueTable[index].xmlName != NULL) &&
           ((parser->langTable->attrValueTable[index].wbxmlToken != tag) ||
            (parser->langTable->attrValueTable[index].wbxmlCodePage != parser->attrCodePage))) {
        index++;
    }

    if (parser->langTable->attrValueTable[index].xmlName == NULL)
        return WBXML_ERROR_UNKNOWN_ATTR_VALUE;

    *value = parser->langTable->attrValueTable[index].xmlName;
    *len = WBXML_STRLEN(*value);

    return WBXML_OK;
}


/**
 * @brief Parse WBXML termstr
 * @param parser [in] The WBXML Parser
 * @param str [out] The resulting parsed string
 * @param len [out] The resulting parsed length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note termstr = charset-dependent string with termination
 */
static WBXMLError parse_termstr(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len)
{
    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing termstr", parser->pos));

    /** @bug "String representation is dependent on the character document encoding and should not
     *          be presumed to include NULL termination" */
    *str = wbxml_buffer_get_cstr(parser->wbxml) + parser->pos;
    *len = WBXML_STRLEN(*str);

    /* Check that the string was NULL terminated */
    if (parser->pos + *len > wbxml_buffer_len(parser->wbxml))
        return WBXML_ERROR_NOT_NULL_TERMINATED_INLINE_STRING;

    /* Skip string (don't forget ending null char) */
    parser->pos = parser->pos + *len + 1;

    WBXML_DEBUG((WBXML_PARSER, "(%d) termstr: %s", parser->pos, *str));

    return WBXML_OK;
}


/**
 * @brief Parse WBXML inline
 * @param parser [in] The WBXML Parser
 * @param str [out] The resulting parsed string
 * @param len [out] The resulting parsed string length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note inline = STR_I termstr
 */
static WBXMLError parse_inline(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len)
{
    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing inline", parser->pos));

    /* Skip STR_I */
    parser->pos++;

    return parse_termstr(parser, str, len);
}


/**
 * @brief Parse WBXML tableref
 * @param parser The WBXML Parser
 * @param str The resulting parsed string
 * @param len The resulting parsed string length
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note tableref = STR_T index
 * @note index = mb_u_int32
 */
static WBXMLError parse_tableref(WBXMLParser *parser, WB_UTINY **str, WB_LONG *len)
{
    WB_ULONG index;
    WBXMLError ret = WBXML_OK;

    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing tableref", parser->pos));

    /* Skip STR_T */
    parser->pos++;

    /* Parse index */
    if ((ret = parse_mb_uint32(parser, &index)) != WBXML_OK)
        return ret;

    return get_strtbl_reference(parser, index, str, len);
}


/**
 * @brief Parse WBXML entcode
 * @param parser [in] The WBXML Parser
 * @param result [out] The entcode parsed
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note entcode = mb_u_int32 // UCS-4 character code
 */
static WBXMLError parse_entcode(WBXMLParser *parser, WB_ULONG *result)
{
    WBXML_DEBUG((WBXML_PARSER, "(%d) Parsing entcode", parser->pos));

    return parse_mb_uint32(parser, result);
}


/**
 * @brief Get a string from String Table
 * @param parser The WBXML Parser
 * @param index Index of string in String Table
 * @param str The resulting parsed string
 * @param len The resulting parsed string length
 * @return WBXML_OK if OK, an error code otherwise
 */
static WBXMLError get_strtbl_reference(WBXMLParser *parser, WB_ULONG index, WB_UTINY **str, WB_LONG *len)
{
    /* Check if strtbl is NULL */
    /** @todo It can be a simple Warning */
    if (parser->strstbl == NULL)
        return WBXML_ERROR_NULL_STRING_TABLE;

    if (index >= wbxml_buffer_len(parser->strstbl))
        return WBXML_ERROR_INVALID_STRTBL_INDEX;

    /** @bug "String representation is dependent on the character document encoding and should not
     *          be presumed to include NULL termination" */
    *str = wbxml_buffer_get_cstr(parser->strstbl) + index;
    *len = WBXML_STRLEN(*str);

    /* Check that the string was correctly NULL terminated in String Table */
    if (index + *len > wbxml_buffer_len(parser->strstbl))
        return WBXML_ERROR_BAD_NULL_TERMINATED_STRING_IN_STRING_TABLE;

    WBXML_DEBUG((WBXML_PARSER, "(%d) String Table Reference: %s", parser->pos, *str));

    return WBXML_OK;
}


/********************************
 *    Basic Types Parse functions
 */

/**
 * @brief Parse UINT8
 * @param parser [in] The WBXML Parser
 * @param result [out] Parsed UINT8
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note u_int8 = 8 bit unsigned integer
 */
static WBXMLError parse_uint8(WBXMLParser *parser, WB_UTINY *result)
{
    if (parser == NULL)
        return WBXML_ERROR_NULL_PARSER;

    if (result == NULL)
        return WBXML_ERROR_BAD_PARAMETER;

    if (parser->pos == wbxml_buffer_len(parser->wbxml))
        return WBXML_ERROR_END_OF_BUFFER;

    if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, result))
        return WBXML_ERROR_END_OF_BUFFER;

    parser->pos++;

    return WBXML_OK;
}


/**
 * @brief Parse a MultiByte UINT32
 * @param parser The WBXML Parser
 * @param result The parsed MultiByte
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note mb_u_int32 = 32 bit unsigned integer, encoded in multi-byte format
 */
static WBXMLError parse_mb_uint32(WBXMLParser *parser, WB_ULONG *result)
{
    WB_ULONG uint = 0, byte_pos;
    WB_UTINY cur_byte;

    if (parser == NULL)
        return WBXML_ERROR_NULL_PARSER;

    if (result == NULL)
        return WBXML_ERROR_BAD_PARAMETER;

    /* It's a 32bit integer, and so it fits to a maximum of 4 bytes */
    for (byte_pos = 0; byte_pos < 5; byte_pos++) {
        /* Get current byte */
        if (!wbxml_buffer_get_char(parser->wbxml, parser->pos, &cur_byte))
            return WBXML_ERROR_END_OF_BUFFER;

        /* Move to next byte */
        parser->pos++;

        /* Update uint value */
        uint = (uint << 7) | ((WB_UTINY)cur_byte & 0x7F);

        /* Check first bit, and stop if value is zero */
        if (!((WB_UTINY)cur_byte & 0x80)) {
            *result = uint;
            return WBXML_OK;
        }
    }

    return WBXML_ERROR_UNVALID_MBUINT32;
}


/****************************************
 * Language Specific Decoding Functions
 */

/**************************************
 * SI 1.0 / SL 1.0
 */

/**
 * @brief Decode a %Datetime Attribute Value
 * @param buff The Attribute Value to convert
 * @return WBXML_OK if OK, another error code otherwise
 * @note Used for:
 *      - SI 1.0: Decode date for 'created' and 'si-expires' attributes
 *      - EMN 1.0: Decode date for 'timestamp' attribute
 */
static WBXMLError decode_datetime(WBXMLBuffer *buff)
{
    WB_ULONG len = 0;

    /* Binary to Hexa */
    if (!wbxml_buffer_binary_to_hex(buff, TRUE))
        return WBXML_ERROR_INTERNAL;

    /* Check Integrity */
    len = wbxml_buffer_len(buff);
    if ((len < 8) || (len > 14) || (len == 9) || (len == 11) || (len == 13))
        return WBXML_ERROR_BAD_DATETIME;

    /* Date */

    /* "1999-" */
    if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *)"-", 4))
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* "1999-04-" */
    if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *)"-", 7))
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* "1999-04-30T" */
    if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *)"T", 10))
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Time */

    /* Append ':' delimiters */
    if (len > 10) {
        if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *)":", 13))
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    if (len > 12) {
        if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *)":", 16))
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Append Trailing Zeros */
    switch (len) {
    case 8:
        if (!wbxml_buffer_append_cstr(buff, (WB_UTINY *)"00:00:00"))
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        break;
    case 10:
        if (!wbxml_buffer_append_cstr(buff, (WB_UTINY *)":00:00"))
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        break;
    case 12:
        if (!wbxml_buffer_append_cstr(buff, (WB_UTINY *)":00"))
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        break;
    default:
        /* 14 : Nothing to do */
        break;
    }

    /* Append ending 'Z' character */
    if (!wbxml_buffer_append_char(buff, 'Z'))
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    return WBXML_OK;
}


/**************************************
 * WV 1.1 / WV 1.2
 */

/**
 * @brief Decode an Opaque Content buffer
 * @param date The Opaque data buffer
 * @param len The Opaque data buffer length
 * @param static_content [out] Is this a static buffer ?
 * @return WBXML_OK if OK, another error code otherwise
 * @note Used for:
 *      - WV 1.1 / 1.2
 */
static WBXMLError decode_opaque_content(WBXMLParser *parser, WB_UTINY **data, WB_LONG *len, WB_BOOL *static_content)
{
    WBXMLWVDataType data_type = WBXML_WV_DATA_TYPE_STRING;
    WBXMLError ret = WBXML_OK;

    *static_content = TRUE;

    /* If this is a Wireless-Village document 1.1 / 1.2 */
    if ((WBXML_STRCMP(parser->langTable->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP11) == 0) ||
        (WBXML_STRCMP(parser->langTable->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP12) == 0))
    {

        /*
         *  Specific WV Opaque Data Type Elements:
         *
         *        Integer:
         *            Code (0x00 / 0x0B)
         *            ContentSize (0x00 / 0x0F)
         *            MessageCount (0x00 / 0x1A)
         *            Validity (0x00 / 0x3C)
         *            KeepAliveTime (0x01 / 0x1C)
         *            SearchFindings (0x01 / 0x25)
         *            SearchID (0x01 / 0x26)
         *            SearchIndex (0x01 / 0x27)
         *            SearchLimit (0x01 / 0x28)
         *            TimeToLive (0x01 / 0x32)
         *            AcceptedCharSet (0x03 / 0x05)
         *            AcceptedContentLength (0x03 / 0x06)
         *            MultiTrans (0x03 / 0x0C)
         *            ParserSize (0x03 / 0x0D)
         *            ServerPollMin (0x03 / 0x0E)
         *            TCPPort (0x03 / 0x12)
         *            UDPPort (0x03 / 0x13)
         *            HistoryPeriod (0x09 / 0x08) [WV 1.2]
         *            MaxWatcherList (0x09 / 0x0A) [WV 1.2]
         *
         *        Date and Time:
         *            DateTime (0x00 / 0x11)
         *            DeliveryTime (0x06 / 0x1A)
         *
         *        Binary:
         *            ContentData (0x00 / 0x0D) (only if we have a: "<ContentEncoding>BASE64</ContentEncoding>" associated)
         */

        /***********************************************
         * Check the Data Type, given the Current Tag
         */

        switch (parser->current_tag->wbxmlCodePage) {
        case 0x00:
            /* Code Page: 0x00 */
            switch (parser->current_tag->wbxmlToken) {
            case 0x0B: /* Code */
            case 0x0F: /* ContentSize */
            case 0x1A: /* MessageCount */
            case 0x3C: /* Validity */
                /* INTEGER */
                data_type = WBXML_WV_DATA_TYPE_INTEGER;
                break;
            case 0x11: /* DateTime */
                /* DATE_AND_TIME */
                data_type = WBXML_WV_DATA_TYPE_DATE_AND_TIME;
                break;
            case 0x0D: /* ContentData */
                /* BINARY */
                /** @todo Check if we have a: "<ContentEncoding>BASE64</ContentEncoding>" associated */
                /*
                if (base64_encoded)
                    data_type = WBXML_WV_DATA_TYPE_BINARY;
                else
                */
                    data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x01:
            /* Code Page: 0x01 */
            switch (parser->current_tag->wbxmlToken) {
            case 0x1C: /* KeepAliveTime */
            case 0x25: /* SearchFindings */
            case 0x26: /* SearchID */
            case 0x27: /* SearchIndex */
            case 0x28: /* SearchLimit */
            case 0x32: /* TimeToLive */
                /* INTEGER */
                data_type = WBXML_WV_DATA_TYPE_INTEGER;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x03:
            /* Code Page: 0x03 */
            switch (parser->current_tag->wbxmlToken) {
            case 0x05: /* AcceptedCharSet */
            case 0x06: /* AcceptedContentLength */
            case 0x0C: /* MultiTrans */
            case 0x0D: /* ParserSize */
            case 0x0E: /* ServerPollMin */
            case 0x12: /* TCPPort */
            case 0x13: /* UDPPort */
                /* INTEGER */
                data_type = WBXML_WV_DATA_TYPE_INTEGER;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x06:
            /* Code Page: 0x06 */
            switch (parser->current_tag->wbxmlToken) {
            case 0x1A: /* DeliveryTime */
                /* DATE AND TIME */
                data_type = WBXML_WV_DATA_TYPE_DATE_AND_TIME;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x09:
            /* Code Page: 0x09 */
            switch (parser->current_tag->wbxmlToken) {
            case 0x08: /* HistoryPeriod */
            case 0x0A: /* MaxWatcherList */
                /* INTEGER */
                data_type = WBXML_WV_DATA_TYPE_INTEGER;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        default:
            data_type = WBXML_WV_DATA_TYPE_STRING;
            break;
        } /* switch */


        /***********************************************
         * Decode the Opaque, given the Data Type found
         */

        switch (data_type) {
        case WBXML_WV_DATA_TYPE_INTEGER:
            /* Decode Integer */
            if ((ret = decode_wv_integer(data, len)) != WBXML_OK)
                return ret;

            *static_content = FALSE;
            return WBXML_OK;
            break;
        case WBXML_WV_DATA_TYPE_DATE_AND_TIME:
            /* Decode Date and Time */
            if ((ret = decode_wv_datetime(data, len)) != WBXML_OK)
                return ret;

            *static_content = FALSE;
            return WBXML_OK;
            break;
        case WBXML_WV_DATA_TYPE_BINARY:
            /** @todo decode_wv_binary() */
            break;
        default:
            /* Do nothing. Keep this data as is. */
            break;
        } /* switch */
    }

    return WBXML_OK;
}


/**
 * @brief Decode a WV Integer encoded in an Opaque
 * @param data The WV Integer to decode
 * @param len Length of Data
 * @return WBXML_OK if OK, another error code otherwise
 * @note Used for:
 *      - WV 1.1 / 1.2
 * @note [OMA-WV-CSP_DataTypes-V1_1-20021001-A.pdf] - 4.1:
 *       "An integer is a number from 0-4294967295 expressed in decimal format."
 */
static WBXMLError decode_wv_integer(WB_UTINY **data, WB_LONG *len)
{
    WB_LONG i = 0;
    WB_ULONG the_int = 0;
    WB_UTINY ch = 0;
    WB_UTINY tmp[11];

    if ((data == NULL) || (len == NULL) || (*len <= 0))
        return WBXML_ERROR_INTERNAL;

    for (i = 0; i < *len; i++) {
        ch = (*data)[i];
        the_int = (the_int << 8) | (ch & 0xff);
    }

    if (the_int > 0xffffffff)
        return WBXML_ERROR_WV_INTEGER_OVERFLOW;

    sprintf((char *)tmp, "%u", the_int);

    if (((*data) = (WB_UTINY *) wbxml_malloc((WBXML_STRLEN(tmp) + 1) * sizeof(WB_UTINY))) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    *len = WBXML_STRLEN(tmp);

    memcpy(*data, tmp, *len);
    (*data)[*len] = '\0';

    return WBXML_OK;
}


/**
 * @brief Decode a WV Date and Time encoded in an Opaque
 * @param data The WV Integer to decode
 * @param len Length of Data
 * @return WBXML_OK if OK, another error code otherwise
 * @note Used for:
 *      - WV 1.1 / 1.2
 * @note
 *  Encoded Format: (6 octets)
 *      - The first 2 bits are reserved, and both must be 0.
 *      - Year is encoded by 12 bits (0 to 4095)
 *      - Month is encoded by 4 bits (1 to 12)
 *      - Day is encoded by 5 bits (1 to 31)
 *      - Hour is encoded by 5 bits (0 to 23)
 *      - Minute is encoded by 6 bits (0 to 59)
 *      - Second is encoded by 6 bits (0 to 59)
 *      - Time zone is encoded in 1 byte [ISO8601].
 *
 *      eg:
 *          Binary:  00 011111010001 1010 10011 01001 110010 011111 01011010
 *          Octets:  (-------)(-------)(--------)(-------)(-------) (------)
 *
 *  Decoded Format:
 *      eg: 20011019T095031Z or 20011019T095031
 */
static WBXMLError decode_wv_datetime(WB_UTINY **data, WB_LONG *len)
{
    WB_UTINY *result = NULL;
    WB_TINY the_year[5], the_month[3],  the_date[3],
            the_hour[3], the_minute[3], the_second[3];
    WB_ULONG the_value = 0;

    /** @todo Test decode_wv_datetime() ! */

    if ((data == NULL) || (*data == NULL) || (len == NULL))
        return WBXML_ERROR_INTERNAL;

    if (*len != 6)
        return WBXML_ERROR_WV_DATETIME_FORMAT;

    if ((result = (WB_UTINY *) wbxml_malloc(17 * sizeof(WB_UTINY))) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Get Year */
    the_value = (WB_ULONG) (((*data)[0] << 6) | (((*data)[1] >> 2) && 0x3F));
    sprintf(the_year, "%u", the_value);

    /* Get Month */
    the_value = (WB_ULONG) ((((*data)[1] && 0x03) << 2) | (((*data)[2] >> 6) && 0x3F));
    sprintf(the_month, "%u", the_value);

    /* Get Day */
    the_value = (WB_ULONG) (((*data)[2] >> 1) && 0x1F);
    sprintf(the_date, "%u", the_value);

    /* Get Hour */
    the_value = (WB_ULONG) ((((*data)[2] && 0x01) << 4) | (((*data)[3] >> 4) && 0x0F));
    sprintf(the_hour, "%u", the_value);

    /* Get Minute */
    the_value = (WB_ULONG) ((((*data)[3] && 0x0F) << 2) | (((*data)[4] >> 6) && 0x03));
    sprintf(the_minute, "%u", the_value);

    /* Get Second */
    the_value = (WB_ULONG) ((*data)[4] && 0x3F);
    sprintf(the_second, "%u", the_value);

    /* Get Time Zone */
    if ((*data)[5] == 'Z')
        sprintf((WB_TINY *) result, "%s%s%sT%s%s%sZ", the_year, the_month, the_date, the_hour, the_minute, the_second);
    else
        sprintf((WB_TINY *) result, "%s%s%sT%s%s%s", the_year, the_month, the_date, the_hour, the_minute, the_second);

    return WBXML_OK;
}
