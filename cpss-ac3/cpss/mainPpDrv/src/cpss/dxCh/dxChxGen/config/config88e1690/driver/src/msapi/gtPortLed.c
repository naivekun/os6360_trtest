#include <Copyright.h>

/**
********************************************************************************
* @file gtPortLed.c
*
* @brief API definitions for LED Control
*
* @version
********************************************************************************
*/
/********************************************************************************
* gtPortLed.c
*
* DESCRIPTION:
*       API definitions for LED Control
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>


static GT_STATUS convertLED2APP
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_LPORT      port,
    IN  GT_CPSS_LED_CFG    cfg,
    IN  GT_U32             value,
    OUT GT_U32             *data
);


static GT_STATUS convertAPP2LED
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_LPORT      port,
    IN  GT_CPSS_LED_CFG    cfg,
    IN  GT_U32             value,
    OUT GT_U32             *data
);


/**
* @internal prvCpssDrvGprtSetLED function
* @endinternal
*
* @brief   This API allows to configure 4 LED sections, Pulse stretch, Blink rate,
*         and special controls.
* @param[in] port                     - the logical  number
* @param[in] cfg                      - GT_CPSS_LED_CFG value
* @param[in] value                    -  to be configured
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtSetLED
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_LPORT      port,
    IN  GT_CPSS_LED_CFG    cfg,
    IN  GT_U32             value
)
{
    GT_STATUS  retVal;         /* Functions return value.      */
    GT_U16     data;
    GT_U32     ptr, conv, mask;
    GT_U8      hwPort;         /* the physical port number     */
    int count = 0x10;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetLED Called.\n"));

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch (cfg)
    {
        case GT_CPSS_LED_CFG_GLOBAL_CONTROL:
            ptr = 0;
            mask = 0x001F;
            break;
        case GT_CPSS_LED_CFG_LED0:
            ptr = 0;
            mask = 0xF;
            break;
        case GT_CPSS_LED_CFG_LED1:
            ptr = 0;
            mask = 0xF0;
            break;
        case GT_CPSS_LED_CFG_LED2:
        case GT_CPSS_LED_CFG_LED3:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        case GT_CPSS_LED_CFG_SPECIAL_LED1:
            ptr = 1;
            mask = (1 << dev->maxPorts) - 1;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED2:
            ptr = 2;
            mask = (1 << dev->maxPorts) - 1;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED3:
            ptr = 3;
            mask = (1 << dev->maxPorts) - 1;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED4:
            ptr = 4;
            mask = (1 << dev->maxPorts) - 1;
            break;
        case GT_CPSS_LED_CFG_PULSE_STRETCH:
            ptr = 6;
            mask = 0x70;
            break;
        case GT_CPSS_LED_CFG_BLINK_RATE:
            ptr = 6;
            mask = 0x7;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_CONTROL:
            ptr = 7;
            mask = (1 << dev->maxPorts) - 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    conv = 0;
    retVal = convertAPP2LED(dev,port,cfg,value,&conv);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* Wait until the Table is ready. */
#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 1;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, hwPort, PORT_ACCESS);
      regAccess.rw_reg_list[0].reg = PRV_CPSS_QD_REG_LED_CONTROL;
      regAccess.rw_reg_list[0].data = 15;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
      }
    }
#else
    do
    {
        if((--count)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDrvHwGetAnyRegField(dev,hwPort,PRV_CPSS_QD_REG_LED_CONTROL,15,1,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    } while(data == 1);
#endif

    /* read the current data */
    data = (GT_U16)(ptr << 12);

    retVal = prvCpssDrvHwSetAnyReg(dev, hwPort, PRV_CPSS_QD_REG_LED_CONTROL, data);
    if(retVal != GT_OK)
      {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev, hwPort, PRV_CPSS_QD_REG_LED_CONTROL,0,11,&data);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    /* overwrite the data */
    data = (GT_U16)((1 << 15) | (ptr << 12) | (conv | (data & ~mask)));

    retVal = prvCpssDrvHwSetAnyReg(dev, hwPort, PRV_CPSS_QD_REG_LED_CONTROL, data);
    if(retVal != GT_OK)
      {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    return GT_OK;
}


/**
* @internal prvCpssDrvGprtGetLED function
* @endinternal
*
* @brief   This API allows to retrieve 4 LED sections, Pulse stretch, Blink rate,
*         and special controls.
* @param[in] port                     - the logical  number
* @param[in] cfg                      - GT_CPSS_LED_CFG value
*
* @param[out] value                    -  to be configured
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtGetLED
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_CPSS_LED_CFG   cfg,
    OUT GT_U32            *value
)
{
    GT_STATUS  retVal;         /* Functions return value.      */
    GT_U16     data;
    GT_U32     ptr;
    GT_U8      hwPort;         /* the physical port number     */
    int count = 0x10;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetLED Called.\n"));

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch (cfg)
    {
        case GT_CPSS_LED_CFG_GLOBAL_CONTROL:
            ptr = 0;
            break;
        case GT_CPSS_LED_CFG_LED0:
            ptr = 0;
            break;
        case GT_CPSS_LED_CFG_LED1:
            ptr = 0;
            break;
        case GT_CPSS_LED_CFG_LED2:
        case GT_CPSS_LED_CFG_LED3:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        case GT_CPSS_LED_CFG_SPECIAL_LED1:
            ptr = 1;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED2:
            ptr = 2;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED3:
            ptr = 3;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED4:
            ptr = 4;
            break;
        case GT_CPSS_LED_CFG_PULSE_STRETCH:
            ptr = 6;
            break;
        case GT_CPSS_LED_CFG_BLINK_RATE:
            ptr = 6;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_CONTROL:
            ptr = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* Wait until the Table is ready. */
#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 1;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, hwPort, PORT_ACCESS);
      regAccess.rw_reg_list[0].reg = PRV_CPSS_QD_REG_LED_CONTROL;
      regAccess.rw_reg_list[0].data = 15;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
      }
    }
#else
    do
    {
        if((--count)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDrvHwGetAnyRegField(dev,hwPort,PRV_CPSS_QD_REG_LED_CONTROL,15,1,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    } while(data == 1);
#endif

    /* read the current data */
    data = (GT_U16)(ptr << 12);

    retVal = prvCpssDrvHwSetAnyReg(dev, hwPort, PRV_CPSS_QD_REG_LED_CONTROL, data);
    if(retVal != GT_OK)
      {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev, hwPort, PRV_CPSS_QD_REG_LED_CONTROL,0,11,&data);
    if(retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    retVal = convertLED2APP(dev,port,cfg,data,value);
    if (retVal != GT_OK)
    {
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }


    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    return GT_OK;
}


static GT_STATUS convertAPP2LED
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_CPSS_LED_CFG  cfg,
    IN  GT_U32           value,
    OUT GT_U32           *data
)
{
    GT_STATUS    retVal = GT_OK;

    switch (cfg)
    {
        case GT_CPSS_LED_CFG_GLOBAL_CONTROL:
            if (port != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *data = value;
            break;
        case GT_CPSS_LED_CFG_LED0:
            switch (value)
            {
                case GT_CPSS_LED_LINK_ACT_SPEED:
                    *data = 0;
                    break;
                case GT_CPSS_LED_LINK_ACT:
                    *data = 3;
                    break;
                case GT_CPSS_LED_LINK:
                    *data = 8;
                    break;
                case GT_CPSS_LED_10_LINK_ACT:
                    *data = 10;
                    break;
                case GT_CPSS_LED_10_LINK:
                    *data = 9;
                    break;
                case GT_CPSS_LED_1000_LINK_ACT:
                    *data = 2;
                    break;
                case GT_CPSS_LED_10_1000_LINK_ACT:
                    *data = 7;
                    break;
                case GT_CPSS_LED_100_1000_LINK_ACT:
                    *data = 1;
                    break;
                case GT_CPSS_LED_100_1000_LINK:
                    *data = 11;
                    break;
                case GT_CPSS_LED_SPECIAL:
                    *data = 4;
                    break;
                case GT_CPSS_LED_DUPLEX_COL:
                    *data = 6;
                    break;
                case GT_CPSS_LED_FORCE_BLINK:
                    *data = 13;
                    break;
                case GT_CPSS_LED_FORCE_OFF:
                    *data = 14;
                    break;
                case GT_CPSS_LED_FORCE_ON:
                    *data = 15;
                    break;
                default:
                    retVal = GT_BAD_PARAM;
                    break;
            }
            break;

        case GT_CPSS_LED_CFG_LED1:
            switch (value)
            {
                case GT_CPSS_LED_LINK_ACT:
                    *data = 5;
                    break;
                case GT_CPSS_LED_100_LINK_ACT:
                    *data = 10;
                    break;
                case GT_CPSS_LED_100_LINK:
                    *data = 9;
                    break;
                case GT_CPSS_LED_1000_LINK:
                    *data = 3;
                    break;
                case GT_CPSS_LED_10_100_LINK_ACT:
                    *data = 1;
                    break;
                case GT_CPSS_LED_10_100_LINK:
                    *data = 11;
                    break;
                case GT_CPSS_LED_10_1000_LINK_ACT:
                    *data = 6;
                    break;
                case GT_CPSS_LED_10_1000_LINK:
                    *data = 7;
                    break;
                case GT_CPSS_LED_SPECIAL:
                    *data = 0;
                    break;
                case GT_CPSS_LED_ACTIVITY:
                    *data = 8;
                    break;
                case GT_CPSS_LED_FORCE_BLINK:
                    *data = 13;
                    break;
                case GT_CPSS_LED_FORCE_OFF:
                    *data = 14;
                    break;
                case GT_CPSS_LED_FORCE_ON:
                    *data = 15;
                    break;
                default:
                    retVal = GT_BAD_PARAM;
                    break;
            }
            *data <<= 4;
            break;

        case GT_CPSS_LED_CFG_LED2:
            switch (value)
            {
                case GT_CPSS_LED_10_LINK_ACT:
                    *data = 6;
                    break;
                case GT_CPSS_LED_100_LINK:
                    *data = 8;
                    break;
                case GT_CPSS_LED_1000_LINK_ACT:
                    *data = 10;
                    break;
                case GT_CPSS_LED_1000_LINK:
                    *data = 9;
                    break;
                case GT_CPSS_LED_10_1000_LINK_ACT:
                    *data = 1;
                    break;
                case GT_CPSS_LED_10_1000_LINK:
                    *data = 11;
                    break;
                case GT_CPSS_LED_100_1000_LINK_ACT:
                    *data = 7;
                    break;
                case GT_CPSS_LED_100_1000_LINK:
                    *data = 3;
                    break;
                case GT_CPSS_LED_SPECIAL:
                    *data = 2;
                    break;
                case GT_CPSS_LED_DUPLEX_COL:
                    *data = 0;
                    break;
                case GT_CPSS_LED_PTP_ACT:
                    *data = 12;
                    break;
                case GT_CPSS_LED_FORCE_BLINK:
                    *data = 13;
                    break;
                case GT_CPSS_LED_FORCE_OFF:
                    *data = 14;
                    break;
                case GT_CPSS_LED_FORCE_ON:
                    *data = 15;
                    break;
                default:
                    retVal = GT_BAD_PARAM;
                    break;
            }
            break;

        case GT_CPSS_LED_CFG_LED3:
            switch (value)
            {
                case GT_CPSS_LED_LINK_ACT:
                    *data = 10;
                    break;
                case GT_CPSS_LED_LINK:
                    *data = 9;
                    break;
                case GT_CPSS_LED_10_LINK:
                    *data = 8;
                    break;
                case GT_CPSS_LED_100_LINK_ACT:
                    *data = 6;
                    break;
                case GT_CPSS_LED_10_1000_LINK_ACT:
                    *data = 7;
                    break;
                case GT_CPSS_LED_SPECIAL:
                    *data = 0;
                    break;
                case GT_CPSS_LED_DUPLEX_COL:
                    *data = 1;
                    break;
                case GT_CPSS_LED_ACTIVITY:
                    *data = 11;
                    break;
                case GT_CPSS_LED_PTP_ACT:
                    *data = 12;
                    break;
                case GT_CPSS_LED_FORCE_BLINK:
                    *data = 13;
                    break;
                case GT_CPSS_LED_FORCE_OFF:
                    *data = 14;
                    break;
                case GT_CPSS_LED_FORCE_ON:
                    *data = 15;
                    break;
                default:
                    retVal = GT_BAD_PARAM;
                    break;
            }
            *data <<= 4;
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED1:
        case GT_CPSS_LED_CFG_SPECIAL_LED2:
        case GT_CPSS_LED_CFG_SPECIAL_LED3:
        case GT_CPSS_LED_CFG_SPECIAL_LED4:
            if (port != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *data = value;
            break;
        case GT_CPSS_LED_CFG_PULSE_STRETCH:
            if (value > 0x4)
                retVal = GT_BAD_PARAM;
            *data = value << 4;
            break;
        case GT_CPSS_LED_CFG_BLINK_RATE:
            if (value > 0x5)
                retVal = GT_BAD_PARAM;
            *data = value;
            break;

        case GT_CPSS_LED_CFG_SPECIAL_CONTROL:
            if (value >= (GT_U32)(1 << dev->maxPorts))
                retVal = GT_BAD_PARAM;
            *data = value;
            break;

        default:
            retVal = GT_BAD_PARAM;
            break;
    }

    return retVal;

}


static GT_STATUS convertLED2APP
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_CPSS_LED_CFG  cfg,
    IN  GT_U32           value,
    OUT GT_U32           *data
)
{
    GT_STATUS retVal = GT_OK;

    switch (cfg)
    {
        case GT_CPSS_LED_CFG_GLOBAL_CONTROL:
            if (port != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *data = value & 0x001F;
            break;
        case GT_CPSS_LED_CFG_LED0:
            value &= 0xF;
            switch (value)
            {
                case 0:
                    *data = GT_CPSS_LED_LINK_ACT_SPEED;
                    break;
                case 1:
                    *data = GT_CPSS_LED_100_1000_LINK_ACT;
                    break;
                case 2:
                    *data = GT_CPSS_LED_1000_LINK_ACT;
                    break;
                case 3:
                    *data = GT_CPSS_LED_LINK_ACT;
                    break;
                case 4:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 5:
                    *data = GT_CPSS_LED_RESERVE;
                    break;
                case 6:
                    *data = GT_CPSS_LED_DUPLEX_COL;
                    break;
                case 7:
                    *data = GT_CPSS_LED_10_1000_LINK_ACT;
                    break;
                case 8:
                    *data = GT_CPSS_LED_LINK;
                    break;
                case 9:
                    *data = GT_CPSS_LED_10_LINK;
                    break;
                case 10:
                    *data = GT_CPSS_LED_10_LINK_ACT;
                    break;
                case 11:
                    *data = GT_CPSS_LED_100_1000_LINK;
                    break;
                case 12:
                    *data = GT_CPSS_LED_PTP_ACT;
                    break;
                case 13:
                    *data = GT_CPSS_LED_FORCE_BLINK;
                    break;
                case 14:
                    *data = GT_CPSS_LED_FORCE_OFF;
                    break;
                case 15:
                    *data = GT_CPSS_LED_FORCE_ON;
                    break;
                default:
                    retVal = GT_FAIL;
                    break;
            }
            break;

        case GT_CPSS_LED_CFG_LED1:
            value >>= 4;
            value &= 0xF;
            switch (value)
            {
                case 0:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 1:
                    *data = GT_CPSS_LED_10_100_LINK_ACT;
                    break;
                case 2:
                    *data = GT_CPSS_LED_10_100_LINK_ACT;
                    break;
                case 3:
                    *data = GT_CPSS_LED_1000_LINK;
                    break;
                case 4:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 5:
                    *data = GT_CPSS_LED_LINK_ACT;
                    break;
                case 6:
                    *data = GT_CPSS_LED_10_1000_LINK_ACT;
                    break;
                case 7:
                    *data = GT_CPSS_LED_10_1000_LINK;
                    break;
                case 8:
                    *data = GT_CPSS_LED_ACTIVITY;
                    break;
                case 9:
                    *data = GT_CPSS_LED_100_LINK;
                    break;
                case 10:
                    *data = GT_CPSS_LED_100_LINK_ACT;
                    break;
                case 11:
                    *data = GT_CPSS_LED_10_100_LINK;
                    break;
                case 12:
                    *data = GT_CPSS_LED_PTP_ACT;
                    break;
                case 13:
                    *data = GT_CPSS_LED_FORCE_BLINK;
                    break;
                case 14:
                    *data = GT_CPSS_LED_FORCE_OFF;
                    break;
                case 15:
                    *data = GT_CPSS_LED_FORCE_ON;
                    break;
                default:
                    retVal = GT_FAIL;
                    break;
            }
            break;

        case GT_CPSS_LED_CFG_LED2:
            value &= 0xF;
            switch (value)
            {
                case 0:
                    *data = GT_CPSS_LED_DUPLEX_COL;
                    break;
                case 1:
                    *data = GT_CPSS_LED_10_1000_LINK_ACT;
                    break;
                case 2:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 3:
                    *data = GT_CPSS_LED_100_1000_LINK;
                    break;
                case 4:
                    *data = GT_CPSS_LED_RESERVE;
                    break;
                case 5:
                    *data = GT_CPSS_LED_RESERVE;
                    break;
                case 6:
                    *data = GT_CPSS_LED_10_LINK_ACT;
                    break;
                case 7:
                    *data = GT_CPSS_LED_100_1000_LINK_ACT;
                    break;
                case 8:
                    *data = GT_CPSS_LED_100_LINK;
                    break;
                case 9:
                    *data = GT_CPSS_LED_1000_LINK;
                    break;
                case 10:
                    *data = GT_CPSS_LED_1000_LINK_ACT;
                    break;
                case 11:
                    *data = GT_CPSS_LED_10_1000_LINK;
                    break;
                case 12:
                    *data = GT_CPSS_LED_PTP_ACT;
                    break;
                case 13:
                    *data = GT_CPSS_LED_FORCE_BLINK;
                    break;
                case 14:
                    *data = GT_CPSS_LED_FORCE_OFF;
                    break;
                case 15:
                    *data = GT_CPSS_LED_FORCE_ON;
                    break;
                default:
                    retVal = GT_FAIL;
                    break;
            }
            break;

        case GT_CPSS_LED_CFG_LED3:
            value >>= 4;
            value &= 0xF;
            switch (value)
            {
                case 0:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 1:
                    *data = GT_CPSS_LED_DUPLEX_COL;
                    break;
                case 2:
                    *data = GT_CPSS_LED_DUPLEX_COL;
                    break;
                case 3:
                    *data = GT_CPSS_LED_SPECIAL;
                    break;
                case 4:
                    *data = GT_CPSS_LED_RESERVE;
                    break;
                case 5:
                    *data = GT_CPSS_LED_RESERVE;
                    break;
                case 6:
                    *data = GT_CPSS_LED_100_LINK_ACT;
                    break;
                case 7:
                    *data = GT_CPSS_LED_10_1000_LINK_ACT;
                    break;
                case 8:
                    *data = GT_CPSS_LED_10_LINK;
                    break;
                case 9:
                    *data = GT_CPSS_LED_LINK;
                    break;
                case 10:
                    *data = GT_CPSS_LED_LINK_ACT;
                    break;
                case 11:
                    *data = GT_CPSS_LED_ACTIVITY;
                    break;
                case 12:
                    *data = GT_CPSS_LED_PTP_ACT;
                    break;
                case 13:
                    *data = GT_CPSS_LED_FORCE_BLINK;
                    break;
                case 14:
                    *data = GT_CPSS_LED_FORCE_OFF;
                    break;
                case 15:
                    *data = GT_CPSS_LED_FORCE_ON;
                    break;
                default:
                    retVal = GT_FAIL;
                    break;
            }
            break;
        case GT_CPSS_LED_CFG_SPECIAL_LED1:
        case GT_CPSS_LED_CFG_SPECIAL_LED2:
        case GT_CPSS_LED_CFG_SPECIAL_LED3:
        case GT_CPSS_LED_CFG_SPECIAL_LED4:
            if (port != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *data = value & ((1 << dev->maxPorts) - 1);
            break;
        case GT_CPSS_LED_CFG_PULSE_STRETCH:
            *data = (value >> 4) & 0x7;
            break;

        case GT_CPSS_LED_CFG_BLINK_RATE:
            *data = value & 0x7;
            break;

        case GT_CPSS_LED_CFG_SPECIAL_CONTROL:
            *data = value & ((1 << dev->maxPorts) - 1);
            break;

        default:
            retVal = GT_BAD_PARAM;
            break;
    }

    return retVal;

}


