/* $Id: saturn.h,v 1.1 2008/12/11 12:18:17 ecd Exp $
 */

#ifndef SATURN_H
#define SATURN_H 1

#define NB_RSTK		8

typedef uint64_t	saturn_reg_t;

typedef struct {
	uint32_t	read_map[256 + 1];
	uint32_t	write_map[256 + 1];
	uint8_t		top_map[256 + 1 + 3];
	saturn_reg_t	A;
	saturn_reg_t	B;
	saturn_reg_t	C;
	saturn_reg_t	D;
	saturn_reg_t	R0;
	saturn_reg_t	R1;
	saturn_reg_t	R2;
	saturn_reg_t	R3;
	saturn_reg_t	R4;
	uint32_t	D0;
	uint32_t	D1;
	uint32_t	P, P4, P4_32;
	uint32_t	ST;
	uint32_t	HST;
	uint32_t	carry;
	int		dec;
	uint32_t	RSTK[NB_RSTK];
	uint32_t	RSTK_i;
	uint32_t	REG_FIELD[32];
	uint32_t	FIELD_START[32];
	uint32_t	FIELD_LENGTH[32];
} saturn_cpu_t;

#define SAT_RPLTOP	0x8076b
#define SAT_RSKTOP	0x806f3		/* RETTOP */
#define SAT_DSKTOP	0x806f8
#define SAT_EDITLINE	0x806fd

#endif /* !(SATURN_H) */
