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
 * @file wbxml_mem.h
 * @ingroup wbxml_mem
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/07/01
 *
 * @brief Memory Wrapper
 */

#ifndef WBXML_MEM_H
#define WBXML_MEM_H

#include <stdlib.h>


/* Define it to use Memory Leak Tracker */
#undef WBXML_USE_LEAKTRACKER

#ifdef WBXML_USE_LEAKTRACKER
#define wbxml_mem_cleam(ptr) (lt_claim_area(ptr))
#else
#define wbxml_mem_cleam(ptr) (ptr)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_mem  
 *  @{ 
 */

/**
 * @brief Alloc a Memory Block
 * @param size Size of Memory to alloc
 * @return The newly mlloced Memory Block, or NULL if not enought memory
 */
WBXML_DECLARE(void *) wbxml_malloc(size_t size);

/**
 * @brief Free a Memory Block
 * @param memblock The Memory Block to free
 */
WBXML_DECLARE(void) wbxml_free(void *memblock);

/**
 * @brief Realloc a Memory Block
 * @param memblock The Memory Block to realloc
 * @param size Size of Memory to realloc
 * @return The newly realloced Memory Block, or NULL if not enought memory
 */
WBXML_DECLARE(void *) wbxml_realloc(void *memblock, size_t size);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_MEM_H */
