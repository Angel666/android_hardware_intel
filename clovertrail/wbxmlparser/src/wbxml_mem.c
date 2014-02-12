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
 * @file wbxml_mem.c
 * @ingroup wbxml_mem
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/11/24
 *
 * @brief WBXML Memory Functions
 */

#include "wbxml.h"


/***************************************************
 *    Public Functions
 */

WBXML_DECLARE(void *) wbxml_malloc(size_t size)
{
#ifdef WBXML_USE_LEAKTRACKER
    return lt_malloc(size);
#else
    return malloc(size);
#endif
}


WBXML_DECLARE(void) wbxml_free(void *memblock)
{
#ifdef WBXML_USE_LEAKTRACKER
    lt_free(memblock);
#else
    free(memblock);
#endif
}


WBXML_DECLARE(void *) wbxml_realloc(void *memblock, size_t size)
{
#ifdef WBXML_USE_LEAKTRACKER
    return lt_realloc(memblock, size);
#else
    return realloc(memblock, size);
#endif
}
