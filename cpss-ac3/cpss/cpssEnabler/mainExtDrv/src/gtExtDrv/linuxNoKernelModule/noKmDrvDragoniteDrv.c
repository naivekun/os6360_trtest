/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

#include <sys/mman.h>
#include <stdio.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <cpss/common/cpssTypes.h>
#include "prvNoKmDrv.h"

#undef        DRAGONITE_DEBUG
#ifdef        DRAGONITE_DEBUG
#define dragonite_debug_printf(x) printf x
#else
#define dragonite_debug_printf(x)
#endif


static CPSS_HW_INFO_STC   drgHwInfo = CPSS_HW_INFO_STC_DEF;
static void              *drgHwDataPtr = NULL;
static DRAGONITE_HW_WRITE_MASKED_FUNC drgHwWriteMaskedFunc = NULL;

typedef struct {
    GT_U32  as; /* address space: 0 - SC, 1- C&M, 2 - DFX */
    GT_U32  addr;
} REGISTER_ADDRESS;
typedef struct {
    REGISTER_ADDRESS ctrl;
    REGISTER_ADDRESS irq_cause;
    REGISTER_ADDRESS irq_mask;
    REGISTER_ADDRESS host2poe_irq;
    REGISTER_ADDRESS jtag;
    REGISTER_ADDRESS spi;
    GT_U32 ctrl_en;
    GT_U32 ctrl_cpu_en;
    GT_U32 ctrl_tcm_en;
    GT_U32 jtag_en;
    GT_U32 jtag_en_mask;
    GT_U32 spi_en;  /* Enable SPI/SDIO bus */
} DRAGONITE_REGS;

static const DRAGONITE_REGS regs_Legacy = {
    { 0,    0x1c }, /* ctrl */
    { 0,    0x64 }, /* irq_cause */
    { 0,    0x68 }, /* irq_mask */
    { 0,    0x6c }, /* host2poe_irq */
    { 2,    0xf8290 }, /* jtag @dfx */
    { 0xFF, 0 },    /* SPI/SDIO */
    (1 << 0), /* ctrl_en */
    (1 << 1), /* ctrl_cpu_en */
    (1 << 3), /* ctrl_tcm_en */
    (1 << 0), /* jtag_en */
    (1 << 0), /* jtag_en_mask */
    0,

};
static const DRAGONITE_REGS regs_Aldrin = {
    { 0,    0x61000000 }, /* ctrl */
    { 0,    0x61000004 }, /* irq_cause */
    { 0,    0x61000008 }, /* irq_mask */
    { 0,    0x6100000c }, /* host2poe_irq */
    { 2,    0x000F8280 }, /* jtag, @dfx - mask 0x600000, val 0x400000 */
    { 2,    0x000F8280 }, /* SPI/SDIO */
    (1 << 0), /* ctrl_en */
    (1 << 1), /* ctrl_cpu_en */
    (1 << 3), /* ctrl_tcm_en */
    (2 << 21),/* jtag_en */
    (3 << 21),/* jtag_en_mask */
    (1 << 18) /* SPI */
};
static const DRAGONITE_REGS regs_Bobk = {
    { 0,    0x61000000 }, /* ctrl */
    { 0,    0x61000004 }, /* irq_cause */
    { 0,    0x61000008 }, /* irq_mask */
    { 0,    0x6100000c }, /* host2poe_irq */
    { 2,    0x000F829C }, /* jtag, @dfx */
    { 2,    0x000F829C }, /* SDIO */
    (1 << 0), /* ctrl_en */
    (1 << 1), /* ctrl_cpu_en */
    (1 << 3), /* ctrl_tcm_en */
    (2 << 13),/* jtag_en */
    (3 << 13),/* jtag_en_mask */
    (3 << 9), /* SDIO and PoE GPP interface to override the MPP functionality */
};
static const DRAGONITE_REGS *drgRegs = NULL;

/* The minimal window size of PEX/MBUS mapping is 64K,
 * so 64K will be mapped to user-space.
 * DTCM_ACTUAL_SIZE is the Actual size, vs dragonite.dtcm.size which is the mapped size
 */
#define DTCM_ACTUAL_SIZE    (32*_1K)
#define D drgHwInfo.resource.dragonite
#define CHK_EXISTS()  \
    if (D.itcm.start == 0 || D.dtcm.start == 0 || drgRegs == NULL) \
        return GT_NOT_INITIALIZED;

struct dragonite_ids_stc {
    const char *name;
    unsigned    devId;
    unsigned    devIdMask;
    const DRAGONITE_REGS  *regs;
} dragonite_ids[] = {
    { "aldrin",    0xc800, 0xff00, &regs_Aldrin },
    { "cetus",     0xbe00, 0xff00, &regs_Bobk },
    { "caelum",    0xbc00, 0xff00, &regs_Bobk },
    { NULL, 0, 0, &regs_Legacy }
};

/**
* @internal extDrvDragoniteDriverInit function
* @endinternal
*
* @brief   Initialize dragonite driver - addresses of ITCM/DTCM, method
*         to access POE configuration registers
*         (in Packet Processor address space)
* @param[in] hwInfoPtr                - hardware info srtuctute of selected PP
* @param[in] hwData                   - custom data, will be first parameter of hwWriteMaskedFunc
* @param[in] hwWriteMaskedFunc        - function to write PP registers
*                                       GT_OK if successful
*
* @note Available only in NOKM
*
*/
GT_STATUS extDrvDragoniteDriverInit(
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN void*              hwData,
    IN DRAGONITE_HW_WRITE_MASKED_FUNC hwWriteMaskedFunc
)
{
    GT_U32 data;
    GT_STATUS rc;
    int i;

    drgHwInfo = *hwInfoPtr;
    drgHwDataPtr = hwData;
    drgHwWriteMaskedFunc = hwWriteMaskedFunc;

    switch (drgHwInfo.busType)
    {
        case CPSS_HW_INFO_BUS_TYPE_PCI_E:
        case CPSS_HW_INFO_BUS_TYPE_PEX_E:
        case CPSS_HW_INFO_BUS_TYPE_MBUS_E:
            /* use extDrvPciConfigReadReg to detect device */
            break;
        default:
            return GT_BAD_PARAM;
    }
    rc = extDrvPciConfigReadReg(
            drgHwInfo.hwAddr.busNo,
            drgHwInfo.hwAddr.devSel,
            drgHwInfo.hwAddr.funcNo,
            0, /* regAddr == 0: vendor,device */
            &data);
    if (rc != GT_OK)
        return rc;
    data = (data >> 16) & 0x0000ffff;
    for (i = 0; dragonite_ids[i].name; i++)
    {
        if ((data & dragonite_ids[i].devIdMask) == dragonite_ids[i].devId)
            break;
    }
    /* TODO: printf dragonite_ids[i].name?:"legacy" */
    drgRegs = dragonite_ids[i].regs;

    return GT_OK;
}


/**
* @internal extDrvDragoniteCtrlRegWrite function
* @endinternal
*
* @brief   Sets a Dragonite ctrl register (a register which is not found in the DTCM area)
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
* @retval GT_BAD_PARAM             - when reg is out of enum range.
*/
GT_STATUS extDrvDragoniteCtrlRegWrite (
    IN extDrvDrgReg_ENT reg,
    IN GT_U32 mask,
    IN GT_U32 value
    )
{
    REGISTER_ADDRESS rg;
    CHK_EXISTS();
    if (!drgHwWriteMaskedFunc)
        return GT_NOT_INITIALIZED;

    switch (reg) {
    case extDrvDrgReg_ctrl:
        rg = drgRegs->ctrl;
        break;
    case extDrvDrgReg_maskIrq:
        rg = drgRegs->irq_mask;
        break;
    case extDrvDrgReg_jtag:
        rg = drgRegs->jtag;
        break;

    default:
        return GT_BAD_PARAM;
    }

    if (rg.as == 0xff)
        return GT_FAIL;
    return drgHwWriteMaskedFunc(drgHwDataPtr,
            rg.as, rg.addr, value, mask);
}


/**
* @internal extDrvDragoniteDevInit function
* @endinternal
*
* @brief   Config and enable dragonite sub-system. CPU still in reset
*/
GT_STATUS extDrvDragoniteDevInit
(
    GT_VOID
)
{
    CHK_EXISTS();
    if (drgHwWriteMaskedFunc == NULL)
        return GT_NOT_INITIALIZED;

    /* Config and enable dragonite sub-system. CPU still in reset */
    drgHwWriteMaskedFunc(drgHwDataPtr,
            drgRegs->ctrl.as, drgRegs->ctrl.addr,
            0xffffffff,
            drgRegs->ctrl_en | drgRegs->ctrl_tcm_en);

    /*TODO: dragonite_setup_xbridge */

#ifdef CONFIG_DRAGONITE_JTAG_ENABLE
    if (drgRegs->jtag.as != 0xff)
    {
        /* Enable JATG access to dragonite */
        drgHwWriteMaskedFunc(drgHwDataPtr,
            drgRegs->jtag.as, drgRegs->jtag.addr,
            drgRegs->jtag_en, drgRegs->jtag_en_mask);
    }
#endif

    /* Enable dragonite SPI/SDIO - if needed */
    if (drgRegs->spi.as != 0xFF)
        drgHwWriteMaskedFunc(drgHwDataPtr,
                drgRegs->spi.as, drgRegs->spi.addr,
                drgRegs->spi_en,
                drgRegs->spi_en);


    /* Disable CPU */
    drgHwWriteMaskedFunc(drgHwDataPtr,
        drgRegs->ctrl.as, drgRegs->ctrl.addr,
        0, drgRegs->ctrl_cpu_en);

    return GT_OK;
}


/**
* @internal extDrvDragoniteUnresetSet function
* @endinternal
*
* @brief   Reset/Unreset Dragonite.
*
* @param[in] unresetEnable            - GT_TRUE - set to un-reset state
*                                      GT_FALSE - set to reset state
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteUnresetSet
(
    GT_BOOL unresetEnable
)
{
    CHK_EXISTS();
    if (drgHwWriteMaskedFunc == NULL)
        return GT_NOT_INITIALIZED;

    drgHwWriteMaskedFunc(drgHwDataPtr,
        drgRegs->ctrl.as, drgRegs->ctrl.addr,
        (unresetEnable == GT_FALSE) ? 0 : 0xffffffff,
        drgRegs->ctrl_cpu_en);

    return GT_OK;
}

/**
* @internal extDrvDragoniteFwDownload function
* @endinternal
*
* @brief   Download FW to instruction shared memory
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
*                                       GT_BAD_PARAM
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteFwDownload
(
    IN GT_VOID  *buf,
    GT_U32      size
)
{
    CHK_EXISTS();

    if (DRAGONITE_FW_LOAD_ADDR_CNS + size > D.itcm.size)
        return GT_BAD_PARAM;

    extDrvDragoniteUnresetSet(GT_FALSE);

    memcpy((void*)(D.itcm.start + DRAGONITE_FW_LOAD_ADDR_CNS), buf, size);


    return GT_OK;
}

/**
* @internal extDrvDragoniteMemRead function
* @endinternal
*
*
* @param[out] buf                      - read buufer
*                                       GT_OK if successful
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
* @retval GT_BAD_PARAM             - when parametrs passed, exceed Drg memory
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMemRead
(
    IN  GT_U32 mem_direction,
    IN  GT_U32 addr,
    OUT GT_8_PTR buf,
    IN  GT_U32 size
)
{
    GT_UINTPTR base;
    GT_UINTPTR mem_size;

    CHK_EXISTS();
    if (mem_direction == ITCM_DIR)
    {
        base = D.itcm.start;
        mem_size = D.itcm.size;
    }
    else
    {
        base = D.dtcm.start;
        mem_size = DTCM_ACTUAL_SIZE;
    }
    if (addr + size > mem_size)
        return GT_BAD_PARAM;

    memcpy((void*)buf, (void*)(base+addr), size);

    return GT_OK;
}

/**
* @internal extDrvDragoniteRegisterRead function
* @endinternal
*
* @brief   Dragonite register read.
*
* @param[in] addr                     - register address to read from.
*
* @param[out] valuePtr                 - (pointer to) returned value
*                                       GT_OK if successful, or
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
* @retval GT_BAD_PTR               - on NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteRegisterRead
(
    IN  GT_U32 addr,
    OUT GT_U32 *valuePtr
)
{
    CHK_EXISTS();

    if (addr + sizeof(GT_U32) > DTCM_ACTUAL_SIZE)
        return GT_BAD_PARAM;

    *valuePtr = *((GT_U32*)(D.dtcm.start + addr));

    return GT_OK;
}

/**
* @internal extDrvDragoniteRegisterWrite function
* @endinternal
*
* @brief   Dragonite register write.
*
* @param[in] addr                     - register address to write.
*                                      value - register value to write.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteRegisterWrite
(
    IN  GT_U32 addr,
    IN  GT_U32 regValue
)
{
    CHK_EXISTS();

    if (addr + sizeof(GT_U32) > DTCM_ACTUAL_SIZE)
        return GT_BAD_PARAM;

    *((GT_U32*)(D.dtcm.start + addr)) = regValue;

    return GT_OK;
}
/**
* @internal extDrvDragoniteMemWrite function
* @endinternal
*
* @brief   Dragonite memory write.
*
* @param[in] mem_direction            - ITCM / DTCM
* @param[in] addr                     - address to write to.
* @param[in] buf                      buf to write.
* @param[in] buf_size                 - size of buf
*                                       GT_OK if successful
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
* @retval GT_BAD_PARAM             - when parametrs passed, exceed Drg memory
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMemWrite
(
    IN  GT_U32 mem_direction,
    IN  GT_U32 addr,
    IN  GT_8_PTR buf,
    IN  GT_U32  buf_size
)
{
    GT_UINTPTR base;
    GT_UINTPTR mem_size;

    CHK_EXISTS();
    if (mem_direction == ITCM_DIR)
    {
        base = D.itcm.start;
        mem_size = D.itcm.size;
    }
    else
    {
        base = D.dtcm.start;
        mem_size = DTCM_ACTUAL_SIZE;
    }
    if (addr + buf_size > mem_size)
        return GT_BAD_PARAM;

    memcpy((void*)(base+addr), (void*)buf, buf_size);

    return GT_OK;
}

/**
* @internal extDrvDragoniteMsgWrite function
* @endinternal
*
* @brief   Dragonite message write.
*
* @param[in] msg[DRAGONITE_DATA_MSG_LEN] - buffer to write.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_READY             - ownership problem
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMsgWrite
(
    IN GT_U8 msg[DRAGONITE_DATA_MSG_LEN]
)
{
    GT_32       i;                          /* loop iterator */
    GT_U32      buf[DRAGONITE_DATA_MSG_LEN];/* Dragonite buffer */

    CHK_EXISTS();

    if (prvNoKm_reg_read(D.dtcm.start, TX_MO_ADDR) != TX_MO_HOST_OWNERSHIP_CODE)
    {
        dragonite_debug_printf(("extDrvDragoniteMsgWrite: warning -> current ownership belongs to POE, regValue = 0x%x\n",
                prvNoKm_reg_read(D.dtcm.start, TX_MO_ADDR)));
        return GT_NOT_READY;
    }

    /* Converting from 'GT_U8 buf' to dragonite memory format */
    for(i = 0; i < DRAGONITE_DATA_MSG_LEN; i++)
    {
        buf[i] = htole32(msg[i]);
    }
    memcpy((void*)(D.dtcm.start + TX_BUF_ADDR), buf, sizeof(buf));

    prvNoKm_reg_write(D.dtcm.start, TX_MO_ADDR, TX_MO_POE_OWNERSHIP_CODE);
    return GT_OK;
}


/**
* @internal extDrvDragoniteMsgRead function
* @endinternal
*
* @brief   Dragonite message write.
*
* @param[out] msg[DRAGONITE_DATA_MSG_LEN] - buffer to read.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_READY             - ownership problem
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMsgRead
(
    OUT GT_U8 msg[DRAGONITE_DATA_MSG_LEN]
)
{
    GT_32       i;                          /* loop iterator */
    GT_U32      buf[DRAGONITE_DATA_MSG_LEN];/* Dragonite buffer */

    CHK_EXISTS();

    if (prvNoKm_reg_read(D.dtcm.start, RX_MO_ADDR) != RX_MO_HOST_OWNERSHIP_CODE)
    {

        dragonite_debug_printf(( "extDrvDragoniteMsgRead: warning --> current ownership belongs to POE, regVal = 0x%x\n",
                prvNoKm_reg_read(D.dtcm.start, RX_MO_ADDR)));
        return GT_NOT_READY;
    }

    memcpy(buf, (void*)(D.dtcm.start + RX_BUF_ADDR), sizeof(buf));

    /* Converting from dragonite memory format - to GT_U8 buf */
    for(i = 0; i < DRAGONITE_DATA_MSG_LEN; i++)
    {
        msg[i] = le32toh(buf[i]);
    }

    prvNoKm_reg_write(D.dtcm.start, RX_MO_ADDR, RX_MO_POE_OWNERSHIP_CODE);
    return GT_OK;
}

/**
* @internal extDrvDragoniteProtect function
* @endinternal
*
* @brief   Protect the Dragonite ITCM area
*
* @param[in] protect                  - true = protect
*                                      flase = unprotect
*                                       GT_OK if successful
*
* @retval GT_NOT_INITIALIZED       - when driver fields were not init by NOKM
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteProtect
(
    IN GT_BOOL protect
)
{
    int prot;

    CHK_EXISTS();

    if (protect)
        prot = PROT_NONE;
    else
        prot = PROT_READ | PROT_WRITE | PROT_EXEC;

    if (mprotect( (void*)D.itcm.start, D.itcm.size, prot) != 0)
        return GT_FAIL;
    return GT_OK;
}




