/* $Id: s3c2410_mmu.h,v 1.3 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef _S3C2410_MMU_H
#define _S3C2410_MMU_H 1

#define S3C2410_MMU_TLB_SIZE	64
#define S3C2410_MMU_TLB_MASK	(S3C2410_MMU_TLB_SIZE - 1)

typedef struct {
	uint32_t		mva;
	uint32_t		mask;
	uint32_t		pa;
	uint32_t		dac;
	int		valid;
} TLB_entry_t;

typedef struct {
	int		victim;
	int		base;
	int		index0;
	int		index1;
	unsigned long	hit0;
	unsigned long	hit1;
	unsigned long	search;
	unsigned long	nsearch;
	unsigned long	walk;
	TLB_entry_t	data[S3C2410_MMU_TLB_SIZE];
} TLB_t;

typedef struct {
	uint32_t		MMUReg[16];
	TLB_t		iTLB;
	TLB_t		dTLB;
} s3c2410_mmu_t;

#endif /* !(_S3C2410_MMU_H) */
