/*
 * WBXML Lib, the WBXML Library.
 * Copyright (C) 2002-2003 Aymerick Jéhanne
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
 * @file wbxml_log.h
 * @ingroup wbxml_log
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/12/04
 *
 * @brief WBXML Log Functions
 */

#ifndef WBXML_LOG_H
#define WBXML_LOG_H

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup wbxml_log  
 *  @{ 
 */
 
/* Define this for verbose mode */
/* #define WBXML_LIB_VERBOSE */

/* Log Macros */
#ifdef WBXML_LIB_VERBOSE
#define WBXML_DEBUG(msg)   wbxml_log_error msg
#define WBXML_WARNING(msg) wbxml_log_debug msg
#define WBXML_ERROR(msg)   wbxml_log_warning msg
#else
#define WBXML_DEBUG(msg)
#define WBXML_WARNING(msg)
#define WBXML_ERROR(msg)
#endif /* WBXML_LIB_VERBOSE */

#define WBXML_PARSER 0x01
#define WBXML_ENCODER 0x02
#define WBXML_CONV 0x03


/**
 * @brief Log a DEBUG message
 * @param type Type of Message
 * @param fmt Message to log
 * @note Do NOT use this function directly, use WBXML_DEBUG() macro instead
 */
WBXML_DECLARE(void) wbxml_log_debug(WB_UTINY type, const WB_TINY *fmt, ...);

/**
 * @brief Log a WARNING message
 * @param type Type of Message
 * @param fmt Message to log
 * @note Do NOT use this function directly, use WBXML_WARNING() macro instead
 */
WBXML_DECLARE(void) wbxml_log_warning(WB_UTINY type, const WB_TINY *fmt, ...);

/**
 * @brief Log an ERROR message
 * @param type Type of Message
 * @param fmt Message to log
 * @note Do NOT use this function directly, use WBXML_ERROR() macro instead
 */
WBXML_DECLARE(void) wbxml_log_error(WB_UTINY type, const WB_TINY *fmt, ...);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WBXML_LOG_H */
