/*===========================================================================*/
#ifndef exp_intr_PKS_INCLUDED
#define exp_intr_PKS_INCLUDED


#define  SHOSTC_intr_re_naf_not_empty_CNS  	4
#define  SHOSTC_intr_re_naf_almost_full_CNS  	5
#define  SHOSTC_intr_re_UART_tx_CNS  		6
#define  SHOSTC_intr_re_UART_rx_CNS  		9

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!          S T R U C T U R E   T Y P E   D E F I N I T I O N
 *!==========================================================================
 *$ TITLE: the interrupt table stc
 *!--------------------------------------------------------------------------
 *$ FILENAME: u:\work95\host_gen\host_d\h_intr\inc\h_intr.stc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: eyran
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 18-Jun-97  15:55:53         CREATION DATE: 18-Jun-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ INCLUDE REQUIRED:
 *!
 *$ REMARKS:
 *!
 *!**************************************************************************
 *!*/

/*
 * If you define specific data types, constants, and macros for
 * this structure, define them here.
 */


/*! this function gets CIB mutex to prevent dead lock when suspending
	CPU tasks on interrupt */
typedef void SHOSTG_cib_db_lock_or_release_FUN(
	/*!		INPUT			*/

	UINT_32           		lock

	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:            */
);

typedef SHOSTG_cib_db_lock_or_release_FUN *SHOSTG_cib_db_lock_or_release_TYP;
/*===========================================================================*/
/*!*****************************************************************************
*$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
*!*****************************************************************************
*!*/
/*!*****************************************************************************
*$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
*!*****************************************************************************
*!*/
/*===========================================================================*/
/**
* @internal SHOSTG_go_intr_go function
* @endinternal
*
*/

extern void SHOSTG_go_intr_go (

    void
);
/*$ END OF SHOSTG_go_intr_go */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_set_interrupt function
* @endinternal
*
*/

extern UINT_32  SHOSTG_set_interrupt (

    /*!     INPUTS:             */

    UINT_32 intr
);
/*$ END OF SHOSTG_set_interrupt */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_init_intr_table function
* @endinternal
*
*/

extern void SHOSTC_init_intr_table (

    void
);
/*$ END OF SHOSTC_init_intr_table */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_bind_interrupt function
* @endinternal
*
*/

extern UINT_32 SHOSTG_bind_interrupt (

    /*!     INPUTS:             */

    UINT_32 priority,

    UINT_32 intr,

    void *fun,

    HANDLE ev
);
/*$ END OF SHOSTG_bind_interrupt */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_bind_interrupt_with_param function
* @endinternal
*
*/

extern UINT_32 SHOSTG_bind_interrupt_with_param (

    /*!     INPUTS:             */

    UINT_32 priority,

    UINT_32 intr,

    void *fun,

    HANDLE ev,

    UINT_32 param
);
/*$ END OF SHOSTG_bind_interrupt_with_param */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_unbind_interrupt_with_param function
* @endinternal
*
*/

extern UINT_32 SHOSTG_unbind_interrupt_with_param (

    /*!     INPUTS:             */

    UINT_32 intr
);
/*$ END OF SHOSTG_unbind_interrupt_with_param */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_interrupt_disable function
* @endinternal
*
*/

extern  UINT_32 SHOSTG_interrupt_disable (void)
;
/*$ END OF SHOSTG_interrupt_disable */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_interrupt_enable function
* @endinternal
*
*/

extern  void  SHOSTG_interrupt_enable (

    UINT_32     new_val
);
/*$ END OF SHOSTG_interrupt_enable */

/*===========================================================================*/
/*===========================================================================*/
/*!*****************************************************RND Template version 4.0
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
 *!=============================================================================
 *$ TITLE: Check if we are inside an interrupt routine
 *!-----------------------------------------------------------------------------
 *$ FILENAME: c:\temp\radlan\gen\intr\EXP\chk_int.fud
 *!-----------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!-----------------------------------------------------------------------------
 *$ AUTHORS: eyran,BoazK,oren-v
 *!-----------------------------------------------------------------------------
 *$ LATEST UPDATE:                       CREATION DATE: 09-Dec-92
 *!*****************************************************************************
 *!
 *!*****************************************************************************
 *!
 *$ FUNCTION: SHOSTG_check_if_into_interrupt
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *!    TRUE if we are inside an interrupt.
 *!    FALSE if not.
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!*****************************************************************************
 *!*/

extern unsigned int SHOSTG_check_if_into_interrupt (
    void
);
/*$ END OF SHOSTG_check_if_into_interrupt */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_interrupt_enable_one function
* @endinternal
*
*/

extern void SHOSTG_interrupt_enable_one (

    /*!     INPUTS:             */

    UINT_32    intr
);
/*$ END OF SHOSTG_interrupt_enable_one */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_interrupt_disable_one function
* @endinternal
*
*/

extern void SHOSTG_interrupt_disable_one (

    /*!     INPUTS:             */

    UINT_32    intr
);
/*$ END OF SHOSTG_interrupt_disable_one */

/*===========================================================================*/

#endif
/*$ END OF intr */

