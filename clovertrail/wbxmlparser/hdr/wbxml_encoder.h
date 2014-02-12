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
 * 10/20/2003 Motorola    Add support for wbxml version.
 */

/**
 * @file wbxml_encoder.h
 * @ingroup wbxml_encoder
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 11/11/02
 *
 * @brief WBXML Encoder
 */

#ifndef WBXML_ENCODER_H
#define WBXML_ENCODER_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_encoder
 *  @{ 
 */

typedef struct WBXMLEncoder_s WBXMLEncoder;

/**
 * @brief Type of XML Generation
 * @note Canonical Form is defined here: http://www.jclark.com/xml/canonxml.html
 */
typedef enum WBXMLEncoderXMLGenType_e {
    WBXML_ENCODER_XML_GEN_COMPACT = 0,  /**< Compact XML generation */
    WBXML_ENCODER_XML_GEN_INDENT,       /**< Indented XML generation */
    WBXML_ENCODER_XML_GEN_CANONICAL     /**< Canonical XML generation */
} WBXMLEncoderXMLGenType;


/* WBXMLEncoder Functions */

/**
 * @brief Create a WBXML Encoder
 * @result Return the newly created WBXMLEncoder, or NULL if not enough memory
 * @warning Do NOT use this function directly, use wbxml_encoder_create() macro instead
 */
WBXML_DECLARE(WBXMLEncoder *) wbxml_encoder_create_real(void);
#define wbxml_encoder_create() wbxml_mem_cleam(wbxml_encoder_create_real())

/**
 * @brief Destroy a WBXML Encoder
 * @param encoder The WBXMLEncoder to free
 */
WBXML_DECLARE(void) wbxml_encoder_destroy(WBXMLEncoder *encoder);


/* Possible options when generating WBXML or XML */

/**
 * @brief Set the WBXML Encoder to ignore empty texts (ie: ignorable Whitespaces) [Default: FALSE]
 * @param encoder [in] The WBXML Encoder to use
 * @param set_ignore [in] TRUE if ignore, FALSE otherwise
 * @warning This behaviour can me overriden by the WBXML_ENCODER_XML_GEN_CANONICAL mode (set by wbxml_encoder_set_xml_gen_type())
 */
WBXML_DECLARE(void) wbxml_encoder_set_ignore_empty_text(WBXMLEncoder *encoder, WB_BOOL set_ignore);

/**
 * @brief Set the WBXML Encoder to remove leading and trailing blanks in texts [Default: FALSE]
 * @param encoder [in] The WBXML Encoder to use
 * @param set_remove [in] TRUE if remove, FALSE otherwise
 * @warning This behaviour can me overriden by the WBXML_ENCODER_XML_GEN_CANONICAL mode (set by wbxml_encoder_set_xml_gen_type())
 */
WBXML_DECLARE(void) wbxml_encoder_set_remove_text_blanks(WBXMLEncoder *encoder, WB_BOOL set_remove);


/* Possible options when generating WBXML */

/**
 * @brief Set if we use String Table when Encoding into WBXML [Default: TRUE]
 * @param encoder [in] The WBXML Encoder
 * @param use_strtbl [in] TRUE if we use String Table, FALSE otherwise
 */
WBXML_DECLARE(void) wbxml_encoder_set_use_strtbl(WBXMLEncoder *encoder, WB_BOOL use_strtbl);


/**
 * @brief Set the WBXML Version of the output document, when generating WBXML [Default: 'WBXML_VERSION_TOKEN_13' (1.3)]
 * @param encoder [in] The WBXML Encoder
 * @param version [in] The WBXML Version^M
 */
WBXML_DECLARE(void) wbxml_encoder_set_wbxml_version(WBXMLEncoder *encoder, WBXMLVersion version);
                                                                                                                            

/* Possible options when generating XML */

/**
 * @brief Set the WBXML Encoder XML Generation Type, when generating XML [Default: WBXML_ENCODER_XML_GEN_COMPACT]
 * @param encoder [in] The WBXML Encoder
 * @param gen_type [in] Generation Type (cf. WBXMLEncoderXMLGen enum)
 */
WBXML_DECLARE(void) wbxml_encoder_set_xml_gen_type(WBXMLEncoder *encoder, WBXMLEncoderXMLGenType gen_type);

/**
 * @brief Set the WBXML Encoder indent, when generating XML in WBXML_ENCODER_XML_GEN_INDENT mode [Default: 0]
 * @param encoder [in] The WBXML Encoder
 * @param indent [in] If 'WBXML_ENCODER_XML_GEN_INDENT' type is used, this is the number of spaces for indent
 */
WBXML_DECLARE(void) wbxml_encoder_set_indent(WBXMLEncoder *encoder, WB_UTINY indent);


/* Encoding Functions */

/**
 * @brief Set the WBXML Tree to encode
 * @param encoder [in] The WBXML Encoder to use
 * @param tree [in] The WBXML Tree to encode
 * @note You MUST call this function before calling following wbxml_encoder_encode() or wbxml_encoder_encode_to_xml() function
 */
WBXML_DECLARE(void) wbxml_encoder_set_tree(WBXMLEncoder *encoder, WBXMLTree *tree);

/**
 * @brief Encode a WBXML Tree to WBXML
 * @param encoder [in] The WBXML Encoder to use
 * @param wbxml [out] Resulting WBXML document
 * @param wbxml_len [out] The resulting WBXML document length
 * @return Return WBXML_OK if no error, an error code otherwise
 * @warning The 'encoder->tree' WBXMLLib Tree MUST be already set with a call to wbxml_encoder_set_tree() function
 */
WBXML_DECLARE(WBXMLError) wbxml_encoder_encode(WBXMLEncoder *encoder, WB_UTINY **wbxml, WB_ULONG *wbxml_len);

/** Wrapper arround wbxml_encoder_encode() to launch a WBXML Encoding */
#define wbxml_encoder_encode_to_wbxml(a,b,c) wbxml_encoder_encode(a,b,c)

/**
 * @brief Encode a WBXML Tree to XML
 * @param encoder [in] The WBXML Encoder to use
 * @param xml [out] Resulting XML document
 * @return Return WBXML_OK if no error, an error code otherwise
 * @warning The 'encoder->tree' WBXMLLib Tree MUST be already set with a call to wbxml_encoder_set_tree() function
 */
WBXML_DECLARE(WBXMLError) wbxml_encoder_encode_to_xml(WBXMLEncoder *encoder, WB_UTINY **xml);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_ENCODER_H */
