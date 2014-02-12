#ifndef _COMPAT_KEXEC_H
#define _COMPAT_KEXEC_H
#include <xen/compat.h>
#include <public/kexec.h>
#pragma pack(4)
#include "xen.h"
#pragma pack(4)
typedef struct compat_kexec_image {

    unsigned int page_list[17];

    unsigned int indirection_page;
    unsigned int start_address;
} compat_kexec_image_t;

typedef struct compat_kexec_exec {
    int type;
} compat_kexec_exec_t;
typedef struct compat_kexec_load {
    int type;
    compat_kexec_image_t image;
} compat_kexec_load_t;
typedef struct compat_kexec_range {
    int range;
    int nr;
    unsigned int size;
    unsigned int start;
} compat_kexec_range_t;
#pragma pack()
#endif /* _COMPAT_KEXEC_H */
