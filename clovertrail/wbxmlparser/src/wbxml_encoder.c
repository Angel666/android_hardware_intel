/*
 * WBXML Lib, the WBXML Library.
 * Copyright (C) 2002  Aymerick J�hanne
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
 * 10/20/2003 Motorola    Add support for wbxml version.
 */

/**
 * @file wbxml_encoder.c
 * @ingroup wbxml_encoder
 *
 * @author Aymerick J�hanne <libwbxml@jehanne.org>
 * @date 11/11/02
 *
 * @brief WBXML Encoder
 * @note Inspired from kannel WML Encoder (http://kannel.3glab.org)
 *
 * @note [OMA WV 1.1] : OMA-WV-CSP_WBXML-V1_1-20021001-A.pdf
 *
 * @todo Parse CDDATA
 * @todo Parse PI
 *
 * @todo Handle Charsets Encoding
 *
 * @todo Really generate ENTITY tokens
 *
 * @todo Handle Namespaces !
 *
 * @todo For canonical XML output: Sort the Attributes
 *
 * @todo When adding string to String Table, check that this is not an Attribute Value / Content Text that
 *       will be tokenized
 *
 * @todo For Wireless-Village CSP :
 *              - Encode "Date and Time" in OPAQUE (OMA-WV-CSP_WBXML-V1_1-20021001-A.pdf - 6.6)
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* For isdigit() */

#include "wbxml.h"


/* WBXML Header:    version     publicid    charset     length
 *                  u_int8      mb_u_int32  mb_u_int32  mb_u_int32
 *                  1 octet     5 octets    5 octets    5 octets   :  16 octets
 * mb_u_int32: 5 octets (to handle continuation bits)
 */
#define WBXML_HEADER_MAX_LEN 16

/* Memory management related defines */
#define WBXML_ENCODER_XML_DOC_MALLOC_BLOCK 5000
#define WBXML_ENCODER_WBXML_DOC_MALLOC_BLOCK 1000

#define WBXML_ENCODER_XML_HEADER_MALLOC_BLOCK 250
#define WBXML_ENCODER_WBXML_HEADER_MALLOC_BLOCK WBXML_HEADER_MAX_LEN

/* WBXML Version: 1.3 */
#define WBXML_ENCODER_VERSION 0x03

/* WBXML Default Charset: UTF-8 (106) */
#define WBXML_ENCODER_DEFAULT_CHARSET 0x6a

/* String Terminating NULL Char */
#define WBXML_STR_END '\0'

/* Minimum String Size needed for String Table - @note Set to '3' for Prov 1.0 */
#define WBXML_ENCODER_STRING_TABLE_MIN 3

/**
 * @brief WBXML Encoder Output Type
 */
typedef enum WBXMLEncoderOutputType_e {
    WBXML_ENCODER_OUTPUT_WBXML = 0,
    WBXML_ENCODER_OUTPUT_XML
} WBXMLEncoderOutputType;

/**
 * @brief The WBXML Encoder
 * @warning For now 'current_tag' field is only used for WV Content Encoding. And for this use, it works.
 *          But this field is reset after End Tag, and as there is no Linked List mecanism, this is bad for
 *          cascading elements: we don't fill this field with parent Tag when parsing End Tag.
 */
struct WBXMLEncoder_s {
    WBXMLTree *tree;                /**< WBXML Tree to Encode */
    WBXMLBuffer *output;            /**< The output (wbxml or xml) we are producing */
    WBXMLList *strstbl;             /**< String Table we are creating */
    const WBXMLTagEntry *current_tag;     /**< Current Tag (See The Warning For This Field !) */
    const WBXMLAttrEntry *current_attr;   /**< Current Attribute */
    WB_ULONG strstbl_len;           /**< String Table Length */
    WB_UTINY tagCodePage;           /**< Current Tag Code Page */
    WB_UTINY attrCodePage;          /**< Current Attribute Code Page */
    WB_BOOL ignore_empty_text;      /**< Do we ignore empty text nodes (ie: ignorable whitespaces)? */
    WB_BOOL remove_text_blanks;     /**< Do we remove leading and trailing blanks in text nodes ? */
    WBXMLEncoderOutputType output_type;  /**< Output Type */
    WBXMLEncoderXMLGenType xml_gen_type; /**< XML Generation Type */
    WB_UTINY indent_delta;               /**< Indent Delta (number of spaces) */
    WB_UTINY indent;                     /**< Current Indent */
    WB_BOOL in_content;                  /**< We are in Content Text (used for indentation when generating XML output) */
    WB_BOOL use_strtbl;                  /**< Do we use String Table when generating WBXML output ? (default: YES) */
    WBXMLVersion wbxml_version;             /**< WBXML Version to use (when generating WBXML output) */

};

/**
 * @brief The WBXML String Table Element
 */
typedef struct WBXMLStringTableElement_t {
    WBXMLBuffer *string; /**< String */
    WB_ULONG offset;     /**< Offset of String in String Table */
    WB_ULONG count;      /**< Number of times this String is referenced in the XML Document */
    WB_BOOL stat;        /**< If set to TRUE, this is a static String that we must not destroy in wbxml_strtbl_element_destroy() function */
} WBXMLStringTableElement;

/**
 * @brief WBXML Value Element Context: In Content or in Attribute Value
 */
typedef enum WBXMLValueElementCtx_e {
    WBXML_VALUE_ELEMENT_CTX_CONTENT = 0,    /**< Text Content */
    WBXML_VALUE_ELEMENT_CTX_ATTR            /**< Attribute Value */
} WBXMLValueElementCtx;

/**
 * @brief WBXML Value Element Type: string / tableref / extension / opaque
 */
typedef enum WBXMLValueElementType_e {
    WBXML_VALUE_ELEMENT_STRING = 0, /**< Inline String */
    WBXML_VALUE_ELEMENT_TABLEREF,   /**< String Table Reference */
    WBXML_VALUE_ELEMENT_EXTENSION,  /**< Extension Token */
    WBXML_VALUE_ELEMENT_OPAQUE,     /**< Opaque Buffer */
    WBXML_VALUE_ELEMENT_ATTR_TOKEN  /**< Attribute Value Token */
} WBXMLValueElementType;

/**
 * @brief WBXML Value Element Structure
 */
typedef struct WBXMLValueElement_t {
    WBXMLValueElementType type;     /**< Cf WBXMLValueElementType enum */
    union {
        WBXMLBuffer *str;                   /**< WBXML_VALUE_ELEMENT_STRING */
        WB_ULONG    index;                  /**< WBXML_VALUE_ELEMENT_TABLEREF */
        const WBXMLExtValueEntry *ext;      /**< WBXML_VALUE_ELEMENT_EXTENSION */
        WBXMLBuffer *buff;                  /**< WBXML_VALUE_ELEMENT_OPAQUE */
        const WBXMLAttrValueEntry *attr;    /**< WBXML_VALUE_ELEMENT_ATTR_TOKEN */
    } u;
} WBXMLValueElement;


/***************************************************
 *    Private Functions prototypes
 */

/*******************************
 * Convertion Functions
 */

static WB_BOOL convert_char_to_ucs4(WB_UTINY ch, WB_ULONG *result);


/*******************************
 * WBXML Tree Parsing Functions
 */

static WBXMLError parse_node(WBXMLEncoder *encoder, WBXMLTreeNode *node);
static WBXMLError parse_element(WBXMLEncoder *encoder, WBXMLTreeNode *node);
static WBXMLError parse_attribute(WBXMLEncoder *encoder, WBXMLAttribute *attribute);
static WBXMLError parse_text(WBXMLEncoder *encoder, WBXMLTreeNode *node);
static WBXMLError parse_cdata(WBXMLEncoder *encoder, WBXMLTreeNode *node);
static WBXMLError parse_pi(WBXMLEncoder *encoder, WBXMLTreeNode *node);


/*******************************
 * WBXML Output Functions
 */

/* Build WBXML Result */
static WBXMLError wbxml_build_result(WBXMLEncoder *encoder, WB_UTINY **wbxml, WB_ULONG *wbxml_len);

/* WBXML Encoding Functions */
static WBXMLError wbxml_encode_end(WBXMLEncoder *encoder);

static WBXMLError wbxml_encode_tag(WBXMLEncoder *encoer, WBXMLTreeNode *node);
static WBXMLError wbxml_encode_tag_literal(WBXMLEncoder *encoder, WB_UTINY *tag, WB_UTINY mask);
static WBXMLError wbxml_encode_tag_token(WBXMLEncoder *encoder, WB_UTINY token, WB_UTINY page);

static WBXMLError wbxml_encode_attr(WBXMLEncoder *encoder, WBXMLAttribute *attribute);
static WBXMLError wbxml_encode_attr_start(WBXMLEncoder *encoder, WBXMLAttribute *attribute, WB_UTINY **value);
static WBXMLError wbxml_encode_value_element_buffer(WBXMLEncoder *encoder, WB_UTINY *value, WBXMLValueElementCtx ctx);
static WBXMLError wbxml_encode_value_element_list(WBXMLEncoder *encoder, WBXMLList *list);
static WBXMLError wbxml_encode_attr_start_literal(WBXMLEncoder *encoder, const WB_UTINY *attr);
static WBXMLError wbxml_encode_attr_token(WBXMLEncoder *encoder, WB_UTINY token, WB_UTINY page);

static WBXMLError wbxml_encode_inline_string(WBXMLEncoder *encoder, WBXMLBuffer *str);
static WBXMLError wbxml_encode_tableref(WBXMLEncoder *encoder, WB_ULONG offset);
static WBXMLError wbxml_encode_inline_integer_extension_token(WBXMLEncoder *encoder, WB_UTINY ext, WB_UTINY value);
static WBXMLError wbxml_encode_entity(WBXMLEncoder *encoder, WB_ULONG value);
static WBXMLError wbxml_encode_opaque(WBXMLEncoder *encoder, WBXMLBuffer *buff);

static WBXMLValueElement *wbxml_value_element_create(void);
static void wbxml_value_element_destroy(WBXMLValueElement *elt);
static void wbxml_value_element_destroy_item(void *elt);

static WBXMLError wbxml_encode_datetime(WBXMLEncoder *encoder, WB_UTINY *buffer);
static WBXMLError wbxml_encode_wv_content(WBXMLEncoder *encoder, WB_UTINY *buffer);
static WBXMLError wbxml_encode_wv_integer(WBXMLEncoder *encoder, WB_UTINY *buffer);
static WBXMLError wbxml_encode_wv_datetime(WBXMLEncoder *encoder, WB_UTINY *buffer);

/* WBXML String Table Functions */
static WBXMLStringTableElement *wbxml_strtbl_element_create(WBXMLBuffer *string, WB_BOOL is_stat);
static void wbxml_strtbl_element_destroy(WBXMLStringTableElement *element);
static void wbxml_strtbl_element_destroy_item(void *element);

static WBXMLError wbxml_strtbl_initialize(WBXMLEncoder *encoder, WBXMLTreeNode *root);
static void wbxml_strtbl_collect_strings(WBXMLEncoder *encoder, WBXMLTreeNode *node, WBXMLList *strings);
static WBXMLError wbxml_strtbl_collect_words(WBXMLList *elements, WBXMLList **result);
static void wbxml_strtbl_construct(WB_UTINY *buff, WBXMLList *strstbl);
static WBXMLError wbxml_strtbl_check_references(WBXMLEncoder *encoder, WBXMLList **strings, WBXMLList **one_ref, WB_BOOL stat_buff);
static WB_BOOL wbxml_strtbl_add_element(WBXMLEncoder *encoder, WBXMLStringTableElement *elt, WB_ULONG *index, WB_BOOL *added);


/*******************************
 * XML Output Functions
 */

/** New Line */
#define WBXML_ENCODER_XML_NEW_LINE "\n"

/* XML Header Macros */
#define WBXML_ENCODER_XML_HEADER "<?xml version=\"1.0\"?>"
#define WBXML_ENCODER_XML_DOCTYPE "<!DOCTYPE "
#define WBXML_ENCODER_XML_PUBLIC " PUBLIC \""
#define WBXML_ENCODER_XML_DTD "\" \""
#define WBXML_ENCODER_XML_END_DTD "\">"

/* Global vars for XML Normalization */
const WB_UTINY xml_lt[5] = "&lt;";      /**< &lt; */
const WB_UTINY xml_gt[5] = "&gt;";      /**< &gt; */
const WB_UTINY xml_amp[6] = "&amp;";    /**< &amp; */
const WB_UTINY xml_quot[7] = "&quot;";  /**< &quot; */
const WB_UTINY xml_slashr[6] = "&#13;"; /**< &#13; */
const WB_UTINY xml_slashn[6] = "&#10;"; /**< &#10; */
const WB_UTINY xml_tab[5] = "&#9;";     /**< &#9; */

/* Build XML Result */
static WBXMLError xml_build_result(WBXMLEncoder *encoder, WB_UTINY **xml);
static WB_BOOL xml_fill_header(WBXMLEncoder *encoder, WBXMLBuffer *header);

/* XML Encoding Functions */
static WBXMLError xml_encode_tag(WBXMLEncoder *encoer, WBXMLTreeNode *node);
static WBXMLError xml_encode_end_tag(WBXMLEncoder *encoder, WBXMLTreeNode *node);

static WBXMLError xml_encode_attr(WBXMLEncoder *encoder, WBXMLAttribute *attribute);
static WBXMLError xml_encode_end_attrs(WBXMLEncoder *encoder);

static WBXMLError xml_encode_text(WBXMLEncoder *encoder, WBXMLBuffer *str);
static WB_BOOL xml_encode_new_line(WBXMLBuffer *buff);
static WB_BOOL xml_normalize(WBXMLBuffer *buff);


/***************************************************
 *    Public Functions
 */

/* WBXMLEncoder Functions */

WBXML_DECLARE(WBXMLEncoder *) wbxml_encoder_create_real(void)
{
    WBXMLEncoder *encoder = NULL;

    encoder = (WBXMLEncoder *) wbxml_malloc(sizeof(WBXMLEncoder));
    if (encoder == NULL) {
        return NULL;
    }

    if ((encoder->strstbl = wbxml_list_create()) == NULL) {
        wbxml_free(encoder);
        return NULL;
    }

    encoder->tree = NULL;
    encoder->output = NULL;

    encoder->current_tag = NULL;
    encoder->current_attr = NULL;

    encoder->strstbl_len = 0;

    encoder->tagCodePage = 0;
    encoder->attrCodePage = 0;

    encoder->ignore_empty_text = FALSE;
    encoder->remove_text_blanks = FALSE;

    encoder->output_type = WBXML_ENCODER_OUTPUT_WBXML;
    encoder->xml_gen_type = WBXML_ENCODER_XML_GEN_COMPACT;

    encoder->indent_delta = 1;
    encoder->indent = 0;
    encoder->in_content = FALSE;

    encoder->use_strtbl = TRUE;

    /* Default Version: WBXML 1.3 */
    encoder->wbxml_version = WBXML_VERSION_13;

    return encoder;
}


WBXML_DECLARE(void) wbxml_encoder_destroy(WBXMLEncoder *encoder)
{
    if (encoder == NULL)
        return;

    wbxml_buffer_destroy(encoder->output);
    wbxml_list_destroy(encoder->strstbl, wbxml_strtbl_element_destroy_item);

    wbxml_free(encoder);
}


/* Possible options when generating WBXML or XML */

WBXML_DECLARE(void) wbxml_encoder_set_ignore_empty_text(WBXMLEncoder *encoder, WB_BOOL set_ignore)
{
    if (encoder == NULL)
        return;

    encoder->ignore_empty_text = set_ignore;
}


WBXML_DECLARE(void) wbxml_encoder_set_remove_text_blanks(WBXMLEncoder *encoder, WB_BOOL set_remove)
{
    if (encoder == NULL)
        return;

    encoder->remove_text_blanks = set_remove;
}


/* Possible options when generating WBXML */

WBXML_DECLARE(void) wbxml_encoder_set_use_strtbl(WBXMLEncoder *encoder, WB_BOOL use_strtbl)
{
    if (encoder == NULL)
        return;

    encoder->use_strtbl = use_strtbl;
}


WBXML_DECLARE(void) wbxml_encoder_set_wbxml_version(WBXMLEncoder *encoder, WBXMLVersion version)
{
    if (encoder == NULL)
        return;

    if (version != WBXML_VERSION_UNKNOWN)
        encoder->wbxml_version = version;
}


/* Possible options when generating XML */

WBXML_DECLARE(void) wbxml_encoder_set_xml_gen_type(WBXMLEncoder *encoder, WBXMLEncoderXMLGenType gen_type)
{
    if (encoder == NULL)
        return;

    encoder->xml_gen_type = gen_type;
}


WBXML_DECLARE(void) wbxml_encoder_set_indent(WBXMLEncoder *encoder, WB_UTINY indent)
{
    if (encoder == NULL)
        return;

    encoder->indent_delta = indent;
}


/* Encoding Functions */

WBXML_DECLARE(void) wbxml_encoder_set_tree(WBXMLEncoder *encoder, WBXMLTree *tree)
{
    if (encoder == NULL)
        return;

    encoder->tree = tree;
}


WBXML_DECLARE(WBXMLError) wbxml_encoder_encode(WBXMLEncoder *encoder, WB_UTINY **wbxml, WB_ULONG *wbxml_len)
{
    WBXMLError ret = WBXML_OK;

    /* Check Parameters */
    if ((encoder == NULL) || (encoder->tree == NULL) || (encoder->tree->lang == NULL) || (wbxml == NULL) || (wbxml_len == NULL))
        return WBXML_ERROR_BAD_PARAMETER;

    /* We output WBXML */
    encoder->output_type = WBXML_ENCODER_OUTPUT_WBXML;

    /* Init ret values */
    *wbxml = NULL;
    *wbxml_len = 0;

    /* Init WBXML Output Buffer */
    encoder->output = wbxml_buffer_create((const WB_UTINY *)"", 0, WBXML_ENCODER_WBXML_DOC_MALLOC_BLOCK);
    if (encoder->output == NULL) {
        wbxml_encoder_destroy(encoder);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Do NOT use String Table for Wireless-Village */
    if (WBXML_STRCMP(encoder->tree->lang->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP11) == 0)
        encoder->use_strtbl = FALSE;

    /* Init String Table */
    if (encoder->use_strtbl) {
        if ((ret = wbxml_strtbl_initialize(encoder, encoder->tree->root)) != WBXML_OK)
            return ret;
    }

    /* Let's begin WBXML Tree Parsing */
    if ((ret = parse_node(encoder, encoder->tree->root)) != WBXML_OK)
        return ret;

    /* Build result */
    return wbxml_build_result(encoder, wbxml, wbxml_len);
}


WBXML_DECLARE(WBXMLError) wbxml_encoder_encode_to_xml(WBXMLEncoder *encoder, WB_UTINY **xml)
{
    WBXMLError ret = WBXML_OK;

    /* Check Parameters */
    if ((encoder == NULL) || (encoder->tree == NULL) || (encoder->tree->lang == NULL) || (xml == NULL))
        return WBXML_ERROR_BAD_PARAMETER;

    /* We output XML */
    encoder->output_type = WBXML_ENCODER_OUTPUT_XML;

    /* Init ret values */
    *xml = NULL;

    /* Init XML Output Buffer */
    encoder->output = wbxml_buffer_create((const WB_UTINY *)"", 0, WBXML_ENCODER_XML_DOC_MALLOC_BLOCK);
    if (encoder->output == NULL) {
        wbxml_encoder_destroy(encoder);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Let's begin WBXML Tree Parsing */
    if ((ret = parse_node(encoder, encoder->tree->root)) != WBXML_OK)
        return ret;

    /* Build result */
    return xml_build_result(encoder, xml);
}


/***************************************************
 *    Private Functions
 */


/****************************
 * Convertion Functions
 */

/**
 * @brief Convert a char to UCS-4
 * @param ch [in] The character to convert
 * @param result [out] The UCS-4 code
 * @return TRUE if convertion succeeded, FALSE otherwise
 */
static WB_BOOL convert_char_to_ucs4(WB_UTINY ch, WB_ULONG *result)
{
    /** @todo convert_char_to_ucs4() */

    return FALSE;
}


/*********************************
 * WBXML Tree Parsing Functions
 */

/**
 * @brief Parse an XML Node
 * @param encoder The WBXML Encoder
 * @param node The node to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 * @note We have recurrency in this function
 */
static WBXMLError parse_node(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    WBXMLError ret = WBXML_OK;

    /* Parse this node */
    switch (node->type) {
        case WBXML_TREE_ELEMENT_NODE:
            ret = parse_element(encoder, node);
		    break;
        case WBXML_TREE_TEXT_NODE:
		    ret = parse_text(encoder, node);
		    break;
        case WBXML_TREE_PI_NODE:
            ret = parse_pi(encoder, node);
            break;
        default:
    		return WBXML_ERROR_XML_NODE_NOT_ALLOWED;
		    break;
    }

    if (ret != WBXML_OK)
        return ret;

    /* Check if node has children */
    if (node->children != NULL) {
        /* Parse Child */
        if ((ret = parse_node(encoder, node->children)) != WBXML_OK)
            return ret;

        /* Add a WBXML End tag */
        if (encoder->output_type == WBXML_ENCODER_OUTPUT_WBXML) {
            if ((ret = wbxml_encode_end(encoder)) != WBXML_OK)
                return ret;

            WBXML_DEBUG((WBXML_ENCODER, "End Element"));
        }
    }

    /* Reset Current Tag */
    encoder->current_tag = NULL;

    /* Always add an XML End Element tag */
    if ((encoder->output_type == WBXML_ENCODER_OUTPUT_XML) &&
        (node->type == WBXML_TREE_ELEMENT_NODE))
    {
        if ((ret = xml_encode_end_tag(encoder, node)) != WBXML_OK)
            return ret;

        WBXML_DEBUG((WBXML_ENCODER, "End Element"));
    }

    /* Parse next node */
    if (node->next != NULL)
        return parse_node(encoder, node->next);
    else
        return WBXML_OK;
}


/**
 * @brief Parse an XML Element
 * @param encoder The WBXML Encoder
 * @param node The element to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_element(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    WBXMLTreeAttribute *attribute = NULL;
    WB_UTINY ret = WBXML_OK;

    WBXML_DEBUG((WBXML_ENCODER, "Element: <%s>", wbxml_tag_get_xml_name(node->name)));

    /* Encode: Element Name */
    switch (encoder->output_type) {
    case WBXML_ENCODER_OUTPUT_WBXML:
        if ((ret = wbxml_encode_tag(encoder, node)) != WBXML_OK)
            return (WBXMLError)ret;
        break;
    case WBXML_ENCODER_OUTPUT_XML:
        if ((ret = xml_encode_tag(encoder, node)) != WBXML_OK)
            return (WBXMLError)ret;
        break;
    default:
        return WBXML_ERROR_INTERNAL;
    }

    /** @todo Check handling of Namespaces */

    /** @todo For Canonical XML Output: Attributes MUST be sorted */

    /* Parse: Attributes List */
    if ((attribute = node->attrs) != NULL)
    {
		while (attribute != NULL) {
            /* Parse: Attribute */
			if ((ret = parse_attribute(encoder, attribute->attr)) != WBXML_OK)
                return (WBXMLError)ret;

			attribute = attribute->next;
		}
    }

    /* Encode: End of attributes */
    switch (encoder->output_type) {
    case WBXML_ENCODER_OUTPUT_WBXML:
        if ((node->attrs != NULL) /** @todo || (node->nsDef) */) {
            if ((ret = wbxml_encode_end(encoder)) != WBXML_OK)
                return (WBXMLError)ret;
        }
        break;
    case WBXML_ENCODER_OUTPUT_XML:
        if ((ret = xml_encode_end_attrs(encoder)) != WBXML_OK)
            return (WBXMLError)ret;
        break;
    default:
        return WBXML_ERROR_INTERNAL;
    }

    return WBXML_OK;
}


/**
 * @brief Parse an XML Attribute
 * @param encoder The WBXML Encoder
 * @param attribute The XML Attribute to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_attribute(WBXMLEncoder *encoder, WBXMLAttribute *attribute)
{
    if (encoder->tree->lang == NULL)
        return WBXML_ERROR_LANG_TABLE_UNDEFINED;

    if (encoder->tree->lang->attrTable == NULL)
        return WBXML_ERROR_ATTR_TABLE_UNDEFINED;

    /* Check that this attribute has a name */
    if (attribute->name == NULL)
        return WBXML_ERROR_XML_NULL_ATTR_NAME;

    WBXML_DEBUG((WBXML_ENCODER, "Attribute: %s = %s", wbxml_attribute_get_xml_name(attribute), wbxml_attribute_get_xml_value(attribute)));

    /* Encode: Attribute */
    switch (encoder->output_type) {
    case WBXML_ENCODER_OUTPUT_WBXML:
        return wbxml_encode_attr(encoder, attribute);
    case WBXML_ENCODER_OUTPUT_XML:
        return xml_encode_attr(encoder, attribute);
    default:
        return WBXML_ERROR_INTERNAL;
    }
}


/**
 * @brief Parse an XML Text
 * @param encoder The WBXML Encoder
 * @param node The text to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_text(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    /* If Canonical Form: "Ignorable white space is considered significant and is treated equivalently to data" */
    if (encoder->xml_gen_type != WBXML_ENCODER_XML_GEN_CANONICAL) {
        /* Ignore blank nodes */
        if ((encoder->ignore_empty_text) && (wbxml_buffer_contains_only_whitespaces(node->content)))
            return WBXML_OK;

        /* Strip Blanks */
        if (encoder->remove_text_blanks)
            wbxml_buffer_strip_blanks(node->content);
    }

    /* Encode Text */
    switch (encoder->output_type) {
    case WBXML_ENCODER_OUTPUT_WBXML:
        return wbxml_encode_value_element_buffer(encoder, wbxml_buffer_get_cstr(node->content), WBXML_VALUE_ELEMENT_CTX_CONTENT);
    case WBXML_ENCODER_OUTPUT_XML:
        return xml_encode_text(encoder, node->content);
    default:
        return WBXML_ERROR_INTERNAL;
    }

    return WBXML_OK;
}


/**
 * @brief Parse an XML CDATA
 * @param encoder The WBXML Encoder
 * @param node The CDATA to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_cdata(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    /** @todo parse_cdata() */

    return WBXML_ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Parse an XML PI
 * @param encoder The WBXML Encoder
 * @param node The PI to parse
 * @return WBXML_OK if parsing is OK, an error code otherwise
 */
static WBXMLError parse_pi(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    /** @todo parse_pi() */

    return WBXML_ERROR_NOT_IMPLEMENTED;
}


/*****************************************
 *  WBXML Output Functions
 */


/****************************
 * Build WBXML Result
 */

/**
 * @brief Build WBXML Result
 * @param encoder [in] The WBXML Encoder
 * @param wbxml [out] Resulting WBXML document
 * @param wbxml_len [out] The resulting WBXML document length
 * @return WBXML_OK if built is OK, an error code otherwise
 * @note WBXML Header = version publicid charset length
 */
static WBXMLError wbxml_build_result(WBXMLEncoder *encoder, WB_UTINY **wbxml, WB_ULONG *wbxml_len)
{
    WBXMLStringTableElement *elt = NULL;
    WBXMLBuffer *header = NULL, *pid = NULL;
    WB_ULONG public_id, public_id_index;
    WB_BOOL pi_in_strtbl = FALSE, added = FALSE;

    /* WBXML Public ID */
    public_id = encoder->tree->lang->publicID->wbxmlPublicID;

    /* Create WBXML Header buffer */
    header = wbxml_buffer_create((const WB_UTINY *)"", 0, WBXML_ENCODER_WBXML_HEADER_MALLOC_BLOCK);
    if (header == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Encode WBXML Version */
    // SHM - changed
    // if (!wbxml_buffer_append_char(header, WBXML_ENCODER_VERSION))
    if (!wbxml_buffer_append_char(header, (WB_UTINY) encoder->wbxml_version))

    {
        wbxml_buffer_destroy(header);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Encode Public ID */
    /* If WBXML Public Id is '0x01' (unknown), add the XML Public ID in the String Table */
    if (public_id == WBXML_PUBLIC_ID_UNKNOWN) {
        if (encoder->tree->lang->publicID->xmlPublicID != NULL) {
            if (((pid = wbxml_buffer_create(encoder->tree->lang->publicID->xmlPublicID,
                                            WBXML_STRLEN(encoder->tree->lang->publicID->xmlPublicID),
                                            WBXML_STRLEN(encoder->tree->lang->publicID->xmlPublicID))) == NULL) ||
                ((elt = wbxml_strtbl_element_create(pid, FALSE)) == NULL))
            {
                wbxml_buffer_destroy(pid);
                wbxml_buffer_destroy(header);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            if (!wbxml_strtbl_add_element(encoder,
                                          elt,
                                          &public_id_index,
                                          &added))
            {
                wbxml_buffer_destroy(header);
                wbxml_strtbl_element_destroy(elt);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            if (!added)
                wbxml_strtbl_element_destroy(elt);

            pi_in_strtbl = TRUE;
        }
    }

    /* publicid = mb_u_int32 | ( zero index ) */
    if (pi_in_strtbl) {
        /* Encode XML Public ID String Table index */
        if (!wbxml_buffer_append_char(header, 0x00) ||
            !wbxml_buffer_append_mb_uint_32(header, public_id_index))
        {
            wbxml_buffer_destroy(header);
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {
        /* Encode WBXML Public ID */
        if (!wbxml_buffer_append_mb_uint_32(header, public_id)) {
            wbxml_buffer_destroy(header);
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    /* Encode Charset (default: UTF-8) and String Table Length */
    /** @todo Handle correctly the charset */
    if (!wbxml_buffer_append_mb_uint_32(header, WBXML_ENCODER_DEFAULT_CHARSET) ||
        !wbxml_buffer_append_mb_uint_32(header, encoder->strstbl_len))
    {
        wbxml_buffer_destroy(header);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Result Buffer Length */
    *wbxml_len = wbxml_buffer_len(header) + encoder->strstbl_len + wbxml_buffer_len(encoder->output);

    /* Create Result Buffer */
    *wbxml = (WB_UTINY *) wbxml_malloc(*wbxml_len * sizeof(WB_UTINY));
    if (*wbxml == NULL) {
        wbxml_buffer_destroy(header);
        *wbxml_len = 0;
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Copy WBXML Header */
    memcpy(*wbxml, wbxml_buffer_get_cstr(header), wbxml_buffer_len(header));

    /* Copy WBXML String Table */
    wbxml_strtbl_construct(*wbxml + wbxml_buffer_len(header), encoder->strstbl);

    /* Copy WBXML Buffer */
    memcpy(*wbxml + wbxml_buffer_len(header) + encoder->strstbl_len, wbxml_buffer_get_cstr(encoder->output), wbxml_buffer_len(encoder->output));

    wbxml_buffer_destroy(header);

    return WBXML_OK;
}


/****************************
 * WBXML Encoding Functions
 */

/**
 * @brief Encode a WBXML End Token
 * @param encoder The WBXML Encoder
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError wbxml_encode_end(WBXMLEncoder *encoder)
{
    /* Append END */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_END))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Tag
 * @param encoder The WBXML Encoder
 * @param node The element to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError wbxml_encode_tag(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    const WBXMLTagEntry *tag = NULL;
    WB_UTINY token = 0x00, page = 0x00;

    if (node->name->type == WBXML_VALUE_TOKEN) {
        token = node->name->u.token->wbxmlToken;
        page = node->name->u.token->wbxmlCodePage;

        /* Current Tag */
        encoder->current_tag = node->name->u.token;
    }
    else {
        /* Search tag in Tags Table */
        if ((tag = wbxml_tables_get_tag_from_xml(encoder->tree->lang, wbxml_tag_get_xml_name(node->name))) != NULL)
        {
            token = tag->wbxmlToken;
            page = tag->wbxmlCodePage;

            /* Current Tag */
            encoder->current_tag = tag;
        }
        else
            encoder->current_tag = NULL;
    }

    /* Check if this element has content */
    if (node->children != NULL)
        token |= WBXML_TOKEN_WITH_CONTENT;

    /* Check if this element has attributes */
    if ((node->attrs != NULL) /** @todo || (node->nsDef) */)
        token |= WBXML_TOKEN_WITH_ATTRS;

    /* Encode Token */
    if ((token & WBXML_TOKEN_MASK) == 0x00)
        return wbxml_encode_tag_literal(encoder, (WB_UTINY *) wbxml_tag_get_xml_name(node->name), token);
    else
        return wbxml_encode_tag_token(encoder, token, page);
}


/**
 * @brief Encode a WBXML Literal Token
 * @param encoder The WBXML Encoder
 * @param tag The literal tag to encode
 * @param mask The WBXML Mask for this tag
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    stag = (literalTag index)
 *          literalTag = LITERAL | LITERAL_A | LITERAL_C | LITERAL_AC
 *          index = mb_u_int32
 */
static WBXMLError wbxml_encode_tag_literal(WBXMLEncoder *encoder, WB_UTINY *tag, WB_UTINY mask)
{
    WBXMLStringTableElement *elt = NULL;
    WBXMLBuffer *buff = NULL;
    WB_ULONG index = 0;
    WB_BOOL added = FALSE;

    /* If String Table generation is disabled, we can't generate this Literal Tag */
    if (!encoder->use_strtbl)
        return WBXML_ERROR_STRTBL_DISABLED;

    /* Add tag in String Table */
    if (((buff = wbxml_buffer_create(tag, WBXML_STRLEN(tag), WBXML_STRLEN(tag))) == NULL) ||
        ((elt = wbxml_strtbl_element_create(buff, FALSE)) == NULL) ||
        (!wbxml_strtbl_add_element(encoder, elt, &index, &added)))
    {
        wbxml_strtbl_element_destroy(elt);
        wbxml_buffer_destroy(buff);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* If already exists in String Table: clean-up */
    if (!added)
        wbxml_strtbl_element_destroy(elt);

    /* Encode literalTag index */
    if ((!wbxml_buffer_append_char(encoder->output, (WB_UTINY) (WBXML_LITERAL | mask))) ||
        (!wbxml_buffer_append_mb_uint_32(encoder->output, index)))
    {
        return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Tag Token
 * @param encoder The WBXML Encoder
 * @param token The WBXML Tag Token to encode
 * @param page The WBXML CodePage for this Token
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    element = ([switchPage] stag)
 *          switchPage = SWITCH_PAGE pageindex
 *          stag = TAG
 *          pageindex = u_int8
 */
static WBXMLError wbxml_encode_tag_token(WBXMLEncoder *encoder, WB_UTINY token, WB_UTINY page)
{
    /* Switch Page if needed */
    if (encoder->tagCodePage != page)
    {
        if ((!wbxml_buffer_append_char(encoder->output, WBXML_SWITCH_PAGE)) ||
            (!wbxml_buffer_append_char(encoder->output, page)))
        {
            return WBXML_ERROR_ENCODER_APPEND_DATA;
        }

        encoder->tagCodePage = page;
    }

    /* Add Token */
    if (!wbxml_buffer_append_char(encoder->output, token))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Attribute
 * @param encoder [in] The WBXML Encoder
 * @param attribute [in] The Attribute to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    attribute = attrStart *attrValue
 */
static WBXMLError wbxml_encode_attr(WBXMLEncoder *encoder, WBXMLAttribute *attribute)
{
    WB_UTINY *value = NULL;
    WBXMLError ret = WBXML_OK;

    /* Encode Attribute Start */
    if ((ret = wbxml_encode_attr_start(encoder, attribute, &value)) != WBXML_OK)
        return ret;

    /* Encode Attribute Value */
    if (value != NULL) {
        if ((ret = wbxml_encode_value_element_buffer(encoder, value, WBXML_VALUE_ELEMENT_CTX_ATTR)) != WBXML_OK)
            return ret;
    }

    /* Reset Current Attribute */
    encoder->current_attr = NULL;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Attribute Start
 * @param encoder [in] The WBXML Encoder
 * @param attribute [in] The Attribute
 * @param value [out] Pointer to the value to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note The 'value' result correspond to the value there is still to encode
 *       For example, in Wireless-Village, to encode:
 *              xmlns="http://www.wireless-village.org/CSP1.1"
 *       We first encode:
 *              xmlns="http://www.wireless-village.org/CSP (start token: 0x05)
 *       Then:
 *              "1.1" as an inline string
 */
static WBXMLError wbxml_encode_attr_start(WBXMLEncoder *encoder, WBXMLAttribute *attribute, WB_UTINY **value)
{
    const WBXMLAttrEntry *attr = NULL;
    WB_UTINY *value_left = NULL;
    WB_UTINY token = 0x00, page = 0x00;

    *value = wbxml_buffer_get_cstr(attribute->value);

    if (attribute->name->type == WBXML_VALUE_TOKEN) {
        /* We already have Token / Page pair for this Attribute Start */
        token = attribute->name->u.token->wbxmlToken;
        page = attribute->name->u.token->wbxmlCodePage;

        /* Current Attribute */
        encoder->current_attr = attribute->name->u.token;

        /* If there is a Start Value associated to the Attribute Name token... */
        if (attribute->name->u.token->xmlValue != NULL)
        {
            /* ... Check that we have it at start of full Attribute Value */
            if (WBXML_STRNCMP(wbxml_buffer_get_cstr(attribute->value),
                              attribute->name->u.token->xmlValue,
                              WBXML_STRLEN(attribute->name->u.token->xmlValue)) == 0)
            {
                /* Check if you have still a part in the Attribute Value to encode */
                if (wbxml_buffer_len(attribute->value) > WBXML_STRLEN(attribute->name->u.token->xmlValue))
                {
                    /* There is still a part in the Value to encode */
                    *value = wbxml_buffer_get_cstr(attribute->value) + WBXML_STRLEN(attribute->name->u.token->xmlValue);
                }
                else
                    *value = NULL;
            }
            else {
                /** @todo Should we stop everything and generate an error ? */
                WBXML_WARNING((WBXML_ENCODER, "wbxml_encode_attr_start() => Attribute Value doesn't match Attribute Token"));

                /* Current Attribute */
                encoder->current_attr = NULL;

                /* Encode Attribute Literal */
                return wbxml_encode_attr_start_literal(encoder, wbxml_attribute_get_xml_name(attribute));
            }
        }

        /* Encode Attribute Token */
        return wbxml_encode_attr_token(encoder, token, page);
    }
    else {
        /* Search in Attribute table */
        if ((attr = wbxml_tables_get_attr_from_xml(encoder->tree->lang,
                                                   attribute->name->u.token->xmlName,
                                                   wbxml_buffer_get_cstr(attribute->value),
                                                   &value_left)) != NULL)
        {
            token = attr->wbxmlToken;
            page = attr->wbxmlCodePage;

            /* Current Attribute */
            encoder->current_attr = attr;

            /* If there is still a part in Attribute Value to encode */
            *value = value_left;

            /* Encode Attribute Token */
            return wbxml_encode_attr_token(encoder, token, page);
        }
        else {
            /* Current Attribute */
            encoder->current_attr = NULL;

            /* Encode Attribute Literal */
            return wbxml_encode_attr_start_literal(encoder, wbxml_attribute_get_xml_name(attribute));
        }
    }
}


/**
 * @brief Encode a WBXML Attribute Value
 * @param encoder The WBXML Encoder
 * @param buffer The Value Element Buffer to encode
 * @param ctx Value Element Context (Attribute Value or Content)
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    attrStart = *attrValue
 *          attrValue = string | extension | entity | opaque
 *
 *   AND:   element = *content
 *          content = string | extension | entity | opaque
 */
static WBXMLError wbxml_encode_value_element_buffer(WBXMLEncoder *encoder, WB_UTINY *buffer, WBXMLValueElementCtx ctx)
{
    WBXMLList *lresult = NULL;
    WBXMLBuffer *buff = NULL;
    WBXMLValueElement *elt = NULL, *new_elt = NULL;
    WBXMLStringTableElement *strtbl_elt = NULL;
    WB_ULONG i = 0, j = 0, index = 0;
    WBXMLError ret = WBXML_OK;

    if ((buffer == NULL) || (*buffer == '\0'))
        return WBXML_OK;


    /*********************************************************
     *  Encoder Language Specific Attribute Values
     */

    if (ctx == WBXML_VALUE_ELEMENT_CTX_ATTR) {
        switch (encoder->tree->lang->publicID->wbxmlPublicID) {
        case WBXML_PUBLIC_ID_SI10:
            /* SI 1.0: Encode date for 'created' and 'si-expires' attributes */
            if ((encoder->current_attr->wbxmlCodePage == 0x00) &&
                ((encoder->current_attr->wbxmlToken == 0x0a) || (encoder->current_attr->wbxmlToken == 0x10)))
            {
                return wbxml_encode_datetime(encoder, buffer);
            }
            break;

        case WBXML_PUBLIC_ID_EMN10:
            /* EMN 1.0: Encode date for 'timestamp' attribute */
            if ((encoder->current_attr->wbxmlCodePage == 0x00) && (encoder->current_attr->wbxmlToken == 0x05))
            {
                return wbxml_encode_datetime(encoder, buffer);
            }
            break;

        default:
            break;
        }
    }


    /*********************************************************
     *  Encoder Language Specific Content Text Values
     */

    /* If this is a Text Content */
    if (ctx == WBXML_VALUE_ELEMENT_CTX_CONTENT)
    {
        /* If this is a Wireless-Village document 1.1 / 1.2 */
        if ((WBXML_STRCMP(encoder->tree->lang->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP11) == 0) ||
            (WBXML_STRCMP(encoder->tree->lang->publicID->xmlPublicID, XML_PUBLIC_ID_WV_CSP12) == 0))
        {
            /* Here we try to encode Specific WV Content. If this buffer is not a WV Data Type buffer, or
             * if it can't be FULLY encoded as an Extension Token, then this function returns WBXML_NOT_ENCODED.
             * If so, the buffer will be encoded as String latter.
             */
            if ((ret = wbxml_encode_wv_content(encoder, buffer)) != WBXML_NOT_ENCODED)
                return ret;
        }
    }


    /*********************************************************
     * @todo Search first for simple cases !
     */


    /*********************************************************
     *  We search the list of Value Elements that represents
     *  this Value buffer
     */

    /* Create Result List */
    if ((lresult = wbxml_list_create()) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Create primary Buffer */
    if ((buff = wbxml_buffer_create_from_cstr(buffer)) == NULL) {
        wbxml_list_destroy(lresult, NULL);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Create Value Element for this buffer */
    if ((elt = wbxml_value_element_create()) == NULL) {
        wbxml_buffer_destroy(buff);
        wbxml_list_destroy(lresult, NULL);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    elt->type = WBXML_VALUE_ELEMENT_STRING;
    elt->u.str = buff;

    /* Append this Buffer to Result List */
    if (!wbxml_list_append(lresult, elt)) {
        wbxml_value_element_destroy(elt);
        wbxml_list_destroy(lresult, NULL);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* If this is an Attribute Value */
    if (ctx == WBXML_VALUE_ELEMENT_CTX_ATTR)
    {
        /*********************************************************
         *  Search for Attribute Value Tokens
         */

        if (encoder->tree->lang->attrValueTable != NULL) {
            /* For each Attribute Value Token */
            j = 0;
            while (encoder->tree->lang->attrValueTable[j].xmlName != NULL)
            {
                /* For each Value Element */
                for (i = 0; i < wbxml_list_len(lresult); i++) {
                    if ((elt = (WBXMLValueElement *) wbxml_list_get(lresult, i)) == NULL)
                        continue;

                    if (elt->type != WBXML_VALUE_ELEMENT_STRING)
                        continue;

                    /* Is this Attribute Value contained in this Buffer ? */
                    if (wbxml_buffer_search_cstr(elt->u.str, encoder->tree->lang->attrValueTable[j].xmlName, 0, &index)) {
                        /* Create new Value Element */
                        if ((new_elt = wbxml_value_element_create()) == NULL) {
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }

                        new_elt->type = WBXML_VALUE_ELEMENT_ATTR_TOKEN;
                        new_elt->u.attr = &(encoder->tree->lang->attrValueTable[j]);

                        /* Insert new Value Element in List */
                        if (!wbxml_list_insert(lresult, (void *) new_elt, i + 1)) {
                            wbxml_value_element_destroy(new_elt);
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }

                        /* Check if there is still the end of the String to encode */
                        if (index + WBXML_STRLEN(encoder->tree->lang->attrValueTable[j].xmlName) < wbxml_buffer_len(elt->u.str)) {
                            /* Create new Value Element */
                            if ((new_elt = wbxml_value_element_create()) == NULL) {
                                wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                            }

                            new_elt->type = WBXML_VALUE_ELEMENT_STRING;
                            new_elt->u.str = wbxml_buffer_create_from_cstr(wbxml_buffer_get_cstr(elt->u.str) + index + WBXML_STRLEN(encoder->tree->lang->attrValueTable[j].xmlName));

                            /* Insert new Value Element in List */
                            if ((new_elt->u.str == NULL) ||
                                !wbxml_list_insert(lresult, (void *) new_elt, i + 2))
                            {
                                wbxml_value_element_destroy(new_elt);
                                wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }

                        /* Remove the Attribute Value found in Value Element */
                        wbxml_buffer_delete(elt->u.str, index, wbxml_buffer_len(elt->u.str) - index);
                    } /* if */
                } /* for */
                j++;
            } /* while */
        } /* if */
    }

    /* If this is a Text Content */
    if (ctx == WBXML_VALUE_ELEMENT_CTX_CONTENT)
    {
        /*********************************************************
         *  Search for Extension Tokens
         */

        /** @todo Finish Extension Tokens Search */

        if (encoder->tree->lang->extValueTable != NULL) {
            /* For each Extension Token */
            j = 0;
            while (encoder->tree->lang->extValueTable[j].xmlName != NULL)
            {
                /* For each Value Element */
                for (i = 0; i < wbxml_list_len(lresult); i++) {
                    if ((elt = (WBXMLValueElement *) wbxml_list_get(lresult, i)) == NULL)
                        continue;

                    if (elt->type != WBXML_VALUE_ELEMENT_STRING)
                        continue;

                    /* Ignores the "1 char Extension Tokens" */
                    if (WBXML_STRLEN(encoder->tree->lang->extValueTable[j].xmlName) < 2)
                        continue;

                    /* Is this Extension Token contained in this Buffer ? */
                    if (wbxml_buffer_search_cstr(elt->u.str, encoder->tree->lang->extValueTable[j].xmlName, 0, &index))
                    {
                        /* Create new Value Element */
                        if ((new_elt = wbxml_value_element_create()) == NULL) {
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }

                        new_elt->type = WBXML_VALUE_ELEMENT_EXTENSION;
                        new_elt->u.ext = &(encoder->tree->lang->extValueTable[j]);

                        /* Insert new Value Element in List */
                        if (!wbxml_list_insert(lresult, (void *) new_elt, i + 1)) {
                            wbxml_value_element_destroy(new_elt);
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }

                        /* Check if there is still the end of the String to encode */
                        if (index + WBXML_STRLEN(encoder->tree->lang->extValueTable[j].xmlName) < wbxml_buffer_len(elt->u.str)) {
                            /* Create new Value Element */
                            if ((new_elt = wbxml_value_element_create()) == NULL) {
                                wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                            }

                            new_elt->type = WBXML_VALUE_ELEMENT_STRING;
                            new_elt->u.str = wbxml_buffer_create_from_cstr(wbxml_buffer_get_cstr(elt->u.str) + index + WBXML_STRLEN(encoder->tree->lang->extValueTable[j].xmlName));

                            /* Insert new Value Element in List */
                            if ((new_elt->u.str == NULL) ||
                                !wbxml_list_insert(lresult, (void *) new_elt, i + 2))
                            {
                                wbxml_value_element_destroy(new_elt);
                                wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }

                        /* Remove the Attribute Value found in Value Element */
                        wbxml_buffer_delete(elt->u.str, index, wbxml_buffer_len(elt->u.str) - index);
                    } /* if */
                } /* for */
                j++;
            } /* while */
        } /* if */
    }


    /*********************************************************
     *  Search for String Table References
     */

    if (encoder->use_strtbl) {
        /* For each String Table Element */
        for (j = 0; j < wbxml_list_len(encoder->strstbl); j++) {
            if ((strtbl_elt = (WBXMLStringTableElement *) wbxml_list_get(encoder->strstbl, j)) == NULL)
                continue;

            /* For each Value Element */
            for (i = 0; i < wbxml_list_len(lresult); i++) {
                if ((elt = (WBXMLValueElement *) wbxml_list_get(lresult, i)) == NULL)
                    continue;

                if (elt->type != WBXML_VALUE_ELEMENT_STRING)
                    continue;

                /* Is the String Table Element contained in this Buffer ? */
                if (wbxml_buffer_search(elt->u.str, strtbl_elt->string, 0, &index)) {
                    /* Create new Value Element */
                    if ((new_elt = wbxml_value_element_create()) == NULL) {
                        wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                    }

                    new_elt->type = WBXML_VALUE_ELEMENT_TABLEREF;
                    new_elt->u.index = strtbl_elt->offset;

                    /* Insert new Value Element in List */
                    if (!wbxml_list_insert(lresult, (void *) new_elt, i + 1)) {
                        wbxml_value_element_destroy(new_elt);
                        wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                    }

                    /* Check if there is still the end of the String to encode */
                    if (index + wbxml_buffer_len(strtbl_elt->string) < wbxml_buffer_len(elt->u.str)) {
                        /* Create new Value Element */
                        if ((new_elt = wbxml_value_element_create()) == NULL) {
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }

                        new_elt->type = WBXML_VALUE_ELEMENT_STRING;
                        new_elt->u.str = wbxml_buffer_create_from_cstr(wbxml_buffer_get_cstr(elt->u.str) + index + wbxml_buffer_len(strtbl_elt->string));

                        /* Insert new Value Element in List */
                        if ((new_elt->u.str == NULL) ||
                            !wbxml_list_insert(lresult, (void *) new_elt, i + 2))
                        {
                            wbxml_value_element_destroy(new_elt);
                            wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);
                            return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }

                    /* Remove the Attribute Value found in Value Element */
                    wbxml_buffer_delete(elt->u.str, index, wbxml_buffer_len(elt->u.str) - index);
                } /* if */
            } /* for */
        } /* for */
    } /* if */


    /*********************************************************
     *  Encode Value Element Buffer
     */

    ret = wbxml_encode_value_element_list(encoder, lresult);

    /* Clean-up */
    wbxml_list_destroy(lresult, wbxml_value_element_destroy_item);

    return ret;
}


/**
 * @brief Encode a WBXML Value Element List
 * @param encoder The WBXML Encoder
 * @param list The Value Element list
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError wbxml_encode_value_element_list(WBXMLEncoder *encoder, WBXMLList *list)
{
    WBXMLValueElement *elt = NULL;
    WB_ULONG i = 0;
    WBXMLError ret = WBXML_OK;

    if (encoder == NULL)
        return WBXML_ERROR_INTERNAL;

    if (list == NULL)
        return WBXML_OK;

    for (i = 0; i < wbxml_list_len(list); i++) {
        if ((elt = (WBXMLValueElement *) wbxml_list_get(list, i)) == NULL)
            continue;

        switch (elt->type) {
        case WBXML_VALUE_ELEMENT_STRING:
            /* Inline String */
            if (wbxml_buffer_len(elt->u.str) > 0) {
                if ((ret = wbxml_encode_inline_string(encoder, elt->u.str)) != WBXML_OK)
                    return ret;
            }
            break;

        case WBXML_VALUE_ELEMENT_TABLEREF:
            /* String Table Reference */
            if ((ret = wbxml_encode_tableref(encoder, elt->u.index)) != WBXML_OK)
                return ret;
            break;

        case WBXML_VALUE_ELEMENT_EXTENSION:
            /* Encode Extension Token */
            if ((ret = wbxml_encode_inline_integer_extension_token(encoder, WBXML_EXT_T_0, elt->u.ext->wbxmlToken)) != WBXML_OK)
                return ret;
            break;

        case WBXML_VALUE_ELEMENT_OPAQUE:
            /** @todo Opaque */
            break;

        case WBXML_VALUE_ELEMENT_ATTR_TOKEN:
            /* Attribute Value Token */
            if ((ret = wbxml_encode_attr_token(encoder, elt->u.attr->wbxmlToken,  elt->u.attr->wbxmlCodePage)) != WBXML_OK)
                return ret;
            break;

        default:
            return WBXML_ERROR_INTERNAL;
        }
    }

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Literal Attribute Start
 * @param encoder The WBXML Encoder
 * @param attr The literal attr name to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    attrStart = (LITERAL index)
 *          index = mb_u_int32
 */
static WBXMLError wbxml_encode_attr_start_literal(WBXMLEncoder *encoder, const WB_UTINY *attr)
{
    WBXMLStringTableElement *elt = NULL;
    WBXMLBuffer *buff = NULL;
    WB_ULONG index = 0;
    WB_BOOL added = FALSE;

    /* If String Table generation is disabled, we can't generate this Literal */
    if (!encoder->use_strtbl)
        return WBXML_ERROR_STRTBL_DISABLED;

    /* Add tag in String Table */
    if (((buff = wbxml_buffer_create(attr, WBXML_STRLEN(attr), WBXML_STRLEN(attr))) == NULL) ||
        ((elt = wbxml_strtbl_element_create(buff, FALSE)) == NULL) ||
        (!wbxml_strtbl_add_element(encoder, elt, &index, &added)))
    {
        wbxml_strtbl_element_destroy(elt);
        wbxml_buffer_destroy(buff);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* If already exists in String Table: clean-up */
    if (!added)
        wbxml_strtbl_element_destroy(elt);

    /* Encode LITERAL index */
    if ((!wbxml_buffer_append_char(encoder->output, WBXML_LITERAL)) ||
        (!wbxml_buffer_append_mb_uint_32(encoder->output, index)))
    {
        return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Attribute Token
 * @param encoder The WBXML Encoder
 * @param token The WBXML Attribute Token to encode
 * @param page The WBXML CodePage for this Token
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note    attrStart = ([switchPage] ATTRSTART)
 *          switchPage = SWITCH_PAGE pageindex
 *          pageindex = u_int8
 *
 *  And:    attrValue = ([switchPage] ATTRVALUE)
 */
static WBXMLError wbxml_encode_attr_token(WBXMLEncoder *encoder, WB_UTINY token, WB_UTINY page)
{
    /* Switch Page if needed */
    if (encoder->attrCodePage != page)
    {
        if ((!wbxml_buffer_append_char(encoder->output, WBXML_SWITCH_PAGE)) ||
            (!wbxml_buffer_append_char(encoder->output, page)))
        {
            return WBXML_ERROR_ENCODER_APPEND_DATA;
        }

        encoder->attrCodePage = page;
    }

    /* Add Token */
    if (!wbxml_buffer_append_char(encoder->output, token))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Inline String
 * @param encoder The WBXML Encoder
 * @param str The String to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError wbxml_encode_inline_string(WBXMLEncoder *encoder, WBXMLBuffer *str)
{
    /* Add STR_I */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_STR_I))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add String */
    if (!wbxml_buffer_append(encoder->output, str))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Null Termination */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_STR_END))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML String Table Reference
 * @param encoder The WBXML Encoder
 * @param offset The String Table offset
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note tableref = STR_T index
 */
static WBXMLError wbxml_encode_tableref(WBXMLEncoder *encoder, WB_ULONG offset)
{
    /* Add WBXML_STR_T */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_STR_T))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add String */
    if (!wbxml_buffer_append_mb_uint_32(encoder->output, offset))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Inline Integer Extension Token
 * @param encoder The WBXML Encoder
 * @param ext Extension Type (WBXML_EXT_T_0, WBXML_EXT_T_1 or WBXML_EXT_T_2)
 * @param value The Extension Token Value
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note  (WBXML 1.3 - 5.8.4.2) Inline integrer extension token = EXT_T* mb_u_int32
 */
static WBXMLError wbxml_encode_inline_integer_extension_token(WBXMLEncoder *encoder, WB_UTINY ext, WB_UTINY value)
{
    /* Add EXT_T* */
    if (!wbxml_buffer_append_char(encoder->output, ext))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Value */
    if (!wbxml_buffer_append_mb_uint_32(encoder->output, (WB_ULONG) value))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Entity
 * @param encoder The WBXML Encoder
 * @param value The Entity to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note  (WBXML 1.3 - 5.8.4.3) entity = ENTITY entcode
 *                              entcode = mb_u_int32
 */
static WBXMLError wbxml_encode_entity(WBXMLEncoder *encoder, WB_ULONG value)
{
    /* Add ENTITY */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_ENTITY))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add entcode */
    if (!wbxml_buffer_append_mb_uint_32(encoder->output, value))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a WBXML Opaque
 * @param encoder The WBXML Encoder
 * @param buff The Buffer to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 * @note  opaque = OPAQUE length *byte
 *        length = mb_u_int32
 */
static WBXMLError wbxml_encode_opaque(WBXMLEncoder *encoder, WBXMLBuffer *buff)
{
    /* Add WBXML_OPAQUE */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_OPAQUE))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Length */
    if (!wbxml_buffer_append_mb_uint_32(encoder->output, wbxml_buffer_len(buff)))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Buffer */
    if (!wbxml_buffer_append(encoder->output, buff))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Create a WBXMLValueElement structure
 * @return The newly created WBXMLValueElement structure, or NULL if not enough memory
 */
static WBXMLValueElement *wbxml_value_element_create(void)
{
    WBXMLValueElement *elt = NULL;

    if ((elt = (WBXMLValueElement*) wbxml_malloc(sizeof(WBXMLValueElement))) == NULL)
        return NULL;

    elt->type = WBXML_VALUE_ELEMENT_STRING;
    elt->u.str = NULL;

    return elt;
}


/**
 * @brief Destroy a WBXMLValueElement structure
 * @param elt The WBXMLValueElement structure to destroy
 */
static void wbxml_value_element_destroy(WBXMLValueElement *elt)
{
    if (elt == NULL)
        return;

    switch (elt->type) {
    case WBXML_VALUE_ELEMENT_STRING:
        wbxml_buffer_destroy(elt->u.str);
        break;
    case WBXML_VALUE_ELEMENT_OPAQUE:
        wbxml_buffer_destroy(elt->u.buff);
        break;
    default:
        /* Nothing to destroy */
        break;
    }

    wbxml_free((void*) elt);
}


/**
 * @brief Destroy a WBXMLValueElement structure (for wbxml_list_destroy() function)
 * @param elt The WBXMLValueElement structure to destroy
 */
static void wbxml_value_element_destroy_item(void *elt)
{
    wbxml_value_element_destroy((WBXMLValueElement *) elt);
}


/****************************************
 * Language Specific Encoding Functions
 */

/*******************
 * SI 1.0 / SL 1.0
 */

/**
 * @brief Encode SI %Datetime attribute value
 * @param encoder The WBXML Encoder
 * @param buffer The %Datetime value to encode
 * @return WBXML_OK if encoded, another error code otherwise
 * @note [SI] - 8.2.2. Encoding of %Datetime
 */
static WBXMLError wbxml_encode_datetime(WBXMLEncoder *encoder, WB_UTINY *buffer)
{
    WBXMLBuffer *tmp = NULL;
    WB_ULONG i = 0;
    WB_UTINY ch = 0;
    WBXMLError ret = WBXML_OK;


    if ((tmp = wbxml_buffer_create_from_cstr(buffer)) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Remove non-digit characters */
    while (i < wbxml_buffer_len(tmp)) {
        /* Get char */
        if (!wbxml_buffer_get_char(tmp, i, &ch)) {
            wbxml_buffer_destroy(tmp);
            return WBXML_ERROR_INTERNAL;
        }

        if (!WBXML_ISDIGIT(ch)) {
            if ((ch != 'T') && (ch != 'Z') && (ch != '-') && (ch != ':')) {
                wbxml_buffer_destroy(tmp);
                return WBXML_ERROR_BAD_DATETIME;
            }

            /* Remove it */
            wbxml_buffer_delete(tmp, i, 1);
        }
        else
            i++;
    }

    /* Convert Ascii to Binary buffer */
    wbxml_buffer_hex_to_binary(tmp);

    /* Remove trailing zero */
    wbxml_buffer_remove_trailing_zeros(&tmp);

    /* Encode it to Opaque */
    ret = wbxml_encode_opaque(encoder, tmp);

    wbxml_buffer_destroy(tmp);

    return ret;
}


/*******************
 * WV 1.1 / WV 1.2
 */

/**
 * @brief Encode a Wireless-Village Content buffer
 * @param encoder The WBXML Encoder
 * @param buffer The buffer to encode
 * @return WBXML_OK if encoded, WBXML_NOT_ENCODED if not encoded, another error code otherwise
 * @note This function encodes a Specific WV Data Type content, or an exact Extension Token.
 *       If not found, this is not encoded... and it will be encoded latter as an Inline String
 *       in wbxml_encode_value_element_buffer(). We don't deal here if this buffer CONTAINS
 *       Extension Tokens.
 */
static WBXMLError wbxml_encode_wv_content(WBXMLEncoder *encoder, WB_UTINY *buffer)
{
    const WBXMLExtValueEntry *ext = NULL;
    WBXMLWVDataType data_type = WBXML_WV_DATA_TYPE_STRING;
    WB_ULONG ucs4_ch = 0;

    /*
     *  Specific Data Type Elements:
     *
     *        Boolean:
     *            Acceptance (0x00 / 0x05)
     *            InUse (0x00 / 0x18)
     *            Poll (0x00 / 0x21)
     *            AllFunctionsRequest (0x01 / 0x06)
     *            CapabilityRequest (0x01 / 0x0B)
     *            CompletionFlag (0x01 / 0x34)
     *            ReceiveList (0x01 / 0x36) [WV 1.2]
     *            AnyContent (0x03 / 0x09)
     *            DefaultList (0x04 / 0x0B)
     *            Auto-Subscribe (0x04 / 0x1E) [WV 1.2]
     *            DeliveryReport (0x06 / 0x08)
     *            JoinGroup (0x07 / 0x21)
     *            JoinedRequest (0x07 / 0x10)
     *            SubscribeNotification (0x07 / 0x22)
     *            CIR (0x09 / 0x05) [WV 1.2]
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

    /****************************************
     * Get the Data Type of Current Element
     */

    if (encoder->current_tag != NULL)
    {
        switch (encoder->current_tag->wbxmlCodePage) {
        case 0x00:
            /* Code Page: 0x00 */
            switch (encoder->current_tag->wbxmlToken) {
            case 0x05: /* Acceptance */
            case 0x18: /* InUse */
            case 0x21: /* Poll */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
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
            switch (encoder->current_tag->wbxmlToken) {
            case 0x06: /* AllFunctionsRequest */
            case 0x0B: /* CapabilityRequest */
            case 0x34: /* CompletionFlag */
            case 0x36: /* ReceiveList */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
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
            switch (encoder->current_tag->wbxmlToken) {
            case 0x09: /* AnyContent */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
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
        case 0x04:
            /* Code Page: 0x04 */
            switch (encoder->current_tag->wbxmlToken) {
            case 0x0B: /* DefaultList */
            case 0x1E: /* Auto-Subscribe */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x06:
            /* Code Page: 0x06 */
            switch (encoder->current_tag->wbxmlToken) {
            case 0x08: /* DeliveryReport */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
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
        case 0x07:
            /* Code Page: 0x07 */
            switch (encoder->current_tag->wbxmlToken) {
            case 0x21: /* JoinGroup */
            case 0x10: /* JoinedRequest */
            case 0x22: /* SubscribeNotification */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
            default:
                /* STRING */
                data_type = WBXML_WV_DATA_TYPE_STRING;
                break;
            }
            break;
        case 0x09:
            /* Code Page: 0x09 */
            switch (encoder->current_tag->wbxmlToken) {
            case 0x05: /* CIR */
                /* BOOLEAN */
                data_type = WBXML_WV_DATA_TYPE_BOOLEAN;
                break;
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
        }
    }


    /****************************************
     * Encode, given the Data Type
     */

    switch (data_type) {
    case WBXML_WV_DATA_TYPE_INTEGER:
        /* Integer: Encode it */
        return wbxml_encode_wv_integer(encoder, buffer);
        break;
    case WBXML_WV_DATA_TYPE_DATE_AND_TIME:
        /* Date and time can be encoded as OPAQUE data or as a string as specified in [ISO8601]. For now we
         * keep the string... but if someone wants to code the Date and time encoding function :-)
         */
        /* return wbxml_encode_wv_datetime(encoder, buffer); */
        break;
    case WBXML_WV_DATA_TYPE_BINARY:
        /** @todo Binary Encoding !! */
        break;
    case WBXML_WV_DATA_TYPE_BOOLEAN:
        /* Booleans are handled by the "T" and "F" extension tokens */
    case WBXML_WV_DATA_TYPE_STRING:
        /* Check if this buffer is an EXACT Extension Token */
        if ((ext = wbxml_tables_get_ext_from_xml(encoder->tree->lang, buffer)) != NULL)
            return wbxml_encode_inline_integer_extension_token(encoder, WBXML_EXT_T_0, ext->wbxmlToken);
        else {
            if (WBXML_STRLEN(buffer) == 1)
            {
                /**
                 * @todo [OMA WV 1.1] - 6.1 : A single character can be encoded as ENTITY (0x02) followed
                 *                           by a mb_u_int32 containing the entity number.
                 */

                /*
                if (convert_char_to_ucs4(*buffer, &ucs4_ch))
                    return wbxml_encode_entity(encoder, ucs4_ch);
                */
            }

            /* Else: noting encoded... this will be latter as an inline string */
        }
        break;
    default:
        /* Hu ? */
        break;
    }

    return WBXML_NOT_ENCODED;
}


/**
 * @brief Encode a Wireless-Village Integer
 * @param encoder The WBXML Encoder
 * @param buffer The buffer that contains the string representation of the integer to encode
 * @return WBXML_OK if OK, another error code otherwise
 */
static WBXMLError wbxml_encode_wv_integer(WBXMLEncoder *encoder, WB_UTINY *buffer)
{
    WB_UTINY octets[4];
    WB_ULONG the_int = 0, start = 0;
    WB_LONG i = 0;

    if ((encoder == NULL) || (buffer == NULL))
        return WBXML_ERROR_INTERNAL;

    the_int = (WB_ULONG) atol((const WB_TINY *) buffer);

    for (i = 3; the_int > 0 && i >= 0; i--) {
        octets[i] = (WB_UTINY)(the_int & 0xff);
        the_int >>= 8;
    }

    start = i + 1;

    /* Add WBXML_OPAQUE */
    if (!wbxml_buffer_append_char(encoder->output, WBXML_OPAQUE))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Integer Length */
    if (!wbxml_buffer_append_mb_uint_32(encoder->output, 4 - start))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Add Integer */
    if (!wbxml_buffer_append_data(encoder->output, octets + start, (WB_UTINY)(4 - start)))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode WV Date and Time content value
 * @param encoder The WBXML Encoder
 * @param buffer The Date and Time value to encode
 * @return WBXML_OK if encoded, another error code otherwise
 * @note [WV] - 6.6 Date and Time
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
static WBXMLError wbxml_encode_wv_datetime(WBXMLEncoder *encoder, WB_UTINY *buffer)
{
    /** @todo Finish wbxml_encode_wv_datetime() */

    return WBXML_ERROR_NOT_IMPLEMENTED;

#if 0
    WBXMLBuffer *tmp = NULL;
    WB_ULONG i = 0, len = 0;
    WB_UTINY ch = 0;
    WBXMLError ret = WBXML_OK;
    WB_BOOL is_utc = FALSE;

    len = WBXML_STRLEN(buffer);

    /* Check Length */
    if ((len != 15) && (len != 16))
        return WBXML_ERROR_WV_DATETIME_FORMAT;

    /* Check position of 'T' */
    if ((*buffer)[8] != 'T')
        return WBXML_ERROR_WV_DATETIME_FORMAT;

    /* Check position of 'Z' */
    if (len == 16) {
        if ((*buffer)[15] != 'Z')
            return WBXML_ERROR_WV_DATETIME_FORMAT;

        /* This is an UTC format */
        is_utc = TRUE;
    }

    /* Create temp Buffer */
    if ((tmp = wbxml_buffer_create_from_cstr(buffer)) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Delete 'T' and 'Z' */
    if (is_utc)
        wbxml_buffer_delete(tmp, 15, 1);

    wbxml_buffer_delete(tmp, 8, 1);

    /* Check if you have only digits characters */
    while (i < wbxml_buffer_len(tmp)) {
        /* Get char */
        if (!wbxml_buffer_get_char(tmp, i, &ch)) {
            wbxml_buffer_destroy(tmp);
            return WBXML_ERROR_INTERNAL;
        }

        if (!WBXML_ISDIGIT(ch)) {
            wbxml_buffer_destroy(tmp);
            return WBXML_ERROR_WV_DATETIME_FORMAT;
        }
        else
            i++;
    }

    /* Convert Ascii to Binary buffer */
    wbxml_buffer_hex_to_binary(tmp);

    /* Set Year */

    /* Set Month */

    /* Set Day */

    /* Set Hour */

    /* Set Minute */

    /* Set Second */

    /* Set Time Zone */

    /* Encode it to Opaque */
    ret = wbxml_encode_opaque(encoder, tmp);

    wbxml_buffer_destroy(tmp);

    return ret;
#endif /* 0 */
}



/****************************
 * String Table Functions
 */

/**
 * @brief Create a String Table element
 * @param string The WBXMLBuffer containing the String
 * @param is_stat If set to TRUE, this Buffer won't be destroyed in wbxml_strtbl_element_destroy() function
 * @return The newly created String Table Element, or NULL if not enought memory
 */
static WBXMLStringTableElement *wbxml_strtbl_element_create(WBXMLBuffer *string, WB_BOOL is_stat)
{
    WBXMLStringTableElement *elt = NULL;

    if ((elt = (WBXMLStringTableElement *) wbxml_malloc(sizeof(WBXMLStringTableElement))) == NULL)
        return NULL;

    elt->string = string;
    elt->offset = 0;
    elt->count = 0;
    elt->stat = is_stat;

    return elt;
}


/**
 * @brief Destroy a String Table element
 * @param element The element to destroy
 */
static void wbxml_strtbl_element_destroy(WBXMLStringTableElement *element)
{
    if (element == NULL)
        return;

    if (!element->stat)
        wbxml_buffer_destroy(element->string);

    wbxml_free(element);
}


/**
 * @brief Destroy a String Table element (for wbxml_list_destroy())
 * @param element The element to destroy
 */
static void wbxml_strtbl_element_destroy_item(void *element)
{
    wbxml_strtbl_element_destroy((WBXMLStringTableElement *) element);
}


/**
 * @brief Initialize the String Table
 * @param encoder The WBXML Encoder
 * @param root The root element of LibXML Tree
 */
static WBXMLError wbxml_strtbl_initialize(WBXMLEncoder *encoder, WBXMLTreeNode *root)
{
    WBXMLList *strings = NULL, *one_ref = NULL;
    WBXMLError ret;

    if ((strings = wbxml_list_create()) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Collect all Strings:
     * [out] 'strings' is the list of pointers to WBXMLBuffer. This Buffers must not be freed.
     */
    wbxml_strtbl_collect_strings(encoder, root, strings);

    /* Insert, in String Table, Strings that are referenced more than one time
     * [out] 'strings' is NULL
     *       'one_ref' is the list of strings referenced only ONE time (list of WBXMLStringTableElement*)
     *       Strings referenced more than one time are added to String Table
     */
    if ((ret = wbxml_strtbl_check_references(encoder, &strings, &one_ref, TRUE)) != WBXML_OK) {
        wbxml_list_destroy(strings, NULL);
        return ret;
    }

    /* 'strings' is destroyed after call of wbxml_strtbl_check_references() */

    /* Split Strings refered only one time in Words
     * [out] 'strings' is the list of words. This words (WBXMLBuffer) must be freed
     */
    if ((ret = wbxml_strtbl_collect_words(one_ref, &strings)) != WBXML_OK) {
        wbxml_list_destroy(one_ref, wbxml_strtbl_element_destroy_item);
        return ret;
    }

    /* Destroy References List */
    wbxml_list_destroy(one_ref, wbxml_strtbl_element_destroy_item);
    one_ref = NULL;

    /* Keep Strings referenced more than one time */
    if (strings != NULL)
	    wbxml_strtbl_check_references(encoder, &strings, &one_ref, FALSE);

    /* 'strings' is destroyed after call of wbxml_strtbl_check_references() */

    /* Cleanup */
    wbxml_list_destroy(one_ref, wbxml_strtbl_element_destroy_item);

    return WBXML_OK;
}


/**
 * @brief Collect Strings in XML Document (in Text Content and Attribute Values)
 * @param encoder [in] The WBXML Encoder
 * @param node [in] The current element node of LibXML Tree
 * @param strings [out] List of WBXMLBuffer buffers corresponding to Collected Strings
 */
static void wbxml_strtbl_collect_strings(WBXMLEncoder *encoder, WBXMLTreeNode *node, WBXMLList *strings)
{
    WBXMLTreeAttribute *attribute = NULL;
    const WBXMLAttrEntry *attr_entry = NULL;
    WB_UTINY *value_left = NULL;

    switch (node->type)
    {
        case WBXML_TREE_TEXT_NODE:
            /* Ignore blank nodes */
            if (wbxml_buffer_contains_only_whitespaces(node->content))
                break;

            /** @todo Shrink / Strip Blanks */

            /* Only add this string if it is big enought */
            if (wbxml_buffer_len(node->content) > WBXML_ENCODER_STRING_TABLE_MIN) {
        	    wbxml_list_append(strings, node->content);
                WBXML_DEBUG((WBXML_ENCODER, "Strtbl - Collecting String: %s", wbxml_buffer_get_cstr(node->content)));
            }
            break;

        case WBXML_TREE_ELEMENT_NODE:
            /* Collect strings in Attributes Values too */
        	attribute = node->attrs;
        	while (attribute != NULL) {
                /* Only add this string if it is big enought */
                if (wbxml_buffer_len(attribute->attr->value) > WBXML_ENCODER_STRING_TABLE_MIN) {
                    /* This mustn't be a tokenisable Attribute Start */
                    attr_entry = wbxml_tables_get_attr_from_xml(encoder->tree->lang,
                                                       (WB_UTINY *) wbxml_attribute_get_xml_name(attribute->attr),
                                                       (WB_UTINY *) wbxml_attribute_get_xml_value(attribute->attr),
                                                       &value_left);

                    /* - If attr_entry is NULL: no Attribute Start found
                     * - If attr_entry is not NULL: and Attribute Start is found, but it can be the one with
                     *   no Attribute Value associated. So just check that the 'value_left' is the same than
                     *   the attribute value we where searching for
                     */
                    if ((attr_entry == NULL) || ((attr_entry != NULL) && (value_left == (WB_UTINY *) wbxml_attribute_get_xml_value(attribute->attr))))
                    {
                        /* It mustn't contain a tokenisable Attribute Value */
                        if (!wbxml_tables_contains_attr_value_from_xml(encoder->tree->lang,
                                                                       (WB_UTINY *) wbxml_attribute_get_xml_value(attribute->attr)))
                        {
        	                wbxml_list_append(strings, attribute->attr->value);
                            WBXML_DEBUG((WBXML_ENCODER, "Strtbl - Collecting String: %s", wbxml_buffer_get_cstr(attribute->attr->value)));
                        }
                    }
                }
            	attribute = attribute->next;
        	}
        	break;

        default:
            /* NOOP */
    	    break;
    }

    if (node->children != NULL)
    	wbxml_strtbl_collect_strings(encoder, node->children, strings);

    if (node->next != NULL)
	    wbxml_strtbl_collect_strings(encoder, node->next, strings);
}


/**
 * @brief Split Strings into Words
 * @param elements [in] List of String Table Elements to split
 * @param result [out] Resulting list of Words
 * @return WBXML_OK is no error, another error code otherwise
 */
static WBXMLError wbxml_strtbl_collect_words(WBXMLList *elements, WBXMLList **result)
{
    WBXMLStringTableElement *elt = NULL;
    WBXMLList *list = NULL, *temp_list = NULL;
    WBXMLBuffer *word = NULL;
    WB_ULONG i = 0;

    *result = NULL;

    for (i = 0; i < wbxml_list_len(elements); i++)
    {
	    elt = (WBXMLStringTableElement *) wbxml_list_get(elements, i);

    	if (list == NULL) {
            if ((list = wbxml_buffer_split_words(elt->string)) == NULL) {
                wbxml_list_destroy(list, wbxml_buffer_destroy_item);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }
    	} else {
            if ((temp_list = wbxml_buffer_split_words(elt->string)) == NULL) {
                wbxml_list_destroy(list, wbxml_buffer_destroy_item);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            while ((word = (WBXMLBuffer *)wbxml_list_extract_first(temp_list)) != NULL) {
                if (!wbxml_list_append(list, word)) {
                    wbxml_buffer_destroy(word);
                    wbxml_list_destroy(temp_list, wbxml_buffer_destroy_item);
                    wbxml_list_destroy(list, wbxml_buffer_destroy_item);
                    return WBXML_ERROR_NOT_ENOUGH_MEMORY;
                }
            }

    	    wbxml_list_destroy(temp_list, NULL);
    	}
    }

    *result = list;

    return WBXML_OK;
}


/**
 * @brief Append the String Table to result Buffer
 * @param buff The Buffer to append the String Table to
 * @param strstbl The String Table to append
 */
static void wbxml_strtbl_construct(WB_UTINY *buff, WBXMLList *strstbl)
{
    WBXMLStringTableElement *elt = NULL;
    WB_ULONG i = 0, buff_index = 0;

    if ((buff == NULL) || (strstbl == NULL))
        return;

    for (i = 0; i < wbxml_list_len(strstbl); i++)
    {
        if ((elt = (WBXMLStringTableElement *)wbxml_list_get(strstbl, i)) == NULL)
            continue;

        memcpy(buff + buff_index, wbxml_buffer_get_cstr(elt->string), wbxml_buffer_len(elt->string));
        /* memcpy(buff + buff_index + wbxml_buffer_len(elt->string), WBXML_STR_END, 1); */
        buff[buff_index + wbxml_buffer_len(elt->string)] = WBXML_STR_END;
        buff_index += wbxml_buffer_len(elt->string) + 1;
    }
}


/**
 * @brief Check strings that have multiple references, add them to string table
 *        and return strings that have only one reference
 * @param encoder The WBXML Encoder
 * @param strings The List of Strings to check (List of WBXMLBuffer) : This list is freed by this function
 * @param one_ref List of strings that have only one reference (List of WBXMLStringTableElement)
 * @param stat_buff If set to TRUE, Buffers referenced by 'strings' must NOT be destroyed.
 * @return WBXML_OK if no error, another error code otherwise
 * @warning All elements of 'strings' list are removed from this list
 */
static WBXMLError wbxml_strtbl_check_references(WBXMLEncoder *encoder, WBXMLList **strings, WBXMLList **one_ref, WB_BOOL stat_buff)
{
    WBXMLList *referenced = NULL, *result = NULL;
    WBXMLBuffer *string = NULL;
    WBXMLStringTableElement *ref = NULL;
    WB_ULONG j = 0;
    WB_BOOL added = FALSE;

    if ((strings == NULL) || (one_ref == NULL))
        return WBXML_ERROR_INTERNAL;

    *one_ref = NULL;

    /* Create list of String References */
    if ((referenced = wbxml_list_create()) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;


    /*********************
     * Count References
     */

    while (wbxml_list_len(*strings) > 0)
    {
        string = (WBXMLBuffer *) wbxml_list_extract_first(*strings);

        /* Check if we have already found this String */
    	for (j = 0; j < wbxml_list_len(referenced); j++)
        {
            ref = (WBXMLStringTableElement *) wbxml_list_get(referenced, j);

            if (wbxml_buffer_compare(ref->string, string) == 0)
            {
                if (!stat_buff)
                    wbxml_buffer_destroy(string);

        		string = NULL;
        		ref->count++;
        		break;
    	    }
    	}

    	if (string != NULL)
        {
            /* New Reference Element */
            if ((ref = wbxml_strtbl_element_create(string, stat_buff)) == NULL)
            {
                wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);

                if (!stat_buff)
                    wbxml_list_destroy(*strings, wbxml_buffer_destroy_item);
                else
                    wbxml_list_destroy(*strings, NULL);

                *strings = NULL;
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            ref->count++;

            if (!wbxml_list_append(referenced, (void *) ref))
            {
                wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);

                if (!stat_buff)
                    wbxml_list_destroy(*strings, wbxml_buffer_destroy_item);
                else
                    wbxml_list_destroy(*strings, NULL);

                *strings = NULL;
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }
    	}
    }


    wbxml_list_destroy(*strings, NULL);
    *strings = NULL;


    /***********************************************
     * Remove Strings that have only One reference
     */

    if ((result = wbxml_list_create()) == NULL) {
        wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    while (wbxml_list_len(referenced) > 0)
    {
    	ref = (WBXMLStringTableElement *) wbxml_list_extract_first(referenced);
        if ((ref->count > 1) && (wbxml_buffer_len(ref->string) > WBXML_ENCODER_STRING_TABLE_MIN)) {
            /* Add Element to String Table */
            if (!wbxml_strtbl_add_element(encoder, ref, NULL, &added)) {
                wbxml_strtbl_element_destroy(ref);
                wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);
                wbxml_list_destroy(result, wbxml_strtbl_element_destroy_item);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }

            if (!added) {
                wbxml_strtbl_element_destroy(ref);
            }
        }
        else {
            /* Add Element in resulting 'not added in String Table' list */
            if (!wbxml_list_append(result, (void *) ref)) {
                wbxml_strtbl_element_destroy(ref);
                wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);
                wbxml_list_destroy(result, wbxml_strtbl_element_destroy_item);
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    wbxml_list_destroy(referenced, wbxml_strtbl_element_destroy_item);

    *one_ref = result;

    return WBXML_OK;
}


/**
 * @brief Add a String in String table
 * @param encoder [in] The WBXML Encoder
 * @param elt [in] The Element to add
 * @param index [out] The index in String Table
 * @param added [out] TRUE if really added, or FALSe if already in String Table
 * @return TRUE if no error, FALSE is Memory Error
 */
static WB_BOOL wbxml_strtbl_add_element(WBXMLEncoder *encoder, WBXMLStringTableElement *elt, WB_ULONG *index, WB_BOOL *added)
{
    WBXMLStringTableElement *elt_tmp = NULL;
    WB_ULONG i = 0;

    if ((encoder == NULL) || (encoder->strstbl == NULL) || (elt == NULL) || (elt->string == NULL))
        return FALSE;

    *added = FALSE;

    /* Check if this element already exists in String Table */
    for (i = 0; i < wbxml_list_len(encoder->strstbl); i++)
    {
        if ((elt_tmp = (WBXMLStringTableElement *)wbxml_list_get(encoder->strstbl, i)) == NULL)
            continue;

        if ((wbxml_buffer_len(elt_tmp->string) == wbxml_buffer_len(elt->string)) &&
            (wbxml_buffer_compare(elt_tmp->string, elt->string) == 0))
        {
            /* The String already exists in the String Table */
            if (index != NULL)
                *index = elt_tmp->offset;
            return TRUE;
        }
    }

    /* Add this string to String Table */
    elt->offset = encoder->strstbl_len;

    if (!wbxml_list_append(encoder->strstbl, (void *) elt))
        return FALSE;

    /* Index in String Table */
    if (index != NULL)
        *index = encoder->strstbl_len;

    /* New String Table length */
    encoder->strstbl_len += wbxml_buffer_len(elt->string) + 1;

    *added = TRUE;

    return TRUE;
}



/*****************************************
 *  XML Output Functions
 */


/****************************
 * Build XML Result
 */

/**
 * @brief Build XML Result
 * @param encoder [in] The WBXML Encoder
 * @param xml [out] Resulting XML document
 * @return WBXML_OK if built is OK, an error code otherwise
 */
static WBXMLError xml_build_result(WBXMLEncoder *encoder, WB_UTINY **xml)
{
    WBXMLBuffer *header = NULL;
    WB_ULONG xml_len = 0;

    /* Create Header Buffer */
    if ((header = wbxml_buffer_create((const WB_UTINY *)"", 0, WBXML_ENCODER_XML_HEADER_MALLOC_BLOCK)) == NULL)
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;

    /* Fill Header Buffer */
    if (!xml_fill_header(encoder, header)) {
        wbxml_buffer_destroy(header);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Result Buffer Length */
    xml_len = wbxml_buffer_len(header) + wbxml_buffer_len(encoder->output);

    /* Create Result Buffer */
    *xml = (WB_UTINY *) wbxml_malloc((xml_len + 1) * sizeof(WB_UTINY));
    if (*xml == NULL) {
        wbxml_buffer_destroy(header);
        return WBXML_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Copy Header to Result */
    memcpy(*xml, wbxml_buffer_get_cstr(header), wbxml_buffer_len(header));

    /* Copy XML Document to Result */
    memcpy(*xml + wbxml_buffer_len(header), wbxml_buffer_get_cstr(encoder->output), wbxml_buffer_len(encoder->output));

    /* NULL Terminated Buffer */
    (*xml)[xml_len] = '\0';

    /* Clean-up */
    wbxml_buffer_destroy(header);

    return WBXML_OK;
}


/**
 * @brief Fill the XML Header
 * @param encoder The WBXML Encoder
 * @param header The Buffer to Fill
 * @return WBXML_OK if built is OK, an error code otherwise
 */
static WB_BOOL xml_fill_header(WBXMLEncoder *encoder, WBXMLBuffer *header)
{
    if ((encoder == NULL) || (header == NULL))
        return FALSE;

    /* <?xml version=\"1.0\"?> */
    if (!wbxml_buffer_append_cstr(header, (WB_UTINY *)WBXML_ENCODER_XML_HEADER))
        return FALSE;

    /* New Line */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        if (!xml_encode_new_line(header))
            return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    /* <!DOCTYPE */
    if (!wbxml_buffer_append_cstr(header, (WB_UTINY *)WBXML_ENCODER_XML_DOCTYPE))
        return FALSE;

    /* Root Element */
    if (!wbxml_buffer_append_cstr(header, encoder->tree->lang->publicID->xmlRootElt))
        return FALSE;

    /*  PUBLIC " */
    if (!wbxml_buffer_append_cstr(header, (WB_UTINY *)WBXML_ENCODER_XML_PUBLIC))
        return FALSE;

    /* Public ID */
    if (!wbxml_buffer_append_cstr(header, encoder->tree->lang->publicID->xmlPublicID))
        return FALSE;

    /* DTD */
    if (!wbxml_buffer_append_cstr(header, (WB_UTINY *)WBXML_ENCODER_XML_DTD))
        return FALSE;

    /* DTD */
    if (!wbxml_buffer_append_cstr(header, encoder->tree->lang->publicID->xmlDTD))
        return FALSE;

    /* "> */
    if (!wbxml_buffer_append_cstr(header, (WB_UTINY *)WBXML_ENCODER_XML_END_DTD))
        return FALSE;

    /* New Line */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        if (!xml_encode_new_line(header))
            return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    return TRUE;
}


/****************************
 * XML Encoding Functions
 */

/**
 * @brief Encode an XML Tag
 * @param encoder The WBXML Encoder
 * @param node The element to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError xml_encode_tag(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    WB_UTINY i;

    /* Indent */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        for (i=0; i<(encoder->indent * encoder->indent_delta); i++) {
            if (!wbxml_buffer_append_char(encoder->output, ' '))
                return WBXML_ERROR_ENCODER_APPEND_DATA;
        }
    }

    /* Append < */
    if (!wbxml_buffer_append_char(encoder->output, '<'))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Append Element Name */
    if (!wbxml_buffer_append_cstr(encoder->output, wbxml_tag_get_xml_name(node->name)))
        return WBXML_ERROR_ENCODER_APPEND_DATA;
    // printf("%s\n", wbxml_tag_get_xml_name(node->name));

    return WBXML_OK;
}


/**
 * @brief Encode an XML End Tag
 * @param encoder The WBXML Encoder
 * @param node Tag
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError xml_encode_end_tag(WBXMLEncoder *encoder, WBXMLTreeNode *node)
{
    WB_UTINY i;

    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        /* Add a New Line if there were content in this element */
        if (encoder->in_content) {
            if (!xml_encode_new_line(encoder->output))
                return WBXML_ERROR_ENCODER_APPEND_DATA;
        }

        encoder->indent--;

        /* Indent End Element */
        for (i=0; i<(encoder->indent * encoder->indent_delta); i++) {
            if (!wbxml_buffer_append_char(encoder->output, ' '))
                return WBXML_ERROR_ENCODER_APPEND_DATA;
        }
    }

    /* Append </ */
    if (!wbxml_buffer_append_cstr(encoder->output, (WB_UTINY *)"</"))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Append Element Name */
    if (!wbxml_buffer_append_cstr(encoder->output, wbxml_tag_get_xml_name(node->name)))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Append > */
    if (!wbxml_buffer_append_char(encoder->output, '>'))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* New Line */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        if (!xml_encode_new_line(encoder->output))
            return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    encoder->in_content = FALSE;

    return WBXML_OK;
}


/**
 * @brief Encode a XML Attribute
 * @param encoder [in] The WBXML Encoder
 * @param attribute [in] The Attribute to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError xml_encode_attr(WBXMLEncoder *encoder, WBXMLAttribute *attribute)
{
    /* Append a space */
    if (!wbxml_buffer_append_char(encoder->output, ' '))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Append Attribute Name */
    if (!wbxml_buffer_append_cstr(encoder->output, wbxml_attribute_get_xml_name(attribute)))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* Append =" */
    if (!wbxml_buffer_append_cstr(encoder->output, (WB_UTINY *)"=\""))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    if (wbxml_attribute_get_xml_value(attribute) != NULL) {
        /* Normalize Attribute Value */
        if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_CANONICAL) {
            WBXMLBuffer *tmp = NULL;

            /* Work with a temporary copy */
            if ((tmp = wbxml_buffer_create_from_cstr(wbxml_attribute_get_xml_value(attribute))) == NULL)
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

            /* Normalize */
            xml_normalize(tmp);

            /* Append Attribute Value */
            if (!wbxml_buffer_append(encoder->output, tmp)) {
                wbxml_buffer_destroy(tmp);
                return WBXML_ERROR_ENCODER_APPEND_DATA;
            }

            wbxml_buffer_destroy(tmp);
        }
        else {
            /* Append Attribute Value */
            if (!wbxml_buffer_append_cstr(encoder->output, wbxml_attribute_get_xml_value(attribute)))
                return WBXML_ERROR_ENCODER_APPEND_DATA;
        }
    }

    /* Append " */
    if (!wbxml_buffer_append_char(encoder->output, '"'))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    return WBXML_OK;
}


/**
 * @brief Encode a End of XML Attributes List
 * @param encoder [in] The WBXML Encoder
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError xml_encode_end_attrs(WBXMLEncoder *encoder)
{
    /* Append > */
    if (!wbxml_buffer_append_char(encoder->output, '>'))
        return WBXML_ERROR_ENCODER_APPEND_DATA;

    /* New Line */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) {
        if (!xml_encode_new_line(encoder->output))
            return WBXML_ERROR_ENCODER_APPEND_DATA;

        encoder->indent++;
    }

    return WBXML_OK;
}


/**
 * @brief Encode an XML Text
 * @param encoder The WBXML Encoder
 * @param str The XML to encode
 * @return WBXML_OK if encoding is OK, an error code otherwise
 */
static WBXMLError xml_encode_text(WBXMLEncoder *encoder, WBXMLBuffer *str)
{
    WB_UTINY i;

    /* Indent */
    if ((encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_INDENT) &&
        (!encoder->in_content))
    {
        /* Indent Content (only indent in first call to xml_encode_text()) */
        for (i=0; i<(encoder->indent * encoder->indent_delta); i++) {
            if (!wbxml_buffer_append_char(encoder->output, ' '))
                return WBXML_ERROR_ENCODER_APPEND_DATA;
        }
    }

    /* Normalize Text */
    if (encoder->xml_gen_type == WBXML_ENCODER_XML_GEN_CANONICAL) {
        WBXMLBuffer *tmp = NULL;

        /* Work with a temporary copy */
        if ((tmp = wbxml_buffer_duplicate(str)) == NULL)
            return WBXML_ERROR_NOT_ENOUGH_MEMORY;

        /* Normalize */
        xml_normalize(tmp);

        /* Append Text */
        if (!wbxml_buffer_append(encoder->output, tmp)) {
            wbxml_buffer_destroy(tmp);
            return WBXML_ERROR_ENCODER_APPEND_DATA;
        }

        wbxml_buffer_destroy(tmp);
    }
    else {
        /* Append Text */
        if (!wbxml_buffer_append(encoder->output, str))
            return WBXML_ERROR_ENCODER_APPEND_DATA;
    }

    encoder->in_content = TRUE;

    return WBXML_OK;
}


/**
 * @brief Append a New Line to a Buffer
 * @param buff The Buffer
 * @return TRUE if added, FALSE otherwise
 */
static WB_BOOL xml_encode_new_line(WBXMLBuffer *buff)
{
    if (buff == NULL)
        return FALSE;

    return wbxml_buffer_append_data(buff, (WB_UTINY *)WBXML_ENCODER_XML_NEW_LINE, WBXML_STRLEN(WBXML_ENCODER_XML_NEW_LINE));

}


/**
 * @brief Normalize an XML Buffer
 * @param buff The Buffer
 * @return WBXML_OK if ok, an Error Code otherwise
 */
static WB_BOOL xml_normalize(WBXMLBuffer *buff)
{
	WB_ULONG i = 0;
    WB_UTINY ch;

	for (i = 0; i < wbxml_buffer_len(buff); i++) {
        if (!wbxml_buffer_get_char(buff, i, &ch))
            continue;

		switch (ch) {
		case '<':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&lt;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_lt, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '>':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&gt;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_gt, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '&':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&amp;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_amp, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '"':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&quot;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_quot, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '\r':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&#13;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_slashr, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '\n':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&#10;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_slashn, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		case '\t':
            /* Remove it */
            wbxml_buffer_delete(buff, i, 1);

            /* Write "&#9;" */
            if (!wbxml_buffer_insert_cstr(buff, (WB_UTINY *) xml_tab, i))
                return WBXML_ERROR_NOT_ENOUGH_MEMORY;

			break;

		default:
            /* Do Nothing */
			break;
		}
	}

    return WBXML_OK;
}
