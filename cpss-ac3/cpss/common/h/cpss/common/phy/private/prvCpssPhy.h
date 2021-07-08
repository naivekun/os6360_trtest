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
* @file prvCpssPhy.h
*
* @brief Definitions and enumerators for port Core Serial Management
* Interface facility.
*
* @version   18
********************************************************************************
*/
#ifndef __prvCpssPhyh
#define __prvCpssPhyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

/* use in accessing Twist-D-XG's SMI control Reg */
#define PRV_CPSS_PHY_XENPAK_DEV_ADDRESS_CNS   0x05 /* DTE device address */
#define PRV_CPSS_PHY_XENPAK_PORT_ADDRESS_CNS  0x01 /* DTE port address   */

/* default values to indicate no SMI configuration */
#define PRV_CPSS_PHY_SMI_NO_PORT_ADDR_CNS   0xFFFF
#define PRV_CPSS_PHY_SMI_NO_CTRL_ADDR_CNS   0xFFFFFFFF
#define PRV_CPSS_PHY_INVALID_SMI_INSTANCE_CNS  0xFFFF

/* macro to get the port's PHY SMI address -- not relevant to XG ports */
#define PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.smiPortAddr

/* macro to get the port's PHY SMI port group -- not relevant to XG ports */
#define PRV_CPSS_PHY_SMI_GROUP_PORT_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.portGroupHostingSmiInterface

#define PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.smiInterface


/* macro to get the port's PHY XSMI interface */
#define PRV_CPSS_PHY_XSMI_PORT_INTERFACE_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].xsmiInterface

/**
* @struct PRV_CPSS_PHY_PORT_SMI_IF_INFO_STC
 *
 * @brief Holds information about the Smi interface to be used for a given
 * port.
*/
typedef struct{

    /** relation of port to SMI */
    GT_U16 smiInterface;

    /** SMI port address. */
    GT_U16 smiPortAddr;


    /** the port group that hosts the SMI to access the port (PHY) */
    GT_U32 portGroupHostingSmiInterface;

} PRV_CPSS_PHY_PORT_SMI_IF_INFO_STC;

/*******************************************************************************
* PRV_CPSS_VCT_PHY_READ_PHY_REGISTER_FUN
*
* DESCRIPTION:
*       Read specified SMI Register on a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev    - physical device number
*       port   - physical port number
*       phyReg - SMI register
*
* OUTPUTS:
*       data  - data read.
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_VCT_PHY_READ_PHY_REGISTER_FUN)
(
     IN  GT_U8     dev,
     IN  GT_PHYSICAL_PORT_NUM  port,
     IN  GT_U8     phyReg,
     OUT GT_U16    *data
);
/*******************************************************************************
* PRV_CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN
*
* DESCRIPTION:
*       Write value to specified SMI Register on a specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev    - physical device number
*       port   - physical port number
*       phyReg - SMI register
*       data   - value to write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN)
(
     IN  GT_U8     dev,
     IN  GT_PHYSICAL_PORT_NUM  port,
     IN  GT_U8     phyReg,
     IN  GT_U16    data
);

/*******************************************************************************
* PRV_CPSS_VCT_PHY_AUTONEG_SET_FUN
*
* DESCRIPTION:
*       This function sets the auto negotiation process between the PP and
*       Phy status.
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       port            - port number
*       enable          - enable/disable Auto Negotiation status
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number
*
* COMMENTS:
*

*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_VCT_PHY_AUTONEG_SET_FUN)
(
     IN  GT_U8     devNum,
     IN  GT_PHYSICAL_PORT_NUM   port,
     IN  GT_BOOL   enable
);

/*******************************************************************************
* PRV_CPSS_VCT_PHY_AUTONEG_GET_FUN
*
* DESCRIPTION:
*       This function sets the auto negotiation process between the PP and
*       Phy status.
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       port            - port number
*       enablePtr       - read Auto Negotiation status
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number
*
* COMMENTS:
*

*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_VCT_PHY_AUTONEG_GET_FUN)
(
     IN  GT_U8     devNum,
     IN  GT_PHYSICAL_PORT_NUM     port,
     OUT GT_BOOL   *enablePtr
);

/**
* @struct PRV_CPSS_VCT_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common VCT functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** pointer to read phy register function; */
    PRV_CPSS_VCT_PHY_READ_PHY_REGISTER_FUN cpssPhyRegisterRead;

    /** pointer to write phy register function; */
    PRV_CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN cpssPhyRegisterWrite;

    /** pointer to set phy auto */
    PRV_CPSS_VCT_PHY_AUTONEG_SET_FUN cpssPhyAutoNegotiationSet;

    /** pointer to get phy auto */
    PRV_CPSS_VCT_PHY_AUTONEG_GET_FUN cpssPhyAutoNegotiationGet;

} PRV_CPSS_VCT_GEN_BIND_FUNC_STC;


/*******************************************************************************
* PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN
*
* DESCRIPTION:
*       Get addresse of control SMI register.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Puma2; Puma3; ExMx; Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The PP to read from.
*       xsmiInterface - XSMI instance
*
* OUTPUTS:
*       ctrlRegAddrPtr - (ptr to) address of SMI control register
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad devNum
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN)
(
    IN GT_U8    devNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT smiInterface,
    OUT GT_U32  *ctrlRegAddrPtr
);
/**
* @struct PRV_CPSS_SMI_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common SMI functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** get SMI management register address */
    PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN cpssPhySmiRegsAddrGet;
} PRV_CPSS_SMI_GEN_BIND_FUNC_STC;


/*******************************************************************************
* PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN
*
* DESCRIPTION:
*       Get addresses of control and address XSMI registers.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The PP to read from.
*       xsmiInterface - XSMI instance
*
* OUTPUTS:
*       ctrlRegAddrPtr - (ptr to) address of XSMI control register
*       addrRegAddrPtr - (ptr to) address of XSMI address register
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad devNum
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN)
(
    IN GT_U8    devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    OUT GT_U32  *ctrlRegAddrPtr,
    OUT GT_U32  *addrRegAddrPtr
);

/**
* @struct PRV_CPSS_XSMI_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common XSMI functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** get addresses of control and address XSMI registers; */
    PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN cpssPhyXSmiRegsAddrGet;

} PRV_CPSS_XSMI_GEN_BIND_FUNC_STC;

/**
* @struct PRV_CPSS_PHY_INFO_STC
 *
 * @brief Holds info regarding virtual functions
 * port.
*/
typedef struct{

    /** structure of pointers to VCT service routines */
    PRV_CPSS_VCT_GEN_BIND_FUNC_STC genVctBindFunc;

    /** structure of pointers to SMI service routines */
    PRV_CPSS_SMI_GEN_BIND_FUNC_STC genSmiBindFunc;

    /** structure of pointers to XSMI service routines */
    PRV_CPSS_XSMI_GEN_BIND_FUNC_STC genXSmiBindFunc;

} PRV_CPSS_PHY_INFO_STC;

/**
* @internal prvCpssVctDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the VCT.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssVctDbRelease
(
    void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPhyh */


