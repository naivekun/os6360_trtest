/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsSiliconIf.c
*
* @brief General silicon related HW Services API
*
* @version   17
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>

#define CALC_MASK_MAC(fieldLen, fieldOffset, mask)     \
            if(((fieldLen) + (fieldOffset)) >= 32)     \
             (mask) = (GT_U32)(0 - (1<< (fieldOffset)));    \
            else                                   \
          (mask) = (((1<<((fieldLen) + (fieldOffset)))) - (1 << (fieldOffset)))


MV_OS_EXACT_DELAY_FUNC   hwsOsExactDelayPtr = NULL;
MV_OS_MICRO_DELAY_FUNC   hwsOsMicroDelayPtr = NULL;
MV_OS_TIME_WK_AFTER_FUNC hwsOsTimerWkFuncPtr = NULL;
MV_OS_MEM_SET_FUNC       hwsOsMemSetFuncPtr = NULL;
MV_OS_FREE_FUNC          hwsOsFreeFuncPtr = NULL;
MV_OS_MALLOC_FUNC        hwsOsMallocFuncPtr = NULL;
MV_SERDES_REG_ACCESS_SET hwsSerdesRegSetFuncPtr = NULL;
MV_SERDES_REG_ACCESS_GET hwsSerdesRegGetFuncPtr = NULL;
MV_SERVER_REG_ACCESS_SET hwsServerRegSetFuncPtr = NULL;
MV_SERVER_REG_ACCESS_GET hwsServerRegGetFuncPtr = NULL;
MV_OS_MEM_COPY_FUNC      hwsOsMemCopyFuncPtr = NULL;
MV_OS_STR_CAT_FUNC       hwsOsStrCatFuncPtr = NULL;
MV_SERVER_REG_FIELD_ACCESS_SET hwsServerRegFieldSetFuncPtr = NULL;
MV_SERVER_REG_FIELD_ACCESS_GET hwsServerRegFieldGetFuncPtr = NULL;
MV_TIMER_GET             hwsTimerGetFuncPtr = NULL;
MV_REG_ACCESS_SET        hwsRegisterSetFuncPtr = NULL;
MV_REG_ACCESS_GET        hwsRegisterGetFuncPtr = NULL;

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
/* Pointer to a function which returns the ILKN registers DB */
MV_SIL_ILKN_REG_DB_GET hwsIlknRegDbGetFuncPtr = NULL;
#endif

/* Contains related data for specific device */
HWS_DEVICE_INFO hwsDeviceSpecInfo[HWS_MAX_DEVICE_NUM];

#if !defined (MV_HWS_FREE_RTOS)
MV_OS_HW_TRACE_ENABLE_FUNC hwsPpHwTraceEnablePtr = NULL;

/* store base address and unit index per unit per device type */
static HWS_UNIT_INFO   hwsDeviceSpecUnitInfo[LAST_SIL_TYPE][LAST_UNIT];
/* device specific functions pointers */
static MV_HWS_DEV_FUNC_PTRS hwsDevFunc[LAST_SIL_TYPE];
#else
/* store base address and unit index per unit per device type */
static HWS_UNIT_INFO   hwsDeviceSpecUnitInfo[1][LAST_UNIT];
/* device specific functions pointers */
static MV_HWS_DEV_FUNC_PTRS hwsDevFunc[1];
#endif /*MV_HWS_FREE_RTOS*/

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
GT_STATUS genInterlakenRegSet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 data, GT_U32 mask);
GT_STATUS genInterlakenRegGet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 *data, GT_U32 mask);
#endif
/* HWS log message buffer size */
#ifdef  MV_HWS_FREE_RTOS
#define HWS_LOG_MSG_BUFFER_SIZE  MV_FW_HWS_LOG_STRING_SIZE
#else
#define HWS_LOG_MSG_BUFFER_SIZE  MV_FW_HWS_LOG_STRING_SIZE * 10
#endif


#ifdef CHX_FAMILY
extern void hwsFalconIfClose(GT_U8 devNum);
/* PATCH for Falcon that currently uses bobcat3 general code */
/* but only in limited places needs 'Falcon awareness'       */
static GT_U32  falconAsBobcat3_numOfTiles = 0;
/* indicate the HWS that the Bobcat3 device is actually Falcon + number of tiles */
/* this function is called by CPSS (for Falcon) prior to bobcat3 initialization */
void hwsFalconAsBobcat3Set(GT_U32 numOfTiles)
{
    falconAsBobcat3_numOfTiles = numOfTiles ? numOfTiles : 1;
}
/* get indication if the Bobcat3 device is actually Falcon */
/* function actually return the number of Tiles (1,2,4)*/
GT_U32  hwsFalconAsBobcat3Check(void)
{
    return falconAsBobcat3_numOfTiles;
}
#endif /*#ifdef CHX_FAMILY*/

GT_STATUS hwsRegisterSetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
)
{
    GT_U32 mask;
    GT_U32 regValue;

    if(hwsRegisterSetFuncPtr == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    regValue = ((fieldData << fieldOffset) & mask);

    return hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, regValue, mask);
}

GT_STATUS hwsRegisterGetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
)
{
    GT_STATUS rc;
    GT_U32 mask;
    GT_U32 regValue;

    if(hwsRegisterGetFuncPtr == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &regValue, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldDataPtr = (GT_U32)((regValue & mask) >> fieldOffset);
    return rc;
}

#ifndef BV_DEV_SUPPORT
/**
* @internal hwsOsLocalMicroDelay function
* @endinternal
*
* @brief   This API is used in case micro sec counter is not supported
*         It will convert micro to msec and round up in case needed
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] microSec                 - Delay in micro sec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsOsLocalMicroDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
)
{
    GT_U32 msec  = (microSec / 1000) + ((microSec % 1000) ? 1 : 0);

    hwsOsExactDelayPtr(devNum, portGroup, msec);

    return GT_OK;
}
#endif /*#ifndef BV_DEV_SUPPORT*/

/**
* @internal mvUnitExtInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit based on
*         unit index.
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] unitNum                  - unit index
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
* @param[out] localUnitNumPtr          - (pointer to) local unit number (support for Bobcat3 multi-pipe)
*                                       None
*/
GT_STATUS  mvUnitExtInfoGet
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    IN  GT_U32          unitNum,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex,
    OUT GT_U32          *localUnitNumPtr
)
{
    if ((baseAddr == NULL) || (unitIndex == NULL) || (localUnitNumPtr == NULL))
    {
        return GT_BAD_PTR;
    }
    if (unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    *baseAddr = hwsDeviceSpecUnitInfo[hwsDeviceSpecInfo[devNum].devType][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[hwsDeviceSpecInfo[devNum].devType][unitId].regOffset;
#else
    *baseAddr = hwsDeviceSpecUnitInfo[0][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[0][unitId].regOffset;
#endif /*MV_HWS_FREE_RTOS*/

    if(localUnitNumPtr)
    {
        *localUnitNumPtr = unitNum;
    }
#ifndef BV_DEV_SUPPORT
    if ( BobcatA0 == HWS_DEV_SILICON_TYPE(devNum) || BobK == HWS_DEV_SILICON_TYPE(devNum) )
    {
        /* fixed units base address */
        switch (unitId)
        {
        case GEMAC_UNIT:
        case XLGMAC_UNIT:
        case MMPCS_UNIT:
        case XPCS_UNIT:
        case PTP_UNIT:
            if (unitNum >= 56)
            {
                /* Unit calculation is: Base + 0x200000 + 0x1000*(K-56): where K (56-71) represents Port
                   in addition, since in register access level we also add INDEX*K (INDEX here is 0x1000),
                   we need to remove here INDEX*56 so total calculation will be
                   INDEX*K - INDEX*56 = INDEX*(K-56) */
                *baseAddr += (0x200000 - ((*unitIndex) * 56));
            }
            break;
        case SERDES_UNIT:
            if ((hwsDeviceSpecInfo[devNum].devType == BobK) && (unitNum >= 20))
            {
                /* Unit calc is: Base + 0x40000 + 0x1000*S: where S (20-35) represents Serdes */
                *baseAddr += 0x40000;
            }
            break;
        default:
            break;
        }
    }
/* this strange !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) I added because noone of xxx_DEV_SUPPORT
    defined in simulation
*/
#endif /*#ifndef BV_DEV_SUPPORT*/
#ifdef CHX_FAMILY
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#ifndef FALCON_ASIC_SIMULATION
    if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        GT_U32  relativeAddr = (*baseAddr) & 0x00FFFFFF;
        GT_U32 localPortIndex;
        *unitIndex = 0;
            /* PATCH for Falcon (that identified as bobcat3) */
        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* GOP units */
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_GOP_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case D2D_EAGLE_UNIT:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case D2D_RAVEN_UNIT:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case RAVEN_UNIT_BASE:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_MAC400:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI400_MAC_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_MAC100:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI64_MAC_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS50:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                        relativeAddr = 0 + (localPortIndex - 1) * 0x1000;
                        break;
                    case 9:
                    case 11:
                    case 13:
                    case 15:
                        relativeAddr = 0x00080000 + (localPortIndex - 9) * 0x1000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00465000; /* start address of PCS50 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS100:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 0:
                    case 2:
                    case 4:
                    case 6:
                        relativeAddr = 0 + localPortIndex * 0x1000;
                        break;
                    case 8:
                    case 10:
                    case 12:
                    case 14:
                        relativeAddr = 0x00080000 + (localPortIndex - 8) * 0x1000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00464000; /* start address of PCS100 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS200:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 4:
                        relativeAddr = 0;
                        break;
                    case 12:
                        relativeAddr = 0x00080000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00461000; /* start address of PCS200 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS400:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 0:
                        relativeAddr = 0;
                        break;
                    case 8:
                        relativeAddr = 0x00080000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00460000; /* start address of PCS400 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_LOW_SP_PCS:
                localPortIndex = unitNum  % 16;
                if(localPortIndex < 8)
                {
                    relativeAddr = 0;
                }
                else
                {
                    relativeAddr = 0x00080000;
                }
                relativeAddr += 0x00474000; /* start address of Low Speed PCS unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_EXT:
                relativeAddr = 0x00478000 + 0x80000 * ((unitNum  % 16) / 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_RSFEC:
                relativeAddr = 0x00470000 + 0x80000 * ((unitNum  % 16) / 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_STATISTICS:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MAC_STATISTICS_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MPFS:
                relativeAddr = 0x406000 + 0x80000 * ((unitNum  % 16) / 8) + 0x4000 * (unitNum  % 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_TSU:
                relativeAddr = 0x405000 + 0x80000 * ((unitNum  % 16) / 8) + 0x4000 * (unitNum  % 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MSDB:
                relativeAddr = 0x508000 + 0x1000 * ((unitNum  % 16) / 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case SERDES_UNIT:
                /* SERDES unit */
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_SERDES_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            default:
                /* to prevent bug if miss some unit */
                return GT_NOT_INITIALIZED;
        }

        *localUnitNumPtr = 0;/* the caller uses this parameter for additional
            address calculations ... we can not allow them to do it. because
            formula can be very complex inside :
            hwsAldrin2GopAddrCalc(...) / hwsAldrin2SerdesAddrCalc(...) */
        return GT_OK;
    }
#endif /*FALCON_ASIC_SIMULATION*/
#endif /* #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3 */
#endif /* #ifdef CHX_FAMILY */

#ifdef FALCON_ASIC_SIMULATION
#ifdef CHX_FAMILY
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsFalconAsBobcat3Check() && (hwsDeviceSpecInfo[devNum].devType == Bobcat3))
    {
        GT_U32  relativeAddr = (*baseAddr) & 0x00FFFFFF;
        GT_U32 localPortIndex;
        /* PATCH for Falcon (that identified as bobcat3) */

        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* GOP units */
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_GOP_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case D2D_EAGLE_UNIT:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case D2D_RAVEN_UNIT:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case RAVEN_UNIT_BASE:
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_MAC400:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI400_MAC_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_MAC100:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI64_MAC_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS50:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                        relativeAddr = 0 + (localPortIndex - 1) * 0x1000;
                        break;
                    case 9:
                    case 11:
                    case 13:
                    case 15:
                        relativeAddr = 0x00080000 + (localPortIndex - 9) * 0x1000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00465000; /* start address of PCS50 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS100:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 0:
                    case 2:
                    case 4:
                    case 6:
                        relativeAddr = 0 + localPortIndex * 0x1000;
                        break;
                    case 8:
                    case 10:
                    case 12:
                    case 14:
                        relativeAddr = 0x00080000 + (localPortIndex - 8) * 0x1000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00464000; /* start address of PCS100 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS200:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 4:
                        relativeAddr = 0;
                        break;
                    case 12:
                        relativeAddr = 0x00080000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00461000; /* start address of PCS200 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_PCS400:
                localPortIndex = unitNum  % 16;
                switch(localPortIndex)
                {
                    case 0:
                        relativeAddr = 0;
                        break;
                    case 8:
                        relativeAddr = 0x00080000;
                        break;
                    default:
                        return GT_BAD_PARAM;
                }
                relativeAddr += 0x00460000; /* start address of PCS400 unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_LOW_SP_PCS:
                localPortIndex = unitNum  % 16;
                if(localPortIndex < 8)
                {
                    relativeAddr = 0;
                }
                else
                {
                    relativeAddr = 0x00080000;
                }
                relativeAddr += 0x00474000; /* start address of Low Speed PCS unit */
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MTI_EXT:
                relativeAddr = 0x00478000 + 0x80000 * ((unitNum  % 16) / 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MPFS:
                relativeAddr = 0x406000 + 0x80000 * ((unitNum  % 16) / 8) + 0x4000 * (unitNum  % 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_MSDB:
                relativeAddr = 0x508000 + 0x1000 * ((unitNum  % 16) / 8);
                hwsFalconAddrCalc(devNum,
                                  HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
                                  (unitNum / 16), /* global mac-to-Raven index */
                                  relativeAddr,
                                  baseAddr);
                break;

            case RAVEN_UNIT_STATISTICS:
                relativeAddr = 0;
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MAC_STATISTICS_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;

            case SERDES_UNIT:
                /* SERDES unit */
                hwsFalconAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_SERDES_E, unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            default:
                /* to prevent bug if miss some unit */
                return GT_NOT_INITIALIZED;
        }

        *localUnitNumPtr = 0;/* the caller uses this parameter for additional
            address calculations ... we can not allow them to do it. because
            formula can be very complex inside :
            hwsAldrin2GopAddrCalc(...) / hwsAldrin2SerdesAddrCalc(...) */
        return GT_OK;
    }
#endif /* #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3 */
#endif /* #ifdef CHX_FAMILY */
#endif /*FALCON_ASIC_SIMULATION*/
#ifndef BV_DEV_SUPPORT
    if (hwsDeviceSpecInfo[devNum].devType == Bobcat3)

    {
        GT_U32  offset=0;

        /* fixed units base address */
        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
                /* baseAddr + (0x1000 * a) + 0x400000 * t: where a (36-36) represents CPU Port, where t (0-1) represents Pipe
                   baseAddr + (0x1000 * a) + 0x400000 * t: where a (0-35)  represents Port_num, where t (0-1) represents Pipe */
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* baseAddr + 0x1000 * a + 0x400000 * t: where a (0-35 in steps of 4) represents Port, where t (0-1) represents Pipe */
                offset = 0x400000;
                break;
            case SERDES_UNIT:
                /* 0x13000000 + 0x1000 * a + 0x80000 * t: where a (0-36) represents SERDES_Num, where t (0-1) represents Pipe */
                offset = 0x80000;
                break;
            default:
                /* to prevent bug if miss some unit */
                                return GT_NOT_INITIALIZED;
        }

        if (unitNum <= 71)
        {
            *baseAddr |= (unitNum / 36) * offset;
            *localUnitNumPtr = (unitNum % 36);
        }
        else if ((unitNum == 72) || (unitNum == 73))
        {
            *baseAddr |= (unitNum % 2) * offset;
            *localUnitNumPtr = 36;
        }
    }
#endif /*#ifndef BV_DEV_SUPPORT*/

#if defined(ALDRIN2_DEV_SUPPORT) || defined(CHX_FAMILY)
    if(hwsDeviceSpecInfo[devNum].devType == Aldrin2)
    {
        GT_U32  relativeAddr = (*baseAddr) & 0x00FFFFFF;

        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* GOP units */
                hwsAldrin2GopAddrCalc(unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case SERDES_UNIT:
                /* SERDES unit */
                hwsAldrin2SerdesAddrCalc(unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            default:
                /* to prevent bug if miss some unit */
                return GT_NOT_INITIALIZED;
        }

        *localUnitNumPtr = 0;/* the caller uses this parameter for additional
            address calculations ... we can not allow them to do it. because
            formula can be very complex inside :
            hwsAldrin2GopAddrCalc(...) / hwsAldrin2SerdesAddrCalc(...) */
        return GT_OK;
    }
#endif
    return GT_OK;
}

/**
* @internal mvUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGet
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex
)
{
    if ((baseAddr == NULL) || (unitIndex == NULL))
    {
        return GT_BAD_PTR;
    }
    if (unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    *baseAddr = hwsDeviceSpecUnitInfo[hwsDeviceSpecInfo[devNum].devType][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[hwsDeviceSpecInfo[devNum].devType][unitId].regOffset;
#else
    *baseAddr = hwsDeviceSpecUnitInfo[0][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[0][unitId].regOffset;
#endif /*MV_HWS_FREE_RTOS*/

    return GT_OK;
}

/**
* @internal mvUnitInfoSet function
* @endinternal
*
* @brief   Init silicon specific base address and index for specified unit
*
* @param[in] devType                  - Device type
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] baseAddr                 - unit base address in device
* @param[in] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoSet
(
    MV_HWS_DEV_TYPE devType,
    MV_HWS_UNITS_ID unitId,
    GT_U32          baseAddr,
    GT_U32          unitIndex
)
{
    if ((devType >= LAST_SIL_TYPE) || (unitId >= LAST_UNIT))
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    hwsDeviceSpecUnitInfo[devType][unitId].baseAddr = baseAddr;
    hwsDeviceSpecUnitInfo[devType][unitId].regOffset = unitIndex;
#else
    hwsDeviceSpecUnitInfo[0][unitId].baseAddr = baseAddr;
    hwsDeviceSpecUnitInfo[0][unitId].regOffset = unitIndex;
#endif /*MV_HWS_FREE_RTOS*/

    return GT_OK;
}

/**
* @internal mvUnitInfoGetByAddr function
* @endinternal
*
* @brief   Return unit ID by unit address in device
*
* @param[in] devNum                   - Device Number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGetByAddr
(
    IN  GT_U8           devNum,
    IN  GT_U32          baseAddr,
    OUT MV_HWS_UNITS_ID *unitId
)
{
    GT_U32 i;

    if (unitId == NULL)
    {
        return GT_BAD_PTR;
    }
    *unitId = LAST_UNIT;
    for (i = 0; i < LAST_UNIT; i++)
    {
#if !defined (MV_HWS_FREE_RTOS)
        if (baseAddr == hwsDeviceSpecUnitInfo[hwsDeviceSpecInfo[devNum].devType][i].baseAddr)
#else
    if (baseAddr == hwsDeviceSpecUnitInfo[0][i].baseAddr)
#endif
        {
            *unitId = i;
            break;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsRedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRedundancyVectorGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *sdVector
)
{
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc == NULL)
    {
                *sdVector = 0;
                return GT_OK;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc(devNum, portGroup, sdVector);
}

/**
* @internal mvHwsClockSelectorConfig function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsClockSelectorConfig
(
        GT_U8                   devNum,
        GT_U32                  portGroup,
        GT_U32                  phyPortNum,
        MV_HWS_PORT_STANDARD    portMode
)
{
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc == NULL)
    {
                return GT_OK;
    }


    CHECK_STATUS_EXT(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc(devNum, portGroup, phyPortNum, portMode),
                     LOG_ARG_MAC_IDX_MAC(phyPortNum));

    return GT_OK;
}

/**
* @internal hwsDeviceSpecGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsDeviceSpecGetFuncPtr(MV_HWS_DEV_FUNC_PTRS **hwsFuncsPtr)
{
        /* TBD - need to update function signature to get device type */
        static GT_BOOL devFuncInitDone = GT_FALSE;

        if(devFuncInitDone == GT_FALSE)
        {
#ifdef  MV_HWS_FREE_RTOS
                hwsOsMemSetFuncPtr(hwsDevFunc, 0, sizeof(MV_HWS_DEV_FUNC_PTRS));
#else
                hwsOsMemSetFuncPtr(hwsDevFunc, 0, sizeof(MV_HWS_DEV_FUNC_PTRS) * LAST_SIL_TYPE);
#endif
                devFuncInitDone = GT_TRUE;
        }

    *hwsFuncsPtr = &hwsDevFunc[0];
}

/**
* @internal genUnitRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
*                                      address   - address to access
* @param[in] data                     -  to write
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterSet
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    MV_HWS_UNITS_ID unitId,
    GT_UOPT         unitNum,
    GT_UREG_DATA    regOffset,
    GT_UREG_DATA    data,
    GT_UREG_DATA    mask
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;

    /* get unit base address and unit index for current device.
       In Bobcat3: the return value of unitNum is localUnitNumPtr (relative value in pipe 0,1) */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, unitNum, &unitAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

    if ((unitAddr == 0) || (unitIndex == 0))
    {
        return GT_BAD_PARAM;
    }
    address = unitAddr + unitIndex * unitNum + regOffset;

    if ((unitId == INTLKN_RF_UNIT) || (unitId == ETI_ILKN_RF_UNIT))
    {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        CHECK_STATUS(genInterlakenRegSet(devNum, portGroup, address, data, mask));
#endif
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, mask));
    }

    return GT_OK;
}

/**
* @internal genUnitRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
*                                      address   - address to access
*
* @param[out] data                     - read data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterGet
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    MV_HWS_UNITS_ID unitId,
    GT_UOPT         unitNum,
    GT_UREG_DATA    regOffset,
    GT_UREG_DATA    *data,
    GT_UREG_DATA    mask
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;

    /* get unit base address and unit index for current device */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, unitNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = unitAddr + unitIndex * unitNum + regOffset;

    if ((unitId == INTLKN_RF_UNIT) || (unitId == ETI_ILKN_RF_UNIT))
    {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        CHECK_STATUS(genInterlakenRegGet(devNum, portGroup, address, data, mask));
#endif
    }
    else
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, data, mask));
    }

    return GT_OK;
}

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
/**
* @internal genInterlakenRegSet function
* @endinternal
*
* @brief   Write ILKN registers.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegSet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 data, GT_U32 mask)
{
    MV_INTLKN_REG *dbArray;
    GT_U32 regData;
    GT_U32 offset;

        if(hwsIlknRegDbGetFuncPtr == NULL)
        {
                return GT_NOT_INITIALIZED;
        }

    dbArray = hwsIlknRegDbGetFuncPtr(devNum, portGroup, address);

        if(dbArray == NULL)
        {
                return GT_NOT_SUPPORTED;
        }

        /*hwsOsPrintf("genInterlakenRegSet address 0x%x\n", address);*/

    if (mask == 0)
    {
        /* store the register data */
        regData = data;
    }
    else
    {
                CHECK_STATUS(genInterlakenRegGet(devNum, portGroup, address, &regData, 0));

                /*hwsOsPrintf("Data: 0x%x", regData);*/

                /* Update the relevant bits at the register data */
        regData = (regData & ~mask) | (data & mask);

                /*hwsOsPrintf("\tNew Data: 0x%x\n", regData);*/
    }

        offset = address & 0x7FF;

        /*hwsOsPrintf("(Set) offset 0x%x\n", offset);*/

    /* store new value in shadow */
    switch (offset)
    {
    case 0:
        dbArray->ilkn0MacCfg0 = regData;
        break;
    case 4:
        dbArray->ilkn0ChFcCfg0 = regData;
        break;
    case 0xC:
        dbArray->ilkn0MacCfg2 = regData;
        break;
    case 0x10:
        dbArray->ilkn0MacCfg3 = regData;
        break;
    case 0x14:
        dbArray->ilkn0MacCfg4 = regData;
        break;
    case 0x1C:
        dbArray->ilkn0MacCfg6 = regData;
        break;
    case 0x60:
        dbArray->ilkn0ChFcCfg1 = regData;
        break;
    case 0x200:
        dbArray->ilkn0PcsCfg0 = regData;
        break;
    case 0x204:
        dbArray->ilkn0PcsCfg1 = regData;
        break;
    case 0x20C:
        dbArray->ilkn0En = regData;
        break;
    case 0x238:
        dbArray->ilkn0StatEn = regData;
        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return hwsRegisterSetFuncPtr(devNum, portGroup, address, regData, 0);
}

/**
* @internal genInterlakenRegGet function
* @endinternal
*
* @brief   Read ILKN registers.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegGet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 *data, GT_U32 mask)
{
    MV_INTLKN_REG *dbArray;
    GT_U32 regData;
    GT_U32 offset;

    devNum = devNum;
    portGroup = portGroup;

    if (data == NULL)
    {
        return GT_BAD_PARAM;
    }

        if(hwsIlknRegDbGetFuncPtr == NULL)
        {
                return GT_NOT_INITIALIZED;
        }

        /*hwsOsPrintf("genInterlakenRegGet address 0x%x\n", address);*/

        dbArray = hwsIlknRegDbGetFuncPtr(devNum, portGroup, address);

        if(dbArray == NULL)
        {
                return GT_NOT_SUPPORTED;
        }

        offset = address & 0x7FF;

        /*hwsOsPrintf("(Set) offset 0x%x\n", offset);*/

        switch (offset)
    {
    case 0:      /*ILKN_0_MAC_CFG_0 = 0,*/
        regData = dbArray->ilkn0MacCfg0;
        break;
    case 4:
        regData = dbArray->ilkn0ChFcCfg0;
        break;
    case 0xC:
        regData = dbArray->ilkn0MacCfg2;
        break;
    case 0x10:
        regData = dbArray->ilkn0MacCfg3;
        break;
    case 0x14:
        regData = dbArray->ilkn0MacCfg4;
        break;
    case 0x1C:
        regData = dbArray->ilkn0MacCfg6;
        break;
    case 0x60:
        regData = dbArray->ilkn0ChFcCfg1;
        break;
    case 0x200:
        regData = dbArray->ilkn0PcsCfg0;
        break;
    case 0x204:
        regData = dbArray->ilkn0PcsCfg1;
        break;
    case 0x20C:
        regData = dbArray->ilkn0En;
        break;
    case 0x238:
        regData = dbArray->ilkn0StatEn;
        break;
    case 0x10C:
                /* these registers offsets are readable */
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &regData, 0));
        break;
    default:
        *data = 0;
        return GT_NOT_SUPPORTED;
    }

    if (mask == 0)
    {
        *data = regData;
    }
    else
    {
        /* Retrieve the relevant bits from the register's data and shift left */
        *data = (regData & mask);
    }
    return GT_OK;
}
#endif /*#ifdef BC2_DEV_SUPPORT*/

#ifndef MV_HWS_REDUCED_BUILD
GT_STATUS prvCpssGenericSrvCpuReNumberDevNum
(
    GT_U8   oldDevNum,
    GT_U8   newDevNum
);

/**
* @internal mvHwsReNumberDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsReNumberDevNum
(
    GT_U8   oldDevNum,
    GT_U8   newDevNum
)
{
    MV_HWS_DEV_TYPE devType;

    if ((newDevNum >= HWS_MAX_DEVICE_NUM) || (hwsDeviceSpecInfo[oldDevNum].devNum != oldDevNum))
    {
        return GT_BAD_PARAM;
    }

    devType = hwsDeviceSpecInfo[oldDevNum].devType;

    prvCpssGenericSrvCpuReNumberDevNum(oldDevNum, newDevNum);
    /* Init device */
    if ((HWS_DEV_SILICON_TYPE(oldDevNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(oldDevNum) == Aldrin2) ||
        (HWS_DEV_SILICON_TYPE(oldDevNum) == Pipe))
    {
        hwsOsMemCopyFuncPtr(hwsDeviceSpecInfo[newDevNum].serdesInfo.cpllInitDoneStatusArr,
                            hwsDeviceSpecInfo[oldDevNum].serdesInfo.cpllInitDoneStatusArr,sizeof(GT_BOOL)*MV_HWS_MAX_CPLL_NUMBER);

        hwsOsMemCopyFuncPtr(hwsDeviceSpecInfo[newDevNum].serdesInfo.cpllCurrentOutFreqArr,
                            hwsDeviceSpecInfo[oldDevNum].serdesInfo.cpllCurrentOutFreqArr,sizeof(MV_HWS_CPLL_OUTPUT_FREQUENCY)*MV_HWS_MAX_CPLL_NUMBER);
    }
    CHECK_STATUS(mvHwsDeviceInit(newDevNum, devType, NULL));

    /* Close device */
    mvHwsDeviceClose(oldDevNum);

    hwsOsMemSetFuncPtr(&hwsDeviceSpecInfo[oldDevNum], 0, sizeof (HWS_DEVICE_INFO));

    return GT_OK;
}

/**
* @internal mvHwsDeviceInit function
* @endinternal
*
* @brief   Init device,
*         according to device type call function to init all software related DB:
*         DevInfo, Port (Port modes, MAC, PCS and SERDES) and address mapping.
* @param[in] devType                  - enum of the device type
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceInit
(
    GT_U8             devNum,
    MV_HWS_DEV_TYPE   devType,
    HWS_OS_FUNC_PTR   *funcPtr
)
{
    GT_STATUS rc = GT_OK;
    switch (devType)
    {
#ifdef CHX_FAMILY
    case Lion2A0:
    case Lion2B0:
        rc = hwsLion2IfInit(devNum, funcPtr);
        break;
    case HooperA0:
        rc = hwsHooperIfInit(devNum, funcPtr);
        break;
    case BobcatA0:
        rc = hwsBobcat2IfInit(devNum, funcPtr);
        break;
    case Alleycat3A0:
        rc = mvHwsAlleycat3IfInit(devNum, funcPtr);
        break;
    case BobK:
        rc = hwsBobKIfInit(devNum, funcPtr);
        break;
    case Aldrin:
        rc = hwsAldrinIfInit(devNum, funcPtr);
        break;
    case Bobcat3:
        rc = hwsBobcat3IfInit(devNum, funcPtr);
        break;
    case Aldrin2:
        rc = hwsAldrin2IfInit(devNum, funcPtr);
        break;
    case Falcon:
        rc = hwsFalconIfInit(devNum, funcPtr);
        break;
#endif /*CHX_FAMILY*/
#ifdef PX_FAMILY
    case Pipe:
        rc = hwsPipeIfInit(devNum, funcPtr);
        break;
#endif /*PX_FAMILY*/
    default:
        break;
    }

    return rc;
}

/**
* @internal mvHwsDeviceClose function
* @endinternal
*
* @brief   Close device,
*         according to device type call function to free all resource allocated for ports initialization.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceClose
(
    GT_U8   devNum
)
{
    switch (hwsDeviceSpecInfo[devNum].devType)
    {
#ifdef CHX_FAMILY
        case Lion2A0:
        case Lion2B0:
            hwsLion2IfClose(devNum);
            break;
        case HooperA0:
            hwsHooperIfClose(devNum);
            break;
        case BobcatA0:
            hwsBobcat2IfClose(devNum);
            break;
        case Alleycat3A0:
            hwsAlleycat3IfClose(devNum);
            break;
        case BobK:
            hwsBobKIfClose(devNum);
            break;
        case Aldrin:
            hwsAldrinIfClose(devNum);
            break;
        case Bobcat3:
            hwsBobcat3IfClose(devNum);
            break;
        case Aldrin2:
            hwsAldrin2IfClose(devNum);
            break;
    case Falcon:
            hwsFalconIfClose(devNum);
            break;
#endif /*CHX_FAMILY*/
#ifdef PX_FAMILY
        case Pipe:
            hwsPipeIfClose(devNum);
            break;
#endif
        default:
            break;
    }

    return GT_OK;
}

#endif

/**
* @internal mvHwsGeneralLog function
* @endinternal
*
* @brief   Hws log message builder and logger functionn for HOST/FW
*
* @param[in] funcNamePtr              - function name pointer
* @param[in] fileNamePtr              - file name pointer
* @param[in] lineNum                  - line number
* @param[in] returnCode               - function return code
*                                      ...            - argument list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Arguments: fileNamePtr, lineNum, returnCode are used only in host side.
*
*/
GT_STATUS mvHwsGeneralLog
(
    const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    char            *fileNamePtr,
    GT_U32          lineNum,
    GT_STATUS       returnCode,
#endif
    ...
)
{
    char buffer[HWS_LOG_MSG_BUFFER_SIZE]; /* String message bufer */
    unsigned int bufferOffset=0;   /* Offset in buffer */
    va_list args;   /* parameter for iterating the arguments */
    GT_U32 val; /* current value read from argument list */
    GT_U8 stop=0; /* stop iterating the arguments */
    GT_U8 iteration=0;  /* iteration number */
    char *msgArgStringPtr;    /* string argument */
    GT_U32 msgArgStringPtrLen=0; /* string argument length */
#ifdef  MV_HWS_FREE_RTOS
    GT_U32 funcNameLength=0; /* function name string length */
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (fileNamePtr==NULL)
    {
        return GT_BAD_PTR;
    }
#endif

    if (funcNamePtr==NULL)
    {
        return GT_BAD_PTR;
    }

#ifdef  HWS_NO_LOG
    /* Prevent 'unused-parameter' compilation error in case of running in host without log enabled */
    lineNum = lineNum;
    returnCode = returnCode;
#endif

    /* Initializing the buffer */
    hwsOsMemSetFuncPtr(buffer, '\0', HWS_LOG_MSG_BUFFER_SIZE);

/* Only in FW side the function name and "Fail" stringshould be a part of the message.
   In cpss, function name and return code is taken as different parameters */
#ifdef  MV_HWS_FREE_RTOS
    /* Maximal function name to copy to buffer, with enought space for '\0' and " Fail " */
    funcNameLength = hwsStrLen(funcNamePtr);
    funcNameLength = ( funcNameLength < (HWS_LOG_MSG_BUFFER_SIZE-7) ) ? funcNameLength : (HWS_LOG_MSG_BUFFER_SIZE-7);
    /* Copying function name */
    hwsOsMemCopyFuncPtr(buffer, funcNamePtr, funcNameLength);
    bufferOffset += funcNameLength;
    /* Copy "fail" */
    hwsOsMemCopyFuncPtr(buffer + bufferOffset, " Fail ", 6);
    bufferOffset += 6;
#endif

#ifdef  MV_HWS_FREE_RTOS
    va_start(args, funcNamePtr);
#else
    va_start(args, returnCode);
#endif
    val = va_arg(args, unsigned int);
    /* While the current argument is not the last argument
       and there is enought space in the buffer */
    while (val!=LOG_ARG_NONE_MAC && stop==0)
    {
        if (iteration>0)
        {
            hwsOsMemCopyFuncPtr(buffer + bufferOffset, ",", 1);
            bufferOffset++;
        }
        else
        {
            iteration++;
        }
        /* Decoding argument */
        switch(val)
        {
            case LOG_ARG_SERDES_FLAG:
                /* Serdes argument */
                /* space check */
                if (bufferOffset+1+8 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " sd: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=8;
                break;

            case LOG_ARG_MAC_FLAG:
                /* Mac argument */
                /* space check */
                if (bufferOffset+1+8 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " mc: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=8;
                break;

            case LOG_ARG_PCS_FLAG:
                /* Pcs argument */
                /* space check */
                if (bufferOffset+1+9 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " pcs: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=9;
                break;

            case LOG_ARG_PORT_FLAG:
                /* Port argument */
                /* space check */
                if (bufferOffset+1+7 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                /* space check */
                hwsSprintf(buffer+bufferOffset, " p: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=7;
                break;

            case LOG_ARG_REG_ADDR_FLAG:
                /* Register address argument */
                /* space check */
                if (bufferOffset+1+15 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " rg: 0x%08x", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=15;
                break;

            case LOG_ARG_STRING_FLAG:
                /* String argument */
                msgArgStringPtr = va_arg(args, char*);
                msgArgStringPtrLen = hwsStrLen(msgArgStringPtr);
                /* space check */
                if (bufferOffset+1+msgArgStringPtrLen >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    /* Get at least some of the string message */
                    if ( hwsMemCpy(buffer+bufferOffset, msgArgStringPtr, HWS_LOG_MSG_BUFFER_SIZE-(bufferOffset+1)) == NULL )
                    {
                        stop=1;
                        break;
                    }
                    /* That's it, buffer is full */
                    bufferOffset += (HWS_LOG_MSG_BUFFER_SIZE-(bufferOffset+1));
                    stop=1;
                    break;
                }
                /* space check */
                hwsSprintf(buffer+bufferOffset, msgArgStringPtr);
                bufferOffset+=msgArgStringPtrLen;
                break;

            case LOG_ARG_GEN_PARAM_FLAG:
                /* Gen param address argument */
                /* space check */
                if (bufferOffset+1+9 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " prm: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=9;
                break;

            default:
                /* Unrecognized argument */
                /* space check */
                if (bufferOffset+1+7 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, "bad_arg", 7);
                bufferOffset+=7;
                break;
        }

        if (stop==0)
        {
            val = va_arg(args, unsigned int);
        }
    }
    va_end(args);

    /* FW log */
    FW_LOG_ERROR(buffer);
    /* Host (cpss) log */
#ifdef  MV_HWS_FREE_RTOS
    HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcNamePtr, " "/*fileNamePtr*/, 0/*lineNum*/, 1/*returnCode*/, buffer);
#else
    if (returnCode != GT_OK)
    {
         /* hwsOsPrintf("%s", buffer); */
        HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcNamePtr, fileNamePtr, lineNum, returnCode, buffer);
    }
    else
    {
        /*  hwsOsPrintf("%s", buffer); */
       HWS_TO_CPSS_LOG_INFORMATION_MAC(funcNamePtr, fileNamePtr, lineNum, buffer);
    }
    return GT_OK;
#endif

}

/*******************************************************************************
* mvHwsGeneralLogStrMsgWrapper
*
* DESCRIPTION:
*       Hws log message builder wrapper function for a log message that is alreay
*       evaluated by the caller.
*
* INPUTS:
*       funcNamePtr    - function name pointer
*       fileNamePtr    - file name pointer
*       lineNum        - line number
*       returnCode     - function return code
*       strMsgPtr      - string message to log
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned int mvHwsGeneralLogStrMsgWrapper
(
    const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    char            *fileNamePtr,
    unsigned int    lineNum,
    int             returnCode,
#endif
    char            *strMsgPtr
)
{
#ifndef  MV_HWS_FREE_RTOS
    return (mvHwsGeneralLog(funcNamePtr, fileNamePtr, lineNum, returnCode, LOG_ARG_STRING_FLAG, strMsgPtr, LOG_ARG_NONE_FLAG));
#else
    return (mvHwsGeneralLog(funcNamePtr, LOG_ARG_STRING_FLAG, strMsgPtr, LOG_ARG_NONE_FLAG));
#endif
}




