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
 * @file wbxml_tree.h
 * @ingroup wbxml_tree
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 03/02/16
 *
 * @brief WBXML Tree
 */

#ifndef WBXML_TREE_H
#define WBXML_TREE_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_tree
 *  @{ 
 */


/****************************************************
 *	WBXML Tree Structures
 */
  

/**
 * @brief WBXML Tree Node Type
 */
typedef enum WBXMLTreeNodeType_e
{
    WBXML_TREE_ELEMENT_NODE = 0, /**< Element Node */
    WBXML_TREE_TEXT_NODE,        /**< Text Node */
    WBXML_TREE_PI_NODE,          /**< PI Node */
} WBXMLTreeNodeType;


/**
 * @brief WBXML Tree Attribute structure
 */
typedef struct WBXMLTreeAttribute_s
{
    WBXMLAttribute  *attr;              /**< Attribute */
    struct WBXMLTreeAttribute_s  *next; /**< Next attribute */
} WBXMLTreeAttribute;


/**
 * @brief WBXML Tree Node structure
 */
typedef struct WBXMLTreeNode_s
{
    WBXMLTreeNodeType   type;       /**< Node Type */
    WBXMLTag            *name;      /**< Node Name (if type is 'WBXML_TREE_ELEMENT_NODE') */
    WBXMLTreeAttribute  *attrs;     /**< Node Attributes (if type is 'WBXML_TREE_ELEMENT_NODE') */
    WBXMLBuffer         *content;   /**< Node Content (if  type is 'WBXML_TREE_TEXT_NODE')  */
        
    struct WBXMLTreeNode_s  *parent;    /**< Parent Node */
    struct WBXMLTreeNode_s  *children;  /**< Children Node */
    struct WBXMLTreeNode_s  *next;      /**< Next sibling Node */
    struct WBXMLTreeNode_s  *prev;      /**< Previous sibling Node */
} WBXMLTreeNode;


/**
 * @brief WBXML Tree structure
 */
typedef struct WBXMLTree_s
{    
    const WBXMLLangEntry  *lang; /**< Language Table */
    WBXMLTreeNode   *root;       /**< Root Element */
} WBXMLTree;


/****************************************************
 *	WBXML Tree Functions
 */

/* WBXMLTreeAttribute */

/**
 * @brief Create a Tree Attribute structure
 * @return The newly created Tree Attribute, or NULL if not enough memory
 */
WBXML_DECLARE(WBXMLTreeAttribute *) wbxml_tree_attribute_create(void);

/**
 * @brief Destroy a Tree Attribute structure
 * @param attr The Tree Attribute structure to destroy
 */
WBXML_DECLARE(void) wbxml_tree_attribute_destroy(WBXMLTreeAttribute *attr);


/* WBXMLTreeNode */

/**
 * @brief Create a Tree Node structure
 * @param type Node type
 * @return The newly created Tree Node, or NULL if not enough memory
 */
WBXML_DECLARE(WBXMLTreeNode *) wbxml_tree_node_create(WBXMLTreeNodeType type);

/**
 * @brief Add a Tree Node to a Tree structure
 * @param tree   The Tree to modify
 * @param parent Parent of the new Tree Node (ie: Position where to add the new Tree Node in Tree structure)
 * @param node   The new Tree Node to add
 * @return TRUE is added, or FALSE if error.
 * @note If 'parent' is NULL: if 'tree' already have a Root Element this function returns FALSE, else 'node' becomes the Root Element of 'tree'
 */
WBXML_DECLARE(WB_BOOL) wbxml_tree_add_node(WBXMLTree *tree, WBXMLTreeNode *parent, WBXMLTreeNode *node);

/**
 * @brief Destroy a Tree Node structure
 * @param node The Tree Node structure to destroy
 */
WBXML_DECLARE(void) wbxml_tree_node_destroy(WBXMLTreeNode *node);


/* WBXMLTree */

/**
 * @brief Create a Tree structure
 * @return The newly created Tree, or NULL if not enough memory
 */
WBXML_DECLARE(WBXMLTree *) wbxml_tree_create(void);

/**
 * @brief Destroy a Tree structure
 * @param tree The Tree structure to destroy
 */
WBXML_DECLARE(void) wbxml_tree_destroy(WBXMLTree *tree);


/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_TREE_H */
