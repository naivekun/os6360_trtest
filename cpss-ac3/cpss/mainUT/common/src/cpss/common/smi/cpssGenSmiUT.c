/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssGenSmiUT.c
*
* @brief Unit tests for API for read/write register of device,
* which connected to SMI master controller of packet processor
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/common/smi/cpssGenSmi.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Max SMI addresses */
#define UT_MAX_SMI_ADDR_CNS 32

/* Generic SMI info structure */
typedef struct {
    CPSS_PHY_SMI_INTERFACE_ENT   smiInterface;
    GT_U32  maxSmiInterfces;
    GT_U32  regAddr;
    GT_U32  regAddrOffset;
    GT_U32  data;
} UT_SMI_INFO_STC;

static UT_SMI_INFO_STC  smiInfoArr[] = {
        /* smiInterface */           /* maxSmiInterfces */ /*regAddr*/     /* regAddrOffset */  /*data*/
        {CPSS_PHY_SMI_INTERFACE_0_E, 2,                    0x54000004,     0x1000000,           0xe14}, /* Aldrin */
        {CPSS_PHY_SMI_INTERFACE_0_E, 1,                    0x0B000004,     0x0,                 0xc24}, /* Pipe */
        {CPSS_PHY_SMI_INTERFACE_0_E, 2,                    0x29000004,     0x1000000,           0xe14}, /* Bobcat3 - Pipe 0 */
        {CPSS_PHY_SMI_INTERFACE_0_E, 2,                    0xA9000004,     0x1000000,           0xe14}  /* Bobcat3 - Pipe 1 */
    };

static GT_STATUS prvUtfDeviceSmiInfoGet
(
    IN  GT_U8           devNum,
    OUT UT_SMI_INFO_STC **devSmiInfoPtrPtr,
    OUT GT_U32          *pipesNumberPtr
)
{
    GT_STATUS rc;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    *pipesNumberPtr = 1;

    /* Get device family */
    rc = prvUtfDeviceFamilyGet(devNum, &devFamily);
    if (rc != GT_OK)
    {
        return rc;
    }   

    switch (devFamily) 
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            *devSmiInfoPtrPtr = &smiInfoArr[0];
            break;
        case CPSS_PX_FAMILY_PIPE_E:
            *devSmiInfoPtrPtr = &smiInfoArr[1];
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            *devSmiInfoPtrPtr = &smiInfoArr[2];
            *pipesNumberPtr = 2;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/*
GT_STATUS cpssSmiRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
);
*/

UTF_TEST_CASE_MAC(cpssSmiRegisterRead)
{
/*
ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL data and valid SMI address.
    Expected: GT_OK ;
    1.1.2. Check for out of range SMI interface. 
           Call function with out of range SMI interface and non-NULL dataPtr.
    Expected: NON GT_OK.
    1.1.3. Check for out of range SMI register. 
           Call function with SMI address [32] and non-NULL dataPtr.
    Expected: NON GT_OK.
    1.1.4. Check for NULL pointer support.
           Call function with valid SMI address and dataPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st      =  GT_OK;
    GT_U8                       dev     =  0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                      smiInterface;
    GT_U32                      pipesNumber;
    GT_U32                      pipe;
    GT_U32                      smiAddr;
    GT_U32                      regAddr;
    GT_U32                      data;
    UT_SMI_INFO_STC             *devSmiInfoPtr = NULL;

    /* Prepare device iterator. API is applicable for every device but
     * we'll check it only for Aldrin and Pipe */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E  |
                                     UTF_LION_E | UTF_LION2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | 
                                     UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceSmiInfoGet(dev, &devSmiInfoPtr, &pipesNumber);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "prvUtfDeviceSmiInfoGet: dev%d", dev);
        for (pipe = 0; pipe < pipesNumber; pipe++, devSmiInfoPtr++)
        {
            /* 1.1.1. Call function with non-NULL data and valid SMI address.
                Expected: GT_OK ; */ 
            for (smiAddr = 0; smiAddr < UT_MAX_SMI_ADDR_CNS; smiAddr++)
            {
                for (smiInterface = devSmiInfoPtr->smiInterface; 
                     smiInterface < devSmiInfoPtr->maxSmiInterfces; smiInterface++)
                {
                    regAddr = devSmiInfoPtr->regAddr + smiInterface * devSmiInfoPtr->regAddrOffset;
                    st = cpssSmiRegisterRead(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, &data);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssSmiRegisterRead: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                                 dev, smiInterface, smiAddr, regAddr);
                }
            }
            /* 1.1.2. Check for out of range SMI register. 
                   Call function with SMI address [32] and non-NULL dataPtr. */
            regAddr = devSmiInfoPtr->regAddr;
            smiAddr = 0;
            smiInterface = devSmiInfoPtr->smiInterface + devSmiInfoPtr->maxSmiInterfces * 2;

            st = cpssSmiRegisterRead(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, &data);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
            "cpssSmiRegisterRead: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                         dev, smiInterface, smiAddr, regAddr);

            /* 1.1.3. Check for out of range SMI register. 
                   Call function with SMI address [32] and non-NULL dataPtr. */
            smiAddr = UT_MAX_SMI_ADDR_CNS;
            smiInterface = devSmiInfoPtr->smiInterface;

            st = cpssSmiRegisterRead(dev, portGroupsBmp, devSmiInfoPtr->smiInterface, smiAddr, regAddr, &data);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssSmiRegisterRead: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                         dev, smiInterface, smiAddr, regAddr);
            /* 1.1.4. Check for NULL pointer support.
                   Call function with valid SMI address and dataPtr [NULL].
            Expected: GT_BAD_PTR. */

            smiAddr = 0;
            smiInterface = devSmiInfoPtr->smiInterface;

            st = cpssSmiRegisterRead(dev, portGroupsBmp, devSmiInfoPtr->smiInterface, smiAddr, regAddr, NULL);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PTR, st,
            "cpssSmiRegisterRead: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                         dev, smiInterface, smiAddr, regAddr);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator - iterate ONLY non-active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssSmiRegisterRead(dev, portGroupsBmp, 0, 0, 0, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssSmiRegisterRead(dev, portGroupsBmp, 0, 0, 0, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssSmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
); 
*/ 

UTF_TEST_CASE_MAC(cpssSmiRegisterWrite)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL data and valid SMI address.
    Expected: GT_OK ;
    1.1.2. Call cpssSmiRegisterRead function with non-NULL
           dataPtr and valid SMI address.
    Expected: GT_OK and *dataPtr == *dataPtrGet
    1.1.3. Check for out of range SMI register.
        Call function with out of range SMI address and valid data.
    Expected: NON GT_OK.
*/

    GT_STATUS                   st      =  GT_OK;
    GT_U8                       dev     =  0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                      pipesNumber;
    GT_U32                      pipe;
    GT_U32                      smiInterface;
    GT_U32                      smiAddr;
    GT_U32                      regAddr;
    GT_U32                      data;
    UT_SMI_INFO_STC             *devSmiInfoPtr = NULL;

    /* Prepare device iterator. API is applicable for every device but
     * we'll check it only for Aldrin and Pipe */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E  |
                                     UTF_LION_E | UTF_LION2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | 
                                     UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceSmiInfoGet(dev, &devSmiInfoPtr, &pipesNumber);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "prvUtfDeviceSmiInfoGet: dev%d", dev);

        for (pipe = 0; pipe < pipesNumber; pipe++, devSmiInfoPtr++)
        {
            /* 1.1.1. Call function with non-NULL data and valid SMI address.
                Expected: GT_OK ; */ 
            for (smiAddr = 0; smiAddr < UT_MAX_SMI_ADDR_CNS; smiAddr++)
            {
                for (smiInterface = devSmiInfoPtr->smiInterface; 
                     smiInterface < devSmiInfoPtr->maxSmiInterfces; smiInterface++)
                {
                    regAddr = devSmiInfoPtr->regAddr + smiInterface * devSmiInfoPtr->regAddrOffset;
                    st = cpssSmiRegisterWrite(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, devSmiInfoPtr->data);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssSmiRegisterWrite: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                                 dev, smiInterface, smiAddr, regAddr);

                    st = cpssSmiRegisterRead(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, &data);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssSmiRegisterRead: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                                 dev, smiInterface, smiAddr, regAddr);
                }
            }
            /* 1.1.2. Check for out of range SMI register. 
                   Call function with SMI address [32] and non-NULL dataPtr. */
            regAddr = devSmiInfoPtr->regAddr;
            smiAddr = 0;
            smiInterface = devSmiInfoPtr->smiInterface + devSmiInfoPtr->maxSmiInterfces * 2;

            st = cpssSmiRegisterWrite(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, devSmiInfoPtr->data);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
            "cpssSmiRegisterWrite: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                         dev, smiInterface, smiAddr, regAddr);

            /* 1.1.3. Check for out of range SMI register. 
                   Call function with SMI address [32] and non-NULL dataPtr. */
            smiAddr = UT_MAX_SMI_ADDR_CNS;
            smiInterface = devSmiInfoPtr->smiInterface;

            st = cpssSmiRegisterWrite(dev, portGroupsBmp, smiInterface, smiAddr, regAddr, devSmiInfoPtr->data);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssSmiRegisterWrite: dev%d, SMI interface%d, SMI addr%d, SMI reg addr%d", 
                                         dev, smiInterface, smiAddr, regAddr);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator - iterate ONLY non-active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssSmiRegisterWrite(dev, portGroupsBmp, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssSmiRegisterWrite(dev, portGroupsBmp, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssXsmiPortGroupRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
)
*/


UTF_TEST_CASE_MAC(cpssXsmiPortGroupRegisterWrite)
{
/* The API is relevant for all devices but the test is enables for
   Aldrin, Bobcat3 only.
   1.     Iterate over all active devices/portGroups
   1.1    Iterate other all XSMI interfaces (1..3 - bobcat3 only)
   1.1.1  Call the API with valid xsmiAddr,regAddr,phyDev. Expected GT_OK.
   1.1.2  Call cpssXsmiPortGroupRegisterRead. Expected GT_OK.
          Ignore read register value  (it will be always 0xFFFFFFFF
          if XSMI is not connected to phy).
   1.2    Call with invalid xsmiInterface. Expected GT_BAD_PARAM
   1.3    Call with invalid xsmiAddr. Expected GT_BAD_PARAM (sip5) or GT_OK(legacy)
   1.4    Call with invalid phyDev (out of 0..31). Expected GT_BAD_PARAM
   2.     Call for not active devices. Extected: GT_NOT_APPLICABLE_DEVICE
   3.     Call for device out of range. Extected: GT_BAD_PARAM
*/
    GT_STATUS          rc;
    GT_U8              dev;
    GT_U32             portGroupId;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    const GT_U32       xsmiAddr = 1; /* PHY address, any valid value */
    const GT_U32       phyDev   = 2; /* PHY device number, any valid value */
    const GT_U32       phyReg   = 3; /* PHY device register, any valid value */
    GT_U16             data = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInf;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator. API is applicable for every device but
     * we'll check it only for aldrin, bobcat3 and Pipe */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E  |
                                     UTF_LION_E | UTF_LION2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices/portgroups. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Get device family */
        rc = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = (GT_U32)1 << portGroupId;

            /* 1.1    Iterate other all XSMI interfaces (1..3 - bobcat3 only) */
            for (xsmiInf =  CPSS_PHY_XSMI_INTERFACE_0_E;
                 xsmiInf <= CPSS_PHY_XSMI_INTERFACE_3_E; xsmiInf++)
            {
#ifdef ASIC_SIMULATION
               /* XSMI 0..3 are placed in separate MG units 0..3 in sip5.20. WM supports only MG0 */
                if (xsmiInf != CPSS_PHY_XSMI_INTERFACE_0_E)
#else
                if ((xsmiInf != CPSS_PHY_XSMI_INTERFACE_0_E)
                    && ((!PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) || (devFamily == CPSS_PX_FAMILY_PIPE_E)))
#endif   /* ASIC_SIMULATION */
                {
                    continue;
                }

                /* 1.1.1  Call the API with valid xsmiAddr,regAddr,phyDev. Expected GT_OK. */
                data = 0xDADA;
                rc = cpssXsmiPortGroupRegisterWrite(dev, portGroupsBmp, xsmiInf,
                                                    xsmiAddr, phyReg, phyDev, data);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, portGroupId, xsmiInf);

                /* 1.1.2  Call cpssXsmiPortGroupRegisterRead. Expected GT_OK. */
                rc = cpssXsmiPortGroupRegisterRead(dev, portGroupsBmp, xsmiInf,
                                                   xsmiAddr, phyReg, phyDev, &data);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, portGroupId, xsmiInf);
            }
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId);

        /* call with invalid IN-parameters */

        /* init IN-parameters with valid data */
        xsmiInf = CPSS_PHY_XSMI_INTERFACE_0_E;
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* 1.2    Call with invalid xsmiInterface (out of 0..3). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterWrite(dev, portGroupsBmp, 7,
                                            xsmiAddr, phyReg, phyDev, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        /* 1.3    Call with invalid xsmiAddr (out of 0..31). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterWrite(dev, portGroupsBmp, xsmiInf,
                                            33, phyReg, phyDev, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            (PRV_CPSS_SIP_5_CHECK_MAC(dev) ? GT_BAD_PARAM : GT_OK), rc, dev);


        /* 1.4    Call with invalid phyDev (out of 0..31). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterWrite(dev, portGroupsBmp, xsmiInf,
                                            xsmiAddr, phyReg, 33, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /* 2.     Call for not active devices.      Extected: GT_NOT_APPLICABLE_DEVICE */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* init IN-parameters with valid data */
    xsmiInf = CPSS_PHY_XSMI_INTERFACE_0_E;
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssXsmiPortGroupRegisterWrite(dev,
                                            portGroupsBmp,
                                            CPSS_PHY_XSMI_INTERFACE_0_E,
                                            0, 0, 0, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

   /* 3.     Call for device out of range. Extected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssXsmiPortGroupRegisterWrite(dev,
                                        portGroupsBmp,
                                        CPSS_PHY_XSMI_INTERFACE_0_E,
                                        0, 0, 0, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssXsmiPortGroupRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    OUT GT_U16  *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssXsmiPortGroupRegisterRead)
{
/* The API is relevant for all devices but the test is enables for
   Aldrin, Bobcat3 only.
   1.     Iterate over all active devices/portGroups
   1.1    Iterate other all XSMI interfaces (1..3 - bobcat3 only)
   1.1.1  Call the API with valid xsmiAddr,regAddr,phyDev. Expected GT_OK.
   1.2    Call with invalid xsmiInterface. Expected GT_BAD_PARAM (sip5) or GT_OK(legacy)
   1.3    Call with invalid xsmiAddr. Expected GT_BAD_PARAM
   1.4    Call with invalid phyDev (out of 0..31). Expected GT_BAD_PARAM
   2.     Call for not active devices. Extected: GT_NOT_APPLICABLE_DEVICE
   3.     Call for device out of range. Extected: GT_BAD_PARAM
*/
    GT_STATUS          rc;
    GT_U8              dev;
    GT_U32             portGroupId;
    GT_PORT_GROUPS_BMP portGroupsBmp;
    const GT_U32       xsmiAddr = 1; /* PHY address, any valid value */
    const GT_U32       phyDev   = 2; /* PHY device number, any valid value */
    const GT_U32       phyReg   = 3; /* PHY device register, any valid value */
    GT_U16             data;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInf;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator. API is applicable for every device but
     * we'll check it only for aldrin, bobcat3 */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
                                     UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E  |
                                     UTF_LION_E | UTF_LION2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices/portgroups. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Get device family */
        rc = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            portGroupsBmp = (GT_U32)1 << portGroupId;

            /* 1.1    Iterate other all XSMI interfaces (1..3 - bobcat3 only) */
            for (xsmiInf =  CPSS_PHY_XSMI_INTERFACE_0_E;
                 xsmiInf <= CPSS_PHY_XSMI_INTERFACE_3_E; xsmiInf++)
            {
#ifdef ASIC_SIMULATION
               /* XSMI 0..3 are placed in separate MG units 0..3 in sip5.20. WM supports only MG0 */
                if (xsmiInf != CPSS_PHY_XSMI_INTERFACE_0_E)
#else
                if ((xsmiInf != CPSS_PHY_XSMI_INTERFACE_0_E)
                    && ((!PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) || (devFamily == CPSS_PX_FAMILY_PIPE_E)))
#endif   /* ASIC_SIMULATION */
                {
                    continue;
                }

                /* 1.1.1  Call the API with valid xsmiAddr,regAddr,phyDev. Expected GT_OK. */
                rc = cpssXsmiPortGroupRegisterRead(dev, portGroupsBmp, xsmiInf,
                                                   xsmiAddr, phyReg, phyDev, &data);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, portGroupId, xsmiInf);
            }
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId);


        /* call with invalid IN-parameters */

        /* init IN-parameters with valid data */
        xsmiInf = CPSS_PHY_XSMI_INTERFACE_0_E;
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* 1.2    Call with invalid xsmiInterface (out of 0..3). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterRead(dev, portGroupsBmp, 7,
                                           xsmiAddr, phyReg, phyDev, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        /* 1.3    Call with invalid xsmiAddr (out of 0..31). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterRead(dev, portGroupsBmp, xsmiInf,
                                            33, phyReg, phyDev, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            (PRV_CPSS_SIP_5_CHECK_MAC(dev) ? GT_BAD_PARAM : GT_OK), rc, dev);

        /* 1.4    Call with invalid phyDev (out of 0..31). Expected GT_BAD_PARAM */
        rc = cpssXsmiPortGroupRegisterRead(dev, portGroupsBmp, xsmiInf,
                                            xsmiAddr, phyReg, 33, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /* 2.     Call for not active devices.      Extected: GT_NOT_APPLICABLE_DEVICE */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* init IN-parameters with valid data */
    xsmiInf = CPSS_PHY_XSMI_INTERFACE_0_E;
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssXsmiPortGroupRegisterRead(dev,
                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           CPSS_PHY_XSMI_INTERFACE_0_E,
                                           0, 0, 0, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

   /* 3.     Call for device out of range. Extected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssXsmiPortGroupRegisterRead(dev,
                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                       CPSS_PHY_XSMI_INTERFACE_0_E,
                                       0, 0, 0, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssGenSmi suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenSmi)
    UTF_SUIT_DECLARE_TEST_MAC(cpssSmiRegisterRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssSmiRegisterWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssXsmiPortGroupRegisterWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssXsmiPortGroupRegisterRead)
UTF_SUIT_END_TESTS_MAC(cpssGenSmi)


