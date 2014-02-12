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
 * 12/04/2003 Motorola    Increase the size for multiple sessions
 */

/**
 * @file wbxml_tree_clb.c
 * @ingroup wbxml_tree
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 03/02/22
 *
 * @brief WBXML Tree Callbacks for WBXML Parser
 */

#include "wbxml.h"
#include <string.h>

#ifdef JUIX_ATTR_VECTORS
extern WB_UTINY *attr_name_global[];
extern WB_UTINY *attr_value_global[];
extern WB_ULONG index_global;
#endif

/* Private Functions Prototypes */
static WBXMLTreeAttribute *construct_attribute_list(WBXMLAttribute **atts);
static void add_node_to_tree(WBXMLTreeNode *node, WBXMLTreeClbCtx *tree_ctx);


/***************************************************
 *    Public Functions
 */

void wbxml_tree_clb_start_document(void *ctx, WB_LONG charset, const WBXMLLangEntry *lang)
{
    WBXMLTreeClbCtx *tree_ctx = (WBXMLTreeClbCtx *) ctx;

    if (tree_ctx->error != WBXML_OK)
        return;

    tree_ctx->tree->lang = lang;
}


void wbxml_tree_clb_end_document(void *ctx)
{
    WBXMLTreeClbCtx *tree_ctx = (WBXMLTreeClbCtx *) ctx;

    if (tree_ctx->error != WBXML_OK)
        return;
}


void wbxml_tree_clb_start_element(void *ctx, WBXMLTag *element, WBXMLAttribute **attrs, WB_BOOL empty)
{
	WB_ULONG j = 0;
	WB_ULONG i = 0;
	WB_UTINY *str = NULL;
    WBXMLTreeClbCtx *tree_ctx = (WBXMLTreeClbCtx *) ctx;
	WB_ULONG public_id = tree_ctx->tree->lang->publicID->wbxmlPublicID;
    WBXMLTreeNode *node = NULL;

    if (tree_ctx->error != WBXML_OK)
        return;

    /* Create a new Node */
    if ((node = wbxml_tree_node_create(WBXML_TREE_ELEMENT_NODE)) == NULL) {
        tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    /* Set Element */
    if ((node->name = wbxml_tag_duplicate(element)) == NULL) {
        wbxml_tree_node_destroy(node);
        tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    /* Set Attributes for JUIX */
    if (attrs != NULL) {
#ifdef JUIX_ATTR_VECTORS
	while (attrs[j] != NULL) {
	    switch (public_id) { 
		case 0x0B: /* for provisioning doc */
		    //printf(" %s=\"%s\"", wbxml_attribute_get_xml_name(attrs[j]), wbxml_attribute_get_xml_value(attrs[j]));
                    if ( index_global >= DOC_ENTRY_SIZE) {
                        /* Do this check only for provisioning case. Very unlikely, SI and SL 
                         * document would have DOC_ENTRY_SIZE line of entry
                         */
                        tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
                        return;
                    }
		    str = (WB_UTINY *)wbxml_attribute_get_xml_value(attrs[j]);
		    if (j == 0) {
			attr_name_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
                        if (attr_name_global[index_global] != NULL){ 
			    memcpy(&attr_name_global[index_global][0], str, strlen((char *)str)+1);
                        }
		    }
		    else {
			attr_value_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
                        if (attr_value_global[index_global] != NULL){ 
			    memcpy(&attr_value_global[index_global][0], str, strlen((char *)str)+1);
                        }
		    }                   
		    j++;				        				
		    break;

		case 0x05:
		case 0x06:
	            if (public_id == 0x05) 
                        index_global++;  // index 0 is reserved for SI		

		    str = (WB_UTINY *)wbxml_attribute_get_xml_name(attrs[j]);
		    attr_name_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
                    if (attr_name_global[index_global] != NULL){ 
		        memcpy(&attr_name_global[index_global][0], str, strlen((char *)str)+1);
                    }

		    str = (WB_UTINY *)wbxml_attribute_get_xml_value(attrs[j]);
		    attr_value_global[index_global] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
                    if (attr_value_global[index_global] != NULL){ 
		        memcpy(&attr_value_global[index_global][0], str, strlen((char *)str)+1);
                    }

		    if (public_id == 0x06)   // for SL
			index_global++; 

		    j++;
		    break;

		    default: // do nothing, will be added if we need to parse others
			break;
	    } // end of switch
	} // end of while loop

	if (public_id != 0x06) 
	    index_global++;   // don NOT increment this for SL
#endif

        if ((node->attrs = construct_attribute_list(attrs)) == NULL) {
            wbxml_tree_node_destroy(node);
            tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
            return;
        }
    }

    /* Add this Node to Tree  */
    add_node_to_tree(node, tree_ctx);
}


void wbxml_tree_clb_end_element(void *ctx, WBXMLTag *element, WB_BOOL empty)
{
    WBXMLTreeClbCtx *tree_ctx = (WBXMLTreeClbCtx *) ctx;

    if (tree_ctx->error != WBXML_OK)
        return;

    if (tree_ctx->current == NULL) {
        tree_ctx->error = WBXML_ERROR_INTERNAL;
        return;
    }

    if (tree_ctx->current->parent == NULL) {
        /* This must be the Root Element */
        if (tree_ctx->current != tree_ctx->tree->root) {
            tree_ctx->error = WBXML_ERROR_INTERNAL;
        }
    }
    else {
        /* Go back one step upper in the tree */
        tree_ctx->current = tree_ctx->current->parent;
    }  

}


void wbxml_tree_clb_characters(void *ctx, WB_UTINY *ch, WB_ULONG start, WB_ULONG length)
{
    WBXMLTreeClbCtx *tree_ctx = (WBXMLTreeClbCtx *) ctx;
    WBXMLTreeNode *node = NULL;
	WB_UTINY *str = NULL;

    if (tree_ctx->error != WBXML_OK)
        return;

    /* Create a new Node */
    if ((node = wbxml_tree_node_create(WBXML_TREE_TEXT_NODE)) == NULL) {
        tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    /* Set Content */
    if ((node->content = wbxml_buffer_create_real(ch + start, length, length)) == NULL) {
        wbxml_tree_node_destroy(node);
        tree_ctx->error = WBXML_ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

#ifdef JUIX_ATTR_VECTORS
    // [JUIX] for SI only, always the 1st element in attr_value_global array.
    if (tree_ctx->tree->lang->publicID->wbxmlPublicID == 0x05) {  
	str = (WB_UTINY *)(node->content->data);
	attr_value_global[0] = (WB_UTINY *) wbxml_malloc(strlen((char *)str)+1);
        if (attr_value_global[0] != NULL) {
   	    memcpy(attr_value_global[0], str, strlen((char *)str)+1);
        }
    }
#endif

    /* Add this Node to Tree  */
    add_node_to_tree(node, tree_ctx);

    /* Go back one step upper in the tree */
    tree_ctx->current = tree_ctx->current->parent;
}


void wbxml_tree_clb_pi(void *ctx, const WB_UTINY *target, WB_UTINY *data)
{
    /** @todo wbxml_tree_clb_pi() */
}


/***************************************************
 *    Private Functions
 */

/**
 * @brief Construct a Tree Attribute List
 * @param attrs The Attribute Table to Duplicate
 * @return The Attribute List, or NULL if Error
 */
static WBXMLTreeAttribute *construct_attribute_list(WBXMLAttribute **attrs)
{
    WBXMLTreeAttribute *attr = NULL, *first = NULL, *curr = NULL;
    WB_ULONG i = 0;

    if (attrs == NULL)
        return NULL;

    while (attrs[i] != NULL) {
        /* Create Tree Attribute */
        if ((attr = wbxml_tree_attribute_create()) == NULL) {
            wbxml_tree_attribute_destroy(first);
            return NULL;
        }

        /* Link it to previous Attribute */
        if (curr != NULL)
            curr->next = attr;

        /* Duplicate Attribute */
        if ((attr->attr = wbxml_attribute_duplicate(attrs[i])) == NULL) {
            wbxml_tree_attribute_destroy(first);
            return NULL;
        }

        /* Keep the first one somewhere */
        if (i == 0)
            first = attr;

        /* Go to next Attribute */
        curr = attr;
        i++;
    }

    return first;
}


/**
 * @brief Add a Node to Tree
 * @param node The Node to add
 * @param tree_ctx The Tree Context
 */
static void add_node_to_tree(WBXMLTreeNode *node, WBXMLTreeClbCtx *tree_ctx)
{
    WBXMLTreeNode *parent = NULL, *tmp = NULL;

    /* This is the new Current Node */
    parent = tree_ctx->current;
    node->parent = parent;    
    tree_ctx->current = node;

    /* Check if this is the Root Element */
    if (parent != NULL) {
		/* This is not the Root Element... search for previous sibbling element */
		if (parent->children != NULL) {
            /* Add this Node to end of Sibbling Node list of Parent */
			tmp = parent->children;
			
			while (tmp->next != NULL)
				tmp = tmp->next;
			
			node->prev = tmp;
			tmp->next = node;
		}
		else {
			/* No previous sibbling element */
			parent->children = node;
		}
    }
    else {
        /* This is the Root Element */
        tree_ctx->tree->root = node;
    }
}
