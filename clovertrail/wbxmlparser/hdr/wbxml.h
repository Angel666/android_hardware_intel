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
 * 12/04/2003 Motorola    Increase the size for multiple sessions
 */

/**
 * @file wbxml.h
 * @ingroup wbxml
 *
 * @author Aymerick J�hanne <libwbxml@jehanne.org>
 * @date 02/11/11
 *
 * @brief WBXML Lib Main Header
 */

#ifndef WBXML_H
#define WBXML_H


/** @addtogroup wbxml
 *  @{
 */

/** WBXML Parser Lib Version */
#define WBXML_LIB_VERSION "0.7.2"

/** Motorola: Total number of line in provisioning xml document. The following
 *  example would be considered to have 3 lines.
 *  <characteristic type="PORT">
 *     <parm name="PORTNBR" value="9203"/>
 *  </characteristic>
 *  For single session, it is usually less than 50 line.
 */
#define DOC_ENTRY_SIZE 300

/* WBXML Lib basic types redefinition */
#define WB_BOOL unsigned char
#define WB_UTINY unsigned char
#define WB_TINY char
#define WB_ULONG unsigned int
#define WB_LONG int

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* WBXML Lib string functions */
#define WBXML_STRLEN(a) strlen((const WB_TINY*)a)
#define WBXML_STRDUP(a) strdup((const WB_TINY*)a)
#define WBXML_STRCMP(a,b) strcmp((const WB_TINY*)a,(const WB_TINY*)b)
#define WBXML_STRNCMP(a,b,c) strncmp((const WB_TINY*)a,(const WB_TINY*)b,c)
#define WBXML_STRSTR(a,b) strstr((const WB_TINY*)a,(const WB_TINY*)b)

#define WBXML_ISDIGIT(a) isdigit(a)

/* For DLL exported functions */
#ifdef WIN32
#define WBXML_DECLARE(type) __declspec(dllexport) type __stdcall
#define WBXML_DECLARE_NONSTD(type) __declspec(dllexport) type
#else
#define WBXML_DECLARE(type) type
#define WBXML_DECLARE_NONSTD(type) type
#endif /* WIN32 */


/* WBXML Global Tokens */
#define WBXML_SWITCH_PAGE 0x00
#define WBXML_END         0x01
#define WBXML_ENTITY      0x02
#define WBXML_STR_I       0x03
#define WBXML_LITERAL     0x04
#define WBXML_EXT_I_0     0x40
#define WBXML_EXT_I_1     0x41
#define WBXML_EXT_I_2     0x42
#define WBXML_PI          0x43
#define WBXML_LITERAL_C   0x44
#define WBXML_EXT_T_0     0x80
#define WBXML_EXT_T_1     0x81
#define WBXML_EXT_T_2     0x82
#define WBXML_STR_T       0x83
#define WBXML_LITERAL_A   0x84
#define WBXML_EXT_0       0xC0
#define WBXML_EXT_1       0xC1
#define WBXML_EXT_2       0xC2
#define WBXML_OPAQUE      0xC3
#define WBXML_LITERAL_AC  0xC4

/* WBXML Tokens Masks */
#define WBXML_TOKEN_MASK              0x3F
#define WBXML_TOKEN_WITH_ATTRS        0x80
#define WBXML_TOKEN_WITH_CONTENT      0x40


/** Wireless-Village Specific Data Types */
typedef enum WBXMLWVDataType_e {
    WBXML_WV_DATA_TYPE_BOOLEAN = 0,     /**< Boolean */
    WBXML_WV_DATA_TYPE_INTEGER,         /**< Integer */
    WBXML_WV_DATA_TYPE_DATE_AND_TIME,   /**< Date and Time */
    WBXML_WV_DATA_TYPE_STRING,          /**< String */
    WBXML_WV_DATA_TYPE_BINARY           /**< Binary */
} WBXMLWVDataType;

/** WBXML Versions (WBXML tokens) */
typedef enum WBXMLVersion_e {
    WBXML_VERSION_UNKNOWN = -1, /**< Unknown WBXML Version */
    WBXML_VERSION_10 = 0x00,    /**< WBXML 1.0 Token */
    WBXML_VERSION_11 = 0x01,    /**< WBXML 1.1 Token */
    WBXML_VERSION_12 = 0x02,    /**< WBXML 1.2 Token */
    WBXML_VERSION_13 = 0x03     /**< WBXML 1.3 Token */
} WBXMLVersion;

/* Includes */
#include "wbxml_log.h"
#include "wbxml_errors.h"
#include "wbxml_mem.h"
#include "wbxml_lists.h"
#include "wbxml_buffers.h"
#include "wbxml_tables.h"
#include "wbxml_elt.h"
#include "wbxml_tree.h"
#include "wbxml_tree_clb.h"
#include "wbxml_handlers.h"
#include "wbxml_parser.h"
#include "wbxml_encoder.h"


/** @} */

#endif /* WBXML_H */
