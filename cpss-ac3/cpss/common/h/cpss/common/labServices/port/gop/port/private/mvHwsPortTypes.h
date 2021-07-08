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
* @file mvHwsPortTypes.h
*
* @brief Ln2CpuPortunit registers and tables classes declaration.
*
* @version   15
********************************************************************************
*/

#ifndef __mvHwsPortTypes_H
#define __mvHwsPortTypes_H

/* General H Files */
/**
* @enum MV_HWS_PORT_MAC_TYPE
 *
 * @brief Defines the different port MAC types.
*/
typedef enum
{
    MAC_NA,
    GEMAC_X,
    GEMAC_SG,
    XGMAC,
    XLGMAC,
    HGLMAC,
    CGMAC,
    INTLKN_MAC,
    TCAM_MAC,

    GEMAC_NET_X,
    GEMAC_NET_SG,

    QSGMII_MAC,

    D2D_MAC,

    MTI_MAC_100,        /*  MAC for 1G to 100G ports.
                            APPLICABLE DEVICES: Falcon */
    MTI_MAC_400,        /*  MAC for 200G/400G ports.
                            APPLICABLE DEVICES: Falcon */
    MTI_MAC_REDUCED,    /*  reduced/CPU MAC for 10G/25G port (port 17 in Raven).
                            APPLICABLE DEVICES: Falcon */

    LAST_MAC

}MV_HWS_PORT_MAC_TYPE;

/**
* @enum MV_HWS_PORT_PCS_TYPE
 *
 * @brief Defines the different port PCS types.
*/
typedef enum
{
    PCS_NA,
    GPCS,
    XPCS,
    MMPCS,
    HGLPCS,
    CGPCS,
    INTLKN_PCS,
    TCAM_PCS,

    GPCS_NET,

    QSGMII_PCS,

    D2D_PCS,

    MTI_PCS_50,         /*  PCS for 10G to 50G odd ports.
                            APPLICABLE DEVICES: Falcon */
    MTI_PCS_100,        /*  PCS for 10G to 100G even ports.
                            APPLICABLE DEVICES: Falcon */
    MTI_PCS_200,        /*  PCS for 200G even ports.
                            APPLICABLE DEVICES: Falcon */
    MTI_PCS_400,        /*  PCS for 400G port.
                            APPLICABLE DEVICES: Falcon */
    MTI_PCS_LOW_SPEED,  /*  Low speed PCS for 10M - 1G ports.
                            APPLICABLE DEVICES: Falcon */

    LAST_PCS

}MV_HWS_PORT_PCS_TYPE;

/**
* @enum MV_HWS_SERDES_SPEED
 *
 * @brief Defines the different values of serdes speed.
*/
typedef enum
{
    SPEED_NA,
    _1_25G,
    _3_125G,
    _3_33G,
    _3_75G,
    _4_25G,
    _5G,
    _6_25G,
    _7_5G,
    _10_3125G,      /* for 10G, 20G, 40G */
    _11_25G,
    _11_5625G,
    _12_5G,
    _10_9375G,
    _12_1875G,      /* for 11.8G, 23.6G, 47.2G */
    _5_625G,
    _5_15625G,      /* for 5G */
    _12_8906G,      /* for 12.5G, 25G, 50G */
    _20_625G,       /* for 40G R2 */
    _25_78125G,     /* for 25G, 50G, 100G */
    _27_5G,         /* for 106G */
    _28_05G,
    _26_5625G,        /* for NRZ mode at 26.5625G,  for PAM4 mode at 53.125G  */
    _28_28125G,       /* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
    _26_5625G_PAM4,   /* for NRZ mode at 26.5625G,  for PAM4 mode at 53.125G  */
    _28_28125G_PAM4,  /* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
    _26_25G,          /* for 102G */
    _27_1G,           /* for 52.5G */

    LAST_MV_HWS_SERDES_SPEED

}MV_HWS_SERDES_SPEED;

/**
* @enum MV_HWS_SERDES_MEDIA
 *
 * @brief Defines the serdes media.
*/
typedef enum{

    XAUI_MEDIA,

    RXAUI_MEDIA

} MV_HWS_SERDES_MEDIA;

/**
* @enum MV_HWS_PORT_FEC_MODE
 *
 * @brief Defines the FEC status.
*/
typedef enum
{
    FEC_NA         = 0,

    FEC_OFF        = (1 << 0),  /* FEC disable */
    FC_FEC         = (1 << 1),  /* Fire Code FEC */
    RS_FEC         = (1 << 2),  /* Reed Solomon FEC (528, 514) */
    RS_FEC_544_514 = (1 << 3),  /* Reed Solomon FEC (544, 514) */

}MV_HWS_PORT_FEC_MODE;

/**
* @enum MV_HWS_SERDES_BUS_WIDTH_ENT
 *
 * @brief Defines the serdes 10Bit status.
*/
typedef enum
{
    _10BIT_ON,
    _10BIT_OFF,
    _10BIT_NA,
    _20BIT_ON,
    _40BIT_ON,
    _80BIT_ON

}MV_HWS_SERDES_BUS_WIDTH_ENT;

/**
* @enum MV_HWS_SERDES_TYPE
 *
 * @brief Defines the different SERDES types.
*/
typedef enum
{
    COM_PHY_H,
    COM_PHY_H_REV2,
    COM_PHY_28NM,
    AVAGO,
#if (!defined(BV_DEV_SUPPORT))
    AVAGO_16NM,
    D2D_PHY_GW16,        /* GW16 500G SERDES */
#endif
    SERDES_LAST

}MV_HWS_SERDES_TYPE;

typedef enum
{
    _156dot25Mhz,
    _78Mhz,
    _25Mhz,
    _125Mhz,
    _312_5Mhz,
    _164_24Mhz
}MV_HWS_REF_CLOCK; /* need to be synchronized with MV_HWS_REF_CLOCK_SUP_VAL enum */

typedef enum
{
    PRIMARY,
    SECONDARY

}MV_HWS_REF_CLOCK_SRC;

/**
* @enum MV_HWS_SERDES_ENCODING_TYPE
 *
 * @brief Defines the different SERDES encoding types.
*/
typedef enum{

    SERDES_ENCODING_NA,

    SERDES_ENCODING_AVAGO_NRZ,

    SERDES_ENCODING_AVAGO_PAM4,

    SERDES_ENCODING_LAST

} MV_HWS_SERDES_ENCODING_TYPE;

/*
 * Typedef: struct MV_HWS_SERDES_CONFIG_STR
 *
 * Description: Defines structure for SERDES power up procedure
 *
 */
typedef struct
{
    MV_HWS_SERDES_TYPE              serdesType;
    MV_HWS_SERDES_SPEED             baudRate;
    MV_HWS_REF_CLOCK                refClock;
    MV_HWS_REF_CLOCK_SRC            refClockSource;
    MV_HWS_SERDES_MEDIA             media;
    MV_HWS_SERDES_BUS_WIDTH_ENT     busWidth;
    MV_HWS_SERDES_ENCODING_TYPE     txEncoding;
    MV_HWS_SERDES_ENCODING_TYPE     rxEncoding;
} MV_HWS_SERDES_CONFIG_STR;
#ifndef BIT_MASK
/* Return the mask including "numOfBits" bits.          */
#define BIT_MASK(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0xFFFFFCFF                     */
#define FIELD_MASK_NOT(offset,len)                      \
        (~(BIT_MASK((len)) << (offset)))

/* Sets the field located at the specified offset & length in data.     */
#define U32_SET_FIELD(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT((offset),(length))) | ((val) <<(offset)))

#endif
#endif /* __mvPortModeElements_H */


