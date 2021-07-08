#include <Copyright.h>

/**
********************************************************************************
* @file gtMisc.c
*
* @brief API definitions for Ip Mapping Table
* EEPROM access
* Scratch and Misc Control
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtMisc.c
*
* DESCRIPTION:
*       API definitions for Ip Mapping Table
*                            EEPROM access
*                            Scratch and Misc Control
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>


/**
* @internal prvCpssDrvGsysSetScratchMiscCtrl function
* @endinternal
*
* @brief   Set Scratch and Misc control data to the Scratch and Misc Control register.
*         The registers of Scratch and Misc control are.
*         Scratch Byte 0
*         Scratch Byte 1
*         GPIO Configuration
*         Reserved for future use
*         GPIO Direction
*         GPIO Data
*         CONFIG Data 0
*         CONFIG Data 1
*         CONFIG Data 2
*         CONFIG Data 3
* @param[in] point                    - Pointer to the Scratch and Misc. Control register.
* @param[in] data                     - Scratch and Misc. Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetScratchMiscCtrl
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            point,
    IN  GT_U8            data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    IN  GT_U16   count, tmpData;
    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetScratchMiscCtrl Called.\n"));

    if (point > GT_CPSS_SCRAT_MISC_REG_MAX)
    {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (data &0xffffff00)
    {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* program QoS Weight Table, 4 sequences at a time */

    /* Wait until the Scratch and Misc control is ready. */
#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 1;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[0].reg = PRV_CPSS_QD_REG_SCRATCH_MISC;
      regAccess.rw_reg_list[0].data = 15;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
      }
    }
#else
    count = 5;
    tmpData = 1;
    while(tmpData ==1)
    {
        if(--count==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_SCRATCH_MISC,15,1,&tmpData);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    }
#endif

  tmpData =  (GT_U16)((1 << 15) | (point << 8) | data);

  retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_SCRATCH_MISC, tmpData);
  if(retVal != GT_OK)
  {
     PRV_CPSS_DBG_INFO(("Failed.\n"));
     prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
     return retVal;
  }

  prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

  return retVal;

}



/**
* @internal prvCpssDrvGsysGetScratchMiscCtrl function
* @endinternal
*
* @brief   Get Scratch and Misc control data from the Scratch and Misc Control register.
*         The register of Scratch and Misc control are.
*         Scratch Byte 0
*         Scratch Byte 1
*         GPIO Configuration
*         Reserved for future use
*         GPIO Direction
*         GPIO Data
*         CONFIG Data 0
*         CONFIG Data 1
*         CONFIG Data 2
*         CONFIG Data 3
* @param[in] point                    - Pointer to the Scratch and Misc. Control register.
*
* @param[out] data                     - Scratch and Misc. Control  read from the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetScratchMiscCtrl
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            point,
    OUT GT_U8            *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U16       count, tmpData;
    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetScratchMiscCtrl Called.\n"));

    if (point > GT_CPSS_SCRAT_MISC_REG_MAX)
    {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (point>0x7f)
    {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* program QoS Weight Table, 4 sequences at a time */

    tmpData =  (GT_U16)((point << 8) | 0);
    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_SCRATCH_MISC, tmpData);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
       return retVal;
    }

#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 2;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[0].reg = PRV_CPSS_QD_REG_SCRATCH_MISC;
      regAccess.rw_reg_list[0].data = 15;
      regAccess.rw_reg_list[1].cmd = HW_REG_READ;
      regAccess.rw_reg_list[1].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[1].reg = PRV_CPSS_QD_REG_SCRATCH_MISC;
      regAccess.rw_reg_list[1].data = 0;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
     }
     tmpData = qdLong2Short(regAccess.rw_reg_list[1].data);
    }
#else
    count = 5;
    do {
        if((--count)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_SCRATCH_MISC, &tmpData);
        if(retVal != GT_OK)
           {
               PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
               return retVal;
        }
   } while (tmpData&0x8000);
#endif

    *data = tmpData&0xff;

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);


    return retVal;
}




/**
* @internal prvCpssDrvGsysSetScratchBits function
* @endinternal
*
* @brief   Set bits to the Scratch and Misc Control register <scratch byte 0 and 1>.
*         These bits are 100% available to software for whatever purpose desired.
*         These bits do not connect to any hardware function.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetScratchBits
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U16            scratch
)
{
    GT_STATUS    retVal;         /* Functions return value.      */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetScratchBits Called.\n"));

    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_SCRAT_0, (GT_U8)(scratch&0xff));
    if(retVal != GT_OK)
       {
           PRV_CPSS_DBG_INFO(("Failed.\n"));
           return retVal;
    }

    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_SCRAT_1, (GT_U8)((scratch>>8)&0xff));
    if(retVal != GT_OK)
       {
           PRV_CPSS_DBG_INFO(("Failed.\n"));
           return retVal;
    }

    return GT_OK;

}

/**
* @internal prvCpssDrvGsysGetScratchBits function
* @endinternal
*
* @brief   Get bits from the Scratch and Misc Control register <scratch byte 0 and 1>.
*         These bits are 100% available to software for whatever purpose desired.
*         These bits do not connect to any hardware function.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetScratchBits
(
    IN  GT_CPSS_QD_DEV    *dev,
    OUT GT_U16            *scratch
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetScratchBits Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_SCRAT_1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *scratch = data;
    *scratch = *scratch<<8;
    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_SCRAT_0, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *scratch |= data;

    return GT_OK;

}



/**
* @internal prvCpssDrvGsysGetGpioConfigMod function
* @endinternal
*
* @brief   Get mode from the Scratch and Misc Control register <GPIO Configuration>.
*         The bits are shared General Purpose Input Output mode Bits:
*         Bit 15 - GT_GPIO_BIT_14:  1:GPIO[14]  0:P5_OUTCLK
*         ...
*         Bit 6 - GT_GPIO_BIT_6:  1:GPIO[6]  0:SE_RCLK1
*         Bit 5 - GT_GPIO_BIT_5:  1:GPIO[5]  0:SE_RCLK0
*         Bit 4 - GT_GPIO_BIT_4:  1:GPIO[4]  0:
*         Bit 3 - GT_GPIO_BIT_3:  1:GPIO[3]  0:
*         Bit 2 - GT_GPIO_BIT_2:  1:GPIO[2]  0:
*         Bit 1 - GT_GPIO_BIT_1:  1:GPIO[1]  0:P6_COL
*         Bit 0 - GT_GPIO_BIT_0:  1:GPIO[0]  0:P6_CRS
*         Now, bits are read only, except for 0, and 7.
*
* @param[out] mode                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioConfigMod
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U32            *mode
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioConfigMod Called.\n"));

    {
        retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_CFG_2, &data);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            return retVal;
        }
        *mode = data;
        *mode <<= 8;
        retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_CFG_1, &data);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            return retVal;
        }
        *mode |= data;
    }


    return GT_OK;

}

/**
* @internal prvCpssDrvGsysSetGpioPinControl function
* @endinternal
*
* @brief   Set GPIO pin control to the Scratch and Misc Control register <GPIO pin control>
*         General Purpose Input Output Interface pin x Control. This control is
*         used to control alternate functions of the GPIO[x] pin when it is not
*         being used as part of some other interface. This control has an effect
*         only if the pin is enabled to be a GPIO pin, i.e., the GPIO[x] Mode bit
*         is a one (Register Index 0x60 of Scratch and Misc., Control).
*         The options are as follows:
*         GT_GPIO_PIN_GPIO
*         GT_GPIO_PIN_PTP_TRIG
*         GT_GPIO_PIN_PTP_EVREQ
*         GT_GPIO_PIN_PTP_EXTCLK
*         GT_GPIO_PIN_RX_CLK0
*         GT_GPIO_PIN_RX_CLK1
*         GT_GPIO_PIN_SDET
*         GT_GPIO_PIN_CLK125
* @param[in] pinNum                   - pin number.
* @param[in] pinCtrl                  - pin control.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioPinControl
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            pinNum,
    IN  GT_U8            pinCtrl
)
{
    GT_STATUS    retVal=GT_OK;         /* Functions return value.      */
    GT_U8 data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetGpioPinControl Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_0+(pinNum/2), &data);
    if(retVal != GT_OK)
    {
      PRV_CPSS_DBG_INFO(("Failed.\n"));
      return retVal;
    }
    if (pinNum&1)
    {
       data &= 0x8f;
       data |= pinCtrl<<4;
    }
    else
    {
       data &= 0xf8;
       data |= pinCtrl;
    }

    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_0+(pinNum/2), data);
    if(retVal != GT_OK)
    {
      PRV_CPSS_DBG_INFO(("Failed.\n"));
      return retVal;
    }

    return retVal;

}


/**
* @internal prvCpssDrvGsysGetGpioPinControl function
* @endinternal
*
* @brief   Get GPIO pin control to the Scratch and Misc Control register <GPIO pin control>.
*         General Purpose Input Output Interface pin x Control. This control is
*         used to control alternate functions of the GPIO[x] pin when it is not
*         being used as part of some other interface. This control has an effect
*         only if the pin is enabled to be a GPIO pin, i.e., the GPIO[x] Mode bit
*         is a one (Register Index 0x60 of Scratch and Misc., Control).
*         The options are as follows:
*         GT_GPIO_PIN_GPIO
*         GT_GPIO_PIN_PTP_TRIG
*         GT_GPIO_PIN_PTP_EVREQ
*         GT_GPIO_PIN_PTP_EXTCLK
*         GT_GPIO_PIN_RX_CLK0
*         GT_GPIO_PIN_RX_CLK1
*         GT_GPIO_PIN_SDET
*         GT_GPIO_PIN_CLK125
* @param[in] pinNum                   - pin number.
*
* @param[out] pinCtrl                  - pin control.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioPinControl
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            pinNum,
    OUT GT_U8            *pinCtrl
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioPinControl Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_CTRL_0+(pinNum/2), &data);
    if(retVal != GT_OK)
    {
      PRV_CPSS_DBG_INFO(("Failed.\n"));
      return retVal;
    }
    if (pinNum&1)
    {
       *pinCtrl = (data >> 4)&0x7;
    }
    else
    {
       *pinCtrl = data&0x7;
    }

    return GT_OK;

}


/**
* @internal prvCpssDrvGsysSetGpioDirection function
* @endinternal
*
* @brief   Set Gpio direction to the Scratch and Misc Control register <GPIO Direction>.
*         The bits are used to control the direction of GPIO[6:0] or GPIO(14:0].
*         When a GPIO's bit is set to a one that GPIO will become an input. When a
*         GPIO's bit is cleared to a zero that GPIO will become an output
*         General Purpose Input Output direction bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
* @param[in] dir                      - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioDirection
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U32            dir
)
{
  GT_STATUS    retVal;         /* Functions return value.      */

  PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetGpioDirection Called.\n"));

  {
    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_1, (GT_U8)(dir&0xff));
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       return retVal;
    }
    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_2, (GT_U8)((dir>>8)&0x7f));
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       return retVal;
    }
  }


  return GT_OK;

}


/**
* @internal prvCpssDrvGsysGetGpioDirection function
* @endinternal
*
* @brief   Get Gpio direction from the Scratch and Misc Control register <GPIO Direction>.
*         The bits are used to control the direction of GPIO[6:0] or GPIO(14:0].
*         When a GPIO's bit is set to a one that GPIO will become an input. When a
*         GPIO's bit is cleared to a zero that GPIO will become an output
*         General Purpose Input Output direction bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
*
* @param[out] dir                      - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioDirection
(
    IN  GT_CPSS_QD_DEV    *dev,
    OUT GT_U32            *dir
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioDirection Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *dir = data;
    *dir <<= 8;
    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DIR_1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *dir |= data;

    return GT_OK;

}

/**
* @internal prvCpssDrvGsysSetGpioSMI function
* @endinternal
*
* @brief   Set Normal SMI to the Scratch and Misc Control register <GPIO Direction>.
*         The bit is used to control the Normal SMI vs. GPIO mode.
*         When P5_MODE is not equal to 0x1 or 0x2 the P5_COL and P5_CRS pins are
*         not needed. In this case, when this bit is set to a one, the P5_COL and
*         P5_CRS pins become MDIO_PHY and MDC_PHY, respectively, if the NO_CPU
*         configuration pin was a one during reset. Else the pins become GPIO pins 7 and 8.
* @param[in] smi                      - SMI OR GT_GPIO
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioSMI
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_BOOL           smi
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;
    GT_U8        point;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetGpioSMI Called.\n"));

    point = 0x02;
    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, point, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    if (smi==GT_FALSE)
        data |= 0x80;
    else
        data &= ~0x80;

    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, point, (GT_U8)(data&0xff));
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }


    return GT_OK;

}


/**
* @internal prvCpssDrvGsysGetGpioSMI function
* @endinternal
*
* @brief   Get Normal SMI from the Scratch and Misc Control register <GPIO Direction>.
*         The bit is used to control the Normal SMI vs. GPIO mode.
*         When P5_MODE is not equal to 0x1 or 0x2 the P5_COL and P5_CRS pins are
*         not needed. In this case, when this bit is set to a one, the P5_COL and
*         P5_CRS pins become MDIO_PHY and MDC_PHY, respectively, if the NO_CPU
*         configuration pin was a one during reset. Else the pins become GPIO pins 7 and 8.
*
* @param[out] smi                      - SMI OR GT_GPIO
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioSMI
(
    IN  GT_CPSS_QD_DEV    *dev,
    OUT GT_BOOL           *smi
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        data;
    GT_U8        point;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioSMI Called.\n"));

    point = 0x02;

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, point, &data);
    if(retVal != GT_OK)
       {
           PRV_CPSS_DBG_INFO(("Failed.\n"));
           return retVal;
    }

    if (data & 0x80)
      *smi=GT_FALSE;
    else
      *smi=GT_TRUE;

    return GT_OK;

}


/**
* @internal prvCpssDrvGsysSetGpioData function
* @endinternal
*
* @brief   Set Gpio data to the Scratch and Misc Control register <GPIO data>.
*         When a GPIO's bit is set to be an input, data written to this bit will go
*         to a holding register but will not appear on the pin nor in this register.
*         Reads of this register will return the actual, real-time, data that is
*         appearing on the GPIO's pin.
*         When a GPIO's bit is set to be an output, data written to this bit will go
*         to a holding register and will appear on the GPIO's pin. Reads of this register
*         will return the actual, real-time, data that is appearing on the GPIO's pin
*         (which in this case should be the data written, but if its isn't that would
*         be an indication of a conflict).
*         When a pin's direction changes from input to output, the data last written
*         to the holding register appears on the GPIO's pin
*         General Purpose Input Output data bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 5 - GT_GPIO_BIT_5
*         Bit 4 - GT_GPIO_BIT_4
*         Bit 3 - GT_GPIO_BIT_3
*         Bit 2 - GT_GPIO_BIT_2
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
* @param[in] data                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioData
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U32            data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetGpioData Called.\n"));

    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_1, (GT_U8)(data&0xff));
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_2, (GT_U8)((data>>8)&0x7f));
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }


    return GT_OK;

}

/**
* @internal prvCpssDrvGsysGetGpioData function
* @endinternal
*
* @brief   get Gpio data to the Scratch and Misc Control register <GPIO data>.
*         When a GPIO's bit is set to be an input, data written to this bit will go
*         to a holding register but will not appear on the pin nor in this register.
*         Reads of this register will return the actual, real-time, data that is
*         appearing on the GPIO's pin.
*         When a GPIO's bit is set to be an output, data written to this bit will go
*         to a holding register and will appear on the GPIO's pin. Reads of this register
*         will return the actual, real-time, data that is appearing on the GPIO's pin
*         (which in this case should be the data written, but if its isn't that would
*         be an indication of a conflict).
*         When a pin's direction changes from input to output, the data last written
*         to the holding register appears on the GPIO's pin
*         General Purpose Input Output data bits are:
*         Bit 15 - GT_GPIO_BIT_14
*         ...
*         Bit 6 - GT_GPIO_BIT_6
*         ...
*         Bit 5 - GT_GPIO_BIT_5
*         Bit 4 - GT_GPIO_BIT_4
*         Bit 3 - GT_GPIO_BIT_3
*         Bit 2 - GT_GPIO_BIT_2
*         Bit 1 - GT_GPIO_BIT_1
*         Bit 0 - GT_GPIO_BIT_0
*
* @param[out] data                     - OR [GT_GPIO_BIT_x]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioData
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U32            *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        tmpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioData Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_2, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *data = tmpData;
    *data <<= 8;
    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO_DAT_1, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *data |= tmpData;

    return GT_OK;

}




/**
* @internal prvCpssDrvGsysSetGpioxPortStallVect function
* @endinternal
*
* @brief   Set GPIO X's Port Stall Vector. The value in this register is sent to the transmit
*         portion of all the ports in the device when GPIO X's Port Stall En (below) is set
*         to a one and the value on the device's GPIO pin matches the value in GPIO
*         X's Port Stall Value (below) and the GPIO pin is a GPIO input. The assertion
*         of this vector to the ports persists as long as the function is enabled (i.e., GPIO
*         X Port Stall En equals a one) and as long as the value on the pin matches the
*         programmed stall value (GPIO X Port Stall Value) and as long as the pin
*         remains a GPIO input.
*         When a port's bit is set to a one in this vector, that port or ports will stop
*         tranmitting their next frame (the current frame is not affected) until this vector
*         is deasserted. Port 0 is controlled by bit 0, port 1 by bit 1, etc.
* @param[in] gpioNum                  - GPIO number.
* @param[in] portStallVec             - GPIO Num's Port Stall Vector.
* @param[in] portStallEn              - GPIO Num's Port Stall Enable.
* @param[in] portStallValue           - GPIO Num's Port Stall Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysSetGpioxPortStallVect
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            gpioNum,
    IN  GT_U8            portStallVec,
    IN  GT_U8            portStallEn,
    IN  GT_U8            portStallValue
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8  data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetGpioxPortStallVect Called.\n"));

    data = portStallVec&0x7f;
    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC0+gpioNum*2, data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    data = (portStallEn&0x1);
    data = data<<1;
    data = data | (portStallValue&1);
    data<<=5;
    retVal = prvCpssDrvGsysSetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC1+gpioNum*2, data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    return GT_OK;

}

/**
* @internal prvCpssDrvGsysGetGpioxPortStallVect function
* @endinternal
*
* @brief   Get GPIO X's Port Stall Vector. The value in this register is sent to the transmit
*         portion of all the ports in the device when GPIO X's Port Stall En (below) is set
*         to a one and the value on the device's GPIO pin matches the value in GPIO
*         X's Port Stall Value (below) and the GPIO pin is a GPIO input. The assertion
*         of this vector to the ports persists as long as the function is enabled (i.e., GPIO
*         X Port Stall En equals a one) and as long as the value on the pin matches the
*         programmed stall value (GPIO X Port Stall Value) and as long as the pin
*         remains a GPIO input.
*         When a port's bit is set to a one in this vector, that port or ports will stop
*         tranmitting their next frame (the current frame is not affected) until this vector
*         is deasserted. Port 0 is controlled by bit 0, port 1 by bit 1, etc.
* @param[in] gpioNum                  - GPIO number.
*
* @param[out] portStallVec             - GPIO Num's Port Stall Vector.
* @param[out] portStallEn              - GPIO Num's Port Stall Enable.
* @param[out] portStallValue           - GPIO Num's Port Stall Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGsysGetGpioxPortStallVect
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            gpioNum,
    OUT GT_U8            *portStallVec,
    OUT GT_U8            *portStallEn,
    OUT GT_U8            *portStallValue
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8  data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetGpioxPortStallVect Called.\n"));

    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC0+gpioNum*2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *portStallVec = data&0x7f;
    retVal = prvCpssDrvGsysGetScratchMiscCtrl(dev, GT_CPSS_SCRAT_MISC_REG_GPIO0_STALL_VEC1+gpioNum*2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    *portStallEn = (data >> 6)&1;
    *portStallValue = (data >> 5)&1;

    return GT_OK;

}



/* gsysGlobal2PointDataSet  */

/**
* @internal gsysGlobal2PointDataSet function
* @endinternal
*
* @brief   Common set indirect register pointer control data from the Global2
*         function registers. The function register are.
*         Scratch & Misc
*         Energy Management
*         IMP Comm/Debug
*         Watch Dog Control
*         QoS Weights
* @param[in] funcReg                  - Global 2 function register.
* @param[in] point                    - Pointer to following Control register.
*                                      Scratch & Misc
*                                      Energy Management
*                                      IMP Comm/Debug
*                                      Watch Dog Control
*                                      QoS Weights
* @param[in] data                     - Scratch and Misc. Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
static GT_STATUS gsysGlobal2PointDataSet
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8             funcReg,
    IN  GT_U8             point,
    IN  GT_U8             data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    IN  GT_U16   count, tmpData;
    PRV_CPSS_DBG_INFO(("gsysGlobal2PointDataSet Called.\n"));

    if (data &0xffffff00)
    {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* program QoS Weight Table, 4 sequences at a time */

    /* Wait until the Scratch and Misc control is ready. */
#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 1;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[0].reg = funcReg;
      regAccess.rw_reg_list[0].data = 15;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
      }
    }
#else
    count = 5;
    tmpData = 1;
    while(tmpData ==1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,funcReg,15,1,&tmpData);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if(--count==0) break;
    }
#endif

  tmpData =  (GT_U16)((1 << 15) | (point << 8) | data);

  retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, funcReg, tmpData);
  if(retVal != GT_OK)
  {
     PRV_CPSS_DBG_INFO(("Failed.\n"));
     prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
     return retVal;
  }

  prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

  return retVal;

}



/**
* @internal gsysGlobal2PointDataGet function
* @endinternal
*
* @brief   Common get indirect register pointer control data from the Global2
*         function registers. The function register are.
*         Scratch & Misc
*         Energy Management
*         IMP Comm/Debug
*         Watch Dog Control
*         QoS Weights
* @param[in] funcReg                  - Global 2 function register.
* @param[in] point                    - Pointer to following Control register.
*                                      Scratch & Misc
*                                      Energy Management
*                                      IMP Comm/Debug
*                                      Watch Dog Control
*                                      QoS Weights
*
* @param[out] data                     -  Control  read from the indirect register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
static GT_STATUS gsysGlobal2PointDataGet
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8             funcReg,
    IN  GT_U8             point,
    OUT  GT_U8            *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U16            tmpData;
    int count=0x10;
    PRV_CPSS_DBG_INFO(("gsysGlobal2PointDataGet Called.\n"));

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    /* program QoS Weight Table, 4 sequences at a time */

    tmpData =  (GT_U16)((point << 8) | 0);
    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, funcReg, tmpData);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
       return retVal;
    }

#ifdef GT_RMGMT_ACCESS
    {
      HW_DEV_REG_ACCESS regAccess;

      regAccess.entries = 2;

      regAccess.rw_reg_list[0].cmd = HW_REG_WAIT_TILL_0;
      regAccess.rw_reg_list[0].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[0].reg = funcReg;
      regAccess.rw_reg_list[0].data = 15;
      regAccess.rw_reg_list[1].cmd = HW_REG_READ;
      regAccess.rw_reg_list[1].addr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
      regAccess.rw_reg_list[1].reg = funcReg;
      regAccess.rw_reg_list[1].data = 0;
      retVal = hwAccessMultiRegs(dev, &regAccess);
      if(retVal != GT_OK)
      {
        prvCpssDrvGtSemGive(dev,dev->vtuRegsSem);
        return retVal;
     }
     tmpData = qdLong2Short(regAccess.rw_reg_list[1].data);
    }
#else
    do {
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, funcReg, &tmpData);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if((count--)==0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    } while (tmpData&0x8000);
#endif

    *data = tmpData&0xff;

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);


    return retVal;
}




/**
* @internal prvCpssDrvGdevSetQosWeightCtrl function
* @endinternal
*
* @brief   Set QoS Weight control data from the QoS Weight Control register.
*         The register of QoS Weight control are one of 64 possible QoS Weight
*         Data registers and the QoS Weight Length register.
* @param[in] point                    - Pointer to the QoS Weight register.
* @param[in] data                     - QoS Weight Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGdevSetQosWeightCtrl
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8             point,
    IN  GT_U8            data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    PRV_CPSS_DBG_INFO(("prvCpssDrvGdevSetQosWeightCtrl Called.\n"));

    {
      if (point > 127)
      {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
      }
    }

    retVal = gsysGlobal2PointDataSet(dev, PRV_CPSS_QD_REG_QOS_WEIGHT, point, data);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("gsysGlobal2PointDataSet Failed.\n"));
       return retVal;
    }


  return retVal;

}



/**
* @internal prvCpssDrvGdevGetQosWeightCtrl function
* @endinternal
*
* @brief   Set QoS Weight control data from the QoS Weight Control register.
*         The register of QoS Weight control are one of 64 possible QoS Weight
*         Data registers and the QoS Weight Length register.
* @param[in] point                    - Pointer to the QoS Weight register.
*
* @param[out] data                     - QoS Weight Control  written to the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if input parameters are beyond range.
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGdevGetQosWeightCtrl
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8             point,
    OUT  GT_U8            *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    PRV_CPSS_DBG_INFO(("prvCpssDrvGdevGetQosWeightCtrl Called.\n"));

    {
      if (point > 127)
      {
        PRV_CPSS_DBG_INFO(("GT_BAD_PARAM\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
      }
    }

    retVal = gsysGlobal2PointDataGet(dev, PRV_CPSS_QD_REG_QOS_WEIGHT, point, data);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("gsysGlobal2PointDataGet Failed.\n"));
       return retVal;
    }

    return retVal;
}



