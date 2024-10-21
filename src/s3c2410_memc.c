/* $Id: s3c2410_memc.c,v 1.4 2008/12/11 12:18:17 ecd Exp $
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include <x49gp.h>
#include <s3c2410.h>


typedef struct {
	uint32_t			bwscon;
	uint32_t			bankcon0;
	uint32_t			bankcon1;
	uint32_t			bankcon2;
	uint32_t			bankcon3;
	uint32_t			bankcon4;
	uint32_t			bankcon5;
	uint32_t			bankcon6;
	uint32_t			bankcon7;
	uint32_t			refresh;
	uint32_t			banksize;
	uint32_t			mrsrb6;
	uint32_t			mrsrb7;

	unsigned int		nr_regs;
	s3c2410_offset_t	*regs;

	x49gp_t			*x49gp;
} s3c2410_memc_t;

static int
s3c2410_memc_data_init(s3c2410_memc_t *memc)
{
	s3c2410_offset_t regs[] = {
		S3C2410_OFFSET(MEMC, BWSCON, 0x00000000, memc->bwscon),
		S3C2410_OFFSET(MEMC, BANKCON0, 0x00000700, memc->bankcon0),
		S3C2410_OFFSET(MEMC, BANKCON1, 0x00000700, memc->bankcon1),
		S3C2410_OFFSET(MEMC, BANKCON2, 0x00000700, memc->bankcon2),
		S3C2410_OFFSET(MEMC, BANKCON3, 0x00000700, memc->bankcon3),
		S3C2410_OFFSET(MEMC, BANKCON4, 0x00000700, memc->bankcon4),
		S3C2410_OFFSET(MEMC, BANKCON5, 0x00000700, memc->bankcon5),
		S3C2410_OFFSET(MEMC, BANKCON6, 0x00018008, memc->bankcon6),
		S3C2410_OFFSET(MEMC, BANKCON7, 0x00018008, memc->bankcon7),
		S3C2410_OFFSET(MEMC, REFRESH, 0x00ac0000, memc->refresh),
		S3C2410_OFFSET(MEMC, BANKSIZE, 0x00000000, memc->banksize),
		S3C2410_OFFSET(MEMC, MRSRB6, 0, memc->mrsrb6),
		S3C2410_OFFSET(MEMC, MRSRB7, 0, memc->mrsrb7),
	};

	memset(memc, 0, sizeof(s3c2410_memc_t));

	memc->regs = malloc(sizeof(regs));
	if (NULL == memc->regs) {
		fprintf(stderr, "%s:%u: Out of memory\n",
			__FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	memcpy(memc->regs, regs, sizeof(regs));
	memc->nr_regs = sizeof(regs) / sizeof(regs[0]);

	return 0;
}

static uint32_t
s3c2410_memc_read(void *opaque, target_phys_addr_t offset)
{
	s3c2410_memc_t *memc = opaque;
	s3c2410_offset_t *reg;

	if (! S3C2410_OFFSET_OK(memc, offset)) {
		return ~(0);
	}

	reg = S3C2410_OFFSET_ENTRY(memc, offset);

#ifdef DEBUG_S3C2410_MEMC
	printf("read  %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-memc", S3C2410_MEMC_BASE,
		reg->name, (unsigned long) offset, *(reg->datap));
#endif

	return *(reg->datap);
}

static void
s3c2410_memc_write(void *opaque, target_phys_addr_t offset, uint32_t data)
{
	s3c2410_memc_t *memc = opaque;
	s3c2410_offset_t *reg;

	if (! S3C2410_OFFSET_OK(memc, offset)) {
		return;
	}

	reg = S3C2410_OFFSET_ENTRY(memc, offset);

#ifdef DEBUG_S3C2410_MEMC
	printf("write %s [%08x] %s [%08lx] data %08x\n",
		"s3c2410-memc", S3C2410_MEMC_BASE,
		reg->name, (unsigned long) offset, data);
#endif

	*(reg->datap) = data;

#ifdef DEBUG_S3C2410_MEMC
	printf("%s:%u: env %p\n", __FUNCTION__, __LINE__, memc->x49gp->env);
#endif
}

static int
s3c2410_memc_load(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_memc_t *memc = module->user_data;
	s3c2410_offset_t *reg;
	int error = 0;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < memc->nr_regs; i++) {
		reg = &memc->regs[i];

		if (NULL == reg->name)
			continue;

		if (x49gp_module_get_u32(module, key, reg->name,
					 reg->reset, reg->datap))
			error = -EAGAIN;
	}

	return error;
}

static int
s3c2410_memc_save(x49gp_module_t *module, GKeyFile *key)
{
	s3c2410_memc_t *memc = module->user_data;
	s3c2410_offset_t *reg;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < memc->nr_regs; i++) {
		reg = &memc->regs[i];

		if (NULL == reg->name)
			continue;

		x49gp_module_set_u32(module, key, reg->name, *(reg->datap));
	}

	return 0;
}

static int
s3c2410_memc_reset(x49gp_module_t *module, x49gp_reset_t reset)
{
	s3c2410_memc_t *memc = module->user_data;
	s3c2410_offset_t *reg;
	int i;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < memc->nr_regs; i++) {
		reg = &memc->regs[i];

		if (NULL == reg->name)
			continue;

		*(reg->datap) = reg->reset;
	}

	return 0;
}

static CPUReadMemoryFunc *s3c2410_memc_readfn[] =
{
	s3c2410_memc_read,
	s3c2410_memc_read,
	s3c2410_memc_read
};

static CPUWriteMemoryFunc *s3c2410_memc_writefn[] =
{
	s3c2410_memc_write,
	s3c2410_memc_write,
	s3c2410_memc_write
};

static int
s3c2410_memc_init(x49gp_module_t *module)
{
	s3c2410_memc_t *memc;
	int iotype;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	memc = malloc(sizeof(s3c2410_memc_t));
	if (NULL == memc) {
		fprintf(stderr, "%s:%u: Out of memory\n",
			__FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	if (s3c2410_memc_data_init(memc)) {
		free(memc);
		return -ENOMEM;
	}

	module->user_data = memc;
	memc->x49gp = module->x49gp;

	iotype = cpu_register_io_memory(s3c2410_memc_readfn,
					s3c2410_memc_writefn, memc);
#ifdef DEBUG_S3C2410_MEMC
	printf("%s: iotype %08x\n", __FUNCTION__, iotype);
#endif
	cpu_register_physical_memory(S3C2410_MEMC_BASE, S3C2410_MAP_SIZE, iotype);
	return 0;
}

static int
s3c2410_memc_exit(x49gp_module_t *module)
{
	s3c2410_memc_t *memc;

#ifdef DEBUG_X49GP_MODULES
	printf("%s: %s:%u\n", module->name, __FUNCTION__, __LINE__);
#endif

	if (module->user_data) {
		memc = module->user_data;
		if (memc->regs)
			free(memc->regs);
		free(memc);
	}

	x49gp_module_unregister(module);
	free(module);

	return 0;
}

int
x49gp_s3c2410_memc_init(x49gp_t *x49gp)
{
	x49gp_module_t *module;

	if (x49gp_module_init(x49gp, "s3c2410-memc",
			      s3c2410_memc_init,
			      s3c2410_memc_exit,
			      s3c2410_memc_reset,
			      s3c2410_memc_load,
			      s3c2410_memc_save,
			      NULL, &module)) {
		return -1;
	}

	return x49gp_module_register(module);
}
