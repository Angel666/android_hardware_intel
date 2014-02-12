#ifndef _COMPAT_XENCOMM_H
#define _COMPAT_XENCOMM_H
#include <xen/compat.h>
#include <public/xencomm.h>
#pragma pack(4)
struct compat_comm_desc {
    uint32_t magic;
    uint32_t nr_addrs;
    uint64_t address[0];
};
#pragma pack()
#endif /* _COMPAT_XENCOMM_H */
