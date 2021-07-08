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
* mvHwsDiagnostic.h
*
* DESCRIPTION:
*
* DEPENDENCIES:
*
******************************************************************************/
#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>



GT_STATUS mvHwsDiagCheckValidInputParams(GT_U8 devNum, MV_HWS_DEV_TYPE devType);
GT_STATUS mvHwsDiagPCSInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagMACInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagSerdesInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagDdr3TipInitDBStatusGet(GT_U8 devNum);


char *DEV_TYPE[LAST_SIL_TYPE] =
{
    "Lion2A0",
    "Lion2B0",
    "Puma3A0",
    "HooperA0",
    "Puma3B0",
    "NP5A0",
    "BobcatA0",
    "Alleycat3A0",
    "NP5B0",
    "BobK",
    "Aldrin",
    "Bobcat3",
    "BearValley"
};

#define MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, pcsMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, pcsMode=%s is wrong\n", devNum, DEV_TYPE[sType], pcsMode); \
    return GT_BAD_PARAM; \
}

#define MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, macMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, macMode=%s is wrong\n", devNum, DEV_TYPE[sType], macMode); \
    return GT_BAD_PARAM; \
}

#define MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, serdesMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, serdesMode=%s is wrong\n", devNum, DEV_TYPE[sType], serdesMode); \
    return GT_BAD_PARAM; \
}

/**
* @internal mvHwsAvsVoltageGet function
* @endinternal
*
* @brief   Get the Avs voltage in mv.
*
* @param[in] devNum                   - system device number
*
* @param[out] avsVol                - Avs voltage in mv
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageGet(GT_U8 devNum, GT_U32 *avsVol) {

    if (avsVol == NULL) {
        return GT_BAD_PTR;
    }
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, 0xf8130, avsVol));
    *avsVol = (*avsVol) >> 3;
    *avsVol = (*avsVol & 0x3ff);
    *avsVol = ((*avsVol) * 10) + 640;

    return GT_OK;
}

/**
* @internal mvHwsAvsVoltageSet function
* @endinternal
*
* @brief   Set the Avs voltage in mv.
*
* @param[in] devNum                  - system device number
* @param[in] vcore                   - Avs voltage requsted in
*       mv.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageSet(GT_U8 devNum, GT_U32 vcore) {
    GT_U32 avsTargetVal;
    GT_U32 avsVal;
    GT_U32 avsDeafVal;
    GT_U32 avsRegVal;
    GT_U32 readData;

    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, 0xf8130, &avsDeafVal));
        /*[12:3]*/
        avsDeafVal = avsDeafVal >> 3;
        avsDeafVal = (avsDeafVal & 0x3ff);
        avsVal = avsDeafVal;
        avsTargetVal = (vcore - 640) / 10;

        while (avsVal != avsTargetVal) {
            if (avsVal < avsTargetVal) {
                avsVal = avsVal + 1;
            } else {
                avsVal = avsVal - 1;
            }

            avsRegVal = (avsVal << 3) | (avsVal << 13);
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, 0xf8130, &readData));

            readData = readData & (~0x7ffff8);
            avsRegVal = avsRegVal & (0x7ffff8);
            avsRegVal = avsRegVal | readData;
            /*set val*/
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, 0xf8130, avsRegVal));

            hwsOsTimerWkFuncPtr(1000);
        }
    } else {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    return GT_OK;
}




/**
* @internal mvHwsDiagDeviceDbCheck function
* @endinternal
*
* @brief   This API checks the DB initialization of PCS, MAC, Serdes
*         and DDR units
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDeviceDbCheck(GT_U8 devNum, MV_HWS_DEV_TYPE devType)
{
    GT_STATUS rc;

    /* If device not valid for hws, we cannot continue to other hws APIs.
       We also cannot use hws check status macro.*/
    rc = (mvHwsDiagCheckValidInputParams(devNum, devType));
    if (rc!=GT_OK)
    {
        return rc;
    }

    CHECK_STATUS(mvHwsDiagPCSInitDBStatusGet(devNum));
    CHECK_STATUS(mvHwsDiagMACInitDBStatusGet(devNum));
    CHECK_STATUS(mvHwsDiagSerdesInitDBStatusGet(devNum));

    /* TBD - Need to add checking to DDR initialization DB */
    /* CHECK_STATUS(mvHwsDiagDdr3TipInitDBStatusGet(devNum)); */

    return GT_OK;
}

/**
* @internal mvHwsDiagCheckValidInputParams function
* @endinternal
*
* @brief   This function checks the validity of input devNum and devType
*         parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagCheckValidInputParams(GT_U8 devNum, MV_HWS_DEV_TYPE devType)
{
    if(devNum >= HWS_MAX_DEVICE_NUM)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is bigger than HWS_MAX_DEVICE_NUM=%d value\n", devNum, HWS_MAX_DEVICE_NUM-1);
        return GT_BAD_PARAM;
    }

    if(hwsDeviceSpecInfo[devNum].devNum != devNum)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is not initialized in the system\n", devNum);
        return GT_BAD_PARAM;
    }

    if(devType >= sizeof(DEV_TYPE)/sizeof(char*))
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=[%d] devType[%d] out of the size of DEV_TYPE array \n" , devNum , devType);
        return GT_BAD_PARAM;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) != devType)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is different than Device type %s\n", devNum, DEV_TYPE[devType]);
        return GT_BAD_PARAM;
    }

    if(hwsDeviceSpecInfo[devNum].devType >= sizeof(DEV_TYPE)/sizeof(char*))
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: the size of DEV_TYPE array is wrong\n");
        return GT_BAD_VALUE;
    }

    return GT_OK;
}

/**
* @internal mvHwsDiagPCSInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of PCS unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagPCSInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr;
    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    switch (sType)
    {
    case Lion2B0:
    case HooperA0:
        if(hwsPcsFuncsPtr[MMPCS].pcsModeCfgFunc != mvHwsMMPcsV2Mode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "MMPCS");
        }

        if(hwsPcsFuncsPtr[HGLPCS].pcsModeCfgFunc != mvHwsHglPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "HGLPCS");
        }

        if(hwsPcsFuncsPtr[XPCS].pcsModeCfgFunc != mvHwsXPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "XPCS");
        }
        break;

    case Alleycat3A0:
    case BobcatA0:
        if(hwsPcsFuncsPtr[XPCS].pcsModeCfgFunc != mvHwsXPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "XPCS");
        }

        if(hwsPcsFuncsPtr[MMPCS].pcsModeCfgFunc != mvHwsMMPcs28nmMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "MMPCS");
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagPCSInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    if((sType == BobcatA0) && (hwsPcsFuncsPtr[INTLKN_PCS].pcsModeCfgFunc != mvHwsIlknPcsMode))
    {
        MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "INTLKN_PCS");
    }

    hwsOsPrintf("PCS Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}

/**
* @internal mvHwsDiagMACInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of MAC unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagMACInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_MAC_FUNC_PTRS *hwsMacFuncsPtr;
    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    switch (sType)
    {
    case Lion2B0:
    case HooperA0:
        if(hwsMacFuncsPtr[GEMAC_X].macModeCfgFunc != mvHwsGeMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_X");
        }

        if(hwsMacFuncsPtr[GEMAC_SG].macModeCfgFunc != mvHwsGeMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_SG");
        }

        if(hwsMacFuncsPtr[HGLMAC].macModeCfgFunc != mvHwsHglMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "HGLMAC");
        }

        if(hwsMacFuncsPtr[XGMAC].macModeCfgFunc != mvHwsXgMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XGMAC");
        }

        if(hwsMacFuncsPtr[XLGMAC].macModeCfgFunc != mvHwsXlgMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XLGMAC");
        }
        break;

    case Alleycat3A0:
    case BobcatA0:
        if(hwsMacFuncsPtr[GEMAC_X].macModeCfgFunc != mvHwsGeMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_X");
        }

        if(hwsMacFuncsPtr[GEMAC_NET_X].macModeCfgFunc != mvHwsGeNetMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_NET_X");
        }

        if(hwsMacFuncsPtr[QSGMII_MAC].macModeCfgFunc != mvHwsQsgmiiMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "QSGMII_MAC");
        }

        if(hwsMacFuncsPtr[GEMAC_SG].macModeCfgFunc != mvHwsGeMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_SG");
        }

        if(hwsMacFuncsPtr[GEMAC_NET_SG].macModeCfgFunc != mvHwsGeNetMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_NET_SG");
        }

        if(hwsMacFuncsPtr[XGMAC].macModeCfgFunc != mvHwsXgMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XGMAC");
        }

        if(hwsMacFuncsPtr[XLGMAC].macModeCfgFunc != mvHwsXlgMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XLGMAC");
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagMACInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    if((sType == BobcatA0) && (hwsMacFuncsPtr[INTLKN_MAC].macModeCfgFunc != mvHwsIlknMacModeCfg))
    {
        MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "INTLKN_MAC");
    }

    hwsOsPrintf("MAC Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}

/**
* @internal mvHwsDiagSerdesInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of Serdes unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagSerdesInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType  = HWS_DEV_SILICON_TYPE(devNum);
#ifndef PX_FAMILY
    HWS_DEV_GOP_REV gopRev = HWS_DEV_GOP_REV(devNum);
#endif
    MV_HWS_SERDES_FUNC_PTRS *hwsSerdesFuncsPtr;
    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    switch (sType)
    {
#ifndef PX_FAMILY
    case Lion2B0:
    case HooperA0:
        if(hwsSerdesFuncsPtr[COM_PHY_H_REV2].serdesPowerCntrlFunc != mvHwsComHRev2SerdesPowerCtrl)
        {
            MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_H_REV2");
        }
        break;
    case Alleycat3A0:
    case BobcatA0:
        switch (gopRev)
        {
        case GOP_28NM_REV1:
            if(hwsSerdesFuncsPtr[COM_PHY_28NM].serdesPowerCntrlFunc != mvHwsComH28nmSerdesPowerCtrl)
            {
                MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
            }
            break;
        case GOP_28NM_REV2:
            if(hwsSerdesFuncsPtr[COM_PHY_28NM].serdesManualRxCfgFunc != mvHwsComH28nmRev3SerdesManualRxConfig)
            {
                MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
            }
            break;

        default:
            hwsOsPrintf("mvHwsDiagSerdesInitDBStatusGet: Unsupported GOP revision\n");
            return GT_FAIL;
        }
        break;
#endif
    default:
        hwsOsPrintf("mvHwsDiagSerdesInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

#ifndef PX_FAMILY
    if((sType == Alleycat3A0) && (hwsSerdesFuncsPtr[COM_PHY_28NM].serdesManualRxCfgFunc != mvHwsComH28nmRev3SerdesManualRxConfig))
    {
        MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
    }
    hwsOsPrintf("Serdes Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);
    return GT_OK;
#endif

}

#if 0
/**
* @internal mvHwsDiagDdr3TipInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of DDR unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDdr3TipInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_TRAINING_IP_FUNC_PTRS *hwsDdr3TipFuncsPtr;

    /* need to fix warning here, the hwsTrainingIpFuncsPtr declared
       as static function in mvHwsDdr3TrainingIpIf.c */
    CHECK_STATUS(hwsTrainingIpFuncsPtr(devNum, &hwsDdr3TipFuncsPtr));

    switch (sType)
    {
    case BobcatA0:
        if(hwsDdr3TipFuncsPtr->trainingIpInitController != mvHwsDdr3TipInitController)
        {
            hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, portMode=%s is wrong\n", devNum, DEV_TYPE[sType], "Ddr3Tip");
            return GT_BAD_PARAM;
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagDdr3TipInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    hwsOsPrintf("Ddr3Tip Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}
#endif



