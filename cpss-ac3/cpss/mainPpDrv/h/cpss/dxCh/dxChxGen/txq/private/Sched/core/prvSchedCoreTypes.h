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
 * @brief declaration of internal TM structures.
 *
* @file tm_core_types.h
*
* $Revision: 2.0 $
 */

#ifndef     SCHED_CORE_TYPES_H
#define     SCHED_CORE_TYPES_H

#include <errno.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>

#ifdef VIRTUAL_LAYER
	/** all macro definitions are in file virtual_layer_core.h*/
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedVirtualLayerCore.h>
#else
	/** here are dummy definitions for case without logical layer*/
	#define	CONVERT_TO_PHYSICAL(level,index) /* do nothing*/
	#define	CONVERT_TO_VIRTUAL(level,index)  /* do nothing*/
	#define	GENERATE_VIRTUAL_ID(level,index) /* do nothing*/
	#define	REMOVE_VIRTUAL_ID(level,index) /* do nothing*/
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define	TM_ELIG_FUNC_TABLE_SIZE	64

#define	TM_NODE_DISABLED_FUN		62

#define VALIDATE_ELIG_FUNCTION(elig_fun) \
    if ((elig_fun >= TM_ELIG_FUNC_TABLE_SIZE) || (elig_fun == TM_NODE_DISABLED_FUN))\
	{ /* maximum function id 0..63 */\
        rc = TM_CONF_ELIG_PRIO_FUNC_ID_OOR;\
        goto out;\
    }\



#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>



#define TM_128M_kB 128*1024  	/*in Kb units*/
#define TM_4M_kB   4*1024 		/*in Kb units*/


#define TM_1K   1024 

/** Delay Size Multiplier (used to calculate TD Threshold)  
 */
#define TM_DELAY_SIZE_MULT   1


/* Physical port capacity (speed in Kbits) */
#define TM_1G_SPEED   1000000  /* 1050000000UL */   /* 1G + 5% */
#define TM_2HG_SPEED  2500000  /* 2625000000UL */   /* 2.5G + 5% */
#define TM_10G_SPEED  10000000 /* 10500000000ULL */ /* 10G + 5% */
#define TM_40G_SPEED  40000000 /* 42000000000ULL */ /* 40G + 5% */
#define TM_50G_SPEED  50000000
#define TM_100G_SPEED 100000000

#define TM_MAX_SPEED TM_100G_SPEED


/*********************************/
/* Internal Databases Structures */
/*********************************/



/** Global arrays structures definitions */

/** Shaping Profile is inside plapform-dependent part
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedCoreTypesPlatform.h>




/** Range of children nodes in symetric distribution 
 */
struct ranges {
    uint32_t norm_range[C_LEVEL+1];
    uint32_t last_range[C_LEVEL+1];
};


struct dwrr_port_quantum_t {
    uint16_t quantum;/*9 bits actually */
};


typedef enum 
{
	TM_NODE_FREE=-1,
	TM_NODE_USED=0,	
	TM_NODE_RESERVED=1,
	TM_NODE_DELETED=2,
}tm_node_status_t;

struct queue_mapping_t
{
	tm_node_status_t	nodeStatus;
    uint16_t 			nodeParent;
};

struct node_mapping_t
{
	tm_node_status_t	nodeStatus;
    uint16_t 			nodeParent;
    int 				childLo;
    int 				childHi;
};

struct port_mapping_t
{
	tm_node_status_t	nodeStatus;
    int 				childLo;
    int 				childHi;
};


/** Queue data structure 
 */
struct tm_queue
{
    struct queue_mapping_t      mapping;
    uint8_t elig_prio_func/*:6*/;               /*6 bits actually */
    /* this value is used for debugging moving nodes/ reshuffling  */ 
    uint8_t node_temporary_disabled/*:1*/;      /*1 bit actually */
    
    uint8_t installed/*:1*/;                    /*1 bit actually */
    uint16_t shaping_profile_ref;
    uint16_t dwrr_quantum/*:14*/;               /*14 bits actually */
#ifdef PER_NODE_SHAPING
   /* for using in read_node_shaping() */
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
  uint32_t eir_bw;   /**< EIR BW in Kbits/sec */
    uint16_t cbs;/**< CBS in kbytes */
    uint16_t ebs;/**< EBS in kbytes */
#endif
}__ATTRIBUTE_PACKED__;


/** A-node data structure 
 */
struct tm_a_node
{
	struct node_mapping_t		mapping;
    uint8_t elig_prio_func/*:6*/;						/*6 bits actually */
    /* this value is used for debugging moving nodes/ reshuffling  */ 
    uint8_t node_temporary_disabled/*:1*/;				/*1 bit actually */
    uint16_t shaping_profile_ref;
    uint16_t dwrr_quantum/*:14*/;						/*14 bits actually */
    uint8_t dwrr_enable_per_priority_mask; 
    
   
#ifdef PER_NODE_SHAPING
   /* for using in read_node_shaping() */
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
    uint16_t cbs;/**< CBS in kbytes */
    uint16_t ebs;/**< EBS in kbytes */
#endif
}__ATTRIBUTE_PACKED__;


/** B-node data structure 
 */
struct tm_b_node
{
	struct node_mapping_t		mapping;
    uint8_t elig_prio_func/*:6*/;					/*6 bits actually */
    /* this value is used for debugging moving nodes/ reshuffling  */ 
    uint8_t node_temporary_disabled/*:1*/;			/*1 bit actually */
 
    uint16_t shaping_profile_ref;
    uint16_t dwrr_quantum/*:14*/;					/*14 bits actually */
    uint8_t dwrr_enable_per_priority_mask;
    
#ifdef PER_NODE_SHAPING
   /* for using in read_node_shaping() */
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
    uint16_t cbs;/**< CBS in kbytes */
    uint16_t ebs;/**< EBS in kbytes */
#endif
}__ATTRIBUTE_PACKED__;


/** C-node data structure 
 */
struct tm_c_node 
{
	struct node_mapping_t		mapping;
    uint8_t elig_prio_func/*:6*/;			/*6 bits actually */
    /* this value is used for debugging moving nodes/ reshuffling  */ 
    uint8_t node_temporary_disabled/*:1*/;  /*1 bit actually */
	
   
    uint16_t shaping_profile_ref;
    uint16_t dwrr_quantum/*:14*/;			/*14 bits actually */
    uint8_t dwrr_enable_per_priority_mask;
    
#ifdef PER_NODE_SHAPING
   /* for using in read_node_shaping() */
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
    uint16_t cbs;/**< CBS in kbytes */
    uint16_t ebs;/**< EBS in kbytes */
#endif
}__ATTRIBUTE_PACKED__;


/** Port data structure 
 */
struct tm_port
{
	struct port_mapping_t		mapping;
    uint8_t elig_prio_func/*:6*/;				/*6 bits actually */
    /* this bit is used for debugging moving nodes/ reshuffling  */ 
    uint8_t node_temporary_disabled/*:1*/;		/*1 bit actually */
 
    struct dwrr_port_quantum_t dwrr_quantum_per_level[8];
    uint8_t dwrr_enable_per_priority_mask;
    

    /* Shaping config */
    uint16_t cir_token;
    uint16_t eir_token;
    uint16_t periods;
    uint8_t min_token_res;
    uint8_t max_token_res;
    uint32_t cir_burst_size;/**< CBS in kbytes */
    uint32_t eir_burst_size;/**< EBS in kbytes */

    struct ranges children_range;   /* for symetric distribution only */
    uint8_t sym_mode/*:1*/;         /* assymetric/symetric tree */ /*1 bit actually */
    
    /*save the values as configured for usage in read api*/
    uint32_t cir_sw;                  /**< Cir in Kbit/sec */
    uint32_t eir_sw;                  /**< EBS in Kbit/sec */


}__ATTRIBUTE_PACKED__;



/* Eligible Priority Functions structures */

struct tm_elig_prio_func_node_entry {
    uint16_t func_out[4];
};


/* Eligible Priority Functions Definitions */

struct tm_elig_prio_func_node
{
    struct tm_elig_prio_func_node_entry tbl_entry[8];
};

struct tm_elig_prio_func_queue
{
    struct tm_elig_prio_func_node_entry tbl_entry;
};



struct prvCpssDxChTxqSchedShadow 
{
  uint32_t magic;
  PRV_CPSS_SCHED_HANDLE rm;                  /**< rm hndl */    

  /* Nodes arrays */
  uint32_t          tm_total_queues;
  uint32_t          tm_total_a_nodes;
  uint32_t          tm_total_b_nodes;
  uint32_t          tm_total_c_nodes;
  uint32_t          tm_total_ports;
  struct tm_queue   *tm_queue_array;
  struct tm_a_node  *tm_a_node_array;
  struct tm_b_node  *tm_b_node_array;
  struct tm_c_node  *tm_c_node_array;
  struct tm_port    *tm_port_array;
  
  /* Global arrays */
  struct prvCpssDxChTxqSchedShapingProfile *tm_shaping_profiles;

  
   /*--------------*/
  
  /* Eligible Priority Function Table arrays */
  struct tm_elig_prio_func_node tm_elig_prio_a_lvl_tbl[TM_ELIG_FUNC_TABLE_SIZE];
  struct tm_elig_prio_func_node tm_elig_prio_b_lvl_tbl[TM_ELIG_FUNC_TABLE_SIZE];
  struct tm_elig_prio_func_node tm_elig_prio_c_lvl_tbl[TM_ELIG_FUNC_TABLE_SIZE];
  struct tm_elig_prio_func_node tm_elig_prio_p_lvl_tbl[TM_ELIG_FUNC_TABLE_SIZE];
  struct tm_elig_prio_func_queue tm_elig_prio_q_lvl_tbl[TM_ELIG_FUNC_TABLE_SIZE];


  /* Scheduling parameters */
  uint32_t freq;                 /**< LAD frequency */
  /* global update states */
  uint8_t periodic_scheme_state; /**< periodic scheme updated/not updated */

  
  /* per level data including port level */
  struct level_config level_data[P_LEVEL+1];
  /* Tree data */
  uint8_t tree_deq_status;       /**< tree DeQ status */
  uint8_t tree_dwrr_enable_per_priority_mask;    /**< tree DWRR priority bitmap for port scheduling */

#ifdef VIRTUAL_LAYER
#else
      /* Reshuffling changes */
      struct tm_tree_change list;
#endif
  
  /* Other registers */
  uint8_t port_ext_bp_en;
  uint8_t dwrr_bytes_burst_limit;
  uint16_t min_pkg_size;         /**<  minimum package size */
  uint8_t port_ch_emit;          /**<  port chunks emitted per scheduler selection */


  /* environment */
  PRV_CPSS_SCHED_HANDLE hEnv;
#ifdef VIRTUAL_LAYER
  /* virtual layer handle */
  PRV_CPSS_SCHED_HANDLE hVirtualLayer;
#endif
   
  struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC init_params;

  /*  quantum limits calculated  according to init parameters  - calculated during libInit*/
  uint32_t  min_port_quantum_bytes;
  uint32_t  max_port_quantum_bytes;
  uint32_t  min_node_quantum_bytes;
  uint32_t  max_node_quantum_bytes;
  /* constant values , defined by platform */
  uint32_t  node_quantum_chunk_size_bytes;
  uint32_t  port_quantum_chunk_size_bytes;

};



/** Internal TM control structure 
 */

#define TM_MAGIC 0x24051974
/* following macro declares and checks validity of tmctl */
#define DECLARE_TM_CTL_PTR(name,value)	struct prvCpssDxChTxqSchedShadow * name= (struct prvCpssDxChTxqSchedShadow *)value;

#define CHECK_TM_CTL_PTR(ptr)	\
	if (!ptr) return -EINVAL;\
    if (ptr->magic != TM_MAGIC) return -EBADF;

#define TM_CTL(name,handle)	DECLARE_TM_CTL_PTR(name,handle);CHECK_TM_CTL_PTR(name);


#define TM_ENV(var) var->hEnv


#define TM_ENV_PDQ_NUM(var) ((struct cpssSchedEnv *)var->hEnv)->pdqNumber




#ifdef __cplusplus
}
#endif


#endif   /* TM_TYPES_H */

