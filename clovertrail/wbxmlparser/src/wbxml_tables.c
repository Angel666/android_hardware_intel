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
 * Copyright (C) 2003,2005 Motorola Inc.
 * 
 * 10/15/2003 Adopt to JUIX use, add more strict data type 
 * 10/11/2005 Add 2 elements: "APPSRV" and "OBEX"
 */

/**
 * @file wbxml_tables.c
 * @ingroup wbxml_tables
 *
 * @author Aymerick Jéhanne <libwbxml@jehanne.org>
 * @date 02/03/17
 *
 * @brief WBXML Tables
 */
#include <string.h> /* For strcmp() */

#include "wbxml.h"

/** 
 * @brief If undefined, only the WML 1.3 tables are used for all WML versions (WML 1.0 / WML 1.1 / WML 1.2 / WML 1.3).
 *        It saves space, and, well, every handset must supports WML 1.3 right now.
 *        If defined, each version has its own exact tables.
 */
#undef WBXML_TABLES_SEPARATE_WML_VERSIONS


/**************************************
 * Languages Public IDs
 */

/* WAP */
WBXMLPublicIDEntry sv_wml10_public_id     = { WBXML_PUBLIC_ID_WML10     , (WB_UTINY *)XML_PUBLIC_ID_WML10,        (WB_UTINY *)"wml",        (WB_UTINY *)"http://www.wapforum.org/DTD/wml.xml"               };
WBXMLPublicIDEntry sv_wta10_public_id     = { WBXML_PUBLIC_ID_WTA10     , (WB_UTINY *)XML_PUBLIC_ID_WTA10,        (WB_UTINY *)"wtai",       (WB_UTINY *)"wtai.dtd"                                          };
WBXMLPublicIDEntry sv_wml11_public_id     = { WBXML_PUBLIC_ID_WML11     , (WB_UTINY *)XML_PUBLIC_ID_WML11,        (WB_UTINY *)"wml",        (WB_UTINY *)"http://www.wapforum.org/DTD/wml_1_1.dtd"           };
WBXMLPublicIDEntry sv_si10_public_id      = { WBXML_PUBLIC_ID_SI10      , (WB_UTINY *)XML_PUBLIC_ID_SI10,         (WB_UTINY *)"si",         (WB_UTINY *)"http://www.wapforum.org/DTD/si.dtd"                };
WBXMLPublicIDEntry sv_sl10_public_id      = { WBXML_PUBLIC_ID_SL10      , (WB_UTINY *)XML_PUBLIC_ID_SL10,         (WB_UTINY *)"sl",         (WB_UTINY *)"http://www.wapforum.org/DTD/sl.dtd"                };
WBXMLPublicIDEntry sv_co10_public_id      = { WBXML_PUBLIC_ID_CO10      , (WB_UTINY *)XML_PUBLIC_ID_CO10,         (WB_UTINY *)"co",         (WB_UTINY *)"http://www.wapforum.org/DTD/co_1.0.dtd"            };
WBXMLPublicIDEntry sv_channel11_public_id = { WBXML_PUBLIC_ID_CHANNEL11 , (WB_UTINY *)XML_PUBLIC_ID_CHANNEL11,    (WB_UTINY *)"channel",    (WB_UTINY *)""                                                  };
WBXMLPublicIDEntry sv_wml12_public_id     = { WBXML_PUBLIC_ID_WML12     , (WB_UTINY *)XML_PUBLIC_ID_WML12,        (WB_UTINY *)"wml",        (WB_UTINY *)"http://www.wapforum.org/DTD/wml12.dtd"             };
WBXMLPublicIDEntry sv_wml13_public_id     = { WBXML_PUBLIC_ID_WML13     , (WB_UTINY *)XML_PUBLIC_ID_WML13,        (WB_UTINY *)"wml",        (WB_UTINY *)"http://www.wapforum.org/DTD/wml13.dtd"             };
WBXMLPublicIDEntry sv_prov10_public_id    = { WBXML_PUBLIC_ID_PROV10    , (WB_UTINY *)XML_PUBLIC_ID_PROV10,       (WB_UTINY *)"wap-provisioningdoc",    (WB_UTINY *)"http://www.wapforum.org/DTD/prov.dtd"  };
WBXMLPublicIDEntry sv_wtawml12_public_id  = { WBXML_PUBLIC_ID_WTAWML12  , (WB_UTINY *)XML_PUBLIC_ID_WTAWML12,     (WB_UTINY *)"wta-wml",    (WB_UTINY *)"http://www.wapforum.org/DTD/wta-wml12.dtd"         };
WBXMLPublicIDEntry sv_channel12_public_id = { WBXML_PUBLIC_ID_CHANNEL12 , (WB_UTINY *)XML_PUBLIC_ID_CHANNEL12,    (WB_UTINY *)"channel",    (WB_UTINY *)"http://www.wapforum.org/DTD/channel12.dtd"         };
WBXMLPublicIDEntry sv_emn10_public_id     = { WBXML_PUBLIC_ID_EMN10     , (WB_UTINY *)XML_PUBLIC_ID_EMN10,        (WB_UTINY *)"emn",        (WB_UTINY *)"http://www.wapforum.org/DTD/emn.dtd"               }; 
WBXMLPublicIDEntry sv_drmrel10_public_id  = { WBXML_PUBLIC_ID_DRMREL10  , (WB_UTINY *)XML_PUBLIC_ID_DRMREL10,     (WB_UTINY *)"o-ex:rights",(WB_UTINY *)"http://www.openmobilealliance.org/DTD/drmrel10.dtd"};

/* SyncML 1.0 */
WBXMLPublicIDEntry sv_syncml_syncml10_public_id = { WBXML_PUBLIC_ID_SYNCML_SYNCML10 , (WB_UTINY *)XML_PUBLIC_ID_SYNCML_SYNCML10,    (WB_UTINY *)"SyncML",  	 (WB_UTINY *)"http://www.syncml.org/docs/syncml_represent_v10_20001207.dtd"  };  
WBXMLPublicIDEntry sv_syncml_devinf10_public_id = { WBXML_PUBLIC_ID_SYNCML_DEVINF10 , (WB_UTINY *)XML_PUBLIC_ID_SYNCML_DEVINF10,    (WB_UTINY *)"DevInf",    (WB_UTINY *)"http://www.syncml.org/docs/syncml_devinf_v10_20001207.dtd"     };

/* SyncML 1.1 */
WBXMLPublicIDEntry sv_syncml_syncml11_public_id = { WBXML_PUBLIC_ID_SYNCML_SYNCML11 , (WB_UTINY *)XML_PUBLIC_ID_SYNCML_SYNCML11,    (WB_UTINY *)"SyncML",    (WB_UTINY *)"http://www.syncml.org/docs/syncml_represent_v11_20020213.dtd"  };
WBXMLPublicIDEntry sv_syncml_devinf11_public_id = { WBXML_PUBLIC_ID_SYNCML_DEVINF11 , (WB_UTINY *)XML_PUBLIC_ID_SYNCML_DEVINF11,    (WB_UTINY *)"DevInf",    (WB_UTINY *)"http://www.syncml.org/docs/devinf_v11_20020215.dtd"            };
WBXMLPublicIDEntry sv_syncml_metinf11_public_id = { WBXML_PUBLIC_ID_SYNCML_METINF11 , (WB_UTINY *)XML_PUBLIC_ID_SYNCML_METINF11,    (WB_UTINY *)"MetInf",    (WB_UTINY *)"http://www.syncml.org/docs/syncml_metinf_v11_20020215.dtd"     };

/* OMA Wireless Village CSP 1.1 / 1.2 */
WBXMLPublicIDEntry sv_wv_csp11_public_id =        { WBXML_PUBLIC_ID_WV_CSP11 , (WB_UTINY *)XML_PUBLIC_ID_WV_CSP11, (WB_UTINY *)"WV-CSP-Message", (WB_UTINY *)"http://www.openmobilealliance.org/DTD/WV-CSP.XML"         };
WBXMLPublicIDEntry sv_wv_csp12_public_id =        { WBXML_PUBLIC_ID_WV_CSP12 , (WB_UTINY *)XML_PUBLIC_ID_WV_CSP12, (WB_UTINY *)"WV-CSP-Message", (WB_UTINY *)"http://www.openmobilealliance.org/DTD/WV-CSP.DTD"         };


/**************************************
 * Languages Tables
 */

#ifdef WBXML_TABLES_SEPARATE_WML_VERSIONS

/********************************************
 *    WML 1.0 (WAP 1.0: "WML-30-Apr-98.pdf")
 */

const WBXMLTagEntry sv_wml10_tag_table[] = {
    { (WB_UTINY *)"a",         0x00, 0x22 },
    { (WB_UTINY *)"access",    0x00, 0x23 },
    { (WB_UTINY *)"b",         0x00, 0x24 },
    { (WB_UTINY *)"big",       0x00, 0x25 },
    { (WB_UTINY *)"br",        0x00, 0x26 },
    { (WB_UTINY *)"card",      0x00, 0x27 },
    { (WB_UTINY *)"do",        0x00, 0x28 },
    { (WB_UTINY *)"em",        0x00, 0x29 },
    { (WB_UTINY *)"fieldset",  0x00, 0x2a },
    { (WB_UTINY *)"go",        0x00, 0x2b },
    { (WB_UTINY *)"head",      0x00, 0x2c },
    { (WB_UTINY *)"i",         0x00, 0x2d },
    { (WB_UTINY *)"img",       0x00, 0x2e },
    { (WB_UTINY *)"input",     0x00, 0x2f },
    { (WB_UTINY *)"meta",      0x00, 0x30 },
    { (WB_UTINY *)"noop",      0x00, 0x31 },
    { (WB_UTINY *)"prev",      0x00, 0x32 },
    { (WB_UTINY *)"onevent",   0x00, 0x33 },
    { (WB_UTINY *)"optgroup",  0x00, 0x34 },
    { (WB_UTINY *)"option",    0x00, 0x35 },
    { (WB_UTINY *)"refresh",   0x00, 0x36 },
    { (WB_UTINY *)"select",    0x00, 0x37 },
    { (WB_UTINY *)"small",     0x00, 0x38 },
    { (WB_UTINY *)"strong",    0x00, 0x39 },
    { (WB_UTINY *)"tab",       0x00, 0x3a }, /* Deprecated */
    { (WB_UTINY *)"template",  0x00, 0x3b },
    { (WB_UTINY *)"timer",     0x00, 0x3c },
    { (WB_UTINY *)"u",         0x00, 0x3d },
    { (WB_UTINY *)"var",       0x00, 0x3e },
    { (WB_UTINY *)"wml",       0x00, 0x3f },
    { NULL,        0x00, 0x00 }
};


const WBXMLAttrEntry sv_wml10_attr_table[] = {
    { (WB_UTINY *)"accept-charset",  NULL,                                0x00, 0x05 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"bottom",                            0x00, 0x06 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"center",                            0x00, 0x07 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"left",                              0x00, 0x08 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"middle",                            0x00, 0x09 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"right",                             0x00, 0x0a },
    { (WB_UTINY *)"align",           (WB_UTINY *)"top",                               0x00, 0x0b },
    { (WB_UTINY *)"alt",             NULL,                                0x00, 0x0c },
    { (WB_UTINY *)"content",         NULL,                                0x00, 0x0d },
    { (WB_UTINY *)"default",         NULL,                                0x00, 0x0e },
    { (WB_UTINY *)"domain",          NULL,                                0x00, 0x0f },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"false",                             0x00, 0x10 },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"true",                              0x00, 0x11 },
    { (WB_UTINY *)"format",          NULL,                                0x00, 0x12 },
    { (WB_UTINY *)"height",          NULL,                                0x00, 0x13 },
    { (WB_UTINY *)"hspace",          NULL,                                0x00, 0x14 },
    { (WB_UTINY *)"idefault",        NULL,                                0x00, 0x15 }, /* Deprecated */
    { (WB_UTINY *)"ikey",            NULL,                                0x00, 0x16 }, /* Deprecated */
    { (WB_UTINY *)"key",             NULL,                                0x00, 0x17 }, /* Deprecated */
    { (WB_UTINY *)"label",           NULL,                                0x00, 0x18 },
    { (WB_UTINY *)"localsrc",        NULL,                                0x00, 0x19 },
    { (WB_UTINY *)"maxlength",       NULL,                                0x00, 0x1a },
    { (WB_UTINY *)"method",          (WB_UTINY *)"get",                               0x00, 0x1b },
    { (WB_UTINY *)"method",          (WB_UTINY *)"post",                              0x00, 0x1c },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"nowrap",                            0x00, 0x1d },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"wrap",                              0x00, 0x1e },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"false",                             0x00, 0x1f },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"true",                              0x00, 0x20 },
    { (WB_UTINY *)"name",            NULL,                                0x00, 0x21 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"false",                             0x00, 0x22 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"true",                              0x00, 0x23 },
    { (WB_UTINY *)"onclick",         NULL,                                0x00, 0x24 }, /* Deprecated */
    { (WB_UTINY *)"onenterbackward", NULL,                                0x00, 0x25 },
    { (WB_UTINY *)"onenterforward",  NULL,                                0x00, 0x26 },
    { (WB_UTINY *)"ontimer",         NULL,                                0x00, 0x27 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"false",                             0x00, 0x28 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"true",                              0x00, 0x29 },
    { (WB_UTINY *)"path",            NULL,                                0x00, 0x2a },
    { (WB_UTINY *)"postdata",        NULL,                                0x00, 0x2b }, /* Deprecated */
    { (WB_UTINY *)"public",          (WB_UTINY *)"false",                             0x00, 0x2c }, /* Deprecated */
    { (WB_UTINY *)"public",          (WB_UTINY *)"true",                              0x00, 0x2d }, /* Deprecated */
    { (WB_UTINY *)"scheme",          NULL,                                0x00, 0x2e },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"false",                             0x00, 0x2f },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"true",                              0x00, 0x30 },
    { (WB_UTINY *)"size",            NULL,                                0x00, 0x31 },
    { (WB_UTINY *)"src",             NULL,                                0x00, 0x32 },
    { (WB_UTINY *)"style",           (WB_UTINY *)"list",                              0x00, 0x33 }, /* Deprecated */
    { (WB_UTINY *)"style",           (WB_UTINY *)"set",                               0x00, 0x34 }, /* Deprecated */
    { (WB_UTINY *)"tabindex",        NULL,                                0x00, 0x35 },
    { (WB_UTINY *)"title",           NULL,                                0x00, 0x36 },
    { (WB_UTINY *)"type",            NULL,                                0x00, 0x37 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"accept",                            0x00, 0x38 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"delete",                            0x00, 0x39 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"help",                              0x00, 0x3a },
    { (WB_UTINY *)"type",            (WB_UTINY *)"password",                          0x00, 0x3b },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onpick",                            0x00, 0x3c },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterbackward",                   0x00, 0x3d },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterforward",                    0x00, 0x3e },
    { (WB_UTINY *)"type",            (WB_UTINY *)"ontimer",                           0x00, 0x3f },
    { (WB_UTINY *)"type",            (WB_UTINY *)"options",                           0x00, 0x45 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"prev",                              0x00, 0x46 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"reset",                             0x00, 0x47 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"text",                              0x00, 0x48 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"vnd.",                              0x00, 0x49 },
    { (WB_UTINY *)"url",             NULL,                                0x00, 0x4a }, /* Deprecated */
    { (WB_UTINY *)"url",             (WB_UTINY *)"http://",                           0x00, 0x4b }, /* Deprecated */
    { (WB_UTINY *)"url",             (WB_UTINY *)"https://",                          0x00, 0x4c }, /* Deprecated */
    { (WB_UTINY *)"user-agent",      NULL,                                0x00, 0x4d }, /* Deprecated */
    { (WB_UTINY *)"value",           NULL,                                0x00, 0x4e },
    { (WB_UTINY *)"vspace",          NULL,                                0x00, 0x4f },
    { (WB_UTINY *)"width",           NULL,                                0x00, 0x50 },
    { (WB_UTINY *)"xml:lang",        NULL,                                0x00, 0x51 },
    { NULL,              NULL,                                0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_wml10_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { (WB_UTINY *)"accept",          0x00, 0x89 },
    { (WB_UTINY *)"bottom",          0x00, 0x8a },
    { (WB_UTINY *)"clear",           0x00, 0x8b },
    { (WB_UTINY *)"delete",          0x00, 0x8c },
    { (WB_UTINY *)"help",            0x00, 0x8d },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"http://www.",     0x00, 0x8f }, 
    { (WB_UTINY *)"http://",         0x00, 0x8e },
    { (WB_UTINY *)"https://www.",    0x00, 0x91 },
    { (WB_UTINY *)"https://",        0x00, 0x90 },    
    { (WB_UTINY *)"list",            0x00, 0x92 }, /* Deprecated */
    { (WB_UTINY *)"middle",          0x00, 0x93 },
    { (WB_UTINY *)"nowrap",          0x00, 0x94 },
    { (WB_UTINY *)"onclick",         0x00, 0x95 }, /* Deprecated */
    { (WB_UTINY *)"onenterbackward", 0x00, 0x96 },
    { (WB_UTINY *)"onenterforward",  0x00, 0x97 },
    { (WB_UTINY *)"ontimer",         0x00, 0x98 },
    { (WB_UTINY *)"options",         0x00, 0x99 },
    { (WB_UTINY *)"password",        0x00, 0x9a },
    { (WB_UTINY *)"reset",           0x00, 0x9b },
    { (WB_UTINY *)"set",             0x00, 0x9c }, /* Deprecated */
    { (WB_UTINY *)"text",            0x00, 0x9d },
    { (WB_UTINY *)"top",             0x00, 0x9e },
    { (WB_UTINY *)"unknown",         0x00, 0x9f },
    { (WB_UTINY *)"wrap",            0x00, 0xa0 },
    { (WB_UTINY *)"www.",            0x00, 0xa1 },
    { NULL,              0x00, 0x00 }
};


/***********************************************
 *    WML 1.1 (WAP 1.1: "SPEC-WML-19990616.pdf")
 */

const WBXMLTagEntry sv_wml11_tag_table[] = {
    { (WB_UTINY *)"a",         0x00, 0x1c },
    { (WB_UTINY *)"anchor",    0x00, 0x22 }, /* WML 1.1 */
    { (WB_UTINY *)"access",    0x00, 0x23 },
    { (WB_UTINY *)"b",         0x00, 0x24 },
    { (WB_UTINY *)"big",       0x00, 0x25 },
    { (WB_UTINY *)"br",        0x00, 0x26 },
    { (WB_UTINY *)"card",      0x00, 0x27 },
    { (WB_UTINY *)"do",        0x00, 0x28 },
    { (WB_UTINY *)"em",        0x00, 0x29 },
    { (WB_UTINY *)"fieldset",  0x00, 0x2a },
    { (WB_UTINY *)"go",        0x00, 0x2b },
    { (WB_UTINY *)"head",      0x00, 0x2c },
    { (WB_UTINY *)"i",         0x00, 0x2d },
    { (WB_UTINY *)"img",       0x00, 0x2e },
    { (WB_UTINY *)"input",     0x00, 0x2f },
    { (WB_UTINY *)"meta",      0x00, 0x30 },
    { (WB_UTINY *)"noop",      0x00, 0x31 },
    { (WB_UTINY *)"p",         0x00, 0x20 }, /* WML 1.1 */
    { (WB_UTINY *)"postfield", 0x00, 0x21 }, /* WML 1.1 */
    { (WB_UTINY *)"prev",      0x00, 0x32 },
    { (WB_UTINY *)"onevent",   0x00, 0x33 },
    { (WB_UTINY *)"optgroup",  0x00, 0x34 },
    { (WB_UTINY *)"option",    0x00, 0x35 },
    { (WB_UTINY *)"refresh",   0x00, 0x36 },
    { (WB_UTINY *)"select",    0x00, 0x37 },
    { (WB_UTINY *)"setvar",    0x00, 0x3e }, /* WML 1.1 */
    { (WB_UTINY *)"small",     0x00, 0x38 }, 
    { (WB_UTINY *)"strong",    0x00, 0x39 },
    { (WB_UTINY *)"table",     0x00, 0x1f }, /* WML 1.1 */
    { (WB_UTINY *)"td",        0x00, 0x1d }, /* WML 1.1 */
    { (WB_UTINY *)"template",  0x00, 0x3b },
    { (WB_UTINY *)"timer",     0x00, 0x3c },
    { (WB_UTINY *)"tr",        0x00, 0x1e }, /* WML 1.1 */
    { (WB_UTINY *)"u",         0x00, 0x3d },
    { (WB_UTINY *)"wml",       0x00, 0x3f },
    { NULL,        0x00, 0x00 }
};


const WBXMLAttrEntry sv_wml11_attr_table[] = {
    { (WB_UTINY *)"accept-charset",  NULL,                                0x00, 0x05 },
    { (WB_UTINY *)"align",           NULL,                                0x00, 0x52 }, /* WML 1.1 */
    { (WB_UTINY *)"align",           (WB_UTINY *)"bottom",                            0x00, 0x06 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"center",                            0x00, 0x07 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"left",                              0x00, 0x08 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"middle",                            0x00, 0x09 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"right",                             0x00, 0x0a },
    { (WB_UTINY *)"align",           (WB_UTINY *)"top",                               0x00, 0x0b },
    { (WB_UTINY *)"alt",             NULL,                                0x00, 0x0c },
    { (WB_UTINY *)"class",           NULL,                                0x00, 0x54 }, /* WML 1.1 */
    { (WB_UTINY *)"columns",         NULL,                                0x00, 0x53 }, /* WML 1.1 */
    { (WB_UTINY *)"content",         NULL,                                0x00, 0x0d },
    { (WB_UTINY *)"content",         (WB_UTINY *)"application/vnd.wap.wmlc;charset=", 0x00, 0x5c }, /* WML 1.1 */
    { (WB_UTINY *)"domain",          NULL,                                0x00, 0x0f },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"false",                             0x00, 0x10 },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"true",                              0x00, 0x11 },
    { (WB_UTINY *)"format",          NULL,                                0x00, 0x12 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"false",                             0x00, 0x56 }, /* WML 1.1 */
    { (WB_UTINY *)"forua",           (WB_UTINY *)"true",                              0x00, 0x57 }, /* WML 1.1 */
    { (WB_UTINY *)"height",          NULL,                                0x00, 0x13 },
    { (WB_UTINY *)"href",            NULL,                                0x00, 0x4a }, /* WML 1.1 */
    { (WB_UTINY *)"href",            (WB_UTINY *)"http://",                           0x00, 0x4b }, /* WML 1.1 */
    { (WB_UTINY *)"href",            (WB_UTINY *)"https://",                          0x00, 0x4c }, /* WML 1.1 */
    { (WB_UTINY *)"hspace",          NULL,                                0x00, 0x14 },
    { (WB_UTINY *)"http-equiv",      NULL,                                0x00, 0x5a }, /* WML 1.1 */
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Content-Type",                      0x00, 0x5b }, /* WML 1.1 */
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Expires",                           0x00, 0x5d }, /* WML 1.1 */
    { (WB_UTINY *)"id",              NULL,                                0x00, 0x55 }, /* WML 1.1 */
    { (WB_UTINY *)"ivalue",          NULL,                                0x00, 0x15 }, /* WML 1.1 */
    { (WB_UTINY *)"iname",           NULL,                                0x00, 0x16 }, /* WML 1.1 */
    { (WB_UTINY *)"label",           NULL,                                0x00, 0x18 },
    { (WB_UTINY *)"localsrc",        NULL,                                0x00, 0x19 },
    { (WB_UTINY *)"maxlength",       NULL,                                0x00, 0x1a },
    { (WB_UTINY *)"method",          (WB_UTINY *)"get",                               0x00, 0x1b },
    { (WB_UTINY *)"method",          (WB_UTINY *)"post",                              0x00, 0x1c },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"nowrap",                            0x00, 0x1d },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"wrap",                              0x00, 0x1e },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"false",                             0x00, 0x1f },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"true",                              0x00, 0x20 },
    { (WB_UTINY *)"name",            NULL,                                0x00, 0x21 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"false",                             0x00, 0x22 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"true",                              0x00, 0x23 },
    { (WB_UTINY *)"onenterbackward", NULL,                                0x00, 0x25 },
    { (WB_UTINY *)"onenterforward",  NULL,                                0x00, 0x26 },
    { (WB_UTINY *)"onpick",          NULL,                                0x00, 0x24 }, /* WML 1.1 */
    { (WB_UTINY *)"ontimer",         NULL,                                0x00, 0x27 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"false",                             0x00, 0x28 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"true",                              0x00, 0x29 },
    { (WB_UTINY *)"path",            NULL,                                0x00, 0x2a },
    { (WB_UTINY *)"scheme",          NULL,                                0x00, 0x2e },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"false",                             0x00, 0x2f },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"true",                              0x00, 0x30 },
    { (WB_UTINY *)"size",            NULL,                                0x00, 0x31 },
    { (WB_UTINY *)"src",             NULL,                                0x00, 0x32 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"http://",                           0x00, 0x58 }, /* WML 1.1 */
    { (WB_UTINY *)"src",             (WB_UTINY *)"https://",                          0x00, 0x59 }, /* WML 1.1 */
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"true",                              0x00, 0x33 }, /* WML 1.1 */
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"false",                             0x00, 0x34 }, /* WML 1.1 */
    { (WB_UTINY *)"tabindex",        NULL,                                0x00, 0x35 },
    { (WB_UTINY *)"title",           NULL,                                0x00, 0x36 },
    { (WB_UTINY *)"type",            NULL,                                0x00, 0x37 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"accept",                            0x00, 0x38 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"delete",                            0x00, 0x39 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"help",                              0x00, 0x3a },
    { (WB_UTINY *)"type",            (WB_UTINY *)"password",                          0x00, 0x3b },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onpick",                            0x00, 0x3c },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterbackward",                   0x00, 0x3d },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterforward",                    0x00, 0x3e },
    { (WB_UTINY *)"type",            (WB_UTINY *)"ontimer",                           0x00, 0x3f },
    { (WB_UTINY *)"type",            (WB_UTINY *)"options",                           0x00, 0x45 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"prev",                              0x00, 0x46 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"reset",                             0x00, 0x47 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"text",                              0x00, 0x48 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"vnd.",                              0x00, 0x49 },
    { (WB_UTINY *)"value",           NULL,                                0x00, 0x4d },
    { (WB_UTINY *)"vspace",          NULL,                                0x00, 0x4e },
    { (WB_UTINY *)"width",           NULL,                                0x00, 0x4f },
    { (WB_UTINY *)"xml:lang",        NULL,                                0x00, 0x50 },
    { NULL,              NULL,                                0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_wml11_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { (WB_UTINY *)"accept",          0x00, 0x89 },
    { (WB_UTINY *)"bottom",          0x00, 0x8a },
    { (WB_UTINY *)"clear",           0x00, 0x8b },
    { (WB_UTINY *)"delete",          0x00, 0x8c },
    { (WB_UTINY *)"help",            0x00, 0x8d },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"http://www.",     0x00, 0x8f },
    { (WB_UTINY *)"http://",         0x00, 0x8e },
    { (WB_UTINY *)"https://www.",    0x00, 0x91 },
    { (WB_UTINY *)"https://",        0x00, 0x90 },    
    { (WB_UTINY *)"middle",          0x00, 0x93 },
    { (WB_UTINY *)"nowrap",          0x00, 0x94 },
    { (WB_UTINY *)"onenterbackward", 0x00, 0x96 },
    { (WB_UTINY *)"onenterforward",  0x00, 0x97 },
    { (WB_UTINY *)"onpick",          0x00, 0x95 }, /* WML 1.1 */
    { (WB_UTINY *)"ontimer",         0x00, 0x98 },
    { (WB_UTINY *)"options",         0x00, 0x99 },
    { (WB_UTINY *)"password",        0x00, 0x9a },
    { (WB_UTINY *)"reset",           0x00, 0x9b },
    { (WB_UTINY *)"text",            0x00, 0x9d },
    { (WB_UTINY *)"top",             0x00, 0x9e },
    { (WB_UTINY *)"unknown",         0x00, 0x9f },
    { (WB_UTINY *)"wrap",            0x00, 0xa0 },
    { (WB_UTINY *)"www.",            0x00, 0xa1 },
    { NULL,              0x00, 0x00 }
};


/***********************************************
 *    WML 1.2 (WAP 1.2: "SPEC-WML-19991104.pdf")
 */

const WBXMLTagEntry sv_wml12_tag_table[] = {
    { (WB_UTINY *)"a",         0x00, 0x1c },
    { (WB_UTINY *)"anchor",    0x00, 0x22 },
    { (WB_UTINY *)"access",    0x00, 0x23 },
    { (WB_UTINY *)"b",         0x00, 0x24 },
    { (WB_UTINY *)"big",       0x00, 0x25 },
    { (WB_UTINY *)"br",        0x00, 0x26 },
    { (WB_UTINY *)"card",      0x00, 0x27 },
    { (WB_UTINY *)"do",        0x00, 0x28 },
    { (WB_UTINY *)"em",        0x00, 0x29 },
    { (WB_UTINY *)"fieldset",  0x00, 0x2a },
    { (WB_UTINY *)"go",        0x00, 0x2b },
    { (WB_UTINY *)"head",      0x00, 0x2c },
    { (WB_UTINY *)"i",         0x00, 0x2d },
    { (WB_UTINY *)"img",       0x00, 0x2e },
    { (WB_UTINY *)"input",     0x00, 0x2f },
    { (WB_UTINY *)"meta",      0x00, 0x30 },
    { (WB_UTINY *)"noop",      0x00, 0x31 },
    { (WB_UTINY *)"p",         0x00, 0x20 },
    { (WB_UTINY *)"postfield", 0x00, 0x21 },
    { (WB_UTINY *)"pre",       0x00, 0x1b },
    { (WB_UTINY *)"prev",      0x00, 0x32 },
    { (WB_UTINY *)"onevent",   0x00, 0x33 },
    { (WB_UTINY *)"optgroup",  0x00, 0x34 },
    { (WB_UTINY *)"option",    0x00, 0x35 },
    { (WB_UTINY *)"refresh",   0x00, 0x36 },
    { (WB_UTINY *)"select",    0x00, 0x37 },
    { (WB_UTINY *)"setvar",    0x00, 0x3e },
    { (WB_UTINY *)"small",     0x00, 0x38 },
    { (WB_UTINY *)"strong",    0x00, 0x39 },
    { (WB_UTINY *)"table",     0x00, 0x1f },
    { (WB_UTINY *)"td",        0x00, 0x1d },
    { (WB_UTINY *)"template",  0x00, 0x3b },
    { (WB_UTINY *)"timer",     0x00, 0x3c },
    { (WB_UTINY *)"tr",        0x00, 0x1e },
    { (WB_UTINY *)"u",         0x00, 0x3d },
    { (WB_UTINY *)"wml",       0x00, 0x3f },
    { NULL,        0x00, 0x00 }
};


const WBXMLAttrEntry sv_wml12_attr_table[] = {
    { (WB_UTINY *)"accept-charset",  NULL,                                0x00, 0x05 },
    { (WB_UTINY *)"accesskey",       NULL,                                0x00, 0x5e }, /* WML 1.2 */
    { (WB_UTINY *)"align",           NULL,                                0x00, 0x52 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"bottom",                            0x00, 0x06 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"center",                            0x00, 0x07 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"left",                              0x00, 0x08 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"middle",                            0x00, 0x09 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"right",                             0x00, 0x0a },
    { (WB_UTINY *)"align",           (WB_UTINY *)"top",                               0x00, 0x0b },
    { (WB_UTINY *)"alt",             NULL,                                0x00, 0x0c },
    { (WB_UTINY *)"class",           NULL,                                0x00, 0x54 },
    { (WB_UTINY *)"columns",         NULL,                                0x00, 0x53 },
    { (WB_UTINY *)"content",         NULL,                                0x00, 0x0d },
    { (WB_UTINY *)"content",         (WB_UTINY *)"application/vnd.wap.wmlc;charset=", 0x00, 0x5c },
    { (WB_UTINY *)"domain",          NULL,                                0x00, 0x0f },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"false",                             0x00, 0x10 },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"true",                              0x00, 0x11 },
    { (WB_UTINY *)"enctype",         NULL,                                0x00, 0x5f }, /* WML 1.2 */
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"application/x-www-form-urlencoded", 0x00, 0x60 }, /* WML 1.2 */
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"multipart/form-data",               0x00, 0x61 }, /* WML 1.2 */
    { (WB_UTINY *)"format",          NULL,                                0x00, 0x12 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"false",                             0x00, 0x56 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"true",                              0x00, 0x57 },
    { (WB_UTINY *)"height",          NULL,                                0x00, 0x13 },
    { (WB_UTINY *)"href",            NULL,                                0x00, 0x4a },
    { (WB_UTINY *)"href",            (WB_UTINY *)"http://",                           0x00, 0x4b },
    { (WB_UTINY *)"href",            (WB_UTINY *)"https://",                          0x00, 0x4c },
    { (WB_UTINY *)"hspace",          NULL,                                0x00, 0x14 },
    { (WB_UTINY *)"http-equiv",      NULL,                                0x00, 0x5a },
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Content-Type",                      0x00, 0x5b },
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Expires",                           0x00, 0x5d },
    { (WB_UTINY *)"id",              NULL,                                0x00, 0x55 },
    { (WB_UTINY *)"ivalue",          NULL,                                0x00, 0x15 },
    { (WB_UTINY *)"iname",           NULL,                                0x00, 0x16 },
    { (WB_UTINY *)"label",           NULL,                                0x00, 0x18 },
    { (WB_UTINY *)"localsrc",        NULL,                                0x00, 0x19 },
    { (WB_UTINY *)"maxlength",       NULL,                                0x00, 0x1a },
    { (WB_UTINY *)"method",          (WB_UTINY *)"get",                               0x00, 0x1b },
    { (WB_UTINY *)"method",          (WB_UTINY *)"post",                              0x00, 0x1c },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"nowrap",                            0x00, 0x1d },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"wrap",                              0x00, 0x1e },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"false",                             0x00, 0x1f },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"true",                              0x00, 0x20 },
    { (WB_UTINY *)"name",            NULL,                                0x00, 0x21 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"false",                             0x00, 0x22 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"true",                              0x00, 0x23 },
    { (WB_UTINY *)"onenterbackward", NULL,                                0x00, 0x25 },
    { (WB_UTINY *)"onenterforward",  NULL,                                0x00, 0x26 },
    { (WB_UTINY *)"onpick",          NULL,                                0x00, 0x24 },
    { (WB_UTINY *)"ontimer",         NULL,                                0x00, 0x27 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"false",                             0x00, 0x28 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"true",                              0x00, 0x29 },
    { (WB_UTINY *)"path",            NULL,                                0x00, 0x2a },
    { (WB_UTINY *)"scheme",          NULL,                                0x00, 0x2e },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"false",                             0x00, 0x2f },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"true",                              0x00, 0x30 },
    { (WB_UTINY *)"size",            NULL,                                0x00, 0x31 },
    { (WB_UTINY *)"src",             NULL,                                0x00, 0x32 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"http://",                           0x00, 0x58 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"https://",                          0x00, 0x59 },
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"true",                              0x00, 0x33 },
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"false",                             0x00, 0x34 },
    { (WB_UTINY *)"tabindex",        NULL,                                0x00, 0x35 },
    { (WB_UTINY *)"title",           NULL,                                0x00, 0x36 },
    { (WB_UTINY *)"type",            NULL,                                0x00, 0x37 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"accept",                            0x00, 0x38 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"delete",                            0x00, 0x39 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"help",                              0x00, 0x3a },
    { (WB_UTINY *)"type",            (WB_UTINY *)"password",                          0x00, 0x3b },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onpick",                            0x00, 0x3c },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterbackward",                   0x00, 0x3d },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterforward",                    0x00, 0x3e },
    { (WB_UTINY *)"type",            (WB_UTINY *)"ontimer",                           0x00, 0x3f },
    { (WB_UTINY *)"type",            (WB_UTINY *)"options",                           0x00, 0x45 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"prev",                              0x00, 0x46 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"reset",                             0x00, 0x47 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"text",                              0x00, 0x48 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"vnd.",                              0x00, 0x49 },
    { (WB_UTINY *)"value",           NULL,                                0x00, 0x4d },
    { (WB_UTINY *)"vspace",          NULL,                                0x00, 0x4e },
    { (WB_UTINY *)"width",           NULL,                                0x00, 0x4f },
    { (WB_UTINY *)"xml:lang",        NULL,                                0x00, 0x50 },
    { NULL,              NULL,                                0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_wml12_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { (WB_UTINY *)"accept",          0x00, 0x89 },
    { (WB_UTINY *)"bottom",          0x00, 0x8a },
    { (WB_UTINY *)"clear",           0x00, 0x8b },
    { (WB_UTINY *)"delete",          0x00, 0x8c },
    { (WB_UTINY *)"help",            0x00, 0x8d },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"http://www.",     0x00, 0x8f },
    { (WB_UTINY *)"http://",         0x00, 0x8e },
    { (WB_UTINY *)"https://www.",    0x00, 0x91 },
    { (WB_UTINY *)"https://",        0x00, 0x90 },    
    { (WB_UTINY *)"middle",          0x00, 0x93 },
    { (WB_UTINY *)"nowrap",          0x00, 0x94 },
    { (WB_UTINY *)"onenterbackward", 0x00, 0x96 },
    { (WB_UTINY *)"onenterforward",  0x00, 0x97 },
    { (WB_UTINY *)"onpick",          0x00, 0x95 },
    { (WB_UTINY *)"ontimer",         0x00, 0x98 },
    { (WB_UTINY *)"options",         0x00, 0x99 },
    { (WB_UTINY *)"password",        0x00, 0x9a },
    { (WB_UTINY *)"reset",           0x00, 0x9b },
    { (WB_UTINY *)"text",            0x00, 0x9d },
    { (WB_UTINY *)"top",             0x00, 0x9e },
    { (WB_UTINY *)"unknown",         0x00, 0x9f },
    { (WB_UTINY *)"wrap",            0x00, 0xa0 },
    { (WB_UTINY *)"www.",            0x00, 0xa1 },
    { NULL,              0x00, 0x00 }
};

#endif /* WBXML_TABLES_SEPARATE_WML_VERSIONS */


/******************************************************
 *    WML 1.3 (WAP 1.2.1: "WAP-191-WML-20000219-a.pdf")
 */

const WBXMLTagEntry sv_wml13_tag_table[] = {
    { (WB_UTINY *)"a",         0x00, 0x1c },
    { (WB_UTINY *)"anchor",    0x00, 0x22 }, /* WML 1.1 */
    { (WB_UTINY *)"access",    0x00, 0x23 },
    { (WB_UTINY *)"b",         0x00, 0x24 },
    { (WB_UTINY *)"big",       0x00, 0x25 },
    { (WB_UTINY *)"br",        0x00, 0x26 },
    { (WB_UTINY *)"card",      0x00, 0x27 },
    { (WB_UTINY *)"do",        0x00, 0x28 },
    { (WB_UTINY *)"em",        0x00, 0x29 },
    { (WB_UTINY *)"fieldset",  0x00, 0x2a },
    { (WB_UTINY *)"go",        0x00, 0x2b },
    { (WB_UTINY *)"head",      0x00, 0x2c },
    { (WB_UTINY *)"i",         0x00, 0x2d },
    { (WB_UTINY *)"img",       0x00, 0x2e },
    { (WB_UTINY *)"input",     0x00, 0x2f },
    { (WB_UTINY *)"meta",      0x00, 0x30 },
    { (WB_UTINY *)"noop",      0x00, 0x31 },
    { (WB_UTINY *)"p",         0x00, 0x20 }, /* WML 1.1 */
    { (WB_UTINY *)"postfield", 0x00, 0x21 }, /* WML 1.1 */
    { (WB_UTINY *)"pre",       0x00, 0x1b },
    { (WB_UTINY *)"prev",      0x00, 0x32 },
    { (WB_UTINY *)"onevent",   0x00, 0x33 },
    { (WB_UTINY *)"optgroup",  0x00, 0x34 },
    { (WB_UTINY *)"option",    0x00, 0x35 },
    { (WB_UTINY *)"refresh",   0x00, 0x36 },
    { (WB_UTINY *)"select",    0x00, 0x37 },
    { (WB_UTINY *)"setvar",    0x00, 0x3e }, /* WML 1.1 */
    { (WB_UTINY *)"small",     0x00, 0x38 },
    { (WB_UTINY *)"strong",    0x00, 0x39 },
    { (WB_UTINY *)"table",     0x00, 0x1f }, /* WML 1.1 */
    { (WB_UTINY *)"td",        0x00, 0x1d }, /* WML 1.1 */
    { (WB_UTINY *)"template",  0x00, 0x3b },
    { (WB_UTINY *)"timer",     0x00, 0x3c },
    { (WB_UTINY *)"tr",        0x00, 0x1e }, /* WML 1.1 */
    { (WB_UTINY *)"u",         0x00, 0x3d },
    { (WB_UTINY *)"wml",       0x00, 0x3f },
    { NULL,        0x00, 0x00 }
};


const WBXMLAttrEntry sv_wml13_attr_table[] = {
    { (WB_UTINY *)"accept-charset",  NULL,                                0x00, 0x05 },
    { (WB_UTINY *)"accesskey",       NULL,                                0x00, 0x5e }, /* WML 1.2 */
    { (WB_UTINY *)"align",           NULL,                                0x00, 0x52 }, /* WML 1.1 */
    { (WB_UTINY *)"align",           (WB_UTINY *)"bottom",                            0x00, 0x06 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"center",                            0x00, 0x07 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"left",                              0x00, 0x08 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"middle",                            0x00, 0x09 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"right",                             0x00, 0x0a },
    { (WB_UTINY *)"align",           (WB_UTINY *)"top",                               0x00, 0x0b },
    { (WB_UTINY *)"alt",             NULL,                                0x00, 0x0c },
    { (WB_UTINY *)"cache-control",   (WB_UTINY *)"no-cache",                          0x00, 0x64 }, /* WML 1.3 */
    { (WB_UTINY *)"class",           NULL,                                0x00, 0x54 }, /* WML 1.1 */
    { (WB_UTINY *)"columns",         NULL,                                0x00, 0x53 }, /* WML 1.1 */
    { (WB_UTINY *)"content",         NULL,                                0x00, 0x0d }, 
    { (WB_UTINY *)"content",         (WB_UTINY *)"application/vnd.wap.wmlc;charset=", 0x00, 0x5c }, /* WML 1.1 */
    { (WB_UTINY *)"domain",          NULL,                                0x00, 0x0f },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"false",                             0x00, 0x10 },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"true",                              0x00, 0x11 },
    { (WB_UTINY *)"enctype",         NULL,                                0x00, 0x5f }, /* WML 1.2 */
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"application/x-www-form-urlencoded", 0x00, 0x60 }, /* WML 1.2 */
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"multipart/form-data",               0x00, 0x61 }, /* WML 1.2 */
    { (WB_UTINY *)"format",          NULL,                                0x00, 0x12 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"false",                             0x00, 0x56 }, /* WML 1.1 */
    { (WB_UTINY *)"forua",           (WB_UTINY *)"true",                              0x00, 0x57 }, /* WML 1.1 */
    { (WB_UTINY *)"height",          NULL,                                0x00, 0x13 },
    { (WB_UTINY *)"href",            NULL,                                0x00, 0x4a }, /* WML 1.1 */
    { (WB_UTINY *)"href",            (WB_UTINY *)"http://",                           0x00, 0x4b }, /* WML 1.1 */
    { (WB_UTINY *)"href",            (WB_UTINY *)"https://",                          0x00, 0x4c }, /* WML 1.1 */
    { (WB_UTINY *)"hspace",          NULL,                                0x00, 0x14 },
    { (WB_UTINY *)"http-equiv",      NULL,                                0x00, 0x5a }, /* WML 1.1 */
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Content-Type",                      0x00, 0x5b }, /* WML 1.1 */
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Expires",                           0x00, 0x5d }, /* WML 1.1 */
    { (WB_UTINY *)"id",              NULL,                                0x00, 0x55 }, /* WML 1.1 */
    { (WB_UTINY *)"ivalue",          NULL,                                0x00, 0x15 }, /* WML 1.1 */
    { (WB_UTINY *)"iname",           NULL,                                0x00, 0x16 }, /* WML 1.1 */
    { (WB_UTINY *)"label",           NULL,                                0x00, 0x18 },
    { (WB_UTINY *)"localsrc",        NULL,                                0x00, 0x19 },
    { (WB_UTINY *)"maxlength",       NULL,                                0x00, 0x1a },
    { (WB_UTINY *)"method",          (WB_UTINY *)"get",                               0x00, 0x1b },
    { (WB_UTINY *)"method",          (WB_UTINY *)"post",                              0x00, 0x1c },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"nowrap",                            0x00, 0x1d },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"wrap",                              0x00, 0x1e },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"false",                             0x00, 0x1f },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"true",                              0x00, 0x20 },
    { (WB_UTINY *)"name",            NULL,                                0x00, 0x21 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"false",                             0x00, 0x22 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"true",                              0x00, 0x23 },
    { (WB_UTINY *)"onenterbackward", NULL,                                0x00, 0x25 },
    { (WB_UTINY *)"onenterforward",  NULL,                                0x00, 0x26 },
    { (WB_UTINY *)"onpick",          NULL,                                0x00, 0x24 }, /* WML 1.1 */
    { (WB_UTINY *)"ontimer",         NULL,                                0x00, 0x27 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"false",                             0x00, 0x28 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"true",                              0x00, 0x29 },
    { (WB_UTINY *)"path",            NULL,                                0x00, 0x2a },
    { (WB_UTINY *)"scheme",          NULL,                                0x00, 0x2e },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"false",                             0x00, 0x2f },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"true",                              0x00, 0x30 },
    { (WB_UTINY *)"size",            NULL,                                0x00, 0x31 },
    { (WB_UTINY *)"src",             NULL,                                0x00, 0x32 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"http://",                           0x00, 0x58 }, /* WML 1.1 */
    { (WB_UTINY *)"src",             (WB_UTINY *)"https://",                          0x00, 0x59 }, /* WML 1.1 */
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"true",                              0x00, 0x33 }, /* WML 1.1 */
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"false",                             0x00, 0x34 }, /* WML 1.1 */
    { (WB_UTINY *)"tabindex",        NULL,                                0x00, 0x35 },
    { (WB_UTINY *)"title",           NULL,                                0x00, 0x36 },
    { (WB_UTINY *)"type",            NULL,                                0x00, 0x37 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"accept",                            0x00, 0x38 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"delete",                            0x00, 0x39 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"help",                              0x00, 0x3a },
    { (WB_UTINY *)"type",            (WB_UTINY *)"password",                          0x00, 0x3b },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onpick",                            0x00, 0x3c },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterbackward",                   0x00, 0x3d },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterforward",                    0x00, 0x3e },
    { (WB_UTINY *)"type",            (WB_UTINY *)"ontimer",                           0x00, 0x3f },
    { (WB_UTINY *)"type",            (WB_UTINY *)"options",                           0x00, 0x45 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"prev",                              0x00, 0x46 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"reset",                             0x00, 0x47 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"text",                              0x00, 0x48 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"vnd.",                              0x00, 0x49 },
    { (WB_UTINY *)"value",           NULL,                                0x00, 0x4d },
    { (WB_UTINY *)"vspace",          NULL,                                0x00, 0x4e },
    { (WB_UTINY *)"width",           NULL,                                0x00, 0x4f },
    { (WB_UTINY *)"xml:lang",        NULL,                                0x00, 0x50 },
    { (WB_UTINY *)"xml:space",       (WB_UTINY *)"preserve",                          0x00, 0x62 }, /* WML 1.3 */
    { (WB_UTINY *)"xml:space",       (WB_UTINY *)"default",                           0x00, 0x63 }, /* WML 1.3 */
    { NULL,              NULL,                                0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_wml13_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { (WB_UTINY *)"accept",          0x00, 0x89 },
    { (WB_UTINY *)"bottom",          0x00, 0x8a },
    { (WB_UTINY *)"clear",           0x00, 0x8b },
    { (WB_UTINY *)"delete",          0x00, 0x8c },
    { (WB_UTINY *)"help",            0x00, 0x8d },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"http://www.",     0x00, 0x8f },
    { (WB_UTINY *)"http://",         0x00, 0x8e },
    { (WB_UTINY *)"https://www.",    0x00, 0x91 },
    { (WB_UTINY *)"https://",        0x00, 0x90 },    
    { (WB_UTINY *)"middle",          0x00, 0x93 },
    { (WB_UTINY *)"nowrap",          0x00, 0x94 },
    { (WB_UTINY *)"onenterbackward", 0x00, 0x96 },
    { (WB_UTINY *)"onenterforward",  0x00, 0x97 },
    { (WB_UTINY *)"onpick",          0x00, 0x95 }, /* WML 1.1 */
    { (WB_UTINY *)"ontimer",         0x00, 0x98 },
    { (WB_UTINY *)"options",         0x00, 0x99 },
    { (WB_UTINY *)"password",        0x00, 0x9a },
    { (WB_UTINY *)"reset",           0x00, 0x9b },
    { (WB_UTINY *)"text",            0x00, 0x9d },
    { (WB_UTINY *)"top",             0x00, 0x9e },
    { (WB_UTINY *)"unknown",         0x00, 0x9f },
    { (WB_UTINY *)"wrap",            0x00, 0xa0 },
    { (WB_UTINY *)"www.",            0x00, 0xa1 },
    { NULL,              0x00, 0x00 }
};


/********************************************
 *    WTA 1.0 (WAP 1.0: "wta-30-apr-98.pdf")
 */

const WBXMLTagEntry sv_wta10_tag_table[] = {
    { (WB_UTINY *)"EVENT",          0x00, 0x05 },
    { (WB_UTINY *)"EVENTTABLE",     0x00, 0x06 },
    { (WB_UTINY *)"TYPE",           0x00, 0x07 },
    { (WB_UTINY *)"URL",            0x00, 0x08 },
    { (WB_UTINY *)"WTAI",           0x00, 0x09 },
    { NULL,             0x00, 0x00 }
};

const WBXMLAttrEntry sv_wta10_attr_table[] = {
    { (WB_UTINY *)"NAME",       NULL,                    0x00, 0x05 },
    { (WB_UTINY *)"VALUE",      NULL,                    0x00, 0x06 },
    { NULL,         NULL,                    0x00, 0x00 }
};


/*************************************************
 *    SI 1.0 ("WAP-167-ServiceInd-20010731-a.pdf")
 */

const WBXMLTagEntry sv_si10_tag_table[] = {
    { (WB_UTINY *)"si",             0x00, 0x05 },
    { (WB_UTINY *)"indication",     0x00, 0x06 },
    { (WB_UTINY *)"info",           0x00, 0x07 },
    { (WB_UTINY *)"item",           0x00, 0x08 },
    { NULL,             0x00, 0x00 }
};


const WBXMLAttrEntry sv_si10_attr_table[] = {
    { (WB_UTINY *)"action",  (WB_UTINY *)"signal-none",             0x00, 0x05 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"signal-low",              0x00, 0x06 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"signal-medium",           0x00, 0x07 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"signal-high",             0x00, 0x08 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"delete",                  0x00, 0x09 },
    { (WB_UTINY *)"created", NULL,                      0x00, 0x0a },
    { (WB_UTINY *)"href",    NULL,                      0x00, 0x0b },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"href",    (WB_UTINY *)"http://www.",             0x00, 0x0d },
    { (WB_UTINY *)"href",    (WB_UTINY *)"http://",                 0x00, 0x0c },
    { (WB_UTINY *)"href",    (WB_UTINY *)"https://www.",            0x00, 0x0f },
    { (WB_UTINY *)"href",    (WB_UTINY *)"https://",                0x00, 0x0e },    
    { (WB_UTINY *)"si-expires", NULL,                   0x00, 0x10 },
    { (WB_UTINY *)"si-id",      NULL,                   0x00, 0x11 },
    { (WB_UTINY *)"class",      NULL,                   0x00, 0x12 },
    { NULL,         NULL,                   0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_si10_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { NULL,              0x00, 0x00 }
};


/**************************************************
 *    SL 1.0 ("WAP-168-ServiceLoad-20010731-a.pdf")
 */

const WBXMLTagEntry sv_sl10_tag_table[] = {
    { (WB_UTINY *)"sl",              0x00, 0x05 },
    { NULL,              0x00, 0x00 }
};


const WBXMLAttrEntry sv_sl10_attr_table[] = {
    { (WB_UTINY *)"action",  (WB_UTINY *)"execute-low",         0x00, 0x05 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"execute-high",        0x00, 0x06 },
    { (WB_UTINY *)"action",  (WB_UTINY *)"cache",               0x00, 0x07 },
    { (WB_UTINY *)"href",    NULL,                  0x00, 0x08 },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"href",    (WB_UTINY *)"http://www.",         0x00, 0x0a },
    { (WB_UTINY *)"href",    (WB_UTINY *)"http://",             0x00, 0x09 },
    { (WB_UTINY *)"href",    (WB_UTINY *)"https://www.",        0x00, 0x0c },
    { (WB_UTINY *)"href",    (WB_UTINY *)"https://",            0x00, 0x0b },    
    { NULL,      NULL,                  0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_sl10_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { NULL,              0x00, 0x00 }
};


/***********************************************
 *    CO 1.0 ("WAP-175-CacheOp-20010731-a.pdf")
 */

const WBXMLTagEntry sv_co10_tag_table[] = {
    { (WB_UTINY *)"co",                     0x00, 0x05 },
    { (WB_UTINY *)"invalidate-object",      0x00, 0x06 },
    { (WB_UTINY *)"invalidate-service",     0x00, 0x07 },
    { NULL,                     0x00, 0x00 }
};


const WBXMLAttrEntry sv_co10_attr_table[] = {
    { (WB_UTINY *)"uri",    NULL,                   0x00, 0x05 },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"uri",    (WB_UTINY *)"http://www.",          0x00, 0x07 },
    { (WB_UTINY *)"uri",    (WB_UTINY *)"http://",              0x00, 0x06 },
    { (WB_UTINY *)"uri",    (WB_UTINY *)"https://www.",         0x00, 0x09 },
    { (WB_UTINY *)"uri",    (WB_UTINY *)"https://",             0x00, 0x08 },    
    { NULL,     NULL,                   0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_co10_attr_value_table[] = {
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { NULL,              0x00, 0x00 }
};


/**********************************************************
 *    PROV 1.0
 *      WAP 2.0: "WAP-183-PROVCONT-20010724-a.pdf"
 *      OMA: "OMA-WAP-ProvCont-v1_1-20021112-C.PDF"
 */

const WBXMLTagEntry sv_prov10_tag_table[] = {
    { (WB_UTINY *)"wap-provisioningdoc",        0x00, 0x05 },
    { (WB_UTINY *)"characteristic",             0x00, 0x06 },
    { (WB_UTINY *)"parm",                       0x00, 0x07 },
    
    { (WB_UTINY *)"characteristic",             0x01, 0x06 }, /* OMA */
    { (WB_UTINY *)"parm",                       0x01, 0x07 }, /* OMA */
    { NULL,                         0x00, 0x00 }
};


const WBXMLAttrEntry sv_prov10_attr_table[] = {
    /* Wap-provisioningdoc */
    { (WB_UTINY *)"version",    NULL,               0x00, 0x45 },
    { (WB_UTINY *)"version",    (WB_UTINY *)"1.0",              0x00, 0x46 },

    /* Characteristic */
    { (WB_UTINY *)"type",        NULL,                  0x00, 0x50 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"PXLOGICAL",           0x00, 0x51 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"PXPHYSICAL",          0x00, 0x52 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"PORT",                0x00, 0x53 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"VALIDITY",            0x00, 0x54 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"NAPDEF",              0x00, 0x55 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"BOOTSTRAP",           0x00, 0x56 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"VENDORCONFIG",        0x00, 0x57 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"CLIENTIDENTITY",      0x00, 0x58 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"PXAUTHINFO",          0x00, 0x59 },
    { (WB_UTINY *)"type",        (WB_UTINY *)"NAPAUTHINFO",         0x00, 0x5a },
    { (WB_UTINY *)"type",        (WB_UTINY *)"ACCESS",              0x00, 0x5b }, /* OMA */
    
    { (WB_UTINY *)"type",        NULL,                  0x01, 0x50 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"PORT",                0x01, 0x53 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"CLIENTIDENTITY",      0x01, 0x58 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"APPLICATION",         0x01, 0x55 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"APPADDR",             0x01, 0x56 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"APPAUTH",             0x01, 0x57 }, /* OMA */
    { (WB_UTINY *)"type",        (WB_UTINY *)"RESOURCE",            0x01, 0x59 }, /* OMA */

    /* Parm */
    { (WB_UTINY *)"name",        NULL,                  0x00, 0x05 },
    { (WB_UTINY *)"value",       NULL,                  0x00, 0x06 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"NAME",                0x00, 0x07 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"NAP-ADDRESS",         0x00, 0x08 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"NAP-ADDRTYPE",        0x00, 0x09 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"CALLTYPE",            0x00, 0x0a },
    { (WB_UTINY *)"name",        (WB_UTINY *)"VALIDUNTIL",          0x00, 0x0b },
    { (WB_UTINY *)"name",        (WB_UTINY *)"AUTHTYPE",            0x00, 0x0c },
    { (WB_UTINY *)"name",        (WB_UTINY *)"AUTHNAME",            0x00, 0x0d },
    { (WB_UTINY *)"name",        (WB_UTINY *)"AUTHSECRET",          0x00, 0x0e },
    { (WB_UTINY *)"name",        (WB_UTINY *)"LINGER",              0x00, 0x0f },
    { (WB_UTINY *)"name",        (WB_UTINY *)"BEARER",              0x00, 0x10 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"NAPID",               0x00, 0x11 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"COUNTRY",             0x00, 0x12 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"NETWORK",             0x00, 0x13 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"INTERNET",            0x00, 0x14 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PROXY-ID",            0x00, 0x15 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PROXY-PROVIDER-ID",   0x00, 0x16 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"DOMAIN",              0x00, 0x17 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PROVURL",             0x00, 0x18 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXAUTH-TYPE",         0x00, 0x19 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXAUTH-ID",           0x00, 0x1a },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXAUTH-PW",           0x00, 0x1b },
    { (WB_UTINY *)"name",        (WB_UTINY *)"STARTPAGE",           0x00, 0x1c },
    { (WB_UTINY *)"name",        (WB_UTINY *)"BASAUTH-ID",          0x00, 0x1d },
    { (WB_UTINY *)"name",        (WB_UTINY *)"BASAUTH-PW",          0x00, 0x1e },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PUSHENABLED",         0x00, 0x1f },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXADDR",              0x00, 0x20 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXADDRTYPE",          0x00, 0x21 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"TO-NAPID",            0x00, 0x22 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PORTNBR",             0x00, 0x23 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"SERVICE",             0x00, 0x24 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"LINKSPEED",           0x00, 0x25 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"DNLINKSPEED",         0x00, 0x26 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"LOCAL-ADDR",          0x00, 0x27 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"LOCAL-ADDRTYPE",      0x00, 0x28 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"CONTEXT-ALLOW",       0x00, 0x29 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"TRUST",               0x00, 0x2a },
    { (WB_UTINY *)"name",        (WB_UTINY *)"MASTER",              0x00, 0x2b },
    { (WB_UTINY *)"name",        (WB_UTINY *)"SID",                 0x00, 0x2c },
    { (WB_UTINY *)"name",        (WB_UTINY *)"SOC",                 0x00, 0x2d },
    { (WB_UTINY *)"name",        (WB_UTINY *)"WSP-VERSION",         0x00, 0x2e },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PHYSICAL-PROXY-ID",   0x00, 0x2f },
    { (WB_UTINY *)"name",        (WB_UTINY *)"CLIENT-ID",           0x00, 0x30 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"DELIVERY-ERR-SDU",    0x00, 0x31 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"DELIVERY-ORDER",      0x00, 0x32 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"TRAFFIC-CLASS",       0x00, 0x33 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"MAX-SDU-SIZE",        0x00, 0x34 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"MAX-BITRATE-UPLINK",  0x00, 0x35 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"MAX-BITRATE-DNLINK",  0x00, 0x36 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"RESIDUAL-BER",        0x00, 0x37 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"SDU-ERROR-RATIO",     0x00, 0x38 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"TRAFFIC-HANDL-PRIO",  0x00, 0x39 },
    { (WB_UTINY *)"name",        (WB_UTINY *)"TRANSFER-DELAY",      0x00, 0x3a },
    { (WB_UTINY *)"name",        (WB_UTINY *)"GUARANTEED-BITRATE-UPLINK",   0x00, 0x3b },
    { (WB_UTINY *)"name",        (WB_UTINY *)"GUARANTEED-BITRATE-DNLINK",   0x00, 0x3c },
    { (WB_UTINY *)"name",        (WB_UTINY *)"PXADDR-FQDN",         0x00, 0x3d }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"PROXY-PW",            0x00, 0x3e }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"PPGAUTH-TYPE",        0x00, 0x3f }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"PULLENABLED",         0x00, 0x47 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"DNS-ADDR",            0x00, 0x48 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"MAX-NUM-RETRY",       0x00, 0x49 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"FIRST-RETRY-TIMEOUT", 0x00, 0x4a }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"REREG-THRESHOLD",     0x00, 0x4b }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"T-BIT",               0x00, 0x4c }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AUTH-ENTITY",         0x00, 0x4e }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"SPI",                 0x00, 0x4f }, /* OMA */
    
    { (WB_UTINY *)"name",        NULL,                  0x01, 0x05 }, /* OMA */
    { (WB_UTINY *)"value",       NULL,                  0x01, 0x06 }, /* OMA */
    { (WB_UTINY *)"name",         (WB_UTINY *)"NAME",                0x01, 0x07 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"INTERNET",            0x01, 0x14 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"STARTPAGE",           0x01, 0x1c }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"TO-NAPID",            0x01, 0x22 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"PORTNBR",             0x01, 0x23 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"SERVICE",             0x01, 0x24 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AACCEPT",             0x01, 0x2e }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AAUTHDATA",           0x01, 0x2f }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AAUTHLEVEL",          0x01, 0x30 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AAUTHNAME",           0x01, 0x31 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AAUTHSECRET",         0x01, 0x32 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"AAUTHTYPE",           0x01, 0x33 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"ADDR",                0x01, 0x34 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"ADDRTYPE",            0x01, 0x35 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"APPID",               0x01, 0x36 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"APROTOCOL",           0x01, 0x37 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"PROVIDER-ID",         0x01, 0x38 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"TO-PROXY",            0x01, 0x39 }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"URI",                 0x01, 0x3a }, /* OMA */
    { (WB_UTINY *)"name",        (WB_UTINY *)"RULE",                0x01, 0x3b }, /* OMA */
    
    { NULL,          NULL,                  0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_prov10_attr_value_table[] = {
    /* ADDRTYPE */
    { (WB_UTINY *)"IPV4",                   0x00, 0x85 },
    { (WB_UTINY *)"IPV6",                   0x00, 0x86 },
    { (WB_UTINY *)"E164",                   0x00, 0x87 },
    { (WB_UTINY *)"ALPHA",                  0x00, 0x88 },
    { (WB_UTINY *)"APN",                    0x00, 0x89 },
    { (WB_UTINY *)"SCODE",                  0x00, 0x8a },
    { (WB_UTINY *)"TETRA-ITSI",             0x00, 0x8b },
    { (WB_UTINY *)"MAN",                    0x00, 0x8c },
/*Add to fix bug.*/
    { (WB_UTINY *)"APPSRV",                 0x00, 0x8d }, /* OMA */
    { (WB_UTINY *)"OBEX",                   0x00, 0x8e }, /* OMA */
/*End to end.*/

    { (WB_UTINY *)"IPV6",                   0x01, 0x86 }, /* OMA */
    { (WB_UTINY *)"E164",                   0x01, 0x87 }, /* OMA */
    { (WB_UTINY *)"ALPHA",                  0x01, 0x88 }, /* OMA */
    { (WB_UTINY *)"APPSRV",                 0x01, 0x8d }, /* OMA */
    { (WB_UTINY *)"OBEX",                   0x01, 0x8e }, /* OMA */

    /* CALLTYPE */
    { (WB_UTINY *)"ANALOG-MODEM",           0x00, 0x90 },
    { (WB_UTINY *)"V.120",                  0x00, 0x91 },
    { (WB_UTINY *)"V.110",                  0x00, 0x92 },
    { (WB_UTINY *)"X.31",                   0x00, 0x93 },
    { (WB_UTINY *)"BIT-TRANSPARENT",        0x00, 0x94 },
    { (WB_UTINY *)"DIRECT-ASYNCHRONOUS-DATA-SERVICE",    0x00, 0x95 },

    /* AAUTHTYPE */
    { (WB_UTINY *)",",                      0x01, 0x90 }, /* OMA */
    { (WB_UTINY *)"HTTP-",                  0x01, 0x91 }, /* OMA */
    { (WB_UTINY *)"BASIC",                  0x01, 0x92 }, /* OMA */
    { (WB_UTINY *)"DIGEST",                 0x01, 0x93 }, /* OMA */
    
    /* AUTHTYPE/PXAUTH-TYPE */
    { (WB_UTINY *)"PAP",                    0x00, 0x9a },
    { (WB_UTINY *)"CHAP",                   0x00, 0x9b },
    { (WB_UTINY *)"HTTP-BASIC",             0x00, 0x9c },
    { (WB_UTINY *)"HTTP-DIGEST",            0x00, 0x9d },
    { (WB_UTINY *)"WTLS-SS",                0x00, 0x9e },
    { (WB_UTINY *)"MD5",                    0x00, 0x9f }, /* OMA */

    /* BEARER */
    { (WB_UTINY *)"GSM-USSD",               0x00, 0xa2 },
    { (WB_UTINY *)"GSM-SMS",                0x00, 0xa3 },
    { (WB_UTINY *)"ANSI-136-GUTS",          0x00, 0xa4 },
    { (WB_UTINY *)"IS-95-CDMA-SMS",         0x00, 0xa5 },
    { (WB_UTINY *)"IS-95-CDMA-CSD",         0x00, 0xa6 },
    { (WB_UTINY *)"IS-95-CDMA-PACKET",      0x00, 0xa7 },
    { (WB_UTINY *)"ANSI-136-CSD",           0x00, 0xa8 },
    { (WB_UTINY *)"ANSI-136-GPRS",          0x00, 0xa9 },
    { (WB_UTINY *)"GSM-CSD",                0x00, 0xaa },
    { (WB_UTINY *)"GSM-GPRS",               0x00, 0xab },
    { (WB_UTINY *)"AMPS-CDPD",              0x00, 0xac },
    { (WB_UTINY *)"PDC-CSD",                0x00, 0xad },
    { (WB_UTINY *)"PDC-PACKET",             0x00, 0xae },
    { (WB_UTINY *)"IDEN-SMS",               0x00, 0xaf },
    { (WB_UTINY *)"IDEN-CSD",               0x00, 0xb0 },
    { (WB_UTINY *)"IDEN-PACKET",            0x00, 0xb1 },
    { (WB_UTINY *)"FLEX/REFLEX",            0x00, 0xb2 },
    { (WB_UTINY *)"PHS-SMS",                0x00, 0xb3 },
    { (WB_UTINY *)"PHS-CSD",                0x00, 0xb4 },
    { (WB_UTINY *)"TRETRA-SDS",             0x00, 0xb5 },
    { (WB_UTINY *)"TRETRA-PACKET",          0x00, 0xb6 },
    { (WB_UTINY *)"ANSI-136-GHOST",         0x00, 0xb7 },
    { (WB_UTINY *)"MOBITEX-MPAK",           0x00, 0xb8 },
    { (WB_UTINY *)"CDMA2000-1X-SIMPLE-IP",  0x00, 0xb9 }, /* OMA */
    { (WB_UTINY *)"CDMA2000-1X-MOBILE-IP",  0x00, 0xba }, /* OMA */

    /* LINKSPEED */
    { (WB_UTINY *)"AUTOBAUDING",            0x00, 0xc5 },

    /* SERVICE */
    { (WB_UTINY *)"CL-WSP",                 0x00, 0xca },
    { (WB_UTINY *)"CO-WSP",                 0x00, 0xcb },
    { (WB_UTINY *)"CL-SEC-WSP",             0x00, 0xcc },
    { (WB_UTINY *)"CO-SEC-WSP",             0x00, 0xcd },
    { (WB_UTINY *)"CL-SEC-WTA",             0x00, 0xce },
    { (WB_UTINY *)"CO-SEC-WTA",             0x00, 0xcf },
    { (WB_UTINY *)"OTA-HTTP-TO",            0x00, 0xd0 }, /* OMA */
    { (WB_UTINY *)"OTA-HTTP-TLS-TO",        0x00, 0xd1 }, /* OMA */
    { (WB_UTINY *)"OTA-HTTP-PO",            0x00, 0xd2 }, /* OMA */
    { (WB_UTINY *)"OTA-HTTP-TLS-PO",        0x00, 0xd3 }, /* OMA */
    
    /* AUTH-ENTITY */
    { (WB_UTINY *)"AAA",                    0x00, 0xe0 }, /* OMA */
    { (WB_UTINY *)"HA",                     0x00, 0xe1 }, /* OMA */

    { NULL,                     0x00, 0x00 }
};


/***************************************************
 *    CHANNEL 1.1 (WAP 1.1: "SPEC-WTA-19990716.pdf")
 */

const WBXMLTagEntry sv_channel11_tag_table[] = {
    { (WB_UTINY *)"channel",        0x00, 0x05 },
    { (WB_UTINY *)"title",          0x00, 0x06 },
    { (WB_UTINY *)"abstract",       0x00, 0x07 },
    { (WB_UTINY *)"resource",       0x00, 0x08 },
    { NULL,             0x00, 0x00 }
};


const WBXMLAttrEntry sv_channel11_attr_table[] = {
    { (WB_UTINY *)"maxspace",   NULL,           0x00, 0x05 },
    { (WB_UTINY *)"base",       NULL,           0x00, 0x06 },
    { (WB_UTINY *)"href",       NULL,           0x00, 0x07 },
    { (WB_UTINY *)"href",       (WB_UTINY *)"http://",      0x00, 0x08 },
    { (WB_UTINY *)"href",       (WB_UTINY *)"https://",     0x00, 0x09 },
    { (WB_UTINY *)"lastmod",    NULL,           0x00, 0x0a },
    { (WB_UTINY *)"etag",       NULL,           0x00, 0x0b },
    { (WB_UTINY *)"md5",        NULL,           0x00, 0x0c },
    { (WB_UTINY *)"success",    NULL,           0x00, 0x0d },
    { (WB_UTINY *)"success",    (WB_UTINY *)"http://",      0x00, 0x0e },
    { (WB_UTINY *)"success",    (WB_UTINY *)"https://",     0x00, 0x0f },
    { (WB_UTINY *)"failure",    NULL,           0x00, 0x10 },
    { (WB_UTINY *)"failure",    (WB_UTINY *)"http://",      0x00, 0x11 },
    { (WB_UTINY *)"failure",    (WB_UTINY *)"https://",     0x00, 0x12 },
    { (WB_UTINY *)"EventId",    NULL,           0x00, 0x13 },
    { NULL,         NULL,           0x00, 0x00 }
};


/***********************************************
 *    WTA WML 1.2 ("WAP-266-WTA-20010908-a.pdf")
 */

const WBXMLTagEntry sv_wtawml12_tag_table[] = {
    /* Code Page 0 (WML 1.2) */
    { (WB_UTINY *)"a",         0x00, 0x1c },
    { (WB_UTINY *)"anchor",    0x00, 0x22 },
    { (WB_UTINY *)"access",    0x00, 0x23 },
    { (WB_UTINY *)"b",         0x00, 0x24 },
    { (WB_UTINY *)"big",       0x00, 0x25 },
    { (WB_UTINY *)"br",        0x00, 0x26 },
    { (WB_UTINY *)"card",      0x00, 0x27 },
    { (WB_UTINY *)"do",        0x00, 0x28 },
    { (WB_UTINY *)"em",        0x00, 0x29 },
    { (WB_UTINY *)"fieldset",  0x00, 0x2a },
    { (WB_UTINY *)"go",        0x00, 0x2b },
    { (WB_UTINY *)"head",      0x00, 0x2c },
    { (WB_UTINY *)"i",         0x00, 0x2d },
    { (WB_UTINY *)"img",       0x00, 0x2e },
    { (WB_UTINY *)"input",     0x00, 0x2f },
    { (WB_UTINY *)"meta",      0x00, 0x30 },
    { (WB_UTINY *)"noop",      0x00, 0x31 },
    { (WB_UTINY *)"p",         0x00, 0x20 },
    { (WB_UTINY *)"postfield", 0x00, 0x21 },
    { (WB_UTINY *)"pre",       0x00, 0x1b },
    { (WB_UTINY *)"prev",      0x00, 0x32 },
    { (WB_UTINY *)"onevent",   0x00, 0x33 },
    { (WB_UTINY *)"optgroup",  0x00, 0x34 },
    { (WB_UTINY *)"option",    0x00, 0x35 },
    { (WB_UTINY *)"refresh",   0x00, 0x36 },
    { (WB_UTINY *)"select",    0x00, 0x37 },
    { (WB_UTINY *)"setvar",    0x00, 0x3e },
    { (WB_UTINY *)"small",     0x00, 0x38 },
    { (WB_UTINY *)"strong",    0x00, 0x39 },
    { (WB_UTINY *)"table",     0x00, 0x1f },
    { (WB_UTINY *)"td",        0x00, 0x1d },
    { (WB_UTINY *)"template",  0x00, 0x3b },
    { (WB_UTINY *)"timer",     0x00, 0x3c },
    { (WB_UTINY *)"tr",        0x00, 0x1e },
    { (WB_UTINY *)"u",         0x00, 0x3d },
    { (WB_UTINY *)"wml",       0x00, 0x3f },

    /* Code Page 1 (WTA) */
    { (WB_UTINY *)"wta-wml",   0x01, 0x3f },
    { NULL,        0x00, 0x00 }
};


const WBXMLAttrEntry sv_wtawml12_attr_table[] = {
    /* Code Page 0 (WML 1.2) */
    { (WB_UTINY *)"accept-charset",  NULL,                                0x00, 0x05 },
    { (WB_UTINY *)"accesskey",       NULL,                                0x00, 0x5e },
    { (WB_UTINY *)"align",           NULL,                                0x00, 0x52 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"bottom",                            0x00, 0x06 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"center",                            0x00, 0x07 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"left",                              0x00, 0x08 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"middle",                            0x00, 0x09 },
    { (WB_UTINY *)"align",           (WB_UTINY *)"right",                             0x00, 0x0a },
    { (WB_UTINY *)"align",           (WB_UTINY *)"top",                               0x00, 0x0b },
    { (WB_UTINY *)"alt",             NULL,                                0x00, 0x0c },
    { (WB_UTINY *)"class",           NULL,                                0x00, 0x54 },
    { (WB_UTINY *)"columns",         NULL,                                0x00, 0x53 },
    { (WB_UTINY *)"content",         NULL,                                0x00, 0x0d },
    { (WB_UTINY *)"content",         (WB_UTINY *)"application/vnd.wap.wmlc;charset=", 0x00, 0x5c },
    { (WB_UTINY *)"domain",          NULL,                                0x00, 0x0f },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"false",                             0x00, 0x10 },
    { (WB_UTINY *)"emptyok",         (WB_UTINY *)"true",                              0x00, 0x11 },
    { (WB_UTINY *)"enctype",         NULL,                                0x00, 0x5f },
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"application/x-www-form-urlencoded", 0x00, 0x60 },    
    { (WB_UTINY *)"enctype",         (WB_UTINY *)"multipart/form-data",               0x00, 0x61 },
    { (WB_UTINY *)"format",          NULL,                                0x00, 0x12 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"false",                             0x00, 0x56 },
    { (WB_UTINY *)"forua",           (WB_UTINY *)"true",                              0x00, 0x57 },
    { (WB_UTINY *)"height",          NULL,                                0x00, 0x13 },
    { (WB_UTINY *)"href",            NULL,                                0x00, 0x4a },
    { (WB_UTINY *)"href",            (WB_UTINY *)"http://",                           0x00, 0x4b },
    { (WB_UTINY *)"href",            (WB_UTINY *)"https://",                          0x00, 0x4c },
    { (WB_UTINY *)"hspace",          NULL,                                0x00, 0x14 },
    { (WB_UTINY *)"http-equiv",      NULL,                                0x00, 0x5a },
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Content-Type",                      0x00, 0x5b },
    { (WB_UTINY *)"http-equiv",      (WB_UTINY *)"Expires",                           0x00, 0x5d },
    { (WB_UTINY *)"id",              NULL,                                0x00, 0x55 },
    { (WB_UTINY *)"ivalue",          NULL,                                0x00, 0x15 },
    { (WB_UTINY *)"iname",           NULL,                                0x00, 0x16 },
    { (WB_UTINY *)"label",           NULL,                                0x00, 0x18 },
    { (WB_UTINY *)"localsrc",        NULL,                                0x00, 0x19 },
    { (WB_UTINY *)"maxlength",       NULL,                                0x00, 0x1a },
    { (WB_UTINY *)"method",          (WB_UTINY *)"get",                               0x00, 0x1b },
    { (WB_UTINY *)"method",          (WB_UTINY *)"post",                              0x00, 0x1c },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"nowrap",                            0x00, 0x1d },
    { (WB_UTINY *)"mode",            (WB_UTINY *)"wrap",                              0x00, 0x1e },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"false",                             0x00, 0x1f },
    { (WB_UTINY *)"multiple",        (WB_UTINY *)"true",                              0x00, 0x20 },
    { (WB_UTINY *)"name",            NULL,                                0x00, 0x21 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"false",                             0x00, 0x22 },
    { (WB_UTINY *)"newcontext",      (WB_UTINY *)"true",                              0x00, 0x23 },
    { (WB_UTINY *)"onenterbackward", NULL,                                0x00, 0x25 },
    { (WB_UTINY *)"onenterforward",  NULL,                                0x00, 0x26 },
    { (WB_UTINY *)"onpick",          NULL,                                0x00, 0x24 },
    { (WB_UTINY *)"ontimer",         NULL,                                0x00, 0x27 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"false",                             0x00, 0x28 },
    { (WB_UTINY *)"optional",        (WB_UTINY *)"true",                              0x00, 0x29 },
    { (WB_UTINY *)"path",            NULL,                                0x00, 0x2a },
    { (WB_UTINY *)"scheme",          NULL,                                0x00, 0x2e },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"false",                             0x00, 0x2f },
    { (WB_UTINY *)"sendreferer",     (WB_UTINY *)"true",                              0x00, 0x30 },
    { (WB_UTINY *)"size",            NULL,                                0x00, 0x31 },
    { (WB_UTINY *)"src",             NULL,                                0x00, 0x32 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"http://",                           0x00, 0x58 },
    { (WB_UTINY *)"src",             (WB_UTINY *)"https://",                          0x00, 0x59 },
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"true",                              0x00, 0x33 },
    { (WB_UTINY *)"ordered",         (WB_UTINY *)"false",                             0x00, 0x34 },
    { (WB_UTINY *)"tabindex",        NULL,                                0x00, 0x35 },
    { (WB_UTINY *)"title",           NULL,                                0x00, 0x36 },
    { (WB_UTINY *)"type",            NULL,                                0x00, 0x37 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"accept",                            0x00, 0x38 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"delete",                            0x00, 0x39 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"help",                              0x00, 0x3a },
    { (WB_UTINY *)"type",            (WB_UTINY *)"password",                          0x00, 0x3b },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onpick",                            0x00, 0x3c },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterbackward",                   0x00, 0x3d },
    { (WB_UTINY *)"type",            (WB_UTINY *)"onenterforward",                    0x00, 0x3e },
    { (WB_UTINY *)"type",            (WB_UTINY *)"ontimer",                           0x00, 0x3f },
    { (WB_UTINY *)"type",            (WB_UTINY *)"options",                           0x00, 0x45 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"prev",                              0x00, 0x46 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"reset",                             0x00, 0x47 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"text",                              0x00, 0x48 },
    { (WB_UTINY *)"type",            (WB_UTINY *)"vnd.",                              0x00, 0x49 },
    { (WB_UTINY *)"value",           NULL,                                0x00, 0x4d },
    { (WB_UTINY *)"vspace",          NULL,                                0x00, 0x4e },
    { (WB_UTINY *)"width",           NULL,                                0x00, 0x4f },
    { (WB_UTINY *)"xml:lang",        NULL,                                0x00, 0x50 },

    /* Code Page 1 (WTA) */
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"href",             (WB_UTINY *)"wtai://wp/mc;",                      0x01, 0x06 },
    { (WB_UTINY *)"href",             (WB_UTINY *)"wtai://wp/sd;",                      0x01, 0x07 },
    { (WB_UTINY *)"href",             (WB_UTINY *)"wtai://wp/ap;",                      0x01, 0x08 },
    { (WB_UTINY *)"href",             (WB_UTINY *)"wtai://ms/ec;",                      0x01, 0x09 },
    { (WB_UTINY *)"href",             (WB_UTINY *)"wtai://",                            0x01, 0x05 },        
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/ic",                        0x01, 0x12 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/cl",                        0x01, 0x13 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/co",                        0x01, 0x14 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/oc",                        0x01, 0x15 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/cc",                        0x01, 0x16 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc/dtmf",                      0x01, 0x17 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-nt/it",                        0x01, 0x21 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-nt/st",                        0x01, 0x22 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-nt/",                          0x01, 0x20 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-pb/",                          0x01, 0x30 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-lg/",                          0x01, 0x38 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-ms/ns",                        0x01, 0x51 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-ms/",                          0x01, 0x50 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-gsm/ru",                       0x01, 0x59 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-gsm/ch",                       0x01, 0x5a },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-gsm/ca",                       0x01, 0x5b },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-gsm/",                         0x01, 0x58 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-pdc",                          0x01, 0x60 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-ansi136/ia",                   0x01, 0x69 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-ansi136/if",                   0x01, 0x6a },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-ansi136",                      0x01, 0x68 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cdma/",                        0x01, 0x70 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-cc",                           0x01, 0x11 },
    { (WB_UTINY *)"type",             (WB_UTINY *)"wtaev-",                             0x01, 0x10 },
    { NULL,               NULL,                                 0x00, 0x00 }
};


const WBXMLAttrValueEntry sv_wtawml12_attr_value_table[] = {
    /* Code Page 0 (WML 1.2) */
    { (WB_UTINY *)".com/",           0x00, 0x85 },
    { (WB_UTINY *)".edu/",           0x00, 0x86 },
    { (WB_UTINY *)".net/",           0x00, 0x87 },
    { (WB_UTINY *)".org/",           0x00, 0x88 },
    { (WB_UTINY *)"accept",          0x00, 0x89 },
    { (WB_UTINY *)"bottom",          0x00, 0x8a },
    { (WB_UTINY *)"clear",           0x00, 0x8b },
    { (WB_UTINY *)"delete",          0x00, 0x8c },
    { (WB_UTINY *)"help",            0x00, 0x8d },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"http://www.",     0x00, 0x8f },
    { (WB_UTINY *)"http://",         0x00, 0x8e },    
    { (WB_UTINY *)"https://www.",    0x00, 0x91 },
    { (WB_UTINY *)"https://",        0x00, 0x90 },    
    { (WB_UTINY *)"middle",          0x00, 0x93 },
    { (WB_UTINY *)"nowrap",          0x00, 0x94 },
    { (WB_UTINY *)"onenterbackward", 0x00, 0x96 },
    { (WB_UTINY *)"onenterforward",  0x00, 0x97 },
    { (WB_UTINY *)"onpick",          0x00, 0x95 },
    { (WB_UTINY *)"ontimer",         0x00, 0x98 },
    { (WB_UTINY *)"options",         0x00, 0x99 },
    { (WB_UTINY *)"password",        0x00, 0x9a },
    { (WB_UTINY *)"reset",           0x00, 0x9b },
    { (WB_UTINY *)"text",            0x00, 0x9d },
    { (WB_UTINY *)"top",             0x00, 0x9e },
    { (WB_UTINY *)"unknown",         0x00, 0x9f },
    { (WB_UTINY *)"wrap",            0x00, 0xa0 },
    { (WB_UTINY *)"www.",            0x00, 0xa1 },
    { NULL,              0x00, 0x00 }
};



/***********************************************
 *    CHANNEL 1.2 ("WAP-266-WTA-20010908-a.pdf")
 */

const WBXMLTagEntry sv_channel12_tag_table[] = {
    { (WB_UTINY *)"channel",        0x00, 0x05 },
    { (WB_UTINY *)"title",          0x00, 0x06 },
    { (WB_UTINY *)"abstract",       0x00, 0x07 },
    { (WB_UTINY *)"resource",       0x00, 0x08 },
    { NULL,             0x00, 0x00 }
};


const WBXMLAttrEntry sv_channel12_attr_table[] = {
    { (WB_UTINY *)"maxspace",       NULL,           0x00, 0x05 },
    { (WB_UTINY *)"base",           NULL,           0x00, 0x06 },
    { (WB_UTINY *)"href",           NULL,           0x00, 0x07 },
    { (WB_UTINY *)"href",           (WB_UTINY *)"http://",      0x00, 0x08 },
    { (WB_UTINY *)"href",           (WB_UTINY *)"https://",     0x00, 0x09 },
    { (WB_UTINY *)"lastmod",        NULL,           0x00, 0x0a },
    { (WB_UTINY *)"etag",            NULL,          0x00, 0x0b },
    { (WB_UTINY *)"md5",            NULL,           0x00, 0x0c },
    { (WB_UTINY *)"success",        NULL,           0x00, 0x0d },
    { (WB_UTINY *)"success",        (WB_UTINY *)"http://",      0x00, 0x0e },
    { (WB_UTINY *)"success",        (WB_UTINY *)"https://",     0x00, 0x0f },
    { (WB_UTINY *)"failure",        NULL,           0x00, 0x10 },
    { (WB_UTINY *)"failure",        (WB_UTINY *)"http://",      0x00, 0x11 },
    { (WB_UTINY *)"failure",        (WB_UTINY *)"https://",     0x00, 0x12 },
    { (WB_UTINY *)"eventid",        NULL,           0x00, 0x13 },
    { (WB_UTINY *)"eventid",            (WB_UTINY *)"wtaev-",   0x00, 0x14 },
    { (WB_UTINY *)"channelid",          NULL,       0x00, 0x15 },
    { (WB_UTINY *)"useraccessible",     NULL,       0x00, 0x16 },
    { NULL,                 NULL,       0x00, 0x00 }
};


/*************************************************
 *    Email Notification 1.0 ("OMA-Push-EMN-v1_0-20020830-C.PDF")
 */

const WBXMLTagEntry sv_emn10_tag_table[] = {
    { (WB_UTINY *)"emn",    0x00, 0x05 },
    { NULL,     0x00, 0x00 }
};

const WBXMLAttrEntry sv_emn10_attr_table[] = {
    { (WB_UTINY *)"timestamp",      NULL,           0x00, 0x05 },
    { (WB_UTINY *)"mailbox",        NULL,           0x00, 0x06 },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"mailat:",      0x00, 0x07 },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"pop://",       0x00, 0x08 },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"imap://",      0x00, 0x09 },
    /* Do NOT change the order in this table please ! */
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"http://www.",  0x00, 0x0b },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"http://",      0x00, 0x0a },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"https://www.", 0x00, 0x0d },
    { (WB_UTINY *)"mailbox",        (WB_UTINY *)"https://",     0x00, 0x0c },    
    { NULL,             NULL,           0x00, 0x00 }
};

const WBXMLAttrValueEntry sv_emn10_attr_value_table[] = {
    { (WB_UTINY *)".com",       0x00, 0x85 },
    { (WB_UTINY *)".edu",       0x00, 0x86 },
    { (WB_UTINY *)".net",       0x00, 0x87 },
    { (WB_UTINY *)".org",       0x00, 0x88 },
    { NULL,         0x00, 0x00 }
};


/*************************************************
 *    Rights Expression Language Version 1.0 ("OMA-Download-DRMREL-v1_0-20020913-a.pdf")
 */
 
const WBXMLTagEntry sv_drmrel10_tag_table[] = {
    { (WB_UTINY *)"o-ex:rights",    0x00, 0x05 },
    { (WB_UTINY *)"o-ex:context",   0x00, 0x06 },
    { (WB_UTINY *)"o-dd:version",   0x00, 0x07 },
    { (WB_UTINY *)"o-dd:uid",       0x00, 0x08 },
    { (WB_UTINY *)"o-ex:agreement", 0x00, 0x09 },
    { (WB_UTINY *)"o-ex:asset",     0x00, 0x0A },
    { (WB_UTINY *)"ds:KeyInfo",     0x00, 0x0B },
    { (WB_UTINY *)"ds:KeyValue",    0x00, 0x0C },
    { (WB_UTINY *)"o-ex:permission",0x00, 0x0D },
    { (WB_UTINY *)"o-dd:play",      0x00, 0x0E },
    { (WB_UTINY *)"o-dd:display",   0x00, 0x0F },
    { (WB_UTINY *)"o-dd:execute",   0x00, 0x10 },
    { (WB_UTINY *)"o-dd:print",     0x00, 0x11 },
    { (WB_UTINY *)"o-ex:constraint",0x00, 0x12 },
    { (WB_UTINY *)"o-dd:count",     0x00, 0x13 },
    { (WB_UTINY *)"o-dd:datetime",  0x00, 0x14 },
    { (WB_UTINY *)"o-dd:start",     0x00, 0x15 },
    { (WB_UTINY *)"o-dd:end",       0x00, 0x16 },
    { (WB_UTINY *)"o-dd:interval",  0x00, 0x17 },
    { NULL,             0x00, 0x00 }
};

const WBXMLAttrEntry sv_drmrel10_attr_table[] = {
    { (WB_UTINY *)"xmlns:o-ex",     NULL,       0x00, 0x05 },
    { (WB_UTINY *)"xmlns:o-dd",     NULL,       0x00, 0x06 },
    { (WB_UTINY *)"xmlns:ds",       NULL,       0x00, 0x07 },
    { NULL,             NULL,       0x00, 0x00 }
};

const WBXMLAttrValueEntry sv_drmrel10_attr_value_table[] = {
    { (WB_UTINY *)"http://odrl.net/1.1/ODRL-EX",        0x00, 0x85 },
    { (WB_UTINY *)"http://odrl.net/1.1/ODRL-DD",        0x00, 0x86 },
    { (WB_UTINY *)"http://www.w3.org/2000/09/xmldsig#", 0x00, 0x87 },
    { NULL,                                 0x00, 0x00 }
};


/******************************************************
 *    SyncML 1.1 ("syncml_represent_v11_20020215.pdf")
 */

const WBXMLTagEntry sv_syncml_syncml11_tag_table[] = {
    /* Code Page 0: SyncML */
    { (WB_UTINY *)"Add",            0x00, 0x05 },
    { (WB_UTINY *)"Alert",          0x00, 0x06 },
    { (WB_UTINY *)"Archive",        0x00, 0x07 },
    { (WB_UTINY *)"Atomic",         0x00, 0x08 },
    { (WB_UTINY *)"Chal",           0x00, 0x09 },
    { (WB_UTINY *)"Cmd",            0x00, 0x0a },
    { (WB_UTINY *)"CmdID",          0x00, 0x0b },
    { (WB_UTINY *)"CmdRef",         0x00, 0x0c },
    { (WB_UTINY *)"Copy",           0x00, 0x0d },
    { (WB_UTINY *)"Cred",           0x00, 0x0e },
    { (WB_UTINY *)"Data",           0x00, 0x0f },
    { (WB_UTINY *)"Delete",         0x00, 0x10 },
    { (WB_UTINY *)"Exec",           0x00, 0x11 },
    { (WB_UTINY *)"Final",          0x00, 0x12 },
    { (WB_UTINY *)"Get",            0x00, 0x13 },
    { (WB_UTINY *)"Item",           0x00, 0x14 },
    { (WB_UTINY *)"Lang",           0x00, 0x15 },
    { (WB_UTINY *)"LocName",        0x00, 0x16 },
    { (WB_UTINY *)"LocURI",         0x00, 0x17 },
    { (WB_UTINY *)"Map",            0x00, 0x18 },
    { (WB_UTINY *)"MapItem",        0x00, 0x19 },
    { (WB_UTINY *)"Meta",           0x00, 0x1a },
    { (WB_UTINY *)"MsgID",          0x00, 0x1b },
    { (WB_UTINY *)"MsgRef",         0x00, 0x1c },
    { (WB_UTINY *)"NoResp",         0x00, 0x1d },
    { (WB_UTINY *)"NoResults",      0x00, 0x1e },
    { (WB_UTINY *)"Put",            0x00, 0x1f },
    { (WB_UTINY *)"Replace",        0x00, 0x20 },
    { (WB_UTINY *)"RespURI",        0x00, 0x21 },
    { (WB_UTINY *)"Results",        0x00, 0x22 },
    { (WB_UTINY *)"Search",         0x00, 0x23 },
    { (WB_UTINY *)"Sequence",       0x00, 0x24 },
    { (WB_UTINY *)"SessionID",      0x00, 0x25 },
    { (WB_UTINY *)"SftDel",         0x00, 0x26 },
    { (WB_UTINY *)"Source",         0x00, 0x27 },
    { (WB_UTINY *)"SourceRef",      0x00, 0x28 },
    { (WB_UTINY *)"Status",         0x00, 0x29 },
    { (WB_UTINY *)"Sync",           0x00, 0x2a },
    { (WB_UTINY *)"SyncBody",       0x00, 0x2b },
    { (WB_UTINY *)"SyncHdr",        0x00, 0x2c },
    { (WB_UTINY *)"SyncML",         0x00, 0x2d },
    { (WB_UTINY *)"Target",         0x00, 0x2e },
    { (WB_UTINY *)"TargetRef",      0x00, 0x2f },
    { (WB_UTINY *)"Reserved for future use",    0x00, 0x30 },
    { (WB_UTINY *)"VerDTD",         0x00, 0x31 },
    { (WB_UTINY *)"VerProto",       0x00, 0x32 },
    { (WB_UTINY *)"NumberOfChanged",0x00, 0x33 },
    { (WB_UTINY *)"MoreData",       0x00, 0x34 },

    /* Code Page 1: MetInf11 */
    { (WB_UTINY *)"Anchor",         0x01, 0x05 },
    { (WB_UTINY *)"EMI",            0x01, 0x06 },
    { (WB_UTINY *)"Format",         0x01, 0x07 },
    { (WB_UTINY *)"FreeID",         0x01, 0x08 },
    { (WB_UTINY *)"FreeMem",        0x01, 0x09 },
    { (WB_UTINY *)"Last",           0x01, 0x0a },
    { (WB_UTINY *)"Mark",           0x01, 0x0b },
    { (WB_UTINY *)"MaxMsgSize",     0x01, 0x0c },
    { (WB_UTINY *)"MaxObjSize",     0x01, 0x15 },
    { (WB_UTINY *)"Mem",            0x01, 0x0d },
    { (WB_UTINY *)"MetInf",         0x01, 0x0e },
    { (WB_UTINY *)"Next",           0x01, 0x0f },
    { (WB_UTINY *)"NextNonce",      0x01, 0x10 },
    { (WB_UTINY *)"SharedMem",      0x01, 0x11 },
    { (WB_UTINY *)"Size",           0x01, 0x12 },
    { (WB_UTINY *)"Type",           0x01, 0x13 },
    { (WB_UTINY *)"Version",        0x01, 0x14 },
    { NULL,             0x00, 0x00 }
};



/*********************************************************
 *    SyncML DevInf 1.1 ("syncml_devinf_v11_20020215.pdf")
 */

const WBXMLTagEntry sv_syncml_devinf11_tag_table[] = {
    { (WB_UTINY *)"CTCap",          0x00, 0x05 },
    { (WB_UTINY *)"CTType",         0x00, 0x06 },
    { (WB_UTINY *)"DataStore",      0x00, 0x07 },
    { (WB_UTINY *)"DataType",       0x00, 0x08 },
    { (WB_UTINY *)"DevId",          0x00, 0x09 },
    { (WB_UTINY *)"DevInf",         0x00, 0x0a },
    { (WB_UTINY *)"DevTyp",         0x00, 0x0b },
    { (WB_UTINY *)"DisplayName",    0x00, 0x0c },
    { (WB_UTINY *)"DSMem",          0x00, 0x0d },
    { (WB_UTINY *)"Ext",            0x00, 0x0e },
    { (WB_UTINY *)"FwV",            0x00, 0x0f },
    { (WB_UTINY *)"HwV",            0x00, 0x10 },
    { (WB_UTINY *)"Man",            0x00, 0x11 },
    { (WB_UTINY *)"MaxGUIDSize",    0x00, 0x12 },
    { (WB_UTINY *)"MaxID",          0x00, 0x13 },
    { (WB_UTINY *)"MaxMem",         0x00, 0x14 },
    { (WB_UTINY *)"Mod",            0x00, 0x15 },
    { (WB_UTINY *)"OEM",            0x00, 0x16 },
    { (WB_UTINY *)"ParamName",      0x00, 0x17 },
    { (WB_UTINY *)"PropName",       0x00, 0x18 },
    { (WB_UTINY *)"Rx",             0x00, 0x19 },
    { (WB_UTINY *)"Rx-Pref",        0x00, 0x1a },
    { (WB_UTINY *)"SharedMem",      0x00, 0x1b },
    { (WB_UTINY *)"Size",           0x00, 0x1c },
    { (WB_UTINY *)"SourceRef",      0x00, 0x1d },
    { (WB_UTINY *)"SwV",            0x00, 0x1e },
    { (WB_UTINY *)"SyncCap",        0x00, 0x1f },
    { (WB_UTINY *)"SyncType",       0x00, 0x20 },
    { (WB_UTINY *)"Tx",             0x00, 0x21 },
    { (WB_UTINY *)"Tx-Pref",        0x00, 0x22 },
    { (WB_UTINY *)"ValEnum",        0x00, 0x23 },
    { (WB_UTINY *)"VerCT",          0x00, 0x24 },
    { (WB_UTINY *)"VerDTD",         0x00, 0x25 },
    { (WB_UTINY *)"Xname",          0x00, 0x26 },
    { (WB_UTINY *)"Xval",           0x00, 0x27 },
    { (WB_UTINY *)"UTC",            0x00, 0x28 },
    { (WB_UTINY *)"SupportNumberOfChanges", 0x00, 0x29 },
    { (WB_UTINY *)"SupportLargeObjs",       0x00, 0x2a },
    { NULL,                0x00, 0x00 }
};


/*********************************************************
 *    SyncML MetInf 1.1 ("syncml_metinf_v11_20020215.pdf")
 */

const WBXMLTagEntry sv_syncml_metinf11_tag_table[] = {
    { (WB_UTINY *)"Anchor",         0x01, 0x05 },
    { (WB_UTINY *)"EMI",            0x01, 0x06 },
    { (WB_UTINY *)"Format",         0x01, 0x07 },
    { (WB_UTINY *)"FreeID",         0x01, 0x08 },
    { (WB_UTINY *)"FreeMem",        0x01, 0x09 },
    { (WB_UTINY *)"Last",           0x01, 0x0a },
    { (WB_UTINY *)"Mark",           0x01, 0x0b },
    { (WB_UTINY *)"MaxMsgSize",     0x01, 0x0c },
    { (WB_UTINY *)"MaxObjSize",     0x01, 0x15 },
    { (WB_UTINY *)"Mem",            0x01, 0x0d },
    { (WB_UTINY *)"MetInf",         0x01, 0x0e },
    { (WB_UTINY *)"Next",           0x01, 0x0f },
    { (WB_UTINY *)"NextNonce",      0x01, 0x10 },
    { (WB_UTINY *)"SharedMem",      0x01, 0x11 },
    { (WB_UTINY *)"Size",           0x01, 0x12 },
    { (WB_UTINY *)"Type",           0x01, 0x13 },
    { (WB_UTINY *)"Version",        0x01, 0x14 },
    { NULL,             0x00, 0x00 }
};


/*****************************************************************************
 *    Wireless Village CSP 1.1 ("OMA-WV-CSP-V1_1-20021001-A.pdf")
 *    Wireless Village CSP 1.2 ("OMA-IMPS-WV-CSP_WBXML-v1_2-20030221-C.PDF")
 */

const WBXMLTagEntry sv_wv_csp_tag_table[] = {
    /* Common ... continue on Page 0x09 */
    { (WB_UTINY *)"Acceptance",     0x00, 0x05 },
    { (WB_UTINY *)"AddList",        0x00, 0x06 },
    { (WB_UTINY *)"AddNickList",    0x00, 0x07 },
    { (WB_UTINY *)"ClientID",       0x00, 0x0A },
    { (WB_UTINY *)"Code",           0x00, 0x0B },
    { (WB_UTINY *)"ContactList",    0x00, 0x0C },
    { (WB_UTINY *)"ContentData",    0x00, 0x0D },
    { (WB_UTINY *)"ContentEncoding",0x00, 0x0E },
    { (WB_UTINY *)"ContentSize",    0x00, 0x0F },
    { (WB_UTINY *)"ContentType",    0x00, 0x10 },
    { (WB_UTINY *)"DateTime",       0x00, 0x11 },
    { (WB_UTINY *)"Description",    0x00, 0x12 },
    { (WB_UTINY *)"DetailedResult", 0x00, 0x13 },
    { (WB_UTINY *)"EntityList",     0x00, 0x14 },
    { (WB_UTINY *)"Group",          0x00, 0x15 },
    { (WB_UTINY *)"GroupID",        0x00, 0x16 },
    { (WB_UTINY *)"GroupList",      0x00, 0x17 },
    { (WB_UTINY *)"InUse",          0x00, 0x18 },
    { (WB_UTINY *)"Logo",           0x00, 0x19 },
    { (WB_UTINY *)"MessageCount",   0x00, 0x1A },
    { (WB_UTINY *)"MessageID",      0x00, 0x1B },
    { (WB_UTINY *)"MessageURI",     0x00, 0x1C },
    { (WB_UTINY *)"MSISDN",         0x00, 0x1D },
    { (WB_UTINY *)"Name",           0x00, 0x1E },
    { (WB_UTINY *)"NickList",       0x00, 0x1F },
    { (WB_UTINY *)"NickName",       0x00, 0x20 },
    { (WB_UTINY *)"Poll",           0x00, 0x21 },
    { (WB_UTINY *)"Presence",       0x00, 0x22 },
    { (WB_UTINY *)"PresenceSubList",0x00, 0x23 },
    { (WB_UTINY *)"PresenceValue",  0x00, 0x24 },
    { (WB_UTINY *)"Property",       0x00, 0x25 },
    { (WB_UTINY *)"Qualifier",      0x00, 0x26 },
    { (WB_UTINY *)"Recipient",      0x00, 0x27 },
    { (WB_UTINY *)"RemoveList",     0x00, 0x28 },
    { (WB_UTINY *)"RemoveNickList", 0x00, 0x29 },
    { (WB_UTINY *)"Result",         0x00, 0x2A },
    { (WB_UTINY *)"ScreenName",     0x00, 0x2B },
    { (WB_UTINY *)"Sender",         0x00, 0x2C },
    { (WB_UTINY *)"Session",        0x00, 0x2D },
    { (WB_UTINY *)"SessionDescriptor",      0x00, 0x2E },
    { (WB_UTINY *)"SessionID",              0x00, 0x2F },
    { (WB_UTINY *)"SessionType",            0x00, 0x30 },
    { (WB_UTINY *)"SName",                  0x00, 0x08 },
    { (WB_UTINY *)"Status",                 0x00, 0x31 },
    { (WB_UTINY *)"Transaction",            0x00, 0x32 },
    { (WB_UTINY *)"TransactionContent",     0x00, 0x33 },
    { (WB_UTINY *)"TransactionDescriptor",  0x00, 0x34 },
    { (WB_UTINY *)"TransactionID",  0x00, 0x35 },
    { (WB_UTINY *)"TransactionMode",0x00, 0x36 },
    { (WB_UTINY *)"URL",            0x00, 0x37 },
    { (WB_UTINY *)"URLList",        0x00, 0x38 },
    { (WB_UTINY *)"User",           0x00, 0x39 },
    { (WB_UTINY *)"UserID",         0x00, 0x3A },
    { (WB_UTINY *)"UserList",       0x00, 0x3B },
    { (WB_UTINY *)"Validity",       0x00, 0x3C },
    { (WB_UTINY *)"Value",          0x00, 0x3D },
    { (WB_UTINY *)"WV-CSP-Message", 0x00, 0x09 },
    
    /* Access ... continue on Page 0x0A */
    { (WB_UTINY *)"AgreedCapabilityList",       0x01, 0x3A }, /* WV 1.2 */
    { (WB_UTINY *)"AllFunctions",               0x01, 0x05 },
    { (WB_UTINY *)"AllFunctionsRequest",        0x01, 0x06 },
    { (WB_UTINY *)"CancelInvite-Request",       0x01, 0x07 },
    { (WB_UTINY *)"CancelInviteUser-Request",   0x01, 0x08 },
    { (WB_UTINY *)"Capability",                 0x01, 0x09 },
    { (WB_UTINY *)"CapabilityList",             0x01, 0x0A },
    { (WB_UTINY *)"CapabilityRequest",          0x01, 0x0B },
    { (WB_UTINY *)"ClientCapability-Request",   0x01, 0x0C },
    { (WB_UTINY *)"ClientCapability-Response",  0x01, 0x0D },
    { (WB_UTINY *)"CompletionFlag",         0x01, 0x34 },
    { (WB_UTINY *)"DigestBytes",            0x01, 0x0E },
    { (WB_UTINY *)"DigestSchema",           0x01, 0x0F },
    { (WB_UTINY *)"Disconnect",             0x01, 0x10 },
    { (WB_UTINY *)"Extended-Request",       0x01, 0x38 }, /* WV 1.2 */
    { (WB_UTINY *)"Extended-Response",      0x01, 0x39 }, /* WV 1.2 */
    { (WB_UTINY *)"Extended-Data",          0x01, 0x3B }, /* WV 1.2 */
    { (WB_UTINY *)"Functions",              0x01, 0x11 },
    { (WB_UTINY *)"GetSPInfo-Request",      0x01, 0x12 },
    { (WB_UTINY *)"GetSPInfo-Response",     0x01, 0x13 },
    { (WB_UTINY *)"InviteID",               0x01, 0x14 },
    { (WB_UTINY *)"InviteNote",             0x01, 0x15 },
    { (WB_UTINY *)"Invite-Request",         0x01, 0x16 },
    { (WB_UTINY *)"Invite-Response",        0x01, 0x17 },
    { (WB_UTINY *)"InviteType",             0x01, 0x18 },
    { (WB_UTINY *)"InviteUser-Request",     0x01, 0x19 },
    { (WB_UTINY *)"InviteUser-Response",    0x01, 0x1A },
    { (WB_UTINY *)"KeepAlive-Request",      0x01, 0x1B },
    { (WB_UTINY *)"KeepAlive-Response",     0x01, 0x29 },
    { (WB_UTINY *)"KeepAliveTime",          0x01, 0x1C },
    { (WB_UTINY *)"Login-Request",          0x01, 0x1D },
    { (WB_UTINY *)"Login-Response",         0x01, 0x1E },
    { (WB_UTINY *)"Logout-Request",         0x01, 0x1F },
    { (WB_UTINY *)"Nonce",                  0x01, 0x20 },
    { (WB_UTINY *)"OtherServer",            0x01, 0x3C }, /* WV 1.2 */
    { (WB_UTINY *)"Password",               0x01, 0x21 },
    { (WB_UTINY *)"Polling-Request",        0x01, 0x22 },
    { (WB_UTINY *)"PresenceAttributeNSName",0x01, 0x3D }, /* WV 1.2 */
    { (WB_UTINY *)"ReceiveList",            0x01, 0x36 }, /* WV 1.2 */
    { (WB_UTINY *)"ResponseNote",           0x01, 0x23 },
    { (WB_UTINY *)"SearchElement",          0x01, 0x24 },
    { (WB_UTINY *)"SearchFindings",         0x01, 0x25 },
    { (WB_UTINY *)"SearchID",               0x01, 0x26 },
    { (WB_UTINY *)"SearchIndex",            0x01, 0x27 },
    { (WB_UTINY *)"SearchLimit",            0x01, 0x28 },
    { (WB_UTINY *)"SearchPairList",         0x01, 0x2A },
    { (WB_UTINY *)"Search-Request",         0x01, 0x2B },
    { (WB_UTINY *)"Search-Response",        0x01, 0x2C },
    { (WB_UTINY *)"SearchResult",           0x01, 0x2D },
    { (WB_UTINY *)"SearchString",           0x01, 0x33 },
    { (WB_UTINY *)"Service-Request",        0x01, 0x2E },
    { (WB_UTINY *)"Service-Response",       0x01, 0x2F },
    { (WB_UTINY *)"SessionCookie",          0x01, 0x30 },
    { (WB_UTINY *)"SessionNSName",          0x01, 0x3E }, /* WV 1.2 */
    { (WB_UTINY *)"StopSearch-Request",     0x01, 0x31 },
    { (WB_UTINY *)"TimeToLive",             0x01, 0x32 },
    { (WB_UTINY *)"TransactionNSName",      0x01, 0x3F }, /* WV 1.2 */
    { (WB_UTINY *)"VerifyID-Request",       0x01, 0x37 }, /* WV 1.2 */
        
    /* Service ... continue on Page 0x08 */
    { (WB_UTINY *)"ADDGM",          0x02, 0x05 },
    { (WB_UTINY *)"AttListFunc",    0x02, 0x06 },
    { (WB_UTINY *)"BLENT",          0x02, 0x07 },
    { (WB_UTINY *)"CAAUT",          0x02, 0x08 },
    { (WB_UTINY *)"CAINV",          0x02, 0x09 },
    { (WB_UTINY *)"CALI",           0x02, 0x0A },
    { (WB_UTINY *)"CCLI",           0x02, 0x0B },
    { (WB_UTINY *)"ContListFunc",   0x02, 0x0C },
    { (WB_UTINY *)"CREAG",          0x02, 0x0D },
    { (WB_UTINY *)"DALI",           0x02, 0x0E },
    { (WB_UTINY *)"DCLI",           0x02, 0x0F },
    { (WB_UTINY *)"DELGR",          0x02, 0x10 },
    { (WB_UTINY *)"FundamentalFeat",0x02, 0x11 },
    { (WB_UTINY *)"FWMSG",          0x02, 0x12 },
    { (WB_UTINY *)"GALS",           0x02, 0x13 },
    { (WB_UTINY *)"GCLI",           0x02, 0x14 },
    { (WB_UTINY *)"GETGM",          0x02, 0x15 },
    { (WB_UTINY *)"GETGP",          0x02, 0x16 },
    { (WB_UTINY *)"GETLM",          0x02, 0x17 },
    { (WB_UTINY *)"GETM",           0x02, 0x18 },
    { (WB_UTINY *)"GETPR",          0x02, 0x19 },
    { (WB_UTINY *)"GETSPI",         0x02, 0x1A },
    { (WB_UTINY *)"GETWL",          0x02, 0x1B },
    { (WB_UTINY *)"GLBLU",          0x02, 0x1C },
    { (WB_UTINY *)"GRCHN",          0x02, 0x1D },
    { (WB_UTINY *)"GroupAuthFunc",  0x02, 0x1E },
    { (WB_UTINY *)"GroupFeat",      0x02, 0x1F },
    { (WB_UTINY *)"GroupMgmtFunc",  0x02, 0x20 },
    { (WB_UTINY *)"GroupUseFunc",   0x02, 0x21 },
    { (WB_UTINY *)"IMAuthFunc",     0x02, 0x22 },
    { (WB_UTINY *)"IMFeat",         0x02, 0x23 },
    { (WB_UTINY *)"IMReceiveFunc",  0x02, 0x24 },
    { (WB_UTINY *)"IMSendFunc",     0x02, 0x25 },
    { (WB_UTINY *)"INVIT",          0x02, 0x26 },
    { (WB_UTINY *)"InviteFunc",     0x02, 0x27 },
    { (WB_UTINY *)"MBRAC",          0x02, 0x28 },
    { (WB_UTINY *)"MCLS",           0x02, 0x29 },
    { (WB_UTINY *)"MF",             0x02, 0x3D }, /* WV 1.2 */
    { (WB_UTINY *)"MG",             0x02, 0x3E }, /* WV 1.2 */
    { (WB_UTINY *)"MM",             0x02, 0x3F }, /* WV 1.2 */
    { (WB_UTINY *)"MDELIV",         0x02, 0x2A },
    { (WB_UTINY *)"NEWM",           0x02, 0x2B },
    { (WB_UTINY *)"NOTIF",          0x02, 0x2C },
    { (WB_UTINY *)"PresenceAuthFunc",   0x02, 0x2D },
    { (WB_UTINY *)"PresenceDeliverFunc",0x02, 0x2E },
    { (WB_UTINY *)"PresenceFeat",       0x02, 0x2F },
    { (WB_UTINY *)"REACT",          0x02, 0x30 },
    { (WB_UTINY *)"REJCM",          0x02, 0x31 },
    { (WB_UTINY *)"REJEC",          0x02, 0x32 },
    { (WB_UTINY *)"RMVGM",          0x02, 0x33 },
    { (WB_UTINY *)"SearchFunc",     0x02, 0x34 },
    { (WB_UTINY *)"ServiceFunc",    0x02, 0x35 },
    { (WB_UTINY *)"SETD",           0x02, 0x36 },
    { (WB_UTINY *)"SETGP",          0x02, 0x37 },
    { (WB_UTINY *)"SRCH",           0x02, 0x38 },
    { (WB_UTINY *)"STSRC",          0x02, 0x39 },
    { (WB_UTINY *)"SUBGCN",         0x02, 0x3A },
    { (WB_UTINY *)"UPDPR",          0x02, 0x3B },
    { (WB_UTINY *)"VRID",           0x02, 0x3E }, /* WV 1.2 : BUG IN SPEC ! The same token than 'MG' */
    { (WB_UTINY *)"WVCSPFeat",      0x02, 0x3C },
    
    /* Client Capability */
    { (WB_UTINY *)"AcceptedCharset",            0x03, 0x05 },
    { (WB_UTINY *)"AcceptedContentLength",      0x03, 0x06 },
    { (WB_UTINY *)"AcceptedContentType",        0x03, 0x07 },
    { (WB_UTINY *)"AcceptedTransferEncoding",   0x03, 0x08 },
    { (WB_UTINY *)"AnyContent",                 0x03, 0x09 },
    { (WB_UTINY *)"DefaultLanguage",            0x03, 0x0A },
    { (WB_UTINY *)"InitialDeliveryMethod",      0x03, 0x0B },
    { (WB_UTINY *)"MultiTrans",                 0x03, 0x0C },
    { (WB_UTINY *)"ParserSize",                 0x03, 0x0D },
    { (WB_UTINY *)"ServerPollMin",              0x03, 0x0E },
    { (WB_UTINY *)"SupportedBearer",            0x03, 0x0F },
    { (WB_UTINY *)"SupportedCIRMethod",         0x03, 0x10 },
    { (WB_UTINY *)"TCPAddress",                 0x03, 0x11 },
    { (WB_UTINY *)"TCPPort",                    0x03, 0x12 },
    { (WB_UTINY *)"UDPPort",                    0x03, 0x13 },    
    
    /* Presence Primitive */
    { (WB_UTINY *)"CancelAuth-Request",             0x04, 0x1E }, /* WV 1.2 */
    { (WB_UTINY *)"CancelAuth-Request",             0x04, 0x05 },
    { (WB_UTINY *)"ContactListProperties",          0x04, 0x06 },
    { (WB_UTINY *)"CreateAttributeList-Request",    0x04, 0x07 },
    { (WB_UTINY *)"CreateList-Request",             0x04, 0x08 },
    { (WB_UTINY *)"DefaultAttributeList",           0x04, 0x09 },
    { (WB_UTINY *)"DefaultContactList",             0x04, 0x0A },
    { (WB_UTINY *)"DefaultList",                    0x04, 0x0B },
    { (WB_UTINY *)"DeleteAttributeList-Request",    0x04, 0x0C },
    { (WB_UTINY *)"DeleteList-Request",             0x04, 0x0D },
    { (WB_UTINY *)"GetAttributeList-Request",       0x04, 0x0E },
    { (WB_UTINY *)"GetAttributeList-Response",      0x04, 0x0F },
    { (WB_UTINY *)"GetList-Request",                0x04, 0x10 },
    { (WB_UTINY *)"GetList-Response",               0x04, 0x11 },
    { (WB_UTINY *)"GetPresence-Request",            0x04, 0x12 },
    { (WB_UTINY *)"GetPresence-Response",           0x04, 0x13 },
    { (WB_UTINY *)"GetReactiveAuthStatus-Request",  0x04, 0x1F }, /* WV 1.2 */
    { (WB_UTINY *)"GetReactiveAuthStatus-Response", 0x04, 0x20 }, /* WV 1.2 */
    { (WB_UTINY *)"GetWatcherList-Request",         0x04, 0x14 },
    { (WB_UTINY *)"GetWatcherList-Response",        0x04, 0x15 },
    { (WB_UTINY *)"ListManage-Request",             0x04, 0x16 },
    { (WB_UTINY *)"ListManage-Response",            0x04, 0x17 },
    { (WB_UTINY *)"PresenceAuth-Request",           0x04, 0x19 },
    { (WB_UTINY *)"PresenceAuth-User",              0x04, 0x1A },
    { (WB_UTINY *)"PresenceNotification-Request",   0x04, 0x1B },
    { (WB_UTINY *)"SubscribePresence-Request",      0x04, 0x1D },
    { (WB_UTINY *)"UnsubscribePresence-Request",    0x04, 0x18 },
    { (WB_UTINY *)"UpdatePresence-Request",         0x04, 0x1C },
    
    /* Presence Attribute */
    { (WB_UTINY *)"Accuracy",           0x05, 0x05 },
    { (WB_UTINY *)"Address",            0x05, 0x06 },
    { (WB_UTINY *)"AddrPref",           0x05, 0x07 },
    { (WB_UTINY *)"Alias",              0x05, 0x08 },
    { (WB_UTINY *)"Altitude",           0x05, 0x09 },
    { (WB_UTINY *)"Building",           0x05, 0x0A },
    { (WB_UTINY *)"Caddr",              0x05, 0x0B },
    { (WB_UTINY *)"Cap",                0x05, 0x2F },
    { (WB_UTINY *)"City",               0x05, 0x0C },
    { (WB_UTINY *)"ClientInfo",         0x05, 0x0D },
    { (WB_UTINY *)"ClientProducer",     0x05, 0x0E },
    { (WB_UTINY *)"ClientType",         0x05, 0x0F },
    { (WB_UTINY *)"ClientVersion",      0x05, 0x10 },
    { (WB_UTINY *)"Cname",              0x05, 0x30 },
    { (WB_UTINY *)"CommC",              0x05, 0x11 },
    { (WB_UTINY *)"CommCap",            0x05, 0x12 },
    { (WB_UTINY *)"Contact",            0x05, 0x31 },
    { (WB_UTINY *)"ContactInfo",        0x05, 0x13 },
    { (WB_UTINY *)"ContainedvCard",     0x05, 0x14 },
    { (WB_UTINY *)"ContentType",        0x05, 0x36 }, /* WV 1.2 */
    { (WB_UTINY *)"Country",            0x05, 0x15 },
    { (WB_UTINY *)"Cpriority",          0x05, 0x32 },
    { (WB_UTINY *)"Crossing1",          0x05, 0x16 },
    { (WB_UTINY *)"Crossing2",          0x05, 0x17 },
    { (WB_UTINY *)"Cstatus",            0x05, 0x33 },
    { (WB_UTINY *)"DevManufacturer",    0x05, 0x18 },
    { (WB_UTINY *)"DirectContent",      0x05, 0x19 },
    { (WB_UTINY *)"FreeTextLocation",   0x05, 0x1A },
    { (WB_UTINY *)"GeoLocation",        0x05, 0x1B },
    { (WB_UTINY *)"Inf_link",           0x05, 0x37 }, /* WV 1.2 */
    { (WB_UTINY *)"InfoLink",           0x05, 0x38 }, /* WV 1.2 */
    { (WB_UTINY *)"Language",           0x05, 0x1C },
    { (WB_UTINY *)"Latitude",           0x05, 0x1D },
    { (WB_UTINY *)"Link",               0x05, 0x39 }, /* WV 1.2 */
    { (WB_UTINY *)"Longitude",          0x05, 0x1E },
    { (WB_UTINY *)"Model",              0x05, 0x1F },
    { (WB_UTINY *)"NamedArea",          0x05, 0x20 },    
    { (WB_UTINY *)"Note",               0x05, 0x34 }, /* WV 1.2 */
    { (WB_UTINY *)"OnlineStatus",       0x05, 0x21 },
    { (WB_UTINY *)"PLMN",               0x05, 0x22 },
    { (WB_UTINY *)"PrefC",              0x05, 0x23 },
    { (WB_UTINY *)"PreferredContacts",  0x05, 0x24 },
    { (WB_UTINY *)"PreferredLanguage",  0x05, 0x25 },
    { (WB_UTINY *)"PreferredContent",   0x05, 0x26 },
    { (WB_UTINY *)"PreferredvCard",     0x05, 0x27 },
    { (WB_UTINY *)"Registration",       0x05, 0x28 },
    { (WB_UTINY *)"StatusContent",      0x05, 0x29 },
    { (WB_UTINY *)"StatusMood",         0x05, 0x2A },
    { (WB_UTINY *)"StatusText",         0x05, 0x2B },
    { (WB_UTINY *)"Street",             0x05, 0x2C },
    { (WB_UTINY *)"Text",               0x05, 0x3A }, /* WV 1.2 */
    { (WB_UTINY *)"TimeZone",           0x05, 0x2D },
    { (WB_UTINY *)"UserAvailability",   0x05, 0x2E },
    { (WB_UTINY *)"Zone",               0x05, 0x35 },
        
    /* Messaging */
    { (WB_UTINY *)"BlockList",                  0x06, 0x05 },
    { (WB_UTINY *)"BlockUser-Request",          0x06, 0x06 },
    { (WB_UTINY *)"DeliveryMethod",             0x06, 0x07 },
    { (WB_UTINY *)"DeliveryReport",             0x06, 0x08 },
    { (WB_UTINY *)"DeliveryReport-Request",     0x06, 0x09 },
    { (WB_UTINY *)"DeliveryTime",               0x06, 0x1A },
    { (WB_UTINY *)"ForwardMessage-Request",     0x06, 0x0A },
    { (WB_UTINY *)"GetBlockedList-Request",     0x06, 0x0B },
    { (WB_UTINY *)"GetBlockedList-Response",    0x06, 0x0C },
    { (WB_UTINY *)"GetMessageList-Request",     0x06, 0x0D },
    { (WB_UTINY *)"GetMessageList-Response",    0x06, 0x0E },
    { (WB_UTINY *)"GetMessage-Request",         0x06, 0x0F },
    { (WB_UTINY *)"GetMessage-Response",        0x06, 0x10 },
    { (WB_UTINY *)"GrantList",                  0x06, 0x11 },
    { (WB_UTINY *)"MessageDelivered",           0x06, 0x12 },
    { (WB_UTINY *)"MessageInfo",                0x06, 0x13 },
    { (WB_UTINY *)"MessageNotification",        0x06, 0x14 },
    { (WB_UTINY *)"NewMessage",                 0x06, 0x15 },
    { (WB_UTINY *)"RejectMessage-Request",      0x06, 0x16 },
    { (WB_UTINY *)"SendMessage-Request",        0x06, 0x17 },
    { (WB_UTINY *)"SendMessage-Response",       0x06, 0x18 },
    { (WB_UTINY *)"SetDeliveryMethod-Request",  0x06, 0x19 },
    
    /* Group */
    { (WB_UTINY *)"AddGroupMembers-Request",    0x07, 0x05 },
    { (WB_UTINY *)"Admin",                      0x07, 0x06 },
    { (WB_UTINY *)"AdminMapList",               0x07, 0x26 }, /* WV 1.2 */
    { (WB_UTINY *)"AdminMapping",               0x07, 0x27 }, /* WV 1.2 */
    { (WB_UTINY *)"CreateGroup-Request",        0x07, 0x07 },
    { (WB_UTINY *)"DeleteGroup-Request",        0x07, 0x08 },
    { (WB_UTINY *)"GetGroupMembers-Request",    0x07, 0x09 },
    { (WB_UTINY *)"GetGroupMembers-Response",   0x07, 0x0A },
    { (WB_UTINY *)"GetGroupProps-Request",      0x07, 0x0B },
    { (WB_UTINY *)"GetGroupProps-Response",     0x07, 0x0C },
    { (WB_UTINY *)"GetJoinedUsers-Request",     0x07, 0x24 }, /* WV 1.2 */
    { (WB_UTINY *)"GetJoinedUsers-Response",    0x07, 0x25 }, /* WV 1.2 */
    { (WB_UTINY *)"GroupChangeNotice",          0x07, 0x0D },
    { (WB_UTINY *)"GroupProperties",            0x07, 0x0E },
    { (WB_UTINY *)"Joined",                     0x07, 0x0F },
    { (WB_UTINY *)"JoinGroup",                  0x07, 0x21 },
    { (WB_UTINY *)"JoinedRequest",              0x07, 0x10 },
    { (WB_UTINY *)"JoinGroup-Request",          0x07, 0x11 },
    { (WB_UTINY *)"JoinGroup-Response",         0x07, 0x12 },
    { (WB_UTINY *)"LeaveGroup-Request",         0x07, 0x13 },
    { (WB_UTINY *)"LeaveGroup-Response",        0x07, 0x14 },
    { (WB_UTINY *)"Left",                       0x07, 0x15 },
    { (WB_UTINY *)"Mapping",                    0x07, 0x28 }, /* WV 1.2 */
    { (WB_UTINY *)"MemberAccess-Request",       0x07, 0x16 },
    { (WB_UTINY *)"Mod",                        0x07, 0x17 },
    { (WB_UTINY *)"ModMapping",                 0x07, 0x29 }, /* WV 1.2 */
    { (WB_UTINY *)"OwnProperties",              0x07, 0x18 },
    { (WB_UTINY *)"RejectList-Request",         0x07, 0x19 },
    { (WB_UTINY *)"RejectList-Response",        0x07, 0x1A },
    { (WB_UTINY *)"RemoveGroupMembers-Request", 0x07, 0x1B },
    { (WB_UTINY *)"SetGroupProps-Request",      0x07, 0x1C },
    { (WB_UTINY *)"SubscribeGroupNotice-Request",   0x07, 0x1D },
    { (WB_UTINY *)"SubscribeGroupNotice-Response",  0x07, 0x1E },
    { (WB_UTINY *)"SubscribeNotification",          0x07, 0x22 },
    { (WB_UTINY *)"SubscribeType",                  0x07, 0x23 },
    { (WB_UTINY *)"UserMapList",                0x07, 0x2A }, /* WV 1.2 */
    { (WB_UTINY *)"UserMapping",                0x07, 0x2B }, /* WV 1.2 */
    { (WB_UTINY *)"Users",                      0x07, 0x1F },
    { (WB_UTINY *)"WelcomeNote",                0x07, 0x20 },

    /* Service ... continued */
    { (WB_UTINY *)"GETAUT",                     0x08, 0x06 }, /* WV 1.2 */
    { (WB_UTINY *)"GETJU",                      0x08, 0x07 }, /* WV 1.2 */
    { (WB_UTINY *)"MP",                         0x08, 0x05 }, /* WV 1.2 */

    /* Common ... continued */
    { (WB_UTINY *)"CIR",                        0x09, 0x05 }, /* WV 1.2 */
    { (WB_UTINY *)"Domain",                     0x09, 0x06 }, /* WV 1.2 */
    { (WB_UTINY *)"ExtBlock",                   0x09, 0x07 }, /* WV 1.2 */
    { (WB_UTINY *)"HistoryPeriod",              0x09, 0x08 }, /* WV 1.2 */
    { (WB_UTINY *)"IDList",                     0x09, 0x09 }, /* WV 1.2 */
    { (WB_UTINY *)"MaxWatcherList",             0x09, 0x0A }, /* WV 1.2 */
    { (WB_UTINY *)"ReactiveAuthState",          0x09, 0x0B }, /* WV 1.2 */
    { (WB_UTINY *)"ReactiveAuthStatus",         0x09, 0x0C }, /* WV 1.2 */
    { (WB_UTINY *)"ReactiveAuthStatusList",     0x09, 0x0D }, /* WV 1.2 */
    { (WB_UTINY *)"Watcher",                    0x09, 0x0E }, /* WV 1.2 */
    { (WB_UTINY *)"WatcherStatus",              0x09, 0x0C }, /* WV 1.2 : Bug in Spec ! Same token then 'ReactiveAuthStatus' */

    /* Access ... continued */
    { (WB_UTINY *)"WV-CSP-NSDiscovery-Request", 0x0A, 0x05 }, /* WV 1.2 */
    { (WB_UTINY *)"WV-CSP-NSDiscovery-Response",0x0A, 0x06 }, /* WV 1.2 */

    { NULL,                         0x00, 0x00 }
};

const WBXMLAttrEntry sv_wv_csp_attr_table[] = {
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.wireless-village.org/CSP",  0x00, 0x05 },
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.wireless-village.org/PA",   0x00, 0x06 },
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.wireless-village.org/TRC",  0x00, 0x07 },
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.openmobilealliance.org/DTD/WV-CSP",     0x00, 0x08 },
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.openmobilealliance.org/DTD/WV-PA",      0x00, 0x09 },
    { (WB_UTINY *)"xmlns",      (WB_UTINY *)"http://www.openmobilealliance.org/DTD/WV-TRC",     0x00, 0x0A },
    { NULL,         NULL,                                   0x00, 0x00 }
};

const WBXMLExtValueEntry sv_wv_csp_ext_table[] = {
    /*
     * DO NOT CHANGE THIS TABLE ORDER PLEASE ! 
     * Extension Tokens must be sorted by length !
     */

    { (WB_UTINY *)"application/vnd.wap.mms-message",    0x04 }, /* Common value token */
    { (WB_UTINY *)"www.wireless-village.org",           0x30 }, /* Common value token */
    { (WB_UTINY *)"GROUP_USER_ID_AUTOJOIN",             0x50 }, /* Access value token */ /* WV 1.2 */
    { (WB_UTINY *)"GROUP_USER_ID_JOINED",               0x40 }, /* Access value token */
    { (WB_UTINY *)"GROUP_USER_ID_OWNER",                0x41 }, /* Access value token */
    { (WB_UTINY *)"USER_EMAIL_ADDRESS",                 0x47 }, /* Access value token */
    { (WB_UTINY *)"USER_MOBILE_NUMBER",                 0x4b }, /* Access value token */
    { (WB_UTINY *)"USER_ONLINE_STATUS",                 0x4c }, /* Access value token */
    { (WB_UTINY *)"application/x-sms",                  0x05 }, /* Common value token */
    { (WB_UTINY *)"PrivateMessaging",                   0x1c }, /* Common value token */
    { (WB_UTINY *)"text/x-vCalendar",                   0x29 }, /* Common value token */
    { (WB_UTINY *)"USER_FIRST_NAME",                    0x48 }, /* Access value token */
    { (WB_UTINY *)"MaxActiveUsers",                     0x13 }, /* Common value token */
    { (WB_UTINY *)"PrivilegeLevel",                     0x1d }, /* Common value token */
    { (WB_UTINY *)"USER_LAST_NAME",                     0x4a }, /* Access value token */
    { (WB_UTINY *)"NOT_AVAILABLE",                      0x70 }, /* Presence value token */
    { (WB_UTINY *)"application/",                       0x03 }, /* Common value token */
    { (WB_UTINY *)"text/x-vCard",                       0x2a }, /* Common value token */
    { (WB_UTINY *)"MOBILE_PHONE",                       0x6f }, /* Presence value token */   
    { (WB_UTINY *)"VIDEO_STREAM",                       0x77 }, /* Presence value token */
    { (WB_UTINY *)"ActiveUsers",                        0x01 }, /* Common value token */
    { (WB_UTINY *)"DisplayName",                        0x0a }, /* Common value token */
    { (WB_UTINY *)"GROUP_TOPIC",                        0x3f }, /* Access value token */
    { (WB_UTINY *)"AccessType",                         0x00 }, /* Common value token */
    { (WB_UTINY *)"AutoDelete",                         0x31 }, /* Common value token */ /* WV 1.2 */
    { (WB_UTINY *)"Restricted",                         0x22 }, /* Common value token */
    { (WB_UTINY *)"ScreenName",                         0x23 }, /* Common value token */
    { (WB_UTINY *)"Searchable",                         0x24 }, /* Common value token */
    { (WB_UTINY *)"text/plain",                         0x28 }, /* Common value token */
    { (WB_UTINY *)"GROUP_NAME",                         0x3e }, /* Access value token */
    { (WB_UTINY *)"USER_ALIAS",                         0x46 }, /* Access value token */
    { (WB_UTINY *)"AUDIO_CALL",                         0x5e }, /* Presence value token */
    { (WB_UTINY *)"IM_OFFLINE",                         0x69 }, /* Presence value token */
    { (WB_UTINY *)"INVINCIBLE",                         0x6c }, /* Presence value token */
    { (WB_UTINY *)"VIDEO_CALL",                         0x76 }, /* Presence value token */
    { (WB_UTINY *)"AVAILABLE",                          0x5f }, /* Presence value token */
    { (WB_UTINY *)"IM_ONLINE",                          0x6a }, /* Presence value token */
    { (WB_UTINY *)"https://",                           0x0f }, /* Common value token */
    { (WB_UTINY *)"AutoJoin",                           0x06 }, /* Common value token */
    { (WB_UTINY *)"Response",                           0x21 }, /* Common value token */
    { (WB_UTINY *)"Validity",                           0x33 }, /* Common value token */ /* WV 1.2 */
    { (WB_UTINY *)"GROUP_ID",                           0x3d }, /* Access value token */
    { (WB_UTINY *)"COMPUTER",                           0x63 }, /* Presence value token */
    { (WB_UTINY *)"DISCREET",                           0x64 }, /* Presence value token */
    { (WB_UTINY *)"Default",                            0x09 }, /* Common value token */
    { (WB_UTINY *)"GRANTED",                            0x35 }, /* Common value token */ /* WV 1.2 */
    { (WB_UTINY *)"http://",                            0x0e }, /* Common value token */
    { (WB_UTINY *)"Outband",                            0x19 }, /* Common value token */
    { (WB_UTINY *)"PENDING",                            0x36 }, /* Common value token */ /* WV 1.2 */
    { (WB_UTINY *)"Private",                            0x1b }, /* Common value token */
    { (WB_UTINY *)"Request",                            0x20 }, /* Common value token */
    { (WB_UTINY *)"USER_ID",                            0x49 }, /* Access value token */
    { (WB_UTINY *)"ANXIOUS",                            0x5c }, /* Presence value token */
    { (WB_UTINY *)"ASHAMED",                            0x5d }, /* Presence value token */
    { (WB_UTINY *)"EXCITED",                            0x66 }, /* Presence value token */
    { (WB_UTINY *)"IN_LOVE",                            0x6b }, /* Presence value token */
    { (WB_UTINY *)"JEALOUS",                            0x6d }, /* Presence value token */
    { (WB_UTINY *)"BASE64",                             0x07 }, /* Common value token */
    { (WB_UTINY *)"Closed",                             0x08 }, /* Common value token */
    { (WB_UTINY *)"image/",                             0x10 }, /* Common value token */
    { (WB_UTINY *)"Inband",                             0x11 }, /* Common value token */
    { (WB_UTINY *)"Public",                             0x1e }, /* Common value token */    
    { (WB_UTINY *)"ShowID",                             0x34 }, /* Common value token */ /* WV 1.2 */  
    { (WB_UTINY *)"WAPSMS",                             0x4d }, /* Access value token */
    { (WB_UTINY *)"WAPUDP",                             0x4e }, /* Access value token */  
    { (WB_UTINY *)"SLEEPY",                             0x74 }, /* Presence value token */
    { (WB_UTINY *)"Admin",                              0x02 }, /* Common value token */
    { (WB_UTINY *)"text/",                              0x27 }, /* Common value token */
    { (WB_UTINY *)"Topic",                              0x2b }, /* Common value token */
    { (WB_UTINY *)"ANGRY",                              0x5b }, /* Presence value token */
    { (WB_UTINY *)"BORED",                              0x60 }, /* Presence value token */
    { (WB_UTINY *)"EMAIL",                              0x65 }, /* Presence value token */
    { (WB_UTINY *)"HAPPY",                              0x67 }, /* Presence value token */
    { (WB_UTINY *)"OTHER",                              0x71 }, /* Presence value token */
    { (WB_UTINY *)"Name",                               0x15 }, /* Common value token */
    { (WB_UTINY *)"None",                               0x16 }, /* Common value token */
    { (WB_UTINY *)"Open",                               0x18 }, /* Common value token */
    { (WB_UTINY *)"Type",                               0x2d }, /* Common value token */
    { (WB_UTINY *)"HTTP",                               0x42 }, /* Access value token */
    { (WB_UTINY *)"STCP",                               0x44 }, /* Access value token */
    { (WB_UTINY *)"SUDP",                               0x45 }, /* Access value token */
    { (WB_UTINY *)"CALL",                               0x61 }, /* Presence value token */
    { (WB_UTINY *)"Mod",                                0x14 }, /* Common value token */
    { (WB_UTINY *)"SMS",                                0x43 }, /* Access value token */
    { (WB_UTINY *)"WSP",                                0x4f }, /* Access value token */
    { (WB_UTINY *)"CLI",                                0x62 }, /* Presence value token */
    { (WB_UTINY *)"MMS",                                0x6e }, /* Presence value token */
    { (WB_UTINY *)"PDA",                                0x72 }, /* Presence value token */
    { (WB_UTINY *)"SAD",                                0x73 }, /* Presence value token */
    { (WB_UTINY *)"SMS",                                0x75 }, /* Presence value token */
    { (WB_UTINY *)"GM",                                 0x32 }, /* Common value token */ /* WV 1.2 */
    { (WB_UTINY *)"GR",                                 0x0d }, /* Common value token */
    { (WB_UTINY *)"IM",                                 0x12 }, /* Common value token */    
    { (WB_UTINY *)"PR",                                 0x1a }, /* Common value token */
    { (WB_UTINY *)"SC",                                 0x26 }, /* Common value token */
    { (WB_UTINY *)"US",                                 0x2f }, /* Common value token */
    { (WB_UTINY *)"IM",                                 0x68 }, /* Presence value token */
    { (WB_UTINY *)"F",                                  0x0b }, /* Common value token */
    { (WB_UTINY *)"G",                                  0x0c }, /* Common value token */
    { (WB_UTINY *)"N",                                  0x17 }, /* Common value token */
    { (WB_UTINY *)"P",                                  0x1f }, /* Common value token */
    { (WB_UTINY *)"S",                                  0x25 }, /* Common value token */
    { (WB_UTINY *)"T",                                  0x2c }, /* Common value token */
    { (WB_UTINY *)"U",                                  0x2e }, /* Common value token */

    { NULL,                                 0x00 }, /* Presence value token */
};


/******************************
 *    Main Table
 */

const WBXMLLangEntry sv_table_entry[] = {
#ifdef WBXML_TABLES_SEPARATE_WML_VERSIONS    
    { &sv_wml10_public_id,          sv_wml10_tag_table,         sv_wml10_attr_table,        sv_wml10_attr_value_table,      NULL },
    { &sv_wml11_public_id,          sv_wml11_tag_table,         sv_wml11_attr_table,        sv_wml11_attr_value_table,      NULL },
    { &sv_wml12_public_id,          sv_wml12_tag_table,         sv_wml12_attr_table,        sv_wml12_attr_value_table,      NULL },
#else /* WBXML_TABLES_SEPARATE_WML_VERSIONS */
    { &sv_wml10_public_id,          sv_wml13_tag_table,         sv_wml13_attr_table,        sv_wml13_attr_value_table,      NULL },
    { &sv_wml11_public_id,          sv_wml13_tag_table,         sv_wml13_attr_table,        sv_wml13_attr_value_table,      NULL },
    { &sv_wml12_public_id,          sv_wml13_tag_table,         sv_wml13_attr_table,        sv_wml13_attr_value_table,      NULL },
#endif /* WBXML_TABLES_SEPARATE_WML_VERSIONS */    
    { &sv_wml13_public_id,          sv_wml13_tag_table,         sv_wml13_attr_table,        sv_wml13_attr_value_table,      NULL }, 
    { &sv_si10_public_id,           sv_si10_tag_table,          sv_si10_attr_table,         sv_si10_attr_value_table,       NULL },
    { &sv_sl10_public_id,           sv_sl10_tag_table,          sv_sl10_attr_table,         sv_sl10_attr_value_table,       NULL },
    { &sv_co10_public_id,           sv_co10_tag_table,          sv_co10_attr_table,         sv_co10_attr_value_table,       NULL },
    { &sv_wta10_public_id,          sv_wta10_tag_table,         sv_wta10_attr_table,        NULL,                           NULL },
    { &sv_channel11_public_id,      sv_channel11_tag_table,     sv_channel11_attr_table,    NULL,                           NULL },
    { &sv_prov10_public_id,         sv_prov10_tag_table,        sv_prov10_attr_table,       sv_prov10_attr_value_table,     NULL },
    { &sv_wtawml12_public_id,       sv_wtawml12_tag_table,      sv_wtawml12_attr_table,     sv_wtawml12_attr_value_table,   NULL },
    { &sv_channel12_public_id,      sv_channel12_tag_table,     sv_channel12_attr_table,    NULL,                           NULL },
    { &sv_emn10_public_id,          sv_emn10_tag_table,         sv_emn10_attr_table,        sv_emn10_attr_value_table,      NULL },
    { &sv_drmrel10_public_id,       sv_drmrel10_tag_table,      sv_drmrel10_attr_table,     sv_drmrel10_attr_value_table,   NULL },
    { &sv_syncml_syncml11_public_id, sv_syncml_syncml11_tag_table, NULL,                    NULL,                           NULL },
    { &sv_syncml_devinf11_public_id, sv_syncml_devinf11_tag_table, NULL,                    NULL,                           NULL },
    { &sv_syncml_metinf11_public_id, sv_syncml_metinf11_tag_table, NULL,                    NULL,                           NULL },
    /** @todo Check if Tag Tables are exactly with SyncML 1.0 */
    { &sv_syncml_syncml10_public_id, sv_syncml_syncml11_tag_table, NULL,                    NULL,                           NULL },
    { &sv_syncml_devinf10_public_id, sv_syncml_devinf11_tag_table, NULL,                    NULL,                           NULL },   
    { &sv_wv_csp11_public_id,        sv_wv_csp_tag_table,       sv_wv_csp_attr_table,       NULL,                           sv_wv_csp_ext_table },
    { &sv_wv_csp12_public_id,        sv_wv_csp_tag_table,       sv_wv_csp_attr_table,       NULL,                           sv_wv_csp_ext_table },
    { NULL,                          NULL,                      NULL,                       NULL,                           NULL }
};


/******************************
 * Public Functions
 */

/* Exported function to return pointer to WBXML Languages Main Table */
WBXML_DECLARE(const WBXMLLangEntry *) wbxml_tables_get_main(void)
{
    return sv_table_entry;
}


WBXML_DECLARE(const WBXMLLangEntry *) wbxml_tables_search_table(const WBXMLLangEntry *main_table,
                                                                const WB_UTINY *public_id, 
                                                                const WB_UTINY *system_id,
                                                                const WB_UTINY *root)
{
    WB_ULONG index;

    if (main_table == NULL)
        return NULL;

    /* Search by XML Public ID  */
    if (public_id != NULL) {
        index = 0;

        while (main_table[index].publicID != NULL) {
            if (WBXML_STRCMP(main_table[index].publicID->xmlPublicID, public_id) == 0)
                return &main_table[index];
            index++;
        }
    }

    /* Search by XML System ID  */
    if (system_id != NULL) {
        index = 0;

        while (main_table[index].publicID != NULL) {
            if (WBXML_STRCMP(main_table[index].publicID->xmlDTD, system_id) == 0) 
                return &main_table[index];
            index++;
        }
    }

    /* Search by XML Root Element  */
    if (root != NULL) {
        index = 0;

        while (main_table[index].publicID != NULL) {
            if (WBXML_STRCMP(main_table[index].publicID->xmlRootElt, root) == 0) 
                return &main_table[index];
            index++;
        }
    }

    return NULL;
}


WBXML_DECLARE(const WBXMLTagEntry *) wbxml_tables_get_tag_from_xml(const WBXMLLangEntry *lang_table,
                                                                   const WB_UTINY *xml_name)
{
    WB_ULONG i = 0;

    if ((lang_table == NULL) || (lang_table->tagTable == NULL) || (xml_name == NULL))
        return NULL;

    while (lang_table->tagTable[i].xmlName != NULL) {
        if (WBXML_STRCMP(lang_table->tagTable[i].xmlName, xml_name) == 0)
            return &(lang_table->tagTable[i]);
        i++;
    }

    return NULL;
}


WBXML_DECLARE(const WBXMLAttrEntry *) wbxml_tables_get_attr_from_xml(const WBXMLLangEntry *lang_table,
                                                                     WB_UTINY *xml_name,
                                                                     WB_UTINY *xml_value,
                                                                     WB_UTINY **value_left)
{
    WB_ULONG i = 0;
    WB_ULONG found_index = 0, found_comp = 0;
    WB_BOOL found = FALSE;

    if ((lang_table == NULL) || (lang_table->attrTable == NULL) || (xml_name == NULL))
        return NULL;

    if (value_left != NULL)
        *value_left = xml_value;

    /* Iterate in Attribute Table */
    while (lang_table->attrTable[i].xmlName != NULL) {
        /* Search for Attribute Name */
        if (WBXML_STRCMP(lang_table->attrTable[i].xmlName, xml_name) == 0) 
        {
            if (lang_table->attrTable[i].xmlValue == NULL) {
                /* This is the token with a NULL Attribute Value */
                if (xml_value == NULL) {
                    /* Well, we got it */
                    return &(lang_table->attrTable[i]);
                }
                else {
                    if (!found) {
                        /* We haven't found yet a better Attribute Token */
                        found = TRUE;
                        found_index = i;
                    }

                    /* Else: We already have found a better Attribute Token, so let's forget this one */
                }
            }
            else {
                /* Check the Attribute Value */
                if (xml_value != NULL)
                {
                    if (WBXML_STRCMP(lang_table->attrTable[i].xmlValue, xml_value) == 0) 
                    {
                        /* We have found the EXACT Attribute Name / Value pair we are searching, well done boy */
                        if (value_left != NULL)
                            *value_left = NULL;

                        return &(lang_table->attrTable[i]);
                    }
                    else {
                        if ((WBXML_STRLEN(lang_table->attrTable[i].xmlValue) < WBXML_STRLEN(xml_value)) &&
                            (found_comp < WBXML_STRLEN(lang_table->attrTable[i].xmlValue)) &&
                            (WBXML_STRNCMP(lang_table->attrTable[i].xmlValue, xml_value, WBXML_STRLEN(lang_table->attrTable[i].xmlValue)) == 0))
                        {
                            /* We have found a better Attribute Value */
                            found = TRUE;
                            found_index = i;
                            found_comp = WBXML_STRLEN(lang_table->attrTable[i].xmlValue);
                        }
                    }
                }

                /* Else: We are searching for the Attribute Token with a NULL Attribute Value associated, so forget this one  */
            }
        }
        i++;
    }

    /* Attribute Name / Value pair not found, but an entry with this Attribute Name, 
     * and (maybe) start of this Attribute Value was found */
    if (found) {
        if (value_left != NULL)
            *value_left = xml_value + found_comp;

        return &(lang_table->attrTable[found_index]);
    }

    /* Attribute Name NOT found */
    return NULL;
}


WBXML_DECLARE(const WBXMLExtValueEntry *) wbxml_tables_get_ext_from_xml(const WBXMLLangEntry *lang_table,
                                                                        WB_UTINY *xml_value)
{
    WB_ULONG i = 0;

    if ((lang_table == NULL) || (lang_table->extValueTable == NULL) || (xml_value == NULL))
        return NULL;

    while (lang_table->extValueTable[i].xmlName != NULL) {
        if (WBXML_STRCMP(lang_table->extValueTable[i].xmlName, xml_value) == 0)
            return &(lang_table->extValueTable[i]);
        i++;
    }

    return NULL;
}


WBXML_DECLARE(WB_BOOL) wbxml_tables_contains_attr_value_from_xml(const WBXMLLangEntry *lang_table,
                                                                 WB_UTINY *xml_value)
{
    WB_ULONG i = 0;

    /** @todo Test wbxml_tables_contains_attr_value_from_xml() */

    if ((lang_table == NULL) || (lang_table->attrValueTable == NULL) || (xml_value == NULL))
        return FALSE;

    while (lang_table->attrValueTable[i].xmlName != NULL)
    {
        /* Is this Attribute Value contained in this XML Buffer ? */
        if (WBXML_STRSTR(xml_value, lang_table->attrValueTable[i].xmlName) != NULL)
            return TRUE;

        i++;
    }

    return FALSE;
}
