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
* @file noKmDrvPciDrvSysfs.c
*
* @brief sysfs based PCI driver implementation
*
* @version   1
********************************************************************************
*/
#if 0

Description:
    This driver use pci-sysfs driver which allocates entries in
    /sys/bus/pci/devices/
    For each devices the following allocated:
        deviceId, vendorId, class, config, irq, resource%d

    So, the driver access PP using this feature
    (mmap resources, read/write config, irq, IDs...)




AC3, BC2, Cetus, Caelum devices:
Read Functional Specification=>Overview=>Address Map section

These devices require confuguration of MBUS <=> PCIe -
    resource <=> bar{1,2} offset XXX =>  PCIe BAR{2,4}

Here:
    resource    - switch, dfx, RAM, dragonite
    bar1        - a resource mapped to PCI BAR2
    bar2        - a resource mapped to PCI BAR4

These devices are configured by extDrvInitDrv():
1. Scan all PCI devices
2. For matching device:
    2.1. map PCI BAR0 (control and management) to userspace
    2.2. resolve size of bar1, bar2
    2.3. configire size of bar1, bar2
    2.4. unmap PCI BAR0, rescan device, mmap it again
    2.5. reset all windows
    2.6. configure and enable windows

#endif

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <glob.h>
#include <pthread.h>

#include "prvNoKmDrv.h"


#define MG_PEX_BAR1_CONTROL     0x41804
#define MG_PEX_BAR2_CONTROL     0x41808

#define UNIT_ID_SWITCH          0x3
#define UNIT_ID_DFX             0x8
#define UNIT_ID_DRAM            0x0
#define UNIT_ID_DRAGONITE       0xa
#define UNIT_ID_BC3_MG0         0x3
#define UNIT_ID_BC3_MG1         0x5
#define UNIT_ID_BC3_MG2         0x6
#define UNIT_ID_BC3_MG3         0x7
#define UNIT_ID_BC3_IHB         0x2

#define RES_OFFSET(_nm) offsetof(CPSS_HW_INFO_STC,resource._nm)

struct pci_decoding_window {
    uint8_t     win;         /* window number 0..5 */
    uint8_t     bar;         /* pci bar map to 1..2, map to PCI BAR2, BAR4 */
    unsigned    base_offset; /* offset from BAR base */
    unsigned    size;        /* window size, 64KB granularity, see A.2.9.1.*/
    unsigned    remap;       /* window remap */
    uint8_t     target_id;   /* A.2.9.1. */
    uint8_t     attr;
    uint8_t     enable;
    int         resource_offset; /* offset of CPSS_HW_INFO_RESOURCE_MAPPING_STC
                                  * in CPSS_HW_INFO_STC or -1 */
};

static struct pci_decoding_window ac3_pci_sysmap[] = {
    /*win  bar offset    size  remap       target_id            attr  enable*/
/* BAR 1*/
    {0,    1,  0x0,      _64M, 0x0,        UNIT_ID_SWITCH,      0x0,  1, RES_OFFSET(switching) },
/* BAR 2*/
    {1,    2,  0x0,      _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {2,    2,  _2M,      _512K,0xfff80000, UNIT_ID_DRAM,        0x3E, 1, RES_OFFSET(sram) },
    {3,    2,  _4M,      _64K, 0x0,        UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.itcm) },
    {4,    2,  _4M+_64K, _64K, 0x04000000, UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.dtcm) },
    {0xff, 0,  0x0,      0,    0x0,        0,                   0x0,  0, -1 }
};

static struct pci_decoding_window bc2_pci_sysmap[] = {
    /*win  bar offset    size  remap       target_id            attr  enable*/
/* BAR 1*/
    {0,    1,  0x0,      _64M, 0x0,        UNIT_ID_SWITCH,      0x0,  1, RES_OFFSET(switching) },
/* BAR 2*/
    {1,    2,  0x0,      _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {2,    2,  _2M,      _2M,  0xffe00000, UNIT_ID_DRAM,        0x3E, 1, RES_OFFSET(sram) },
    {0xff, 0,  0x0,      0,    0x0,        0,                   0x0,  0, -1 }
};

static struct pci_decoding_window bobk_pci_sysmap[] = {
    /*win  bar offset    size  remap       target_id            attr  status*/
/* BAR 1*/
    {0,    1,  0x0,      _64M, 0x0,        UNIT_ID_SWITCH,      0x0,  1, RES_OFFSET(switching) },
/* BAR 2*/
    {1,    2,  0x0,      _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {2,    2,  _2M,      _512K,0xfff80000, UNIT_ID_DRAM,        0x3E, 1, RES_OFFSET(sram) },
    {3,    2,  _4M,      _64K, 0x0,        UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.itcm)},
    {4,    2,  _4M+_64K, _64K, 0x04000000, UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.dtcm)},
    {0xff, 0,  0x0,      0,    0x0,        0,                   0x0,  0, -1 }
};

static struct pci_decoding_window aldrin_pci_sysmap[] = {
    /*win  bar offset    size  remap       target_id            attr  status*/
/* BAR 1*/
    {0,    1,  0x0,      _64M, 0x0,        UNIT_ID_SWITCH,      0x0,  1, RES_OFFSET(switching) },
/* BAR 2*/
    {1,    2,  0x0,      _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {2,    2,  _4M,      _64K, 0x0,        UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.itcm) },
    {3,    2,  _4M+_64K, _64K, 0x04000000, UNIT_ID_DRAGONITE,   0x0,  1, RES_OFFSET(dragonite.dtcm) },
    {0xff, 0,  0x0,      0,    0x0,        0,                   0x0,  0, -1 }
};

static struct pci_decoding_window bc3_pci_sysmap[] = {
    /*win  bar offset       size  remap       target_id            attr  status*/
/* BAR 1*/
    /* MG0 */
    {0,    1,  0x0,           _4M,  0x0,        UNIT_ID_BC3_MG0,     0x0,  1, RES_OFFSET(switching) },
    /* MG1, legacy addr compl or compl region1==1, allow CM3#2 direct access */
    {1,    1,  _4M,           _1M,  0x0,        UNIT_ID_BC3_MG1,     0x0,  1, RES_OFFSET(mg1) },
    {2,    1,  _4M+_1M,       _512K,0x0,        UNIT_ID_BC3_MG2,     0x0,  1, RES_OFFSET(mg2) },
    {3,    1,  _4M+_2M,       _512K,0x0,        UNIT_ID_BC3_MG3,     0x0,  1, RES_OFFSET(mg3) },
/* BAR 2*/
    {4,    2,  0x0,           _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {5,    2,  _1M,           _64K, 0x0,        UNIT_ID_BC3_IHB,     0x0,  1, -1 },
    {0xff, 0,  0x0,             0,  0x0,        0,                   0x0,  0, -1 }
};

static struct pci_decoding_window pipe_pci_sysmap[] = {
    /*win  bar offset       size  remap       target_id            attr  status*/
/* BAR 1*/
    /* MG0 */
    {0,    1,  0x0,          _64M, 0x0,         UNIT_ID_SWITCH,      0x0,  1, RES_OFFSET(switching) },
/* BAR 2*/
    {1,    2,  0x0,           _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {0xff, 0,  0x0,             0,  0x0,        0,                   0x0,  0, -1 }
};

/* currently for EMULATOR use same mapping as in BC3 */
static struct pci_decoding_window aldrin2_pci_sysmap[] = {
    /*win  bar offset       size  remap       target_id            attr  status*/
/* BAR 1*/
    /* MG0 */
    {0,    1,  0x0,           _4M,  0x0,        UNIT_ID_BC3_MG0,     0x0,  1, RES_OFFSET(switching) },
    /* MG1, legacy addr compl or compl region1==1, allow CM3#2 direct access */
    {1,    1,  _4M,           _1M,  0x0,        UNIT_ID_BC3_MG1,     0x0,  1, RES_OFFSET(mg1) },
    {2,    1,  _4M+_1M,       _512K,0x0,        UNIT_ID_BC3_MG2,     0x0,  1, RES_OFFSET(mg2) },
    {3,    1,  _4M+_2M,       _512K,0x0,        UNIT_ID_BC3_MG3,     0x0,  1, RES_OFFSET(mg3) },
/* BAR 2*/
    {4,    2,  0x0,           _1M,  0x0,        UNIT_ID_DFX,         0x0,  1, RES_OFFSET(resetAndInitController) },
    {5,    2,  _1M,           _64K, 0x0,        UNIT_ID_BC3_IHB,     0x0,  1, -1 },
    {0xff, 0,  0x0,             0,  0x0,        0,                   0x0,  0, -1 }
};


struct prestera_config {
    const char *name;
    unsigned    devId;
    unsigned    devIdMask;
    struct      pci_decoding_window* sysmap;
    PRV_NOKM_DEVFAMILY_ENT  devFamily;
} prestera_sysmap[] = {
    { "bobcat2",   0xfc00, 0xff00, bc2_pci_sysmap, PRV_NOKM_DEVFAMILY_BOBCAT2_E },
    { "alleycat3", 0xf400, 0xff00, ac3_pci_sysmap, PRV_NOKM_DEVFAMILY_AC3_E },
    { "xcat3s",    0xf500, 0xff00, ac3_pci_sysmap, PRV_NOKM_DEVFAMILY_AC3_E },
    { "cetus",     0xbe00, 0xff00, bobk_pci_sysmap, PRV_NOKM_DEVFAMILY_BOBK_E },
    { "caelum",    0xbc00, 0xff00, bobk_pci_sysmap, PRV_NOKM_DEVFAMILY_BOBK_E },
    { "aldrin",    0xc800, 0xff00, aldrin_pci_sysmap, PRV_NOKM_DEVFAMILY_ALDRIN_E },
    { "bobcat3",   0xd400, 0xff00, bc3_pci_sysmap, PRV_NOKM_DEVFAMILY_BOBCAT3_E },
    { "pipe"   ,   0xc400, 0xff00, pipe_pci_sysmap, PRV_NOKM_DEVFAMILY_PIPE_E }, /* CPSS_98PX1012_CNS & 0xFF00 */
    { "aldrin2",   0xcc00, 0xff00, aldrin2_pci_sysmap, PRV_NOKM_DEVFAMILY_ALDRIN2_E },/*CPSS_98EX5520_CNS*/
    { "no_sysmap", 0x0000, 0x0000, NULL, PRV_NOKM_DEVFAMILY_UNKNOWN_E }, /* Always caugth here if no decoding window required */

    { NULL,0,0,NULL }
};


/*
 * Based on
 * Bobcat2 Control and Management Subsystem
 */

static GT_U32 mv_window_ctl_reg(int win)
{
    if (win < 5)
        return 0x41820+win*0x10;
    if (win == 5)
        return 0x41880;
    return 0x41c00+0x10*(win-6); /* win 6..15 */
}
static GT_U32 mv_window_base_reg(int win)
{
    return mv_window_ctl_reg(win) + 4;
}
static GT_U32 mv_window_remap_reg(int win)
{
    return mv_window_ctl_reg(win) + 0xc;
}


/***************************************************/
/*   resize PCI bar (AC3, BC2, BobK)               */
/***************************************************/
static int mv_resize_bar(GT_UINTPTR regsBase, GT_U32 ctrlReg, unsigned size)
{
    uint32_t data;
    /* first read control reg if it need to be changed */
    data = prvNoKm_reg_read(regsBase, ctrlReg);
    if ((data | 1) == (SIZE_TO_BAR_REG(size) | 1))
        return 0;
    /* Disable BAR before reconfiguration */
    prvNoKm_reg_write(regsBase, ctrlReg, data & 0xfffffffe);
    /* Resize */
    prvNoKm_reg_write(regsBase, ctrlReg, SIZE_TO_BAR_REG(size));
    /* Enable BAR */
    prvNoKm_reg_write_field(regsBase, ctrlReg, 0x1, 0x1);
    return 1;
}
/***************************************************/
/*   calculate total bar size (AC3, BC2, BobK)     */
/*   must be power of 2                            */
/***************************************************/
static unsigned mv_calc_bar_size(struct pci_decoding_window *win_map, uint8_t bar)
{
    uint8_t target;
    unsigned size = 0;

    for (target = 0; win_map[target].win != 0xff; target++) {
        if (!win_map[target].enable)
            continue;

        if (win_map[target].bar != bar)
            continue;

        while (size < win_map[target].base_offset + win_map[target].size)
            size = (size ? (size << 1) : 0x10000);

    }
    return size;
}

/* open file in /sys/bus/pci/devices/$domain:$bus:$dev.$func/ */
static GT_STATUS sysfs_pci_open(
    DBDF_DECL,
    IN  const char *name,
    IN  int     flags,
    OUT int     *fd
)
{
    char fname[128];
    if (pciDomain > 0xffff || pciBus > 0xff || pciDev > 31 || pciFunc > 7)
        return GT_BAD_PARAM;
    sprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/%s",
            pciDomain, pciBus, pciDev, pciFunc, name);
    *fd = open(fname, flags);
    if (*fd < 0)
        return GT_FAIL;
    return GT_OK;
}

static GT_STATUS sysfs_pci_readNum(
    DBDF_DECL,
    IN  const char *name,
    OUT unsigned *val
)
{
    GT_STATUS ret;
    int fd, n;
    char buf[64];

    ret = sysfs_pci_open(DBDF, name, O_RDONLY, &fd);
    if (ret != GT_OK)
        return ret;
    n=read(fd, buf, sizeof(buf));
    close(fd);
    if (n <= 0)
        return GT_FAIL;
    buf[n] = 0;
    if (sscanf(buf, "%i", val) != 1)
        return GT_FAIL;
    return GT_OK;
}


/* open file to read/write register */
static GT_STATUS sysfs_pci_open_regs_and_lseek(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    IN  int     flags,
    OUT int     *fd
)
{
    if (regAddr & 3 || regAddr >= 64)
        return GT_BAD_PARAM;
    if (sysfs_pci_open(DBDF, "config", flags, fd) != GT_OK)
        return GT_FAIL;
    if (lseek(*fd, regAddr, SEEK_SET) < 0)
    {
        close(*fd);
        return GT_FAIL;
    }
    return GT_OK;
}

/**
* @internal prvExtDrvPciSysfsConfigWriteReg function
* @endinternal
*
* @brief   This routine write register to the PCI configuration space.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigWriteReg
(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    int fd, n;
    GT_STATUS ret;
    ret = sysfs_pci_open_regs_and_lseek(DBDF, regAddr, O_RDWR, &fd);
    if (ret != GT_OK)
        return ret;
    /* data to LE */
    data = htole32(data);
    n = (int)write(fd, &data, 4);
    close(fd);
    return (n == 4) ? GT_OK : GT_FAIL;
}


/**
* @internal prvExtDrvPciSysfsConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigReadReg
(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    int fd, n;
    GT_STATUS ret;

    ret = sysfs_pci_open_regs_and_lseek(DBDF, regAddr, O_RDONLY, &fd);
    if (ret != GT_OK)
        return ret;
    n = (int)read(fd, data, 4);
    if (n != 4)
    {
        close(fd);
        return GT_FAIL;
    }
    /* LE to CPU */
    *data = le32toh(*data);

    close(fd);
    return GT_OK;
}

static GT_STATUS sysfs_pci_map_resource(
    DBDF_DECL,
    IN  const char  *resname,
    IN  GT_UINTPTR   maxSize,
    IN  GT_UINTPTR   barOffset,
    OUT GT_UINTPTR  *mappedBase,
    OUT GT_UINTPTR  *mappedSize,
    OUT int         *fdPtr
)
{
    int fd;
    GT_STATUS ret;
    struct stat st;
    void *vaddr = NULL;

    ret = sysfs_pci_open(DBDF, resname, O_RDWR, &fd);
    if (ret != GT_OK)
        return ret;
    if (fstat(fd, &st) < 0)
    {
        close(fd);
        return GT_FAIL;
    }
    if (maxSize+barOffset > st.st_size || maxSize == 0)
        maxSize = ((GT_UINTPTR)st.st_size) - barOffset;

#ifdef SHARED_MEMORY
    if (fdPtr == NULL)
    {
        vaddr = (void*)prvNoKmDrv_resource_virt_addr;
        prvNoKmDrv_resource_virt_addr += maxSize;
    }
#endif
    vaddr = mmap(vaddr,
                maxSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED
#ifdef SHARED_MEMORY
                | ((vaddr == NULL) ? 0 : MAP_FIXED)
#endif
                , fd,
                (off_t)barOffset);
    if (MAP_FAILED == vaddr)
    {
        close(fd);
        return GT_FAIL;
    }
    *mappedBase = (GT_UINTPTR)vaddr;
    *mappedSize = (GT_UINTPTR)maxSize;
    if (fdPtr != NULL)
    {
        *fdPtr = fd;
    }
#ifdef SHARED_MEMORY
    else
    {
        FILE *f;
        f = fopen(SHMEM_PP_MAPPINGS_FILENAME, "a");
        if (f == NULL)
            return GT_FAIL;
        fprintf(f, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/%s %p %p %p\n",
                pciDomain, pciBus, pciDev, pciFunc, resname,
                vaddr, (void*)((GT_UINTPTR)maxSize), (void*)barOffset);
        fclose(f);
    }
#endif
    return GT_OK;
}

/***************************************************/
/*   read PEX addresses of PCI BAR0, BAR1, BAR2    */
/***************************************************/
static GT_STATUS sysfs_read_pex_bar_addresses(
    DBDF_DECL,
    OUT unsigned long long *res
)
{
    int i;
    GT_U32 word;

    for (i = 0; i < 3; i++)
    {
        /* read PEX BARx internal register */
        prvExtDrvPciSysfsConfigReadReg(DBDF, 0x10 + i*8, &word);
        if (word == 0 || word & 1)
            continue;
        res[i] = (unsigned long long)(word & 0xffff0000);
        if ((word & 0x0000006) == 4) /* type == 2 == 64bit address */
        {
            /* read PEX BARx internal (high) register */
            prvExtDrvPciSysfsConfigReadReg(DBDF, 0x10 + i*8 + 4, &word);
            res[i] |= ((unsigned long long)word) << 32;
        }
    }
    return GT_OK;
}

static GT_STATUS sysfs_pci_write(
    DBDF_DECL,
    const char *fname,
    const char *str
)
{
    int fd, n;
    /* enable device */
    if (sysfs_pci_open(DBDF, fname, O_WRONLY, &fd) != GT_OK)
    {
        printf("  Failed to open '%s' for write\n", fname);
        return GT_FAIL;
    }
    n = (int)write(fd, str, strlen(str));
    close(fd);
    return (n == strlen(str)) ? GT_OK : GT_FAIL;
}

static int sysfs_pci_detect_dev(
    DBDF_DECL,
    int print_found
)
{
    unsigned pciVendorId, pciDeviceId, pciClass;
    int i;
    if (sysfs_pci_readNum(DBDF, "vendor", &pciVendorId) != GT_OK)
        return -1;
    if (pciVendorId != 0x11ab)
        return -1;
    if (sysfs_pci_readNum(DBDF, "class", &pciClass) != GT_OK)
        return -1;
    if (pciClass != 0x020000 && pciClass != 0x058000)
        return -1;
    if (sysfs_pci_readNum(DBDF, "device", &pciDeviceId) != GT_OK)
        return -1;
#ifdef NOKMDRV_INFO
    if (print_found)
    {
        NOKMDRV_IPRINTF(("Found marvell device %04x:%04x @%04x:%02x:%02x.%d\n",
                pciVendorId, pciDeviceId,
                pciDomain, pciBus, pciDev, pciFunc));
    }
#endif

    for (i = 0; prestera_sysmap[i].sysmap; i++)
    {
        if ((pciDeviceId & prestera_sysmap[i].devIdMask) ==
                prestera_sysmap[i].devId)
        {
            /* found in table */
            return i;
        }
    }
    return i;
}

/* SIP5 devices:
 * Configure PEX BAR1, BAR2 size
 * Force PEX rescan if changed
 */
static void sysfs_pci_configure_pex(
    DBDF_DECL
)
{
    int i, fd;
    unsigned size, current_size;
    uint32_t data;
    struct pci_decoding_window* sysmap = NULL;
    GT_UINTPTR regsBase, regsSize;

    i = sysfs_pci_detect_dev(DBDF, 1);
    if (i < 0)
        return;
    sysmap = prestera_sysmap[i].sysmap;
    if (prestera_sysmap[i].name)
        NOKMDRV_IPRINTF(("  Configuring %s...\n", prestera_sysmap[i].name));
    /***************************************************/
    /** pre-Configure DMA                             **/
    /** Configure parameter for mvDmaDrv              **/
    /**                                               **/
    /** mvDmaDrv will allocate DMA memory for this    **/
    /** PCI device. It will also configure IOMMU,     **/
    /** so DMA transactions from PP to system memory  **/
    /** will be allowed                               **/
    /***************************************************/
    mvDmaDrvOffset =
            ((pciDomain & 0xffff) << 16) |
            ((pciBus & 0xff) << 8) |
            ((pciDev & 0x1f) << 3) |
            (pciFunc & 0x7);

    if (!sysmap) /* no matching entries, nothing to do */
    {
        /* legacy device, 32-bit phys addresses allowed */
        if (prvExtDrvDmaPhys64 & 0xffffffff00000000L)
        {
            static GT_U32   alreadyPrintWarning = 0;

            if(alreadyPrintWarning == 0)
            {
                alreadyPrintWarning = 1;

                fprintf(stderr, "\r\n\n");
                fprintf(stderr, "**** A physical DMA address exceeds 32-bit limit\n");
                fprintf(stderr, "**** This is not supported on legacy devices\n");
                fprintf(stderr, "**** Supported: AC3, BC2, BOBK, ...\n");
                fprintf(stderr, "**** Please set envronment variable MVPP_DENY64BITPHYS\n");
                fprintf(stderr, "**** to deny such physical addresses\n\n");
                fprintf(stderr, "sleep 5 seconds...");
                sleep(5);
                fprintf(stderr, "done\n");
            }
        }
        return;
    }

    /* enable device */
    if (sysfs_pci_write(DBDF, "enable", "1\n") != GT_OK)
        return;
    /***************************************************/
    /** Configure PCI BARs                            **/
    /***************************************************/
    /* map pci bar0 */
    if (sysfs_pci_map_resource(DBDF, "resource0", _1M, 0, &regsBase, &regsSize, &fd) != GT_OK)
    {
        printf("  Failed to map resource0, device not configured\n");
        return;
    }

    /* configure BAR1 size */
    i = 0;
    size = mv_calc_bar_size(sysmap, 1);
    data = prvNoKm_reg_read(regsBase, MG_PEX_BAR1_CONTROL);
    current_size = (data & 0xffff0000) + 0x10000;
    /* avoid resize if BAR1 is enabled and have acceptable size */
    if (current_size < size || current_size > _64M || (size != 0 && (data & 1) == 0))
        i += mv_resize_bar(regsBase, MG_PEX_BAR1_CONTROL, size);
    /* configure BAR2 size */
    size = mv_calc_bar_size(sysmap, 2);
    data = prvNoKm_reg_read(regsBase, MG_PEX_BAR2_CONTROL);
    current_size = (data & 0xffff0000) + 0x10000;
    /* avoid resize if BAR2 is enabled and have acceptable size */
    if (current_size < size || current_size > _8M || (size != 0 && (data & 1) == 0))
        i += mv_resize_bar(regsBase, MG_PEX_BAR2_CONTROL, size);

    /* unmap pci bar 0 */
    munmap((void*)regsBase, regsSize);
    close(fd);

    if (i == 0)
        return;
    /* at least one of bar change its size */
    /* rescan device */
    if (sysfs_pci_write(DBDF, "remove", "1\n") != GT_OK)
        return;
    usleep(500000);
    if ((fd = open("/sys/bus/pci/rescan", O_WRONLY)) < 0)
    {
        perror("  Failed to open '/sys/bus/pci/rescan' for write");
        return;
    }
    if (write(fd,"1\n",2) != 2)
    {
        close(fd);
        return;
    }
    close(fd);
    usleep(500000);

    /* enable device */
    sysfs_pci_write(DBDF, "enable", "1\n");
}

static GT_STATUS sysfs_pci_configure_map(
    DBDF_DECL,
    IN  GT_U32              flags,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr,
    OUT GT_BOOL            *isSip5
)
{
    struct pci_decoding_window *sysmap, *win_map;
    GT_UINTPTR bar0Base = 0;
    GT_UINTPTR bar0Size = 0;
/* map BAR1, BAR2 don't split per-resource */
#define COMPAT_LEGACY_BARS
#ifdef COMPAT_LEGACY_BARS
    GT_UINTPTR barBase[2], barSize[2];
#endif
    unsigned long long barPhys[3] = { 0, 0, 0 };
    int i;

    *isSip5 = GT_FALSE;
    i = sysfs_pci_detect_dev(DBDF, 0);
    if (i < 0)
    {
        return GT_FAIL;
    }

    memset(hwInfoPtr, 0, sizeof(*hwInfoPtr));
    hwInfoPtr->busType = CPSS_HW_INFO_BUS_TYPE_PEX_E;
    hwInfoPtr->hwAddr.busNo = (pciDomain << 8) | pciBus;
    hwInfoPtr->hwAddr.devSel = pciDev;
    hwInfoPtr->hwAddr.funcNo = pciFunc;

    sysmap = prestera_sysmap[i].sysmap;

#define R hwInfoPtr->resource

    /* config: BAR0*/
    sysfs_pci_map_resource(DBDF, "resource0", _1M, 0,
            &bar0Base, &bar0Size, NULL);

    sysfs_read_pex_bar_addresses(DBDF, barPhys);
    R.cnm.start = bar0Base;
    R.cnm.size  = bar0Size;
    R.cnm.phys  = (GT_PHYSICAL_ADDR)barPhys[0];

    if (!sysmap) /* not a SIP5, EAGLE or legacy device */
    {
        GT_UINTPTR   bar2MaxSize = _64M;

        if (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E)
        {
            bar2MaxSize = _4M;
        }
        else if (flags & MV_EXT_DRV_CFG_FLAG_EAGLE_E)
        {
            /* Eyalo: currently no 2G support */
            bar2MaxSize = _4M;
            /*bar2MaxSize = _2G*/;
        }

        /* BAR2 */
        sysfs_pci_map_resource(DBDF, "resource2", bar2MaxSize, 0,
                &(R.switching.start), &(R.switching.size), NULL);
        R.switching.phys  = (GT_PHYSICAL_ADDR)barPhys[1];

        /* BAR4 */
        sysfs_pci_map_resource(DBDF, "resource4", 0, 0,
                &(R.resetAndInitController.start),
                &(R.resetAndInitController.size), NULL);
        R.resetAndInitController.phys  = (GT_PHYSICAL_ADDR)barPhys[2];

        /* NOTE: we return *isSip5 = GT_FALSE; because we not want the caller to
           do 'DMA init'

           the 'DMA init' will be done at later stage by the application after 'cpss phase1'
           by calling : prvNoKmDrv_configure_dma_per_devNum(...)
        */
        return GT_OK;
    }

    /* SIP5+ devices */
    *isSip5 = GT_TRUE;
    if (!R.cnm.start)
        return GT_FAIL;

    NOKMDRV_IPRINTF(("Configuring %s @ %04x:%02x:%02x.%x PEX mapping...\n",
            prestera_sysmap[i].name, DBDF));
    /* disable all windows which points bar2 & bar4 */
    for (i = 0; i < 6; i++)
    {
        prvNoKm_reg_write(R.cnm.start, mv_window_ctl_reg(i), 0);
        prvNoKm_reg_write(R.cnm.start, mv_window_base_reg(i), 0);
        prvNoKm_reg_write(R.cnm.start, mv_window_remap_reg(i), 0);
    }

#ifdef COMPAT_LEGACY_BARS
    /* Calculate mapping size for BAR1 (add addresses for MG1, MG2, MG3) */
    barSize[0] = (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E) ? _4M : _64M;
    for (win_map = sysmap; win_map->win != 0xff; win_map++)
    {
        if (win_map->bar != 1)
            continue;
        if ((GT_UINTPTR)win_map->base_offset + (GT_UINTPTR)win_map->size > barSize[0])
            barSize[0] = (GT_UINTPTR)win_map->base_offset + (GT_UINTPTR)win_map->size;
    }

    sysfs_pci_map_resource(DBDF, "resource2", barSize[0], 0,
            &(barBase[0]), &(barSize[0]), NULL);
    sysfs_pci_map_resource(DBDF, "resource4", 0, 0,
            &(barBase[1]), &(barSize[1]), NULL);
#endif
    /* configire windows */
    for (win_map = sysmap; win_map->win != 0xff; win_map++)
    {
        GT_U32 win_ctl, win_base, win_remap, ctl_val, phys;

        if (!win_map->enable)
            continue;

        if (win_map->win >= 6)
        {
            NOKMDRV_IPRINTF(("  bad window number: %d\n", win_map->win));
            return GT_FAIL;
        }

        win_ctl = mv_window_ctl_reg(win_map->win);
        win_base = mv_window_base_reg(win_map->win);
        win_remap = mv_window_remap_reg(win_map->win);
        ctl_val =   SIZE_TO_BAR_REG(win_map->size) |
                    (win_map->target_id << 4) |
                    (win_map->attr << 8) |
                    (((win_map->bar == 1) ? 0 : 1) << 1) |
                    0x1 /* enable window */;
        phys = (GT_U32)(barPhys[win_map->bar]);
        phys += win_map->base_offset;

        prvNoKm_reg_write(R.cnm.start, win_base, phys);
        prvNoKm_reg_write(R.cnm.start, win_remap, win_map->remap | 0x1/*remap_enable*/);
        prvNoKm_reg_write(R.cnm.start, win_ctl, ctl_val);

        NOKMDRV_IPRINTF(("  BAR%d: win%d_ctrl = 0x%08x, win_base = 0x%08x\n",
                win_map->bar, win_map->win,
                prvNoKm_reg_read(R.cnm.start, win_ctl),
                prvNoKm_reg_read(R.cnm.start, win_base)));
        if (win_map->resource_offset >= 0)
        {
#ifndef COMPAT_LEGACY_BARS
            GT_UINTPTR base = 0, size = 0, mapSize;
            mapSize = win_map->size;
            if ((win_map->resource_offset == RES_OFFSET(switching))
                && (flags & MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E)
                && (mapSize > _4M))
            {
                mapSize = _4M;
            }
            if (sysfs_pci_map_resource(DBDF,
                    (win_map->bar == 1) ? "resource2" : "resource4",
                    mapSize, win_map->base_offset,
                    &base, &size, NULL) == GT_OK)
            {
                CPSS_HW_INFO_RESOURCE_MAPPING_STC *resource;
                resource = (CPSS_HW_INFO_RESOURCE_MAPPING_STC*)
                        (((GT_UINTPTR)hwInfoPtr)+win_map->resource_offset);
                resource->start = base;
                resource->size = size;
                resource->phys = (GT_PHYSICAL_ADDR)win_map->remap;
                if (win_map->resource_offset <= RES_OFFSET(resetAndInitController))
                    resource->phys = (GT_PHYSICAL_ADDR)phys;
            }
#else /* defined(COMPAT_LEGACY_BARS) */
            CPSS_HW_INFO_RESOURCE_MAPPING_STC *resource;
            resource = (CPSS_HW_INFO_RESOURCE_MAPPING_STC*)
                    (((GT_UINTPTR)hwInfoPtr)+win_map->resource_offset);
            if ((barBase[win_map->bar-1] != 0) && (barSize[win_map->bar-1] > win_map->base_offset))
            {
                resource->start = barBase[win_map->bar-1] + win_map->base_offset;
                resource->size = win_map->size;
                if (win_map->resource_offset <= RES_OFFSET(resetAndInitController))
                    resource->phys = barPhys[win_map->bar] + win_map->base_offset;
            }
#endif
        }
    }

    return GT_OK;

}
/**
* @internal prvExtDrvPciSysfsConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources userspace and detects IRQ
*
* @param[out] hwInfoPtr                - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigure
(
    DBDF_DECL,
    IN  GT_U32                  flags,
    OUT CPSS_HW_INFO_STC       *hwInfoPtr
)
{
    GT_STATUS   rc;
    GT_BOOL     isSip5;
    unsigned    pciInt;
    GT_U32      family;


    rc = sysfs_pci_configure_map(DBDF, flags, hwInfoPtr, &isSip5);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* NOTE: in sip6 we will get :  isSip5 == GT_FALSE !
       the call to prvNoKmDrv_configure_dma replaced by prvNoKmDrv_configure_dma_per_devNum
       prvNoKmDrv_configure_dma_per_devNum will also set the replacement of:
        prvNoKm_reg_write_field(R.cnm.start, 0x40004, 0x4, 0x4);
       */
    if (isSip5 == GT_TRUE)
    {
        NOKMDRV_IPRINTF(("  Configuring DMA...\n"));
#ifdef USE_PP_SRAM
#warning "The option USE_PP_SRAM not supported now"
#endif
        /* Enable PCIe bus mastering.
         * Should be enabled for SDMA
         * PP will generate master transactions as End Point
         *
         * PCI Express Command and Status Register
         * 0x40004
         * Bit 2: Master Enable. This bit controls the ability of the device
         *        to act as a master on the PCI Express port.
         *        When set to 0, no memory or I/O read/write request packets
         *        are generated to PCI Express.
         */
        prvNoKm_reg_write_field(R.cnm.start, 0x40004, 0x4, 0x4);

        prvNoKmDrv_configure_dma(R.switching.start, 0);

        if (prvExtDrvMvIntDrvConnected() == GT_TRUE)
        {
            family = prvNoKmDevId & 0xff00;
            if ((family == 0xbc00) || (family == 0xbe00) || (family == 0xf400) || (family == 0xf500) || (family == 0xfc00))
            {
                /* Enable switch IRQ for AC3/BC2/BobK
                 * For all Switching Core Int
                 *     Interrupt Source i Control Register
                 *     0x20b00+4*i  i=33..36
                 *         bit 28: Interrupt enable i
                 *         bit 31: Endpoint Mask i
                 */
                prvNoKm_reg_write_field(R.cnm.start, 0x20b00+4*33, 0x90000000, 0x90000000);
                prvNoKm_reg_write_field(R.cnm.start, 0x20b00+4*34, 0x90000000, 0x90000000);
                prvNoKm_reg_write_field(R.cnm.start, 0x20b00+4*35, 0x90000000, 0x90000000);
                prvNoKm_reg_write_field(R.cnm.start, 0x20b00+4*36, 0x90000000, 0x90000000);
            }

            /* Try to enable MSI interrupts */
            if (prvExtDrvMvIntDrvEnableMsi(DBDF) == GT_OK)
                prvNoKm_reg_write_field(R.cnm.start, 0x40050, 0x00010000, 0x00010000);
        }
    }

    /* interrupt */
    rc = sysfs_pci_readNum(DBDF, "irq", &pciInt);
    if (rc != GT_OK)
    {
        pciInt = 0x11ab0000 | (noKmMappingsNum & 0xffff);
    }

#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    if (prvExtDrvMvIntDrvConnected() == GT_FALSE)
    {
        pciInt = 0x11ab0000 | (noKmMappingsNum & 0xffff);
    }
#endif

    hwInfoPtr->irq.switching = pciInt;
    hwInfoPtr->intMask.switching = (GT_UINTPTR)pciInt;

    noKmMappingsList[noKmMappingsNum] = *hwInfoPtr;
    noKmMappingsNum++;

    return GT_OK;
}

/**
* @internal prvExtDrvSysfsInitDevices function
* @endinternal
*
* @brief   Scan for pci devices and initialize them
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note Linux only
*
*/
GT_STATUS prvExtDrvSysfsInitDevices(void)
{
    DIR *dir;
    struct dirent *d;
    unsigned pciDomain,pciBus,pciDev,pciFunc;

    dir = opendir("/sys/bus/pci/devices");
    if (dir == NULL)
    {
        return GT_FAIL;
    }

    while (1)
    {
        d = readdir(dir);
        if (!d)
            break;
        if (sscanf(d->d_name,"%x:%x:%x.%x", &pciDomain, &pciBus, &pciDev, &pciFunc) < 4)
            continue;

        sysfs_pci_configure_pex(DBDF);
    }
    closedir(dir);

    return GT_OK;
}

/**
* @internal prvExtDrvSysfsFindDev function
* @endinternal
*
* @brief   This routine returns the next instance of the given device (defined by
*         vendorId & devId).
* @param[in] vendorId                 - The device vendor Id.
* @param[in] devId                    - The device Id.
* @param[in] instance                 - The requested device instance.
*
* @param[out] busNo                    - PCI bus number.
* @param[out] devSel                   - the device devSel.
* @param[out] funcNo                   - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvSysfsFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
)
{
    unsigned pciBus, pciDev, pciFunc, pciDomain;
    unsigned pciDevId, pciVendId, pciClass;
    glob_t globbuf;
    size_t  c;

    GT_U32 i = 0;

    if (glob("/sys/bus/pci/devices/*", 0, NULL, &globbuf) != 0)
    {
        return GT_FAIL;
    }

    for (c = 0; c < globbuf.gl_pathc; c++)
    {
        if (sscanf(globbuf.gl_pathv[c],"/sys/bus/pci/devices/%x:%x:%x.%x", &pciDomain, &pciBus, &pciDev, &pciFunc) < 4)
            continue;
        if (sysfs_pci_readNum(DBDF, "vendor", &pciVendId) != GT_OK)
            continue;
        if (vendorId != pciVendId)
            continue;
        if (sysfs_pci_readNum(DBDF, "class", &pciClass) != GT_OK)
            continue;
        if (pciClass != 0x020000 && pciClass != 0x058000)
            continue;
        if (sysfs_pci_readNum(DBDF, "device", &pciDevId) != GT_OK)
            continue;
        if (devId != pciDevId)
            continue;
        if (instance == i)
        {
            /* found */
            *busNo = (pciDomain << 8) | pciBus;
            *devSel = pciDev;
            *funcNo = pciFunc;
            globfree(&globbuf);
            return GT_OK;
        }
        i++;
    }
    globfree(&globbuf);
    return GT_FAIL;
}

#ifdef SHARED_MEMORY
void prvNoKmSysfsRemap(
    const char *fname,
    void *vaddr,
    void *vsize,
    void *offset
)
{
    int fd;
    void *vaddrm;

#ifdef NOKMDRV_DEBUG
    printf("Map '%s'+%p to %p (0x%lx)\n",fname,offset,vaddr,(unsigned long)((GT_UINTPTR)vsize));
#endif
    fd = open(fname, O_RDWR);
    if (fd < 0)
    {
        perror("can't open resource");
        return;
    }

    vaddrm = mmap(vaddr,
                (size_t)((GT_UINTPTR)vsize),
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FIXED,
                fd,
                (off_t)((GT_UINTPTR)offset));
    if (MAP_FAILED == vaddrm)
    {
        close(fd);
        perror("can't mmap resource");
    }
}
#endif /* SHARED_MEMORY */

/**
* @internal prvNoKmSysfsUnmap function
* @endinternal
*
* @brief   This routine deletes the mappings for the specified address range.
* @param[in] addr                 - start address.
* @param[in] length               - address region size.
*
* @retval GT_OK                   - on success,
* @retval GT_FAIL                 - othersise.
*/
GT_STATUS prvNoKmSysfsUnmap(
   IN void *addr,
   IN size_t length
)
{
    if (addr == NULL || length == 0)
        return GT_BAD_PARAM;

    if (munmap(addr, length) == -1)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal prvExtDrvSysfsConfigDev function
* @endinternal
*
* @brief   This routine enables the PCI device.
*
* @param[in] busNo                 - PCI bus number.
* @param[in] devSel                - the device devSel.
* @param[in] funcNo                - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvSysfsConfigDev
(
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc
)
{
    GT_STATUS rc;
    unsigned  pciDomain = 0;
    unsigned  pciEnable;

    rc = sysfs_pci_readNum(DBDF, "enable", &pciEnable);
    if(rc == GT_OK) {
        if(pciEnable == 0)
            sysfs_pci_configure_pex(DBDF);
    }

    return rc;
}
