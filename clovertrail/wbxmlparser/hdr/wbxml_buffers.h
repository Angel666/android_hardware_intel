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
 * @file wbxml_buffers.h 
 * @ingroup wbxml_buffers
 *
 * @brief Generic Buffers
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/03/12
 *
 * @note Original idea: Kannel Project (http://kannel.3glab.org/)
 */

#ifndef WBXML_BUFFERS_H
#define WBXML_BUFFERS_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct WBXMLBuffer_s WBXMLBuffer;
struct WBXMLBuffer_s
{
    WB_UTINY *data;             /**< The data */
    WB_ULONG len;               /**< Length of data in buffer */
    WB_ULONG malloced;          /**< Length of buffer */
    WB_ULONG malloc_block;      /**< Malloc Block Size */
};


/** @addtogroup wbxml_buffers  
 *  @{ 
 */

/**
 * @brief Create a Buffer
 * @param data The initial data for buffer
 * @param len Size of data
 * @param malloc_block Size of malloc blocks (tune this parameter to avoid too many rallocations)
 * @return The newly created Buffer, or NULL if not enought memory
 * @warning Do NOT use this function directly, use wbxml_buffer_create() macro instead
 */
WBXML_DECLARE(WBXMLBuffer *) wbxml_buffer_create_real(const WB_UTINY *data, WB_ULONG len, WB_ULONG malloc_block);

/** Wrapper around wbxml_buffer_create_real() to track Memory */
#define wbxml_buffer_create(a,b,c) wbxml_mem_cleam(wbxml_buffer_create_real(a,b,c))

/** Wrapper around wbxml_buffer_create() when creating buffer with a C String (NULL Terminated) */
#define wbxml_buffer_create_from_cstr(a) wbxml_buffer_create(a,WBXML_STRLEN(a),WBXML_STRLEN(a))

/**
 * @brief Destroy a Buffer
 * @param buff The Buffer to destroy
 */
WBXML_DECLARE(void) wbxml_buffer_destroy(WBXMLBuffer *buff);

/**
 * @brief Destroy a Buffer
 * @param buff The Buffer to destroy
 */
WBXML_DECLARE_NONSTD(void) wbxml_buffer_destroy_item(void *buff);

/**
 * @brief Duplicate a Buffer
 * @param buff The Buffer to duplicate
 * @return The duplicated buffer, or NULL if not enough memory
 */
WBXML_DECLARE(WBXMLBuffer *) wbxml_buffer_duplicate(WBXMLBuffer *buff);

/**
 * @brief Get data length of a buffer
 * @param buff The Buffer
 * @return The Buffer data length
 */
WBXML_DECLARE(WB_ULONG) wbxml_buffer_len(WBXMLBuffer *buff);

/**
 * @brief Get a byte from a Buffer
 * @param buff The Buffer
 * @param pos Byte position in buffer
 * @param result The resulting char
 * @return TRUE if OK, or FALSE if error
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_get_char(WBXMLBuffer *buff, WB_ULONG pos, WB_UTINY *result);

/**
 * @brief Set a byte in a Buffer
 * @param buff The Buffer
 * @param pos Byte position in buffer
 * @param ch The character to set
 */
WBXML_DECLARE(void) wbxml_buffer_set_char(WBXMLBuffer *buff, WB_ULONG pos, WB_UTINY ch);

/**
 * @brief Get pointer to internal buffer data
 * @param buff The Buffer
 * @return Pointer to buffer data, or "" if buffer is NULL or empty
 */
WBXML_DECLARE(WB_UTINY *) wbxml_buffer_get_cstr(WBXMLBuffer *buff);

/**
 * @brief Append a Buffer into another one
 * @param to The Buffer to modify
 * @param buff The Buffer to insert
 * @param pos The position of insertion in 'to'
 * @return TRUE if data inserted, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_insert(WBXMLBuffer *to, WBXMLBuffer *buff, WB_ULONG pos);

/**
 * @brief Append a C String into a WBXMLBuffer
 * @param to The Buffer to modify
 * @param str The BC String to insert
 * @param pos The position of insertion in 'to'
 * @return TRUE if data inserted, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_insert_cstr(WBXMLBuffer *to, WB_UTINY *str, WB_ULONG pos);

/**
 * @brief Append a Buffer to another Buffer
 * @param dest The destination Buffer
 * @param buff The Buffer to append
 * @return TRUE if buffer appended, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_append(WBXMLBuffer *dest, WBXMLBuffer *buff);


/**
 * @brief Append data to a Buffer
 * @param buff The Buffer
 * @param data Data to append
 * @param len Data length
 * @return TRUE if data appended, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_append_data(WBXMLBuffer *buff, const WB_UTINY *data, WB_ULONG len);

/**
 * @brief Append a C String (NULL terminated) to a Buffer
 * @param buff The Buffer
 * @param data String to append
 * @return TRUE if data appended, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_append_cstr(WBXMLBuffer *buff, const WB_UTINY *data);

/**
 * @brief Append a byte to a Buffer
 * @param buff The Buffer
 * @param ch Byte to append
 * @return TRUE if byte appended, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_append_char(WBXMLBuffer *buff, WB_UTINY ch);

/**
 * @brief Append a Multibyte Integer
 * @param buff The Buffer
 * @param value The value to append
 * @return TRUE if value appended, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_append_mb_uint_32(WBXMLBuffer *buff, WB_ULONG value);

/**
 * @brief Delete a range of Bytes in a Buffer
 * @param buff The Buffer
 * @param pos Position where to start deletion
 * @param len Number of bytes to delete
 */
WBXML_DECLARE(void) wbxml_buffer_delete(WBXMLBuffer *buff, WB_ULONG pos, WB_ULONG len);

/**
 * @brief Shrink all spaces in a Buffer
 * @param buff The Buffer to shrink
 * @note Replace every consecutive sequence of spaces into one unique whitespace
 */
WBXML_DECLARE(void) wbxml_buffer_shrink_blanks(WBXMLBuffer *buff);

/**
 * @brief Remove whitespaces at beginning and end of a Buffer
 * @param buff The Buffer to strip
 */
WBXML_DECLARE(void) wbxml_buffer_strip_blanks(WBXMLBuffer *buff);

/**
 * @brief Compare two Buffers
 * @param buff1
 * @param buff2
 * @return 0 if they are equal, negative if `buff1' is less than `buff2' and positive if greater
 */
WBXML_DECLARE(WB_LONG) wbxml_buffer_compare(WBXMLBuffer *buff1, WBXMLBuffer *buff2);

/**
 * @brief Split a Buffer into words at whitespace
 * @param buff The buffer to split
 * @return The List of splitted Words, or NULL if not enought memory
 * @warning Do NOT use this function directly, use wbxml_buffer_split_words() macro instead
 */
WBXML_DECLARE(WBXMLList *) wbxml_buffer_split_words_real(WBXMLBuffer *buff);
#define wbxml_buffer_split_words(a) wbxml_mem_cleam(wbxml_buffer_split_words_real(a))

/**
 * @brief Search a char in Buffer
 * @param to The buffer to search into
 * @param ch The char to search
 * @param pos Position to start searching in 'to' buffer
 * @param result The start position of char in 'to' buffer
 * @return TRUE if char successfully found in 'to' buffer, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_search_char(WBXMLBuffer *to, WB_UTINY ch, WB_ULONG pos, WB_ULONG *result);

/**
 * @brief Search a Buffer in another Buffer
 * @param to The buffer to search into
 * @param search The buffer to search
 * @param pos Position to start searching in 'to' buffer
 * @param result The start position of 'search' buffer in 'to' buffer
 * @return TRUE if successfully found 'search' in 'to' buffer, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_search(WBXMLBuffer *to, WBXMLBuffer *search, WB_ULONG pos, WB_ULONG *result);

/**
 * @brief Search a C String in a WBXMLBuffer Buffer
 * @param to The buffer to search into
 * @param search The C String to search
 * @param pos Position to start searching in 'to' buffer
 * @param result The start position of 'search' buffer in 'to' buffer
 * @return TRUE if successfully found 'search' in 'to' buffer, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_search_cstr(WBXMLBuffer *to, WB_UTINY *search, WB_ULONG pos, WB_ULONG *result);

/**
 * @brief Check if a buffer contains only Whitespaces
 * @param buffer The buffer to check
 * @return TRUE if it contains only whitespaces, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_contains_only_whitespaces(WBXMLBuffer *buffer);

/**
 * @brief Convert an Hexa buffer to Binary
 * @param buffer The buffer to convert
 */
WBXML_DECLARE(void) wbxml_buffer_hex_to_binary(WBXMLBuffer *buffer);

/**
 * @brief Convert an Binary buffer to Hexa
 * @param buffer The buffer to convert
 * @param uppercase Do we convert to Uppercase Hexa ?
 * @return TRUE if converted, FALSE otherwise
 */
WBXML_DECLARE(WB_BOOL) wbxml_buffer_binary_to_hex(WBXMLBuffer *buffer, WB_BOOL uppercase);

/**
 * @brief Remove trailing Zeros
 * @param buffer The buffer
 */
WBXML_DECLARE(void) wbxml_buffer_remove_trailing_zeros(WBXMLBuffer **buffer);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_BUFFERS_H */
