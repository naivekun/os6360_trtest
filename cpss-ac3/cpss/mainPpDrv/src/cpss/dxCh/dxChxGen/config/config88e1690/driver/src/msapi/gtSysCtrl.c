#include <Copyright.h>

/**
********************************************************************************
* @file gtSysCtrl.c
*
* @brief API definitions for system global control.
*
* @version   5
********************************************************************************
*/
/********************************************************************************
* gtSysCtrl.c
*
* DESCRIPTION:
*       API definitions for system global control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <gtSysCtrl.h>
#include <gtDrvSwRegs.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <gtUtils.h>
#include <msApiInternal.h>

static GT_STATUS writeSwitchMacReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_ETHERADDR    *mac
);

static GT_STATUS readSwitchMacReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_ETHERADDR    *mac
);

/**
* @internal prvCpssDrvGsysSetDuplexPauseMac function
* @endinternal
*
* @brief   This routine sets the full duplex pause src Mac Address.
*         MAC address should be an Unicast address.
*         For different MAC Addresses per port operation,
*         use gsysSetPerPortDuplexPauseMac API.
* @param[in] mac                      - The Mac address to be set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGsysSetDuplexPauseMac
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_ETHERADDR     *mac
)
{
    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if the device has Switch MAC Register, we need the special operation */
    return writeSwitchMacReg(dev,mac);
}


/**
* @internal gsysGetDuplexPauseMac function
* @endinternal
*
* @brief   This routine Gets the full duplex pause src Mac Address.
*         For different MAC Addresses per port operation,
*         use gsysGetPerPortDuplexPauseMac API.
*
* @param[out] mac                      - the Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS gsysGetDuplexPauseMac
(
    IN  GT_CPSS_QD_DEV   *dev,
    OUT GT_ETHERADDR     *mac
)
{
    PRV_CPSS_DBG_INFO(("gsysGetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return readSwitchMacReg(dev,mac);
}



/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/

/*
 * Write to Switch MAC Register
 */
static GT_STATUS writeSwitchMacReg
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_ETHERADDR      *mac
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          data;     /* temporary Data storage */
    GT_U16          i;

    GT_U32      retryCount = 0; /* Counter for busy wait loops */

    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
    {
        /* Wait until the device is ready. */
        data = 1;
        while(data == 1)
        {
            retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SWITCH_MAC,15,1,&data);
            if(retVal != GT_OK)
            {
                return retVal;
            }
            /* check that the number of iterations does not exceed the limit */
            PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
        }

        data = (1 << 15) | (i << 8) | mac->arEther[i];

        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SWITCH_MAC,data);
        if(retVal != GT_OK)
           {
               return retVal;
        }
    }

    return GT_OK;
}

/*
 * Read from Switch MAC Register
 */
static GT_STATUS readSwitchMacReg
(
    IN  GT_CPSS_QD_DEV    *dev,
    OUT GT_ETHERADDR      *mac
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          data;     /* temporary Data storage */
    GT_U16            i;
    GT_U32      retryCount = 0; /* Counter for busy wait loops */


    /* Wait until the device is ready. */
    data = 1;
    while(data == 1)
    {
           retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SWITCH_MAC,15,1,&data);
           if(retVal != GT_OK)
        {
               return retVal;
        }
           /* check that the number of iterations does not exceed the limit */
           PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
    {
        data = i << 8;

        retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SWITCH_MAC,data);
        if(retVal != GT_OK)
           {
               return retVal;
        }

        retVal = prvCpssDrvHwGetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SWITCH_MAC,&data);
        if(retVal != GT_OK)
           {
               return retVal;
        }

        if (i == 0)
            mac->arEther[i] = data & 0xFE;    /* bit 0 is for diffAddr */
        else
            mac->arEther[i] = data & 0xFF;
    }

    return GT_OK;
}



