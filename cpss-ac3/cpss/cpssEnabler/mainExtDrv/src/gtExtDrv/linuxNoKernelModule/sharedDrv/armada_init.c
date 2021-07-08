/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file armada_init.c
*
* @brief Initialize platform drivers for Marvell Armada SoC
* The following drivers will be initialized
* Armada38x:
* /SoC          (formal driver, holds all mappings)
* /SoC/internal-regs
* /SoC/internal-regs/smi@0x72004
* /smi0 -> /SoC/internal-regs/smi@0x72004
* Armada39x: TBD
*
* @version   1
********************************************************************************
*/
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/generic/cpssHwInfo.h>
#include "../prvNoKmDrv.h"
#include "../drivers/mvResources.h"
#include <sys/mman.h>
#include <termios.h>


/* externs */
GT_STATUS mbus_map_resource(
    IN  int                         mbusResource,
    IN  GT_UINTPTR                  maxSize,
    OUT CPSS_HW_INFO_RESOURCE_MAPPING_STC *mappingPtr
);
CPSS_HW_DRIVER_STC *armadaSmiCreateDrv(CPSS_HW_DRIVER_STC *parent, GT_U32 as, GT_U32 smiMgmtReg);

/* TODO: will be in cpss/generic/driver/cpssHwDriverGeneric.h */
CPSS_HW_DRIVER_STC *cpssHwDriverSip5SlaveSMICreateDrv(
    IN  CPSS_HW_DRIVER_STC *smi,
    IN  GT_U32              phy
);


#define MAX_MBUS_RES 1
typedef struct CPSS_HW_DRIVER_SOC_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_HW_INFO_RESOURCE_MAPPING_STC res[MAX_MBUS_RES];
} CPSS_HW_DRIVER_SOC_STC;

static GT_STATUS stubErr(void) { return GT_FAIL; }
static void drvSoCDestroy(CPSS_HW_DRIVER_SOC_STC *drv)
{
    int i;
    for (i = 0; i < MAX_MBUS_RES; i++)
    {
        if (drv->res[i].start)
        {
            munmap((void*)(drv->res[i].start), (size_t)(drv->res[i].size));
            drv->res[i].start = 0;
            drv->res[i].size = 0;
            drv->res[i].phys = 0;
        }
    }
}
static CPSS_HW_DRIVER_SOC_STC drvSoC = {
    { /* .common */
        (CPSS_HW_DRIVER_METHOD_READ)stubErr,
        (CPSS_HW_DRIVER_METHOD_WRITE_MASK)stubErr,
        (CPSS_HW_DRIVER_METHOD_DESTROY)drvSoCDestroy,
        NULL, /* .name */
        0,    /* .numRefs */
        NULL  /* .parent*/
    },
    { /* .res */
        { 0, 0, 0 }
    }
};

/* This generic code will be moved later to $CPSS/common */
typedef struct {
    GT_U32  as;
    GT_U32  regAddr;
    GT_U32  value;
    GT_U32  mask;
    const char *comment;
} PRV_DRIVER_REG_WR_STC;

static GT_STATUS drv_do_script(CPSS_HW_DRIVER_STC *drv, const PRV_DRIVER_REG_WR_STC *data)
{
    GT_STATUS rc;
    GT_U32  value;
    if (drv == NULL || data == NULL)
        return GT_BAD_PARAM;
    for (; data->comment; data++)
    {
        value = data->value;
        rc = drv->writeMask(drv, data->as, data->regAddr, &value, 1, data->mask);
        if (rc != GT_OK)
            return rc;
    }
    return rc;
}

static const PRV_DRIVER_REG_WR_STC armada385_init_internal_regs[] = {
    { 0, 0x18104, 0x00000000, 0x20000000, "GPIO output enable pin 29" },
    { 0, 0x18100, 0x00000000, 0x20000000, "Reset GPIO pin 29" },
    { 0, 0x1800c, 0x00000000, 0x00f00000, "Set MPP 29 to 0 (SMI)" },
    { 0, 0x18014, 0x00000000, 0x000f0000, "Set MPP 44 to 0 (SMI)" },
    /* Configure SMI speed */
    /* 0 - normal speed(/128), 1 - fast mode(/16), 2 - accel (/8) */
    { 0, 0x72014, 0x00000002, 0x00000003, "Set MDC clock divider to 8 (accelerate mode)" },
    { 0, 0, 0, 0, NULL }
};
/*
 * Init Armada38x
 *
 * drivers initialized:
 *   /SoC                   (formal driver, holds all mappings)
 *   /SoC/internal-regs
 *   /SoC/internal-regs/smi@0x72004
 *   /smi0 -> /SoC/internal-regs/smi@0x72004
 *
 */
static void soc_init_armada38x(void)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *internalRegs, *smi;

    printf("Initialize Armada 38x SoC drivers\n");
    fflush(stdout);
    tcdrain(1);
    rc = mbus_map_resource(MV_RESOURCE_MBUS_RUNIT, _1M, &(drvSoC.res[0]));
    if (rc != GT_OK)
    {
        fprintf(stderr, "failed to map ARMADAXP internal registers rc=%d\n", rc);
        return;
    }
    cpssHwDriverRegister(&drvSoC.common, "SoC");
    internalRegs = cpssHwDriverGenMmapCreateDrv(drvSoC.res[0].start, drvSoC.res[0].size);
    internalRegs->parent = &drvSoC.common;
    cpssHwDriverRegister(internalRegs, "internal-regs"); /* /SoC/internal-regs */

    drv_do_script(internalRegs, armada385_init_internal_regs);

    smi = armadaSmiCreateDrv(internalRegs, 0, 0x72004);
    if (smi)
    {
        cpssHwDriverRegister(smi, "smi@0x72004"); /* /SoC/internal-regs/smi@0x72004 */
        cpssHwDriverAddAlias(smi, "/smi0");
        printf("/smi0 driver created\n");
    }
}

/*
 * Init ARMADA-39x
 *
 * drivers initialized:
 *   /SoC/PSS
 *   /SoC/PSS/smi0
 *
 */
void soc_init_armada39x(void)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *pss, *smi;

    rc = mbus_map_resource(MV_RESOURCE_MBUS_PSS_PORTS, _4M, &(drvSoC.res[0]));
    if (rc != GT_OK)
    {
        fprintf(stderr, "failed to map ARMADA 385 ports subsystem registers rc=%d\n", rc);
        return;
    }
    cpssHwDriverRegister(&drvSoC.common, "SoC");
    pss = cpssHwDriverGenMmapAc8CreateDrv(drvSoC.res[0].start, drvSoC.res[0].size, 0x80);
    pss->parent = &drvSoC.common;
    cpssHwDriverRegister(pss, "PSS"); /* /SoC/PSS */

    smi = armadaSmiCreateDrv(pss, 0, 0x05000000);
    if (smi)
    {
        cpssHwDriverRegister(smi, "smi@0x05000000"); /* /SoC/PSS/smi@0x05000000 */
        cpssHwDriverAddAlias(smi, "/smi0");
    }
    /* TODO: XSMI */
}

void soc_init(void)
{
    int status;
    if (cpssHwDriverLookup("/SoC") != NULL)
    {
        /* already initialized */
        return;
    }
    status = system("cat /proc/cpuinfo | grep -q 'Armada .*38[0-9][^0-9]'");
    if (status == 0) /* Armada 38x */
    {
        soc_init_armada38x();
    }
    /* TODO: Armada 39x */
}


