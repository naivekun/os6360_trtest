/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief declaration of internal TM structures - platform_dependent parts.
 *
* @file tm_core_types_platform.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_CORE_TYPES_PLATFORM_H
#define   	SCHED_CORE_TYPES_PLATFORM_H


#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>





struct level_config
{
	/* ...PerConf register content */
    uint8_t shaping_status;     /**< shaping status for level (enabled/disabled)*/
    uint32_t per_interval;      /**< interval in which a single level node
                                  * periodic update is performed (has HW reg)*/
    uint8_t token_res_exp; /**< tokens resolution exponent */
    uint8_t shaper_dec;         /**< Shaper Decoupling Enable/Disable */
    /**< Periodic rate shaping parameters  - ...PerRateShpPrms content*/
    uint16_t L;
    uint16_t K;
    uint16_t N;
    /* scrubbing slots allocation ScrubSlotAlloc register content*/
    uint8_t scrub_slots;
	/* private registerless variables */
	uint32_t unit;         /* minimal b/w in case of max dividers & token=1 */
	/* only for testing */
	uint32_t level_update_frequency; /**< periodic update rate per level */
};


/** Shaping Profile
 */
struct prvCpssDxChTxqSchedShapingProfile
{
    enum schedLevel level;  
    uint16_t min_token;
    uint16_t max_token;
    uint16_t min_burst_size;/**< CBS in kbytes */
    uint16_t max_burst_size;/**< EBS in kbytes */
    uint8_t min_div_exp;
    uint8_t min_token_res;
    uint8_t max_div_exp;
    uint8_t max_token_res;
    uint32_t use_counter;
    /* for using in read_profile()... */
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
} __ATTRIBUTE_PACKED__;

/* constant is used for  infinite shaping profile , which is valid for all levels */
#define	ALL_LEVELS ((enum schedLevel)(-1))


#endif   /* TM_TYPES_H */

