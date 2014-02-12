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
 * @file wbxml_tree_clb.h
 * @ingroup wbxml_tree
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 03/02/22
 *
 * @brief WBXML Tree Callbacks for WBXML Parser
 */

#ifndef WBXML_TREE_CLB_H
#define WBXML_TREE_CLB_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_tree
 *  @{ 
 */

/** Parser Tester Context Structure */
typedef struct WBXMLTreeClbCtx_s {
    WBXMLTree *tree;        /**< The WBXML Tree we are constructing */
    WBXMLTreeNode *current; /**< Current Tree Node */
    WBXMLError error;       /**< Error while parsing Document */
} WBXMLTreeClbCtx;

/**
 * @brief Start Document Callback
 * @param ctx User data
 * @param charset Charset (IANA code)
 * @param lang Language Table for this Document (cf: wbxml_table.[h|c])
 */
void wbxml_tree_clb_start_document(void *ctx, WB_LONG charset, const WBXMLLangEntry *lang);

/**
 * @brief End Document Callback
 * @param ctx User data
 */
void wbxml_tree_clb_end_document(void *ctx);

/**
 * @brief Start Element Callback
 * @param ctx User data
 * @param element The Tag Element
 * @param atts The attributes attached to the element
 * @param empty Set to TRUE if this is an empty element
 */
void wbxml_tree_clb_start_element(void *ctx, WBXMLTag *element, WBXMLAttribute **atts, WB_BOOL empty);

/**
 * @brief End Element Callback
 * @param ctx User data
 * @param element The Tag Element
 * @param empty Set to TRUE if this is an empty element
 */
void wbxml_tree_clb_end_element(void *ctx, WBXMLTag *element, WB_BOOL empty);

/**
 * @brief Characters Callback
 * @param ctx User data
 * @param ch The characters
 * @param start The start position in the array
 * @param length The number of characters to read from the array
 */
void wbxml_tree_clb_characters(void *ctx, WB_UTINY *ch, WB_ULONG start, WB_ULONG length);

/**
 * @brief Processing Instruction Callback
 * @param ctx User data
 * @param target The processing instruction target.
 * @param data The processing instruction data, or null if  none was supplied. The data does
 *            not include any whitespace separating it from the target
 */
void wbxml_tree_clb_pi(void *ctx, const WB_UTINY *target, WB_UTINY *data);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_TREE_CLB_H */
