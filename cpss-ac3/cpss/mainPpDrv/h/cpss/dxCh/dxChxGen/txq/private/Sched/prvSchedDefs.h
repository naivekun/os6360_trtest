/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief TM Configuration Library Public Definitions.
 *
* @file tm_defs.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_DEFS_H
#define   	SCHED_DEFS_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedOsRelatedDefinitions.h>





#ifndef MIN
	#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif /* MIN */

#ifndef MAX
	#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif  /* MAX */

#define PER_NODE_SHAPING
/* #define VIRTUAL_LAYER */
#define USE_DEFAULT_CURVES



/*---------------- Constants definitions-------------*/



#define SCHED_MTU          2000   /* bytes */



/** Infinite Profile */
#define SCHED_INF_SHP_PROFILE  0

#ifdef PER_NODE_SHAPING
	#define SCHED_DIRECT_NODE_SHAPING	(uint16_t)(-1) /* read only profile for node, set automatically if node shaping was configured directly */
#endif




/* TM INVALID constants */
/** 32 bit Invalid data indicator */
#define TM_INVAL         0xFFFFFFFF


/** 8 bit Invalid data indicator */
#define TM_INVAL_8_BIT        0xFF

/** 16 bit Invalid data indicator */
#define TM_INVAL_16_BIT        0xFFFF



/* Status constants */
/** Enable indicator */
#define TM_ENABLE  1
/** Disable indicator */
#define TM_DISABLE 0


/** Maximum Bandwidth (in Kbits/sec) */
#define TM_MAX_BW 100000000 /* 100GBit/sec */

/** Maximum Burst (in Kbytes) */
#define TM_MAX_BURST (100 * 1024)

enum
{ 
    /** Eligible function for dequeue disable (the node is not eligible)
    *  - the last function in eligible function array - reserved for queues and nodes**/
	TM_ELIG_DEQ_DISABLE = 63,
	/** The size of  eligible functions array  **/
};


enum  elig_func_node {
    /** Eligible Priority 1 **/
    TM_ELIG_N_PRIO1 = 0,
    /** Eligible function priority 5 **/
    TM_ELIG_N_PRIO5 = 1,
    /** Eligible function shaping **/
    TM_ELIG_N_SHP = 2,
    /** Eligible function min shaping **/
    TM_ELIG_N_MIN_SHP = 3,
    /** Eligible function propagated priority **/
    TM_ELIG_N_PPA = 4,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) **/
    TM_ELIG_N_PPA_SP = 5,
    /** Eligible function propagated with shaping  **/
    TM_ELIG_N_PPA_SHP = 6,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) with min shaper **/
    TM_ELIG_N_PPA_SP_MIN_SHP = 7,
    /** Eligible function propagated priority and shaping with shaper
     * ignore for propagated priority = 3 **/
    TM_ELIG_N_PPA_SHP_IGN = 8,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) with min shaper with shaper
     * ignore for propagated priority = 3 **/
    TM_ELIG_N_PPA_MIN_SHP_SP_IGN = 9,
    /** Node is always eligible in priority 0 **/
    TM_ELIG_N_FP0 = 10,
    /** Node is always eligible in priority 1 **/
    TM_ELIG_N_FP1 = 11,
    /** Node is always eligible in priority 2 **/
    TM_ELIG_N_FP2 = 12,
    /** Node is always eligible in priority 3 **/
    TM_ELIG_N_FP3 = 13,
    /** Node is always eligible in priority 4 **/
    TM_ELIG_N_FP4 = 14,
    /** Node is always eligible in priority 5 **/
    TM_ELIG_N_FP5 = 15,
    /** Node is always eligible in priority 6 **/
    TM_ELIG_N_FP6 = 16,
    /** Node is always eligible in priority 7 **/
    TM_ELIG_N_FP7 = 17,
    /** Node is eligible in priority 0 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP0 = 18,
    /** Node is eligible in priority 1 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP1 = 19,
    /** Node is eligible in priority 2 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP2 = 20,
    /** Node is eligible in priority 3 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP3 = 21,
    /** Node is eligible in priority 4 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP4 = 22,
    /** Node is eligible in priority 5 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP5 = 23,
    /** Node is eligible in priority 6 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP6 = 24,
    /** Node is eligible in priority 7 when minTB is positive **/
    TM_ELIG_N_MIN_SHP_FP7 = 25,
    /** Eligible function propagated with MaxTB include MinTB shaping **/
    TM_ELIG_N_MAX_INC_MIN_SHP = 26,
    /** Node is always eligible in propagated priority **/
    TM_ELIG_N_PP = 27,
    /** Node is eligible when MinTB is positive **/
    TM_ELIG_N_MIN_SHP_PP = 28,
    /** Eligible function with propagated priority and shaping **/
    TM_ELIG_N_PP_SHP = 29,
    /** Eligible function with min max shaping **/
    TM_ELIG_N_SHP_4P_MIN_4P_MAX = 30,
    /** Eligible function with min max shaping, the pp influences
     * which shaper will be used **/
    TM_ELIG_N_SHP_PP_TB = 31,
    /** Eligible function with min max shaping,SchedPrio0 = Prop0(Min)+Prop0..7(Max)
     * SchedPrio1..7 = SchedPrio1..7(Min), PropOut = Prop  **/
    TM_ELIG_N_SHP_PP_MAX_TB_0 = 32,
    /*Eligible only if more then min and less then max ,fixed priority 0)*/
    TM_ELIG_N_MAX_LIM_SHP_FP0 = 33
/** PPA - Propagated priority according to RevA . i.e. - only
 * propagated priority 0...3 are supported. when node gets propagated
 * priority 4...7 it is not eligible **/
};

/** Eligible functions for queue nodes enumerator */
enum elig_func_queue {
    /** Eligible function priority 0 **/
    TM_ELIG_Q_PRIO0 = 0,
    /** Eligible function priority 1 **/
    TM_ELIG_Q_PRIO1 = 1,
    /** Eligible function priority 2 **/
    TM_ELIG_Q_PRIO2 = 2,
    /** Eligible function priority 3 **/
    TM_ELIG_Q_PRIO3 = 3,
    /** Eligible function min shaping priority 0 **/
    TM_ELIG_Q_MIN_SHP_PRIO0 = 4,
    /** Eligible function min shaping priority 1 **/
    TM_ELIG_Q_MIN_SHP_PRIO1 = 5,
    /** Eligible function min shaping priority 2 **/
    TM_ELIG_Q_MIN_SHP_PRIO2 = 6,
    /** Eligible function min shaping priority 3 **/
    TM_ELIG_Q_MIN_SHP_PRIO3 = 7,
    /** Eligible function priority 4 **/
    TM_ELIG_Q_PRIO4 = 8,
    /** Eligible function priority 5 **/
    TM_ELIG_Q_PRIO5 = 9,
    /** Eligible function priority 6 **/
    TM_ELIG_Q_PRIO6 = 10,
    /** Eligible function priority 7 **/
    TM_ELIG_Q_PRIO7 = 11,
    /** Eligible function min shaping priority 4 **/
    TM_ELIG_Q_MIN_SHP_PRIO4 = 12,
    /** Eligible function min shaping priority 5 **/
    TM_ELIG_Q_MIN_SHP_PRIO5 = 13,
    /** Eligible function min shaping priority 6 **/
    TM_ELIG_Q_MIN_SHP_PRIO6 = 14,
    /** Eligible function min shaping priority 7 **/
    TM_ELIG_Q_MIN_SHP_PRIO7 = 15,
    /** Eligible function with shaping **/
    TM_ELIG_Q_SHP = 16,
    /** Eligible function propagated with MaxTB include MinTB shaping **/
    TM_ELIG_Q_MAX_INC_MIN_SHP = 17,
    /** Eligible function min max shaping, scheduling priority 0/0, propagated priority 0/0 **/
    TM_ELIG_Q_SHP_SCHED00_PROP00 = 18,
    /** Eligible function min max shaping, scheduling priority 1/0, propagated priority 1/0 **/
    TM_ELIG_Q_SHP_SCHED10_PROP10 = 19,
    /** Eligible function min max shaping, scheduling priority 2/0, propagated priority 2/0 **/
    TM_ELIG_Q_SHP_SCHED20_PROP20 = 20,
    /** Eligible function min max shaping, scheduling priority 3/0, propagated priority 3/0 **/
    TM_ELIG_Q_SHP_SCHED30_PROP30 = 21,
    /** Eligible function min max shaping, scheduling priority 4/0, propagated priority 4/0 **/
    TM_ELIG_Q_SHP_SCHED40_PROP40 = 22,
    /** Eligible function min max shaping, scheduling priority 5/0, propagated priority 5/0 **/
    TM_ELIG_Q_SHP_SCHED50_PROP50 = 23,
    /** Eligible function min max shaping, scheduling priority 6/0, propagated priority 6/0 **/
    TM_ELIG_Q_SHP_SCHED60_PROP60 = 24,
    /** Eligible function min max shaping, scheduling priority 7/0, propagated priority 7/0 **/
    TM_ELIG_Q_SHP_SCHED70_PROP70 = 25
};


/*---------------------- Enumerated Types---------------- */

/** scheduler levels */
enum schedLevel
{
    Q_LEVEL = 0, /**< Queue Level */
    A_LEVEL,     /**< A-nodes Level */
    B_LEVEL,     /**< B-nodes Level */
    C_LEVEL,     /**< C-nodes Level */
    P_LEVEL      /**< Ports Level */
};

#define	PARENT_LEVEL(level) level+1
#define	CHILD_LEVEL(level) level-1

/** Port's physical bandwidth */

/** Token bucket usage */
enum token_bucket {
    MIN_TOKEN_BUCKET = 0, /**< Use Min token bucket */
    MAX_TOKEN_BUCKET      /**< Use Max tiken bucket */
};



/*------------------- Global Parameters Data structures-----------------*/

/** tm lib init params */
struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC {
    uint32_t schedLibMtu;            /**< tm MTU unit, if 0 then MTU is SCHED_MTU */
    uint32_t  schedLibPdqNum;
    /* ... more tm lib init params here */
}; 

/** Periodic Update Rate configuration params per level */
struct schedPerLevelPeriodicParams 
{
    uint8_t per_state;  /** Periodic shaper update Enable/Disable */
    uint8_t shaper_dec;   /**< Shaper Decoupling Enable/Disable */
    uint32_t per_interval;      /**< interval in which a single level node
                                  * periodic update is performed (has HW reg)*/
};




/** Shared Shaper Profile configuration structure */
struct tm_shaping_profile_params {
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint16_t cbs;    /**< CBS in kbytes */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
    uint16_t ebs;    /**< EBS in kbytes */
};


/*----------------- Nodes Parameters Data Structures---------------*/
/* Note: only drop mode 0 is supported in current version.
 *  Drop profile reference fields are present only for queues and
 *  ports
*/

/** Queue Parameters Data Structure */
struct tm_queue_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< Queue DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
};


/** A-Node Parameters Data Structure */
struct tm_a_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    uint8_t dwrr_priority[8];      /**< DWRR Priority for Queue Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint32_t num_of_children;      /**< Number of children nodes */
};


/** B-Node Parameters Data Structure */
struct tm_b_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    
    uint8_t dwrr_priority[8];      /**< DWRR Priority for A-Node Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** C-Node Parameters Data Structure */
struct tm_c_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    
    uint8_t dwrr_priority[8];      /**< DWRR Priority for B-Node Scheduling */
    
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** Port Parameters Data Structure */
struct tm_port_params {
    uint32_t cir_bw;               /**< CIR BW in Kbits/sec */
    uint32_t eir_bw;               /**< EIR BW in Kbits/sec */
    uint32_t cbs;                  /**< CBS in bytes */
    uint32_t ebs;                  /**< EBS in bytes */
    uint16_t quantum[8];           /**< DWRR Quantum for each instance in TM_PORT_QUANTUM_UNIT */
    
    uint8_t dwrr_priority[8];      /**< DWRR Priority for C-Node Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** Port status data structure */
struct tm_port_status {
    uint32_t max_bucket_level;  /**< Maximal Shaper Bucket level */
    uint32_t min_bucket_level;  /**< Minimal Shaper Bucket level */
    uint32_t deficit[8];        /**< DWRR Deficit per instance */
};


/** Node status data structure */
struct schedNodeStatus {
    uint32_t max_bucket_level;  /**< Maximal Shaper Bucket level */
    uint32_t min_bucket_level;  /**< Minimal Shaper Bucket level */
    uint32_t min_bucket_sign;  /**< Minimal Shaper Bucket sign 0 -pos 1 - neg */
    uint32_t deficit;           /**< DWRR Deficit */
};



/** TM Blocks Error Information */
struct tm_error_info {
    uint16_t error_counter; /**< TM Block Error counter */
    uint16_t exception_counter; /**< TM Block Exception Counter */
};






#ifdef VIRTUAL_LAYER_TO_REMOVE_IN_FUTURE
#else
	/** Reshuffling index/range change structure */
	struct tm_tree_change {
		uint32_t index; /**< Index of changed parent node */
		uint32_t old_index;   /**< Old index/range */
		uint32_t new_index;   /**< New index/range */
		struct tm_tree_change *next; /**< Pointer to the next change */
	};
#endif

/** Eligible Priority Function Data structures */
struct tm_elig_prio_func_out {
	uint8_t max_tb;             /**< Use Max Token Bucket   */
    uint8_t min_tb;             /**< Use Min Token Bucket   */
    uint8_t prop_prio;          /**< Propagated priority    */
    uint8_t sched_prio;         /**< Scheduling priority    */
    uint8_t elig;               /**< Eligibility            */
};

/** Eligible Priority Function storage */
union tm_elig_prio_func{
	struct tm_elig_prio_func_out queue_elig_prio_func[4];		/**< Eligible Priority Functions for queues   */
	struct tm_elig_prio_func_out node_elig_prio_func[8][4];		/**< Eligible Priority Functions for intermediate nodes   */
};




struct tm_quantum_limits /* applicable for port or nodes  quantum limits */
{
    uint32_t minQuantum;	/* minimal possible quantum value in bytes */
    uint32_t maxQuantum;	/* maximal possible quantum value in bytes */
    uint32_t resolution;	/* quantum granularity in bytes*/
};


typedef void * PRV_CPSS_SCHED_HANDLE;


struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC
{
	/* TM tree structure parameters */
	uint16_t	numOfQueues;
	uint16_t	numOfAnodes;
	uint16_t	numOfBnodes;
	uint16_t	numOfCnodes;
	uint16_t	numOfPorts;

};



#endif   /* TM_DEFS_H */

