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
* @file phyMacInit.h
*
* @brief
*
* @version   3
********************************************************************************
*/
#include <gtOs/gtGenTypes.h>
#include <cpss/generic/phy/cpssGenPhySmi.h> 
#include <cpss/common/cpssTypes.h> 
#include <cpss/generic/extMac/cpssExtMacDrv.h> 

/* PHY INFO data base unit per port*/
typedef struct {
        GT_U8   devNum;
        GT_PHYSICAL_PORT_NUM  portNum;
        CPSS_PHY_SMI_INTERFACE_ENT smiBus;
        GT_U32 smiAddr;
        GT_U32 phySlice;
} phy1540MInfo;


typedef struct {
    phy1540MInfo * phyInfoPtr;
    GT_U32 phyInfoDBSize;
} phy1540MDataBase;
            

/* smi write */
typedef GT_STATUS (* CPSS_SMI_REGISTER_WRITE_FUNC)
(
 IN  GT_U8   devNum,
 IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
 IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
 IN  GT_U32  smiAddr, 
 IN  GT_U32  regAddr,
 OUT GT_U16  data
);


/* smi read */
typedef GT_STATUS (* CPSS_SMI_REGISTER_READ_FUNC)
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr, 
    IN  GT_U32  regAddr,
    OUT GT_U16  *data
);

/* start smi transaction */
typedef GT_STATUS (* CPSS_SMI_START_TRANSACTION_FUNC)
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL *pollStatus
);

/* finish smi transaction */
typedef GT_STATUS (* CPSS_SMI_FINISH_TRANSACTION_FUNC)
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  pollStatus
);


/**
* @internal macDrv1540MInit function
* @endinternal
*
* @brief   - creates PHYMAC 1540 object
*         - init of phy1540Info database
*         - bind of SMI read/write functions
*         - bind of the object unit functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAMETER         - if phy data base is not correct
*/
GT_STATUS macDrv1540MInit
(
    IN  CPSS_SMI_REGISTER_WRITE_FUNC  smiWriteRegister,
    IN  CPSS_SMI_REGISTER_READ_FUNC   smiReadRegister,
    IN  CPSS_SMI_START_TRANSACTION_FUNC  smiStartTransaction,
    IN  CPSS_SMI_FINISH_TRANSACTION_FUNC  smiFinishTransaction,
    IN  phy1540MDataBase * phyMacDataBasePtr,
    OUT CPSS_MACDRV_OBJ_STC ** macDrvObjPtr
);





