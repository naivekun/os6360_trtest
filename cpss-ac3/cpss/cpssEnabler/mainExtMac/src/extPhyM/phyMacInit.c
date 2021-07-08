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
* @file phyMacInit.c
*
* @brief The file includes:
* I. PHYMAC object init functions:
* - macDrv1540MInit
* - macPhyDrvDataBaseInit
*
* II. PHYMAC object member functions:
* - macPhySpeedSet;           (1)
* - macPhySpeedGet;           (2)
* - macPhyDuplexANSet;         (3)  - in 1540 MACSEC only full duplex used
* - macPhyDuplexANGet;         (4)  - in 1540 MACSEC only full duplex used
* - macPhyFlowCntlANSet;        (5)
* - macPhyFlowCntlANGet;        (6)
* - macPhySpeedANSet;          (7)
* - macPhySpeedANGet;          (8)
* - macPhyFlowControlEnableSet;     (9)
* - macPhyFlowControlEnableGet;     (10)
* - macPhyPeriodicFlowCntlSet;     (11)
* - macPhyPeriodicFlowCntlGet;     (12)
* - macPhyBackPressureSet;       (13)  - in 1540 MACSEC only full duplex used
* - macPhyBackPressureGet;       (14)  - in 1540 MACSEC only full duplex used
* - macPhyPortLinkGet;         (15)
* - macPhyDuplexSet;          (16)  - in 1540 MACSEC only full duplex used
* - macPhyDuplexGet;          (17)  - in 1540 MACSEC only full duplex used
* - macPhyPortEnableSet;        (18)
* - macPhyPortEnableGet;        (19)
* - macPhyExcessiveCollisionDropSet;  (20)
* - macPhyExcessiveCollisionDropGet;  (21)
* - macPhyPaddingEnableSet;       (22)
* - macPhyPaddingEnableGet;       (23)
* - macPhyPreambleLengthSet;      (24)  (??? no set field)
* - macPhyPreambleLengthGet;      (25)  (??? no set field)
* - macPhyCRCCheckSet;         (26)
* - macPhyCRCCheckGet;         (27)
* - macPhyMRUSet;            (28)
* - macPhyMRUGet;            (29)
*
* @version   5
********************************************************************************
*/
#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/generic/phy/private/prvCpssGenPhySmi.h>
#include <gtOs/gtGenTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <extPhyM/phyMacInit.h>
#include <extPhyM/phyMacMSec.h>
#include <extPhyM/phyMacRegs1540M.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>


#define PHYMAC_DBG_ERROR  osPrintf

int osPrintf(const char* format, ...);

/* creation of pointer to macPhy1540MObj object*/
CPSS_MACDRV_OBJ_STC * macPhy1540MObj = NULL;

CPSS_SMI_REGISTER_WRITE_FUNC  macPhy1540MSmiWriteRegister = NULL;
CPSS_SMI_REGISTER_READ_FUNC  macPhy1540MSmiReadRegister = NULL;

CPSS_SMI_START_TRANSACTION_FUNC  macPhy1540MSmiStartTransaction = NULL;
CPSS_SMI_FINISH_TRANSACTION_FUNC  macPhy1540MSmiFinishTransaction = NULL;



phy1540MDataBase *phy1540DataBasePtr=NULL;


/**************** phyMac static functions ********************/

/**
* @internal macPhySpeedSet function
* @endinternal
*
* @brief   This function sets speed to PHY SIDE MAC and SWITCH SIDE MAC of
*         1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    -  value to set
* @param[in] stage                    -  to run (PRE or POST)
* @param[in] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[in] switchSpeedSetPtr        - the value to set on switch MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhySpeedSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORT_SPEED_ENT   ssSpeed;
    CPSS_PORT_SPEED_ENT   psSpeed;

    /* speed can be defined as 10, 100 or 1000 for 1540 */
    if (speed > CPSS_PORT_SPEED_1000_E)
    {
        return GT_BAD_PARAM;
    }
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set speed value for SS and PS MACs */
        ssSpeed = speed;
        psSpeed = speed;

        /* set both MACs */
        rc = phyMacMSecSetSpeed(devNum,portNum,
                      ssSpeed,psSpeed, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedSet \n");
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedSetPtr = speed;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhySpeedGet function
* @endinternal
*
* @brief   This function returns the speed of PHY SIDE MAC from
*         1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      speed                    - speed value
* @param[in] stage                    -  to run (PRE or POST)
* @param[in] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchSpeedSetPtr        - not used
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhySpeedGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_SPEED_ENT     *speedPtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedGetPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORT_SPEED_ENT   ssSpeed;
    CPSS_PORT_SPEED_ENT   psSpeed;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetSpeed(devNum,portNum,
                      &ssSpeed,&psSpeed, PHYMAC_PHY_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedGet \n");
            return rc;
        }
        /* value in switch MAC does not matter */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedGetPtr = psSpeed;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyDuplexANSet function
* @endinternal
*
* @brief   This function sets duplex status (half or full) for Auto Negotiation
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      duplexState              - duplex value
* @param[in] stage                    -  to run (PRE or POST)
* @param[in] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchSpeedSetPtr        - not used
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyDuplexANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL              *  doPpMacConfigPtr,
    OUT GT_BOOL              *  switchDuplexANSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = state;
    GT_BOOL   psState = state;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetANDuplex(devNum,portNum,
                      ssState,psState, PHYMAC_PHY_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedGet \n");
            return rc;
        }
        /* value in switch MAC does not matter */
        *doPpMacConfigPtr = GT_TRUE;
        *switchDuplexANSetPtr = state;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}
/**
* @internal macPhyDuplexANGet function
* @endinternal
*
* @brief   This function returns Phy Side MAC duplex status (half or full) for Auto Negotiation
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      duplexState               - speed value
* @param[in] stage                    -  to run (PRE or POST)
* @param[in] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchSpeedSetPtr        - not used
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyDuplexANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchDuplexANGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;

    /***** get duplex mode from switch ***/
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* get duplex values from both MACs */
        rc = phyMacMSecGetANDuplex(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedGet rc - %d \n",rc);
            return rc;
        }
        /* no read the duplex state from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchDuplexANGetPtr = psState;

        /* set the return value as the phy side duplex value */
        *statePtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}
/**
* @internal macPhyFlowCntlANSet function
* @endinternal
*
* @brief   This function sets flow control status for Auto Negotiation
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - Flow control AN enable/disable
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
* @param[in] stage                    -  to run (PRE or POST)
* @param[in] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[in] targetState              - Flow control AN enable/disable  for switch MAC
* @param[in] targetPauseAdvertise     - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable for
*                                      switch MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyFlowCntlANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT GT_BOOL                 *targetPauseAdvertise
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = state;
    GT_BOOL   ssPauseAdvertise = pauseAdvertise;
    GT_BOOL   psState = state;
    GT_BOOL   psPauseAdvertise = pauseAdvertise;

    /*  set flow control for SS & PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set flow control values to both MACs */
        rc = phyMacMSecSetANFlowControl(devNum,portNum,
                      ssState,ssPauseAdvertise,
                      psState,psPauseAdvertise,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedGet \n");
            return rc;
        }
        /*  set flow control for Switch MACs */
        *targetState = state;
        *targetPauseAdvertise = pauseAdvertise;

        /* switch MAC should be configured */
        *doPpMacConfigPtr = GT_TRUE;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyFlowCntlANGet function
* @endinternal
*
* @brief   This function sets flow control status for Auto Negotiation
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
* @param[in] stage                    -  to run (PRE or POST)
*                                      targetState               - Flow control AN enable/disable  for switch MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyFlowCntlANGet
(

    IN   GT_U8                  devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr,
    OUT  GT_BOOL                *pauseAdvertisePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT  GT_BOOL                *targetStatePtr,
    OUT  GT_BOOL                *switchFlowCntlANGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   ssPauseAdvertisePtr;
    GT_BOOL   psState;
    GT_BOOL   psPauseAdvertisePtr;

    /*  get flow control from  PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* get flow control values from PS MAC */
        rc = phyMacMSecGetANFlowControl(devNum,portNum,
                      &ssState,&ssPauseAdvertisePtr,
                      &psState,&psPauseAdvertisePtr,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyFlowCntlANGet \n");
            return rc;
        }

        /* get switch MAC values*/
        *switchFlowCntlANGetPtr = GT_TRUE;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}
/**
* @internal macPhySpeedANSet function
* @endinternal
*
* @brief   This function enables/disables speed Auto Negotiation on SS and PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
* @param[in] stage                    -  to run (PRE or POST)
*                                      targetState               - Flow control AN enable/disable  for switch MAC
*
* @param[out] state                    - Flow control AN enable/disable
*                                      pauseAdvertise            - Advertise symmetric flow control support in
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      targetPauseAdvertise      - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable for
*                                      switch MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhySpeedANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
)
{
    GT_STATUS rc = GT_OK;

    GT_BOOL   ssState = state;
    GT_BOOL   psState = state;

    /*  set speed AN enable/disable for SS & PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* get flow control values from PS MAC */
        rc = phyMacMSecSetSpeedANEnable(devNum,portNum,
                      ssState,psState,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedANSet - rc %d \n",rc);
            return rc;
        }

        /* set speed AN enable/disable on switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedANSetPtr = state;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}
/**
* @internal macPhySpeedANGet function
* @endinternal
*
* @brief   This function returns status of Auto-Negotiation enable on specified port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*                                      state               - Flow control AN enable/disable  for switch MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhySpeedANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANGetPtr
)
{
    GT_STATUS rc = GT_OK;

    GT_BOOL   ssState;
    GT_BOOL   psState;

    /*  get speed AN enable/disable for SS & PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* get  speed AN values from PS & SS MAC */
        rc = phyMacMSecGetSpeedANEnable(devNum,portNum,
                      &ssState,&psState,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedANSet - rc %d \n",rc);
            return rc;
        }

        /* get speed AN value from  switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedANGetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal macPhyFlowControlEnableSet function
* @endinternal
*
* @brief   This function sets flow control mode on SS and PS MACs
*         The following Flow control medes are defined:
*         typedef enum
*         (
*         CPSS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,
*         CPSS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,
*         CPSS_PORT_FLOW_CONTROL_RX_ONLY_E,
*         CPSS_PORT_FLOW_CONTROL_TX_ONLY_E
*         )CPSS_PORT_FLOW_CONTROL_ENT;
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - flow control mode
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[out] switchFlowCntlSetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *switchFlowCntlSetPtr
)
{
    GT_STATUS rc = GT_OK;

    CPSS_PORT_FLOW_CONTROL_ENT  ssState = state;
    CPSS_PORT_FLOW_CONTROL_ENT  psState = state;

    /*  set flow control mode for SS & PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        /* set flow control mode for PS & SS MAC */
        rc = phyMacMSecSetFlowControlEnable(devNum,portNum,
                      ssState,psState,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedANSet - rc %d \n",rc);
            return rc;
        }

        /* set speed AN enable/disable on switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchFlowCntlSetPtr = state;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}


/**
* @internal macPhyFlowControlEnableGet function
* @endinternal
*
* @brief   This function gets flow control mode from SS and PS MACs
*         The following typedef defines the flow control medes:
*         typedef enum
*         (
*         CPSS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,
*         CPSS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,
*         CPSS_PORT_FLOW_CONTROL_RX_ONLY_E,
*         CPSS_PORT_FLOW_CONTROL_TX_ONLY_E
*         )CPSS_PORT_FLOW_CONTROL_ENT;
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      state                     - flow control mode
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] statePtr                 - ptr to required value
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[out] switchFlowCntlGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *switchFlowCntlGetPtr
)
{
    GT_STATUS rc = GT_OK;

    CPSS_PORT_FLOW_CONTROL_ENT  ssState;
    CPSS_PORT_FLOW_CONTROL_ENT  psState;

    /*  set flow control mode for SS & PS MACs */
    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        /* set flow control mode for PS & SS MAC */
        rc = phyMacMSecGetFlowControlEnable(devNum,portNum,
                      &ssState,&psState,
                      PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedANSet - rc %d \n",rc);
            return rc;
        }

        /* return Phy Side MAC value */
        *statePtr = psState;

        /* not need flow control mode from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchFlowCntlGetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyPeriodicFlowCntlSet function
* @endinternal
*
* @brief   This function gets flow control mode from SS and PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - flow control mode
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] state                    - ptr to required value
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchFlowCntlGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPeriodicFlowCntlSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT    state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPeriodicFlowCntlSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = (GT_BOOL) state;
    GT_BOOL   psState = (GT_BOOL) state;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        if ((state != CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E) &&
            (state != CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        rc = phyMacMSecSetPeriodicFlowControlEnable(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPeriodFlowCntlSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchPeriodicFlowCntlSetPtr = (GT_BOOL) state;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyPeriodicFlowCntlGet function
* @endinternal
*
* @brief   This function gets periodic flow control mode from SS and PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] enable                   - ptr to required value
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchFlowCntlGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPeriodicFlowCntlGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT  GT_BOOL                     *enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPeriodicFlowCntlGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetPeriodicFlowControlEnable(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPeriodFlowCntlSet rc - %d \n",rc);
            return rc;
        }
        /* not need to read  value from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchPeriodicFlowCntlGetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyBackPressureSet function
* @endinternal
*
* @brief   This function sets Back Pressure value (in half duplex only) in SS and PS MACs
*         This function do nothing because 1540 macsec works only in full duplex mode
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - required value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[out] switchBPSetPtr           - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyBackPressureSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = state;
    GT_BOOL   psState = state;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetBackPressureEnable(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPeriodFlowCntlSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchBPSetPtr = state;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyBackPressureGet function
* @endinternal
*
* @brief   This function gets Back Pressure value (in half duplex only) from SS and PS MACs
*         This function do nothing because 1540 macsec works only in full duplex mode
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyBackPressureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     *statePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetBackPressureEnable(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPeriodFlowCntlSet rc - %d \n",rc);
            return rc;
        }
        /* not need to read  value from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchBPGetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;

}

/**
* @internal macPhyPortLinkGet function
* @endinternal
*
* @brief   This function returns link status value PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      state                     - required value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchFlowCntlGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPortLinkGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isLinkUpPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchLinkStatusGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssIsLinkUp;
    GT_BOOL   psIsLinkUp;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetLinkStatus(devNum,portNum,
                      &ssIsLinkUp,&psIsLinkUp, PHYMAC_PHY_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhySpeedGet \n");
            return rc;
        }
        /* not need to read  value from switch MAC ??? */
        *doPpMacConfigPtr = GT_TRUE;
        *switchLinkStatusGetPtr = psIsLinkUp;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyDuplexSet function
* @endinternal
*
* @brief   This function sets duplex mode in SS and PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] dMode                    - required value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
* @param[out] switchDuplexSetPtr       - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyDuplexSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DUPLEX_ENT        dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORT_DUPLEX_ENT   ssState = dMode;
    CPSS_PORT_DUPLEX_ENT   psState = dMode;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetDuplexMode(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyDuplexSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchDuplexSetPtr = dMode;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/*******************************************************************************
* macPhyDuplexGet (17)
*
* DESCRIPTION:
*       This function returns duplex mode from PS MACs
*
* INPUTS:
*    devNum                    - device number
*    portNum                   - port number
*    stage                     - stage to run (PRE or POST)
*
* OUTPUTS:
*    dModePtr                  - ptr to required value
*    *doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*    *switchDuplexGetPtr     - target switch MAC value
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on wrong parameters and set failed
*       GT_BAD_PARAM             - wrong parameter
*
* COMMENTS:
*
*
*
*******************************************************************************/
/* (17) Get Port Duplex Mode */
static GT_STATUS macPhyDuplexGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_DUPLEX_ENT        *dModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexGetPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORT_DUPLEX_ENT   ssState;
    CPSS_PORT_DUPLEX_ENT   psState;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* get from both MACs */
        rc = phyMacMSecGetDuplexMode(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyDuplexGet rc - %d \n",rc);
            return rc;
        }
        /* no get from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchDuplexGetPtr = psState;

        /* set return value - Phy Side Mac Duplex value */
        *dModePtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyPortEnableSet function
* @endinternal
*
* @brief   This function sets port enable/disable for SS and PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - required value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC speed should be set
*                                      switchDuplexGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = enable;
    GT_BOOL   psState = enable;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetPortEnable(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPortEnableSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *targetEnableSetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}


/**
* @internal macPhyPortEnableGet function
* @endinternal
*
* @brief   This function returns port enable/disable value from PS MACs
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] statePtr                 - ptr to required value
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be get
*                                      switchDuplexGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPortEnableGet
(
    IN   GT_U8                  devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL               *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchPortEnableGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetPortEnable(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPortEnableSet rc - %d \n",rc);
            return rc;
        }
        /* no get from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchPortEnableGetPtr = psState;

        /* set return value */
        *statePtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyExcessiveCollisionDropSet function
* @endinternal
*
* @brief   This function sets Excessive Collision Drop enable/disable value
*         for PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - required enable/disable value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be get
*                                      switchDuplexGetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyExcessiveCollisionDropSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropSetPtr
)
{
    /* get value from switch MAC */
    *doPpMacConfigPtr = GT_TRUE;

    /* Nothing to do - works only in half duplex mode */
    return GT_OK;
}

/**
* @internal macPhyExcessiveCollisionDropGet function
* @endinternal
*
* @brief   This function returns Excessive Collision Drop enable/disable value
*         from PS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyExcessiveCollisionDropGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropGetPtr
)
{
    /* Nothing to do - works only in half duplex mode */
    return GT_OK;
}

/**
* @internal macPhyPaddingEnableSet function
* @endinternal
*
* @brief   This function sets Padding enable/disable mode
*         from PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - required enable/disable value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be get
* @param[out] switchPaddingSetPtr      - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPaddingEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchPaddingSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = enable;
    GT_BOOL   psState = enable;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetPaddingEnable(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPaddingEnableSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchPaddingSetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyPaddingEnableGet function
* @endinternal
*
* @brief   This function returns Padding enable/disable value
*         from PS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPaddingEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchPaddingGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetPaddingEnable(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyPortEnableSet rc - %d \n",rc);
            return rc;
        }
        /* no get from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchPaddingGetPtr = psState;

        /* set return value */
        *statePtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyPreambleLengthSet function
* @endinternal
*
* @brief   This function sets Preamble Length
*         for PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - port direction
* @param[in] length                   - lenght
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be set
* @param[out] targetDirection          - target direction
* @param[out] switchPreambleLengthPtr  - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirection,
    OUT GT_U32                  *switchPreambleLengthPtr
)
{
    /* get value from switch MAC */
    *doPpMacConfigPtr = GT_TRUE;

    /* ??? No such field in 1540 MAC */
    return GT_OK;
}

/**
* @internal macPhyPreambleLengthGet function
* @endinternal
*
* @brief   This function sets Preamble Length
*         for PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - port direction
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyPreambleLengthGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DIRECTION_ENT     direction,
    OUT GT_U32                      *lengthPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT     *targetDirection,
    OUT GT_U32                      *switchPreambleLengthPtr
)
{
    /* get value from switch MAC */
    *doPpMacConfigPtr = GT_TRUE;

    /* ??? No such field in 1540 MAC */
    return GT_OK;
}

/**
* @internal macPhyCRCCheckSet function
* @endinternal
*
* @brief   This function enable/disable CRC check mode
*         for PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - required enable/disable value
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be set
* @param[out] switchCRCCheckSetPtr     - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyCRCCheckSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState = enable;
    GT_BOOL   psState = enable;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetCRCCheckEnable(devNum,portNum,
                      ssState,psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyCRCCheckSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchCRCCheckSetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyCRCCheckGet function
* @endinternal
*
* @brief   This function returns enable/disable CRC check mode
*         status for PS MAC
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyCRCCheckGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   ssState;
    GT_BOOL   psState;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetCRCCheckEnable(devNum,portNum,
                      &ssState,&psState, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyCRCCheckSet rc - %d \n",rc);
            return rc;
        }
        /* don't get value from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchCRCCheckGetPtr = psState;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyMRUSet function
* @endinternal
*
* @brief   This function sets MRU for
*         status for PS and SS MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mruSize                  - MRU size
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - indicates if the switch MAC value should be get
*                                      switchCRCCheckSetPtr       - target switch MAC value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyMRUSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  mruSize,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUSetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   ssMruSize = mruSize;
    GT_U32   psMruSize = mruSize;


    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecSetMRU(devNum,portNum,
                      ssMruSize,psMruSize, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyMRUSet rc - %d \n",rc);
            return rc;
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchMRUSetPtr = mruSize;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyMRUGet function
* @endinternal
*
* @brief   This function returns MRU value
*         for PS MAC
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS macPhyMRUGet
(
    IN   GT_U8                  devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32                 *mruSizePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUGetPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32   ssMruSize;
    GT_U32   psMruSize;

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        /* set both MACs */
        rc = phyMacMSecGetMRU(devNum,portNum,
                      &ssMruSize,&psMruSize, PHYMAC_BOTH_SIDE_SET_E);
        if (rc != GT_OK) {
            PHYMAC_DBG_ERROR("  >>> MACPHY ERROR - macPhyCRCCheckSet rc - %d \n",rc);
            return rc;
        }
        /* don't get the value from switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchMRUGetPtr = psMruSize;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal macPhyDrvDataBaseInit function
* @endinternal
*
* @brief   - copy of the phy data base from application to phy driver
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS macPhyDrvDataBaseInit
(
    phy1540MDataBase * applPhyMacDataBasePtr
)
{
    GT_U32 cnt;
    GT_U32 dbSize = applPhyMacDataBasePtr->phyInfoDBSize;

    /* memory allocation for phy object */
    phy1540DataBasePtr = osMalloc(sizeof(phy1540MDataBase));
    if (phy1540DataBasePtr == NULL) {
        return GT_FAIL;
    }

    phy1540DataBasePtr->phyInfoPtr = osMalloc(dbSize*sizeof(phy1540MInfo));

    /* NULL set for allocated memory */
    osMemSet(phy1540DataBasePtr->phyInfoPtr,0,
                 sizeof(dbSize*sizeof(phy1540MInfo)));

    /* set data base size */
    phy1540DataBasePtr->phyInfoDBSize = applPhyMacDataBasePtr->phyInfoDBSize;

    /* copy data base by items */
    for (cnt = 0; cnt < dbSize; cnt++) {
        phy1540DataBasePtr->phyInfoPtr[cnt] = applPhyMacDataBasePtr->phyInfoPtr[cnt];
    }
    return GT_OK;
}

/*(1) */static GT_STATUS internal_macPhySpeedSet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhySpeedSet(devNum,portNum,speed,stage,doPpMacConfigPtr,switchSpeedSetPtr);
}

/*(2) */static GT_STATUS internal_macPhySpeedGet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     *speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhySpeedGet(devNum,portNum,speed,stage,doPpMacConfigPtr,switchSpeedSetPtr);
}

/*(3) */static GT_STATUS internal_macPhyDuplexANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL              *  doPpMacConfigPtr,
    OUT GT_BOOL              *  switchDuplexANSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyDuplexANSet(devNum,portNum,state,stage,doPpMacConfigPtr,switchDuplexANSetPtr);
}
/*(4) */static GT_STATUS internal_macPhyDuplexANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL              *  doPpMacConfigPtr,
    OUT GT_BOOL              *  switchDuplexANSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyDuplexANGet(devNum,portNum,state,stage,doPpMacConfigPtr,switchDuplexANSetPtr);
}

/*(5) */static GT_STATUS internal_macPhyFlowCntlANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT GT_BOOL                 *targetPauseAdvertise
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyFlowCntlANSet(devNum,portNum,state,pauseAdvertise,stage,doPpMacConfigPtr,targetState,targetPauseAdvertise);
}
/*(6) */static GT_STATUS internal_macPhyFlowCntlANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *state,
    IN  GT_BOOL                 *pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT GT_BOOL                 *targetPauseAdvertise
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyFlowCntlANGet(devNum,portNum,state,pauseAdvertise,stage,doPpMacConfigPtr,targetState,targetPauseAdvertise);
}

/*(7) */static GT_STATUS internal_macPhySpeedANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhySpeedANSet(devNum,portNum,state,stage,doPpMacConfigPtr,switchSpeedANSetPtr);
}

/*(8) */static GT_STATUS internal_macPhySpeedANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhySpeedANGet(devNum,portNum,state,stage,doPpMacConfigPtr,switchSpeedANSetPtr);
}

/*(9) */static GT_STATUS internal_macPhyFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *switchFlowCntlSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyFlowControlEnableSet(devNum,portNum,state,stage,doPpMacConfigPtr,switchFlowCntlSetPtr);
}

/*(10)*/static GT_STATUS internal_macPhyFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *state,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *switchFlowCntlGetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyFlowControlEnableGet(devNum,portNum,state,stage,doPpMacConfigPtr,switchFlowCntlGetPtr);
}

/*(11)*/static GT_STATUS internal_macPhyPeriodicFlowCntlSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT    state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPeriodicFlowCntlSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPeriodicFlowCntlSet(devNum,portNum,state,stage,doPpMacConfigPtr,switchPeriodicFlowCntlSetPtr);
}

/*(12)*/static GT_STATUS internal_macPhyPeriodicFlowCntlGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     *state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPeriodicFlowCntlSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPeriodicFlowCntlGet(devNum,portNum,state,stage,doPpMacConfigPtr,switchPeriodicFlowCntlSetPtr);
}

/*(13)*/static GT_STATUS internal_macPhyBackPressureSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyBackPressureSet(devNum,portNum,state,stage,doPpMacConfigPtr,switchBPSetPtr);
}

/*(14)*/static GT_STATUS internal_macPhyBackPressureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     *state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyBackPressureGet(devNum,portNum,state,stage,doPpMacConfigPtr,switchBPSetPtr);
}

/*(15)*/static GT_STATUS internal_macPhyPortLinkGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isLinkUpPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchLinkStatusGetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPortLinkGet(devNum,portNum,isLinkUpPtr,stage,doPpMacConfigPtr,switchLinkStatusGetPtr);
}

/*(16)*/static GT_STATUS internal_macPhyDuplexSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DUPLEX_ENT        dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyDuplexSet(devNum,portNum,dMode,stage,doPpMacConfigPtr,switchDuplexSetPtr);
}

/*(17)*/static GT_STATUS internal_macPhyDuplexGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DUPLEX_ENT        *dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyDuplexGet(devNum,portNum,dMode,stage,doPpMacConfigPtr,switchDuplexSetPtr);
}

/*(18)*/static GT_STATUS internal_macPhyPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPortEnableSet(devNum,portNum,enable,stage,doPpMacConfigPtr,targetEnableSetPtr);
}
/*(19)*/static GT_STATUS internal_macPhyPortEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     *enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPortEnableGet(devNum,portNum,enable,stage,doPpMacConfigPtr,targetEnableSetPtr);
}

/*(20)*/static GT_STATUS internal_macPhyExcessiveCollisionDropSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyExcessiveCollisionDropSet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchExcColDropSetPtr);
}

/*(21)*/static GT_STATUS internal_macPhyExcessiveCollisionDropGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyExcessiveCollisionDropGet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchExcColDropSetPtr);
}

/*(22)*/static GT_STATUS internal_macPhyPaddingEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchPaddingSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPaddingEnableSet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchPaddingSetPtr);
}

/*(23)*/static GT_STATUS internal_macPhyPaddingEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchPaddingSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPaddingEnableGet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchPaddingSetPtr);
}

/*(24)*/static GT_STATUS internal_macPhyPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirection,
    OUT GT_U32                  *switchPreambleLengthPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPreambleLengthSet(devNum,portNum,direction,length,stage,doPpMacConfigPtr,targetDirection,switchPreambleLengthPtr);
}
/*(25)*/static GT_STATUS internal_macPhyPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   *length,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirection,
    OUT GT_U32                  *switchPreambleLengthPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyPreambleLengthGet(devNum,portNum,direction,length,stage,doPpMacConfigPtr,targetDirection,switchPreambleLengthPtr);
}

/*(26)*/static GT_STATUS internal_macPhyCRCCheckSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyCRCCheckSet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchCRCCheckSetPtr);
}
/*(27)*/static GT_STATUS internal_macPhyCRCCheckGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 *enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyCRCCheckGet(devNum,portNum,enable,stage,doPpMacConfigPtr,switchCRCCheckSetPtr);
}

/*(27)*/static GT_STATUS internal_macPhyMRUSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  mruSize,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyMRUSet(devNum,portNum,mruSize,stage,doPpMacConfigPtr,switchMRUSetPtr);
}

/*(27)*/static GT_STATUS internal_macPhyMRUGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  *mruSize,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUSetPtr
)
{
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portNum);
    return  macPhyMRUGet(devNum,portNum,mruSize,stage,doPpMacConfigPtr,switchMRUSetPtr);
}

/**
* @internal macDrv1540MInit function
* @endinternal
*
* @brief   - Creation and initialization of macDrv1540M object
*         - Binds SMI read/write register functions
* @param[in] smiWriteRegister         - ptr to write register function
* @param[in] smiReadRegister          - ptr to read register function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS macDrv1540MInit
(
    IN  CPSS_SMI_REGISTER_WRITE_FUNC  smiWriteRegister,
    IN  CPSS_SMI_REGISTER_READ_FUNC   smiReadRegister,
    IN  CPSS_SMI_START_TRANSACTION_FUNC  smiStartTransaction,
    IN  CPSS_SMI_FINISH_TRANSACTION_FUNC  smiFinishTransaction,
    IN  phy1540MDataBase * phyMacDataBasePtr,
    OUT CPSS_MACDRV_OBJ_STC ** macDrvObjPtr
)
{
    GT_STATUS rc;

    /* memory allocation for phy object */
    macPhy1540MObj = osMalloc(sizeof(CPSS_MACDRV_OBJ_STC));

    *macDrvObjPtr = macPhy1540MObj;

    /* NULL set for allocated memory */
    osMemSet(macPhy1540MObj,0,sizeof(CPSS_MACDRV_OBJ_STC));

    /* object function init */

    macPhy1540MObj->macDrvMacSpeedSetFunc = internal_macPhySpeedSet;       /* 1 */
    macPhy1540MObj->macDrvMacSpeedGetFunc = internal_macPhySpeedGet;       /* 2 */
    macPhy1540MObj->macDrvMacDuplexANSetFunc = internal_macPhyDuplexANSet;    /* 3 */
    macPhy1540MObj->macDrvMacDuplexANGetFunc = internal_macPhyDuplexANGet;    /* 4 */
    macPhy1540MObj->macDrvMacFlowCntlANSetFunc = internal_macPhyFlowCntlANSet;  /* 5 */
    macPhy1540MObj->macDrvMacFlowCntlANGetFunc = internal_macPhyFlowCntlANGet;  /* 6 */
    macPhy1540MObj->macDrvMacSpeedANSetFunc = internal_macPhySpeedANSet;     /* 7 */
    macPhy1540MObj->macDrvMacSpeedANGetFunc = internal_macPhySpeedANGet;     /* 8 */
    macPhy1540MObj->macDrvMacFlowCntlSetFunc = internal_macPhyFlowControlEnableSet; /* 9 */
    macPhy1540MObj->macDrvMacFlowCntlGetFunc = internal_macPhyFlowControlEnableGet; /* 10 */
    macPhy1540MObj->macDrvMacPeriodFlowCntlSetFunc = internal_macPhyPeriodicFlowCntlSet;    /* 11 */
    macPhy1540MObj->macDrvMacPeriodFlowCntlGetFunc = internal_macPhyPeriodicFlowCntlGet;    /* 12 */
    macPhy1540MObj->macDrvMacBackPrSetFunc = internal_macPhyBackPressureSet;     /* 13 */
    macPhy1540MObj->macDrvMacBackPrGetFunc = internal_macPhyBackPressureGet;     /* 14 */
    macPhy1540MObj->macDrvMacPortlinkGetFunc = internal_macPhyPortLinkGet;     /* 15 */
    macPhy1540MObj->macDrvMacDuplexSetFunc = internal_macPhyDuplexSet;     /* 16 */
    macPhy1540MObj->macDrvMacDuplexGetFunc = internal_macPhyDuplexGet;     /* 17 */
    macPhy1540MObj->macDrvMacPortEnableSetFunc = internal_macPhyPortEnableSet;     /* 18 */
    macPhy1540MObj->macDrvMacPortEnableGetFunc = internal_macPhyPortEnableGet;     /* 19 */
    macPhy1540MObj->macDrvMacExcessiveCollisionDropSetFunc = internal_macPhyExcessiveCollisionDropSet;  /* 20 */
    macPhy1540MObj->macDrvMacExcessiveCollisionDropGetFunc = internal_macPhyExcessiveCollisionDropGet;  /* 21 */
    macPhy1540MObj->macDrvMacPaddingEnableSetFunc = internal_macPhyPaddingEnableSet;     /* 22 */
    macPhy1540MObj->macDrvMacPaddingEnableGetFunc = internal_macPhyPaddingEnableGet;     /* 23 */
    macPhy1540MObj->macDrvMacPreambleLengthSetFunc = internal_macPhyPreambleLengthSet;     /* 24 */
    macPhy1540MObj->macDrvMacPreambleLengthGetFunc = internal_macPhyPreambleLengthGet;     /* 25 */
    macPhy1540MObj->macDrvMacCRCCheckSetFunc = internal_macPhyCRCCheckSet;        /* 26 */
    macPhy1540MObj->macDrvMacCRCCheckGetFunc = internal_macPhyCRCCheckGet;        /* 27 */
    macPhy1540MObj->macDrvMacMRUSetFunc = internal_macPhyMRUSet;        /* 28 */
    macPhy1540MObj->macDrvMacMRUGetFunc = internal_macPhyMRUGet;        /* 29 */

    /*********** SMI read/write init **********/

    macPhy1540MSmiWriteRegister =  smiWriteRegister;
    macPhy1540MSmiReadRegister  =  smiReadRegister;

    /*********** SMI start/finish transaction **********/

    macPhy1540MSmiStartTransaction  = smiStartTransaction;
    macPhy1540MSmiFinishTransaction = smiFinishTransaction;

    /*********** PHY INFO Data Base init **********/
    rc = macPhyDrvDataBaseInit(phyMacDataBasePtr);
    if (rc != GT_OK) {
        return rc;
    }

    return GT_OK;
}



