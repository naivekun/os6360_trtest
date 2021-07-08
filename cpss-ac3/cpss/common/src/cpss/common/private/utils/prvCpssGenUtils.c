/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssGenUtils.c
*
* @brief CPSS generic wrappers for serdes optimizer and port configuration
* black box
*
* @version   12
********************************************************************************
*/

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

GT_STATUS gtStatus;

#ifdef CHX_FAMILY
extern GT_VOID hwsAldrin2SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

extern GT_VOID hwsBobcat3SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

extern GT_VOID hwsFalconSerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);
#else
/* stubs for PX_FAMILY */
#define hwsAldrin2SerdesAddrCalc(a,b,cPtr) *(cPtr) = 0
#define hwsBobcat3SerdesAddrCalc(a,b,cPtr) *(cPtr) = 0
#define hwsFalconSerdesAddrCalc(dev,a,b,cPtr)  *(cPtr) = 0
#endif
/**
* @internal genRegisterSet function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to write to.
* @param[in] data                     - Data to be written to register.
* @param[in] mask                     - Mask for selecting the written bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    #ifdef EXMXPM_FAMILY
        return prvCpssHwPpPortGroupWriteRegBitMask(devNum,portGroup,address,
                                        (mask != 0) ? mask : 0xffffffff,data);
    #else
        return cpssDrvPpHwRegBitMaskWrite(devNum,portGroup,address,
                                        (mask != 0) ? mask : 0xffffffff,data);
    #endif
}

/**
* @internal genRegisterGet function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] data                     - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroup,
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
)
{
    CPSS_NULL_PTR_CHECK_MAC(data);

    #ifdef EXMXPM_FAMILY
            return prvCpssHwPpPortGroupReadRegBitMask(devNum,portGroup,address,
                                    (mask != 0) ? mask : 0xffffffff,data);
    #else
        return cpssDrvPpHwRegBitMaskRead(devNum,portGroup,address,
                                     (mask != 0) ? mask : 0xffffffff,data);
    #endif
}

/**
* @internal gtBreakOnFail function
* @endinternal
*
* @brief   Treat failure
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
GT_VOID gtBreakOnFail
(
        GT_VOID
)
{
}

/**
* @internal prvCpssGenPortGroupSerdesWriteRegBitMask function
* @endinternal
*
* @brief   Implement write access to SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] data                     -  to write
* @param[in] mask                     -  write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssGenPortGroupSerdesWriteRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 data,
    IN  GT_U32 mask
)
{
    GT_U32  realRegAddr;
    GT_U32  baseAddr;

    if(regType > 1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    baseAddr = (0 == regType) ? 0x13000000 : 0x13000800;

    realRegAddr = baseAddr + regAddr + 0x1000*serdesNum;

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
       (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        if(serdesNum > 11)
        {
            realRegAddr = 0x13000000 + regAddr + 0x1000*serdesNum + 0x40000;
        }
        else
        {
            realRegAddr = 0x13000000 + regAddr + 0x1000*serdesNum;
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        hwsAldrin2SerdesAddrCalc(serdesNum, regAddr, &realRegAddr);
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        hwsBobcat3SerdesAddrCalc(serdesNum, regAddr, &realRegAddr);
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        hwsFalconSerdesAddrCalc(devNum,serdesNum, regAddr, &realRegAddr);
    }

    return genRegisterSet(devNum,portGroup,realRegAddr,data,mask);
}
/* bobcat3 pipe1 address indication */
#define BOBCAT3_PIPE_1_INDICATION_CNS 0x80000000

/**
* @internal prvCpssGenPortGroupSerdesReadRegBitMask function
* @endinternal
*
* @brief   Implement read access from SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] mask                     - read mask
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssGenPortGroupSerdesReadRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32 mask
)
{
    GT_U32  realRegAddr;
    GT_U32  baseAddr;

    if(regType > 1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    baseAddr = (0 == regType) ? 0x13000000 : 0x13000800;

    realRegAddr = baseAddr + regAddr + 0x1000*serdesNum;


    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))/* use AVAGO SERDES */
    {
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            /* do nothing */
        }
        else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            hwsAldrin2SerdesAddrCalc(serdesNum, regAddr, &realRegAddr);
        }
        else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            hwsBobcat3SerdesAddrCalc(serdesNum, regAddr, &realRegAddr);
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {

            hwsFalconSerdesAddrCalc(devNum,serdesNum, regAddr, &realRegAddr);
        }
        else /* Bobk */
        {
            if(serdesNum > 11)
            {
                realRegAddr = 0x13000000 + regAddr + 0x1000*serdesNum + 0x40000;
            }
            else
            {
                realRegAddr = 0x13000000 + regAddr + 0x1000*serdesNum;
            }
        }
    }

    return genRegisterGet(devNum,portGroup,realRegAddr,dataPtr,mask);
}




