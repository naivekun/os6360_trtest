/*******************************************************************************
*              (c), Copyright 2011, Marvell International Ltd.                 *
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
* @file wraplCpssDxChDeviceInfo.h
*
* @brief device info wrapper
*
* @version   5
********************************************************************************
*/

#ifndef __wraplCpssDxChDeviceInfo__
#define __wraplCpssDxChDeviceInfo__

#include <cpssCommon/wraplCpssExtras.h>
#include <cpssCommon/wrapCpssDebugInfo.h>


/***** macroses ********/

/* check physical port or CPU port -- use the generic macro */
#define WRL_PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC \
        WRL_PRV_CPSS_PHY_PORT_OR_CPU_PORT_CHECK_MAC
        
/* macro to get a pointer on the DxCh device
    devNum - the device id of the DxCh device               */        
#define WRL_PRV_CPSS_DXCH_PP_MAC PRV_CPSS_DXCH_PP_MAC 
       
/***** macroses ********/        
        

/**
* @internal prvCpssDxChIsValidInterface function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Checks that CPSS_INTERFACE_INFO_STC interface is valid.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - checked interface
*                                      P_CALLING_FORMAL_DATA - general debug information (environment variables
*                                      etc); could be empty
*
* @param[out] isValidInterfacePtr      - interface validness
* @param[out] errorMessagePtrPtr       - error message
*                                       GT_TRUE, if exists, otherwise GT_FALSE
*/
GT_BOOL prvCpssDxChIsValidInterface
(
    IN  GT_U8                   devNum,
    IN  CPSS_INTERFACE_INFO_STC *interfacePtr,
    OUT GT_BOOL                 *isValidInterfacePtr,
    OUT GT_CHAR_PTR             *errorMessagePtrPtr      
    P_CALLING_FORMAL_DATA    
);

/**
* @internal wrlIsVplsModeEnabled function
* @endinternal
*
* @brief   Check VPLS mode enabled
*
* @note   APPLICABLE DEVICES:      all devices
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in] L                        - lua state
*                                       1;
*/

int wrlIsVplsModeEnabled
(
    IN lua_State *L
);

/**
* @internal wrlIsTmEnabled function
* @endinternal
*
* @brief   Check Traffic Manager is enabled
*
* @note   APPLICABLE DEVICES:      all devices
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in] L                        - lua state
*                                       1;
*/
int wrlIsTmEnabled
(
    IN lua_State *L
);

#endif /* __wraplCpssDxChDeviceInfo__ */


