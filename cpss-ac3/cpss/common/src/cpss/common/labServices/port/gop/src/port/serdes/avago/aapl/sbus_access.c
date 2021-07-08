/* AAPL CORE Revision: 2.1.0 */

/* Copyright 2014 Avago Technologies. All rights reserved.                   */
/*                                                                           */
/* This file is part of the AAPL CORE library.                               */
/*                                                                           */
/* AAPL CORE is free software: you can redistribute it and/or modify it      */
/* under the terms of the GNU Lesser General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* AAPL CORE is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with AAPL CORE.  If not, see http://www.gnu.org/licenses.           */

/* AAPL Revision: 1.2.1 */
/* Template for the user_supplied functions. */
/* */

/** Doxygen File Header */
/** @file */
/** @brief User-supplied functions. */
#include "aapl.h"

#ifndef MV_HWS_BIN_HEADER

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
/*
#if !defined(UNIX) && !defined(__KERNEL__)
typedef unsigned long int GT_U32;
#else
typedef uint32_t GT_U32;
#endif
*/
#else

/* use mv_hws_avago_if.h for genRegisterSet/Get functions */
#include <mv_hws_avago_if.h>
#include <gtOs/gtGenTypes.h>
#include <mvSiliconIf.h>

#endif

/* BobK and Aldrin SBC addresses */
#define SBC_BOBK_UNIT_BASE_ADDRESS          (0x60000000)
#define SBC_UNIT_REG_ADDR(base, reg)        ((base) | (reg))
#define SBC_BOBK_MASTER_BASE_ADDRESS        (0x60040000)

/* Pipe SBC addresses */
#define SBC_PIPE_UNIT_BASE_ADDRESS          (0x06000000)
#define SBC_PIPE_MASTER_BASE_ADDRESS        (0x06040000)


/* Bobcat3 SBC addresses */
#define SBC_BC3_UNIT0_BASE_ADDRESS          (0x24000000)
#define SBC_BC3_UNIT1_BASE_ADDRESS          (0xA4000000)
#define SBC_BC3_UNIT0_REG_ADDR(reg)         (SBC_BC3_UNIT0_BASE_ADDRESS | reg)
#define SBC_BC3_UNIT1_REG_ADDR(reg)         (SBC_BC3_UNIT1_BASE_ADDRESS | reg)
#define SBC_BC3_MASTER_UNIT0_BASE_ADDRESS   (0x24040000)
#define SBC_BC3_MASTER_UNIT1_BASE_ADDRESS   (0xA4040000)

/* SBC addresses */
#define SBC_MASTER_SERDES_NUM_SHIFT         (10)  /* Serdes number offset: 0x400 */
#define SBC_MASTER_REG_ADDR_SHIFT           (2)
#define SBC_UNIT_COMMOM_CTRL_REG_ADDR       (0x0)
#define SBC_UNIT_SOFT_RESET                 (0x1)
#define SBC_UNIT_INTERNAL_ROM_ENABLE        (0x2)


/*******************************************************************************
* user_supplied_pex_address
*
* DESCRIPTION:
*       Build PEX address from sbus_addr, and reg_addr
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pex register address
*******************************************************************************/
unsigned int user_supplied_pex_address
(
    unsigned int devNum,
    unsigned char sbus_addr,
    unsigned char reg_addr
)
{
    unsigned int serdesAddress;
    GT_U32       regBaseAddr;

    /* Read Common control register */
    if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        regBaseAddr = SBC_PIPE_MASTER_BASE_ADDRESS;
    }
    else
    {
        regBaseAddr = SBC_BOBK_MASTER_BASE_ADDRESS;
    }

    serdesAddress = ((regBaseAddr)                              |
                     (sbus_addr << SBC_MASTER_SERDES_NUM_SHIFT) |
                     (reg_addr  << SBC_MASTER_REG_ADDR_SHIFT));

    return serdesAddress;
}

/*******************************************************************************
* user_supplied_pex_units_address
*
* DESCRIPTION:
*       Build PEX address from sbus_addr, and reg_addr according to
*       the SBC_Unit number.
*       Relevant for Bobcat3
*
* INPUTS:
*       sbus_addr - Serdes number
*       reg_addr  - SBC master register address
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pex register address
*******************************************************************************/
unsigned int user_supplied_pex_units_address
(
    unsigned int sbus_addr,
    unsigned int reg_addr
)
{
    unsigned int  serdesAddress;
    unsigned int  sbcMasterAddress;

    if (sbus_addr & 1 << 8)
    {
        /* ring 1*/
        sbcMasterAddress = SBC_BC3_MASTER_UNIT1_BASE_ADDRESS;
    }
    else
    {
        /* ring 0*/
        sbcMasterAddress = SBC_BC3_MASTER_UNIT0_BASE_ADDRESS;
    }

    sbus_addr &= 0xfffffeff;

    serdesAddress = ((sbcMasterAddress)                           |
                     ((sbus_addr) << SBC_MASTER_SERDES_NUM_SHIFT) |
                     (reg_addr  << SBC_MASTER_REG_ADDR_SHIFT));

    return serdesAddress;
}

/*******************************************************************************
* user_supplied_sbus_function
*
* DESCRIPTION:
*       Execute an sbus command
*
* INPUTS:
*       aapl      - Pointer to Aapl_t structure
*       addr      - SBus slice address
*       reg_addr  - SBus register to read/write
*       command   - 0 = reset, 1 = write, 2 = read
*       sbus_data - Data to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Reads, returns read data
*       Writes and reset, returns 0
*******************************************************************************/
unsigned int user_supplied_sbus_function
(
    void *          ptr,
    unsigned int    addr,
    unsigned char   reg_addr,
    unsigned char   command,
    unsigned int    sbus_data,
    int             recv_data_back
)
{
    Aapl_t *aapl = (Aapl_t *)ptr;
    Avago_addr_t addr_struct;
    unsigned int commandAddress;
    unsigned int mask      = 0xFFFFFFFF;
    GT_U32       data      = 0;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    unsigned int rcode     = 0x0;

    avago_addr_to_struct(addr,&addr_struct);
    if (command == 1/*Write Command - WRITE_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_address(devNum, addr_struct.sbus, reg_addr);
        hwsRegisterSetFuncPtr(devNum, portGroup, commandAddress, sbus_data, mask);
    }
    else if (command == 2/*Read Command - READ_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_address(devNum, addr_struct.sbus, reg_addr);
        hwsRegisterGetFuncPtr(devNum, portGroup, commandAddress, &data, mask);
        /* return data in case of read command */
        rcode = data;
    }
    else if (command == 0/*Soft Reset - RESET_SBUS_DEVICE*/)
    {
        user_supplied_sbus_soft_reset(aapl);
    }
    else if (command == 3/*Hard Reset - CORE_SBUS_RESET*/)
    {
        /* This command is part of the entire chip reset */
        /* The AAPL start execute after chip reset, therefore this command can be removed */
    }

    return rcode;
}

#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BV_DEV_SUPPORT)
/*******************************************************************************
* user_supplied_sbus_units_function
*
* DESCRIPTION:
*       Execute an sbus command according to the SBC_Unit number.
*       Relevant for Bobcat3
*
* INPUTS:
*       aapl      - Pointer to Aapl_t structure
*       addr      - SBus slice address
*       reg_addr  - SBus register to read/write
*       command   - 0 = reset, 1 = write, 2 = read
*       sbus_data - Data to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Reads, returns read data
*       Writes and reset, returns 0
*******************************************************************************/
unsigned int user_supplied_sbus_units_function
(
    void *          ptr,
    unsigned int    addr,
    unsigned char   reg_addr,
    unsigned char   command,
    unsigned int    sbus_data,
    int             recv_data_back
)
{
    Aapl_t *aapl = (Aapl_t *)ptr;
    unsigned int commandAddress;
    unsigned int mask      = 0xFFFFFFFF;
    GT_U32       data      = 0;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    unsigned int rcode     = 0x0;

    if (command == 1/*Write Command - WRITE_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_units_address(addr, reg_addr);
        hwsRegisterSetFuncPtr(devNum, portGroup, commandAddress, sbus_data, mask);
    }
    else if (command == 2/*Read Command - READ_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_units_address(addr, reg_addr);
        hwsRegisterGetFuncPtr (devNum, portGroup, commandAddress, &data, mask);
        /* return data in case of read command */
        rcode = data;
    }
    else if (command == 0/*Soft Reset - RESET_SBUS_DEVICE*/)
    {
        user_supplied_sbus_units_soft_reset(aapl);
    }
    else if (command == 3/*Hard Reset - CORE_SBUS_RESET*/)
    {
        /* This command is part of the entire chip reset */
        /* The AAPL start execute after chip reset, therefore this command can be removed */
    }

    return rcode;
}
#endif /* !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BV_DEV_SUPPORT) */
/**
* @internal user_supplied_sbus_soft_reset function
* @endinternal
*
* @brief   Execute Software reset
*
* @param[in] aapl                     - Pointer to Aapl_t structure
*                                       None
*/
void user_supplied_sbus_soft_reset
(
    Aapl_t *aapl
)
{
    unsigned int mask      = 0xFFFFFFFF;
    GT_U32       data      = 0;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    GT_U32       regAddr;

    /* Read Common control register */
    if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        regAddr = SBC_UNIT_REG_ADDR(SBC_PIPE_UNIT_BASE_ADDRESS, SBC_UNIT_COMMOM_CTRL_REG_ADDR);
    }
    else
    {
        regAddr = SBC_UNIT_REG_ADDR(SBC_BOBK_UNIT_BASE_ADDRESS, SBC_UNIT_COMMOM_CTRL_REG_ADDR);
    }
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, mask);

    if (data & SBC_UNIT_SOFT_RESET)
        return;

    /* Set SBC in reset */
    data &= ~SBC_UNIT_SOFT_RESET;
    /* Clear internal ROM enable - loading ROM from the application */
    data &= ~SBC_UNIT_INTERNAL_ROM_ENABLE;
    hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask);
    /* Take SBC out of reset */
    data |= SBC_UNIT_SOFT_RESET;
    hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask);
}

/**
* @internal user_supplied_sbus_units_soft_reset function
* @endinternal
*
* @brief   Execute Software reset command to SBC_Units.
*         Relevant for Bobcat3
* @param[in] aapl                     - Pointer to Aapl_t structure
*                                       None
*/
void user_supplied_sbus_units_soft_reset
(
    Aapl_t *aapl
)
{
    unsigned int mask      = 0xFFFFFFFF;
    GT_U32       data      = 0;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    unsigned char i;

    for (i=0; i < 2; i++)
    {
        /* Read Common control register */
        hwsRegisterGetFuncPtr(devNum,
                       portGroup,
                       (i == 0) ? SBC_BC3_UNIT0_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR) : SBC_BC3_UNIT1_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR),
                       &data,
                       mask);

        if (data & SBC_UNIT_SOFT_RESET)
            return;

        /* Set SBC in reset */
        data &= ~SBC_UNIT_SOFT_RESET;
        /* Clear internal ROM enable - loading ROM from the application */
        data &= ~SBC_UNIT_INTERNAL_ROM_ENABLE;

        hwsRegisterSetFuncPtr(devNum,
                       portGroup,
                       (i == 0) ? SBC_BC3_UNIT0_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR) : SBC_BC3_UNIT1_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR),
                       data,
                       mask);

        /* Take SBC out of reset */
        data |= SBC_UNIT_SOFT_RESET;
        hwsRegisterSetFuncPtr(devNum,
                       portGroup,
                       (i == 0) ? SBC_BC3_UNIT0_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR) : SBC_BC3_UNIT1_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR),
                       data,
                       mask);
    }
}


