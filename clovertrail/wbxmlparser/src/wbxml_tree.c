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
 * 11/14/2003 Motorola    Add new function to add node. 
 */

/**
 * @file wbxml_tree.c
 * @ingroup wbxml_tree
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 03/02/18
 *
 * @brief WBXML Tree Functions
 */
#include "wbxml.h"


/***************************************************
 *    Public Functions
 */

/* WBXMLTreeAttribute */

WBXML_DECLARE(WBXMLTreeAttribute *) wbxml_tree_attribute_create(void)
{
    WBXMLTreeAttribute *result = NULL;
    
    if ((result = (WBXMLTreeAttribute *) wbxml_malloc(sizeof(WBXMLTreeAttribute))) == NULL)
        return NULL;

    result->attr = NULL;
    result->next = NULL;

    return result;
}


WBXML_DECLARE(void) wbxml_tree_attribute_destroy(WBXMLTreeAttribute *attr)
{
    WBXMLTreeAttribute *next = NULL;

    while (attr != NULL) 
    {
        next = attr->next;

        wbxml_attribute_destroy(attr->attr);   
        wbxml_free(attr);

        attr = next;
    }
}


/* WBXMLTreeNode */

WBXML_DECLARE(WBXMLTreeNode *) wbxml_tree_node_create(WBXMLTreeNodeType type)
{
    WBXMLTreeNode *result = NULL;
    
    if ((result = (WBXMLTreeNode *) wbxml_malloc(sizeof(WBXMLTreeNode))) == NULL)
        return NULL;

    result->type = type;
    result->name = NULL;
    result->attrs = NULL;
    result->content = NULL;

    result->parent = NULL;
    result->children = NULL;
    result->next = NULL;
    result->prev = NULL;

    return result;
}

WBXML_DECLARE(WB_BOOL) wbxml_tree_add_node(WBXMLTree *tree, WBXMLTreeNode *parent, WBXMLTreeNode *node)
{
    WBXMLTreeNode *tmp = NULL;
                                                                                                                            
    if ((tree == NULL) || (node == NULL))
        return FALSE;
                                                                                                                            
    /* Set parent to new node */
    node->parent = parent;
                                                                                                                            
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
        /* We do NOT allow replacement of an existing Tree Node */
        if (tree->root != NULL)
            return FALSE;
                                                                                                                            
        /* This is the Root Element */
        tree->root = node;
    }
                                                                                                                            
    return TRUE;
}

WBXML_DECLARE(void) wbxml_tree_node_destroy(WBXMLTreeNode *node)
{
    if (node == NULL)
        return;

    wbxml_tag_destroy(node->name);
    wbxml_tree_attribute_destroy(node->attrs);
    wbxml_buffer_destroy(node->content);

    wbxml_free(node);
}


/* WBXMLTree */

WBXML_DECLARE(WBXMLTree *) wbxml_tree_create(void)
{
    WBXMLTree *result = NULL;
    
    if ((result = (WBXMLTree *) wbxml_malloc(sizeof(WBXMLTree))) == NULL)
        return NULL;

    result->lang = NULL;
    result->root = NULL;

    return result;
}


WBXML_DECLARE(void) wbxml_tree_destroy(WBXMLTree *tree)
{
    WBXMLTreeNode *current_node = NULL, *previous_node = NULL, *tmp_node = NULL;
    WB_BOOL end_of_walk= FALSE;

    if (tree == NULL)
        return;

	/* Let's go through the tree (iteratively) to free all the nodes */
	current_node = tree->root;

	while (!end_of_walk)
    {
		if (current_node == NULL) {
			if (previous_node == NULL) {
				end_of_walk = TRUE;
				break;
			}
			else {
				if (previous_node->parent == NULL) {
					/* End of parsing, we have parsed the last child of root node */
					end_of_walk = TRUE;
					break;
				}
				else {
					/* Let's parse next child of parent element */					
					current_node = previous_node->next;
					tmp_node = previous_node->parent;

					/* Destroy this node (leaf) */
					wbxml_tree_node_destroy(previous_node);

					previous_node = tmp_node;
				}
			}
		}
		else {
			previous_node = current_node;
			current_node = current_node->children;		
		}
	}

    wbxml_tree_node_destroy(tree->root);
    wbxml_free(tree);
}
