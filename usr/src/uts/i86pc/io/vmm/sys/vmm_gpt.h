/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2019 Joyent, Inc.
 * Copyright 2021 Oxide Computer Company
 */

#ifndef _VMM_GPT_H
#define	_VMM_GPT_H

#include <sys/types.h>

typedef struct vmm_pt_ops vmm_pt_ops_t;
struct vmm_pt_ops {
	void *		(*vpo_init)(uint64_t *);
	void		(*vpo_free)(void *);
	uint64_t	(*vpo_wired_cnt)(void *);
	int		(*vpo_is_wired)(void *, uint64_t, uint_t *);
	int		(*vpo_map)(void *, uint64_t, pfn_t, uint_t, uint_t,
			    uint8_t);
	uint64_t	(*vpo_unmap)(void *, uint64_t, uint64_t);
};

extern struct vmm_pt_ops ept_ops;
extern struct vmm_pt_ops rvi_ops;

/*
 * Constants for the nodes in the GPT radix tree.  Note
 * that, in accordance with hardware page table descriptions,
 * the root of the tree is referred to as "LEVEL4" while the
 * leaf level is "LEVEL1".
 */
enum vmm_gpt_node_level {
	LEVEL4 = 0,
	LEVEL3,
	LEVEL2,
	LEVEL1,
	MAX_GPT_LEVEL,
};

/*
 * The vmm_pte_ops structure contains function pointers for format-specific
 * operations on page table entries.  The operations are as follows:
 *
 * vpeo_map_table: Creates a PTE that maps an inner node in the page table.
 * vpeo_map_page: Creates a leaf entry PTE that maps a page of physical memory.
 * vpeo_pte_pfn: Returns the PFN contained in the given PTE.
 * vpeo_pte_is_present: Returns true IFF the PTE maps a present page.
 * vpeo_pte_prot: Returns a bitmask of protection bits for the PTE.
 *   The bits correspond to the standard mmap(2) bits: PROT_READ, PROT_WRITE,
 *   PROT_EXEC.
 * vpeo_reset_dirty: Resets the dirty bit on the given PTE.  If the second
 *   argument is `true`, the bit will be set, otherwise it will be cleared.
 *   Returns non-zero if the previous value of the bit was set.
 * vpeo_reset_accessed: Resets the accessed bit on the given PTE.  If the
 *   second argument is `true`, the bit will be set, otherwise it will be
 *   cleared.  Returns non-zero if the previous value of the bit was set.
 */
typedef struct vmm_pte_ops vmm_pte_ops_t;
struct vmm_pte_ops {
	uint64_t	(*vpeo_map_table)(pfn_t);
	uint64_t	(*vpeo_map_page)(pfn_t, uint_t, uint8_t);
	pfn_t		(*vpeo_pte_pfn)(uint64_t);
	bool		(*vpeo_pte_is_present)(uint64_t);
	uint_t		(*vpeo_pte_prot)(uint64_t);
	uint_t		(*vpeo_reset_dirty)(uint64_t *, bool);
	uint_t		(*vpeo_reset_accessed)(uint64_t *, bool);
};

struct vmm_gpt;
typedef struct vmm_gpt vmm_gpt_t;

vmm_gpt_t *ept_create(void);
vmm_gpt_t *rvi_create(void);

vmm_gpt_t *vmm_gpt_alloc(vmm_pte_ops_t *);
void vmm_gpt_free(vmm_gpt_t *);

void *vmm_gpt_root_kaddr(vmm_gpt_t *);
pfn_t vmm_gpt_root_pfn(vmm_gpt_t *);
uint64_t *vmm_gpt_lookup(vmm_gpt_t *, uint64_t);
void vmm_gpt_walk(vmm_gpt_t *, uint64_t, uint64_t **, enum vmm_gpt_node_level);
void vmm_gpt_populate_entry(vmm_gpt_t *, uint64_t);
void vmm_gpt_populate_region(vmm_gpt_t *, uint64_t, uint64_t);
void vmm_gpt_vacate_region(vmm_gpt_t *, uint64_t, uint64_t);
bool vmm_gpt_map(vmm_gpt_t *, uint64_t, pfn_t, uint_t, uint8_t);
bool vmm_gpt_unmap(vmm_gpt_t *, uint64_t);
size_t vmm_gpt_unmap_region(vmm_gpt_t *, uint64_t, uint64_t);

bool vmm_gpt_is_mapped(vmm_gpt_t *, uint64_t, uint_t *);
size_t vmm_gpt_mapped_count(vmm_gpt_t *);
uint_t vmm_gpt_reset_accessed(vmm_gpt_t *, uint64_t *, bool);
uint_t vmm_gpt_reset_dirty(vmm_gpt_t *, uint64_t *, bool);

#endif /* _VMM_GPT_H */
