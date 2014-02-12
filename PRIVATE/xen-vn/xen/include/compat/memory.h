#ifndef _COMPAT_MEMORY_H
#define _COMPAT_MEMORY_H
#include <xen/compat.h>
#include <public/memory.h>
#pragma pack(4)
#include "xen.h"
#pragma pack(4)
struct compat_memory_reservation {
    COMPAT_HANDLE(compat_pfn_t) extent_start;

    compat_ulong_t nr_extents;
    unsigned int extent_order;

    unsigned int mem_flags;
    domid_compat_t domid;
};
typedef struct compat_memory_reservation compat_memory_reservation_t;
DEFINE_COMPAT_HANDLE(compat_memory_reservation_t);
struct compat_memory_exchange {

    struct compat_memory_reservation in;
    struct compat_memory_reservation out;
    compat_ulong_t nr_exchanged;
};
typedef struct compat_memory_exchange compat_memory_exchange_t;
DEFINE_COMPAT_HANDLE(compat_memory_exchange_t);
struct compat_machphys_mfn_list {

    unsigned int max_extents;

    COMPAT_HANDLE(compat_pfn_t) extent_start;

    unsigned int nr_extents;
};
typedef struct compat_machphys_mfn_list compat_machphys_mfn_list_t;
DEFINE_COMPAT_HANDLE(compat_machphys_mfn_list_t);
struct compat_machphys_mapping {
    compat_ulong_t v_start, v_end;
    compat_ulong_t max_mfn;
};
typedef struct compat_machphys_mapping compat_machphys_mapping_t;
DEFINE_COMPAT_HANDLE(compat_machphys_mapping_t);

struct compat_add_to_physmap {

    domid_compat_t domid;

    unsigned int space;

    compat_ulong_t idx;

    compat_pfn_t gpfn;
};
typedef struct compat_add_to_physmap compat_add_to_physmap_t;
DEFINE_COMPAT_HANDLE(compat_add_to_physmap_t);
struct compat_memory_map {

    unsigned int nr_entries;

    COMPAT_HANDLE(void) buffer;
};
typedef struct compat_memory_map compat_memory_map_t;
DEFINE_COMPAT_HANDLE(compat_memory_map_t);
struct compat_foreign_memory_map {
    domid_compat_t domid;
    struct compat_memory_map map;
};
typedef struct compat_foreign_memory_map compat_foreign_memory_map_t;
DEFINE_COMPAT_HANDLE(compat_foreign_memory_map_t);

struct compat_pod_target {

    uint64_t target_pages;

    uint64_t tot_pages;
    uint64_t pod_cache_pages;
    uint64_t pod_entries;

    domid_compat_t domid;
};
typedef struct compat_pod_target compat_pod_target_t;
#pragma pack()
#endif /* _COMPAT_MEMORY_H */
