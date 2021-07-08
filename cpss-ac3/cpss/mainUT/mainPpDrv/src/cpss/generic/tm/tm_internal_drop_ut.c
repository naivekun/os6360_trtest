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
 * @brief Resource Manager UT functions implementation.
 *
* @file rm_ut.c
*
* $Revision: 1 $
 */

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <cpss/generic/tm/tm_internal_ut_defs.h>
#include <stdio.h>
#include "tm_core_types.h"
#include "tm_ctl.h"
#include "tm_drop.h"

#define TM_UT_INTERNAL_USE_DP_NUM 1

struct drop_profiles_ut_stc
{
    char        *dp_name_ptr;
    int         dp_max; /* reduce default dp */
    int         level;  /* enum tm_level */
/*    uint8_t     cos; */
    int			cos;
    char        *bad_param_msg_ptr;
	uint16_t   *dp_ut_indexes_arr;
	int		   dp_ut_index;

	uint16_t   *dp_ut_aging_indexes_arr;
	int		   dp_ut_aging_index;
	int		   dp_ut_aging_ranges;
};

 
static int tm_drop_profile_alloc_ut(struct tm_ctl                   *tm_ctl_PTR, 
                                    struct tm_drop_profile_params   *dp_params_ptr,
                                    struct drop_profiles_ut_stc     *dp_arr,
                                    int                             dp_arr_size,
                                    int                             interval)
{
    int         i,j,rc;
    char        msg[128];
    uint16_t    dp_index = 0;

    for (i = 0; i < dp_arr_size; i++)  
    {
        sprintf(msg, "%s: dp: %s cos: %d", "alloc max profiles", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 
        for (j = 0; j < dp_arr[i].dp_max; j+=interval) 
        {
            rc = tm_create_drop_profile(tm_ctl_PTR,
                                        dp_arr[i].level,
                                        (uint8_t)dp_arr[i].cos,
                                        dp_params_ptr,
                                        &dp_index);

            tm_ut_verify_equal_mac(0, rc, msg);

			dp_arr[i].dp_ut_indexes_arr[j] = dp_index;
        }

		if (interval == 1)
			dp_arr[i].dp_ut_index = j;

        /* verify fail on another alloc */
        sprintf(msg, "%s: dp: %s cos: %d", "no free drop profiles", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 
        rc = tm_create_drop_profile(tm_ctl_PTR,
                                    dp_arr[i].level,
                                    (uint8_t)dp_arr[i].cos,
                                    dp_params_ptr,
                                    &dp_index);

        tm_ut_verify_not_equal_mac(0, rc, msg);
    }

    return 0;
}

static int tm_drop_profile_delete_ut(struct tm_ctl                  *tm_ctl_PTR, 
                                     struct drop_profiles_ut_stc    *dp_arr,
                                     int                            dp_arr_size,
                                     uint16_t                       interval)
{
    int         i,rc;
    char        msg[128];
    uint16_t    dp_index = 0;
    uint16_t	j;

    for (i = 0; i < dp_arr_size; i++)
    {
        sprintf(msg, "%s: dp: %s cos: %d", "delete drop profiles", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 
		for (j = 0; j < dp_arr[i].dp_max; j+=interval) 
        {
			dp_index = dp_arr[i].dp_ut_indexes_arr[j];

            rc = tm_delete_drop_profile(tm_ctl_PTR,
                                        dp_arr[i].level,
                                        (uint8_t)dp_arr[i].cos,
                                        dp_index);

            tm_ut_verify_equal_mac(0, rc, msg);

        }

        sprintf(msg, "%s: dp: %s cos: %d", "delete wrong drop profile", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 
        rc = tm_delete_drop_profile(tm_ctl_PTR,
                                    dp_arr[i].level,
                                    (uint8_t)dp_arr[i].cos,
                                    j+1);

        tm_ut_verify_not_equal_mac(0, rc, msg);
    }

    return 0;
}


static int tm_drop_profile_alloc_bad_params_ut(struct tm_ctl                    *tm_ctl_PTR, 
                                               struct tm_drop_profile_params    *dp_params_ptr,
                                               struct drop_profiles_ut_stc      *dp_arr,
                                               int                              dp_arr_size)
{
    int         i,rc;
    char        msg[128];
    uint16_t    dp_index = 0;

    for (i = 0; i < dp_arr_size; i++)
    {
        sprintf(msg, "%s: dp: %s cos: %d", "drop profile alloc with bad params", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 

        rc = tm_create_drop_profile(tm_ctl_PTR,
                                    dp_arr[i].level,
                                    (uint8_t)dp_arr[i].cos,
                                    dp_params_ptr,
                                    &dp_index);

         tm_ut_verify_not_equal_mac(0, rc, msg);

    }

    return 0;
}

static int tm_drop_profile_delete_bad_params_ut(struct tm_ctl               *tm_ctl_PTR, 
                                                struct drop_profiles_ut_stc *dp_arr,
                                                int                         dp_arr_size)
{
    int         i,rc;
    char        msg[128];

    for (i = 0; i < dp_arr_size; i++)
    {
        sprintf(msg, "%s: dp: %s cos: %d", "drop profile delete with bad params", dp_arr[i].dp_name_ptr, dp_arr[i].cos); 
        rc = tm_delete_drop_profile(tm_ctl_PTR,
                                    dp_arr[i].level,
                                    (uint8_t)dp_arr[i].cos,
                                    1);

        tm_ut_verify_not_equal_mac(0, rc, msg);
    }

    return 0;
}

static int tm_aging_drop_profile_alloc_ut(struct tm_ctl                   *tm_ctl_PTR, 
										  struct tm_drop_profile_params   profile_arr[TM_AGING_PROFILES],
										  struct drop_profiles_ut_stc     *dp_ut_params_ptr,										  
                                          int                             interval)
{
    int         i,j,rc;
    char        msg[128];
    uint16_t    dp_index = 0;
	
    sprintf(msg, "%s: dp: %s", "alloc max aging profiles", dp_ut_params_ptr->dp_name_ptr); 
    for (i = 0, j = 0; j < dp_ut_params_ptr->dp_ut_aging_ranges; j += interval, i += interval) 
    {
		dp_index = 0;
        rc = tm_create_aging_profile(tm_ctl_PTR,
                                     profile_arr,
	                                 &dp_index);

        tm_ut_verify_equal_mac(0, rc, msg);

		dp_ut_params_ptr->dp_ut_aging_indexes_arr[i] = dp_index;		
    }

	if (interval == 1)
		dp_ut_params_ptr->dp_ut_aging_index = i; /*dp_ut_params_ptr->dp_max / TM_AGING_PROFILES;*/

    /* verify fail on another alloc */
    sprintf(msg, "%s: dp: %s", "no free aging drop profiles", dp_ut_params_ptr->dp_name_ptr); 
	rc = tm_create_aging_profile(tm_ctl_PTR,
		                         profile_arr,
		                         &dp_index);

    tm_ut_verify_not_equal_mac(0, rc, msg);
    

    return 0;
}

static int tm_aging_drop_profile_delete_ut(struct tm_ctl				*tm_ctl_PTR, 
										   struct drop_profiles_ut_stc	*dp_ut_params_ptr,										  
								           uint16_t						interval)
{
	int         j,rc;
	char        msg[128];
	uint16_t    dp_index = 0;

	sprintf(msg, "%s: dp: %s", "delete max aging profiles", dp_ut_params_ptr->dp_name_ptr); 
	for (j = 0; j < dp_ut_params_ptr->dp_ut_aging_ranges; j += interval) 
	{
		dp_index = dp_ut_params_ptr->dp_ut_aging_indexes_arr[j];

		rc = tm_delete_aging_profile(tm_ctl_PTR,
									 dp_index);

		tm_ut_verify_equal_mac(0, rc, msg);

		dp_ut_params_ptr->dp_ut_aging_indexes_arr[j] = 0;
	}

    sprintf(msg, "%s: dp: %s", "delete wrong aging profile", dp_ut_params_ptr->dp_name_ptr); 
	rc = tm_delete_aging_profile(tm_ctl_PTR,
							     dp_index + interval);

    tm_ut_verify_not_equal_mac(0, rc, msg);

    return 0;
}


static int tm_mixed_drop_profile_alloc_ut(struct tm_ctl                   *tm_ctl_PTR, 
										  struct tm_drop_profile_params   profile_arr[TM_AGING_PROFILES],
										  struct drop_profiles_ut_stc     *dp_ut_params_ptr)										  
{
	int         jj,kk,j,dp_nums,rc;
	char        msg[128];
	uint16_t    dp_index = 0;


	sprintf(msg, "%s: dp: %s", "alloc mixed aging profiles", dp_ut_params_ptr->dp_name_ptr); 
	for (jj = 0, kk = 0, j = 0; j < dp_ut_params_ptr->dp_max;)
	{
		dp_index = 0;

		dp_nums = 1;
		if (j % 4 == 0)  /* creating sequential DPs*/
		{
			dp_nums = 2;
		}

		while (dp_nums > 0 && j < dp_ut_params_ptr->dp_max)
		{
			rc = tm_create_drop_profile(tm_ctl_PTR,
										dp_ut_params_ptr->level,
										(uint8_t)(dp_ut_params_ptr->cos),
										&profile_arr[0],
										&dp_index);

			tm_ut_verify_equal_mac(0, rc, msg);
			dp_ut_params_ptr->dp_ut_indexes_arr[jj++] = dp_index;
			dp_nums--;
			j++;
		}

		dp_nums = 1;
		if (j % 3 == 0)  /* creating sequential aging DPs*/
		{
			dp_nums = 2;
		}

		while (dp_nums > 0 && j < dp_ut_params_ptr->dp_max)
		{		
			rc = tm_create_aging_profile(tm_ctl_PTR,
										 profile_arr,
										 &dp_index);

			tm_ut_verify_equal_mac(0, rc, msg);

			dp_ut_params_ptr->dp_ut_aging_indexes_arr[kk++] = dp_index;		
			dp_nums--;
			j += TM_AGING_PROFILES;
		}
	}

	dp_ut_params_ptr->dp_ut_index = jj;
	dp_ut_params_ptr->dp_ut_aging_index = kk;

	return 0;
}

static int tm_mixed_drop_profile_delete_ut(struct tm_ctl                   *tm_ctl_PTR, 
										   struct drop_profiles_ut_stc     *dp_ut_params_ptr)
{
	int			i,max_dp,rc;
	char        msg[128];
	uint16_t	dp_index = 0;

	sprintf(msg, "%s: dp: %s", "delete mixed aging profiles", dp_ut_params_ptr->dp_name_ptr); 

	/* try to delete aging DP with rgular DP*/
	for (i = 0; i < dp_ut_params_ptr->dp_ut_aging_index; i += 2)
	{
		dp_index = dp_ut_params_ptr->dp_ut_aging_indexes_arr[i]; 

		rc = tm_delete_drop_profile(tm_ctl_PTR,
									dp_ut_params_ptr->level,
									(uint8_t)(dp_ut_params_ptr->cos),
									dp_index);

		tm_ut_verify_not_equal_mac(0, rc, msg);

	}

	/* try to delete DP with aging DP*/
	for (i = 0; i < dp_ut_params_ptr->dp_ut_index; i += 2)
	{
		dp_index = dp_ut_params_ptr->dp_ut_indexes_arr[i]; 

		rc = tm_delete_aging_profile(tm_ctl_PTR,
								     dp_index);

		tm_ut_verify_not_equal_mac(0, rc, msg);

	}

	if (dp_ut_params_ptr->dp_ut_aging_index > dp_ut_params_ptr->dp_ut_index)
		max_dp = dp_ut_params_ptr->dp_ut_aging_index;
	else
		max_dp = dp_ut_params_ptr->dp_ut_index;

	for (i = 0; i < max_dp; i++)
	{
		if (i < dp_ut_params_ptr->dp_ut_index)
		{
			dp_index = dp_ut_params_ptr->dp_ut_indexes_arr[i]; 

			rc = tm_delete_drop_profile(tm_ctl_PTR,
										dp_ut_params_ptr->level,
										(uint8_t)(dp_ut_params_ptr->cos),
										dp_index);

			tm_ut_verify_equal_mac(0, rc, msg);
		}

		if (i < dp_ut_params_ptr->dp_ut_aging_index)
		{
			dp_index = dp_ut_params_ptr->dp_ut_aging_indexes_arr[i]; 

			rc = tm_delete_aging_profile(tm_ctl_PTR,
										 dp_index);

			tm_ut_verify_equal_mac(0, rc, msg);
		}
	}

	dp_ut_params_ptr->dp_ut_index = 0;
	dp_ut_params_ptr->dp_ut_aging_index = 0;

	return 0;
}



int tm_drop_profiles_ut(uint8_t devNum, int num_of_iterations)
{
    tm_handle       tm_hndl_PTR = 0;
    struct tm_ctl   *tm_ctl_PTR = NULL;
    char            test_name[128];
    int 			rc,iteration,i;

	/* because TM_NUM_PORT_DROP_PROF is not constant  but alias of the function get_tm_port_count() 
	   we can't use it for static table initialization -  need postprocessing */
    struct drop_profiles_ut_stc dp_arr[]=
    {
        {"QUEUE_DROP_PROF"           , TM_NUM_QUEUE_DROP_PROF , Q_LEVEL,  TM_INVAL},
        {"A_NODE_DROP_PROF"          , TM_NUM_A_NODE_DROP_PROF, A_LEVEL,  TM_INVAL}, 
        {"B_NODE_DROP_PROF"          , TM_NUM_B_NODE_DROP_PROF, B_LEVEL,  TM_INVAL}, 
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  0       },  
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  1       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  2       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  3       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  4       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  5       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  6       },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  7       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  0       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  1       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  2       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  3       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  4       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  5       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  6       },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  7       },
        {"PORT_DROP_PROF"            , 0 /*TM_NUM_PORT_DROP_PROF*/    , P_LEVEL,  TM_INVAL}
    };
    struct drop_profiles_ut_stc dp_arr_bad_params[]=
    {

        {"QUEUE_DROP_PROF"           , TM_NUM_QUEUE_DROP_PROF , 200    ,  TM_INVAL, "level"    },
        {"A_NODE_DROP_PROF"          , TM_NUM_A_NODE_DROP_PROF, -1     ,  TM_INVAL, "level"    },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  TM_INVAL, "cos INVAL"},
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  10      , "cos 10"   },
        {"C_NODE_DROP_PER_COS_PROF"  , TM_NUM_C_NODE_DROP_PROF, C_LEVEL,  -2      , "cos -2"   },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/  , P_LEVEL,  10      , "cos 10"   },
        {"PORT_DROP_PER_COS_PROF"    , 0 /*TM_NUM_PORT_DROP_PROF*/  , P_LEVEL,  -2      , "cos -2"   }
    };

    int dp_arr_size = sizeof(dp_arr) / sizeof(dp_arr[0]); 
    int dp_arr_bad_params_size = sizeof(dp_arr_bad_params) / sizeof(dp_arr_bad_params[0]); 

    struct tm_drop_profile_params dp_params = {0}; 
 
	dp_params.drop_mode = DISABLED;
	dp_params.cbtdBytes = DROP_BW_KBITS_TO_BYTES(200000);

    rc = cpssTmTestTmCtlLibInit(devNum);
    tm_ut_verify_equal_mac(0, rc, "cpssTmTestTmCtlLibInit");

    tm_hndl_PTR = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;
	
    tm_ctl_PTR = (struct tm_ctl*)tm_hndl_PTR;
	tm_ut_verify_tm_lib_hndl_mac(tm_ctl_PTR);

	/* postprocessing :TM_NUM_PORT_DROP_PROF assignment*/
	for (i = 11 ; i < 20 ; i++)	dp_arr[i].dp_max = get_tm_port_count(TM_ENV(tm_ctl_PTR));
	for (i = 5 ; i < 7 ; i++)	dp_arr_bad_params[i].dp_max = get_tm_port_count(TM_ENV(tm_ctl_PTR));
	/* allocate dp indexes arrays */
    for (i = 0; i < dp_arr_size; i++)
    {
        dp_arr[i].dp_max -= TM_UT_INTERNAL_USE_DP_NUM;
		dp_arr[i].dp_ut_indexes_arr = (uint16_t*)tm_malloc(sizeof(uint16_t) * dp_arr[i].dp_max);
    }

    for (i = 0; i < dp_arr_bad_params_size; i++)
    {
        dp_arr_bad_params[i].dp_max -= TM_UT_INTERNAL_USE_DP_NUM;
    }


/*    rm_ctl_PTR = (struct rmctl*)tm_ctl_PTR->rm; */

	for (iteration = 1; iteration <= num_of_iterations; iteration++)
	{

        sprintf(test_name, "%s: iteration: %d", "drop profiles alloc", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_alloc_ut(tm_ctl_PTR, &dp_params, dp_arr, dp_arr_size, 1),
                              test_name);


        sprintf(test_name, "%s: iteration: %d", "drop profiles delete", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_delete_ut(tm_ctl_PTR, dp_arr, dp_arr_size, 1),
                              test_name);

        sprintf(test_name, "%s: iteration: %d", "drop profiles alloc with bad params", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_alloc_bad_params_ut(tm_ctl_PTR, &dp_params, dp_arr_bad_params, dp_arr_bad_params_size),
                              test_name);

        sprintf(test_name, "%s: iteration: %d", "drop profiles alloc after free", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_alloc_ut(tm_ctl_PTR, &dp_params, dp_arr, dp_arr_size, 1),
                              test_name);


        sprintf(test_name, "%s: iteration: %d", "drop profiles delete with bad params", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_delete_bad_params_ut(tm_ctl_PTR, dp_arr_bad_params, dp_arr_bad_params_size),
                              test_name);

        sprintf(test_name, "%s: iteration: %d", "drop profiles delete with intervals", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_delete_ut(tm_ctl_PTR, dp_arr, dp_arr_size, 3),
                              test_name);

        sprintf(test_name, "%s: iteration: %d", "drop profiles realloc deleted intervals", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_alloc_ut(tm_ctl_PTR, &dp_params, dp_arr, dp_arr_size, 3),
                              test_name);


        sprintf(test_name, "%s: iteration: %d", "drop profiles delete", iteration);
        tm_ut_verify_test_start(test_name);        
        tm_ut_verify_test_mac(0,
                              tm_drop_profile_delete_ut(tm_ctl_PTR, dp_arr, dp_arr_size, 1),
                              test_name);
	}

	for (i = 0; i < dp_arr_size; i++)
	{
		tm_free(dp_arr[i].dp_ut_indexes_arr);
	}

    return 0;
}

int tm_aging_drop_profiles_ut(uint8_t devNum, int num_of_iterations)
{
	tm_handle       tm_hndl_PTR = 0;
	struct tm_ctl   *tm_ctl_PTR = NULL;
	char            test_name[128];
	int 			rc,iteration,i;

	struct drop_profiles_ut_stc dp_aging = {"QUEUE_DROP_AGING_PROF", TM_NUM_QUEUE_DROP_PROF-TM_UT_INTERNAL_USE_DP_NUM, Q_LEVEL, TM_INVAL};

	struct tm_drop_profile_params dp_params_arr[TM_AGING_PROFILES];

	tm_memset(dp_params_arr, 0, sizeof(dp_params_arr));

	for (i = 0; i < TM_AGING_PROFILES; i++)
	{
		dp_params_arr[i].drop_mode = DISABLED;
		dp_params_arr[i].cbtdBytes = DROP_BW_KBITS_TO_BYTES(200000);
	}

	dp_aging.dp_ut_aging_ranges = dp_aging.dp_max / TM_AGING_PROFILES;

	/* first DP is reserved check whether last range is full */
	if (dp_aging.dp_max + 1 - (dp_aging.dp_ut_aging_ranges * TM_AGING_PROFILES) < TM_AGING_PROFILES)
		dp_aging.dp_ut_aging_ranges--;


    rc = cpssTmTestTmCtlLibInit(devNum);
	tm_ut_verify_equal_mac(0, rc, "cpssTmTestTmCtlLibInit");

    tm_hndl_PTR = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

	tm_ctl_PTR = (struct tm_ctl*)tm_hndl_PTR;
	tm_ut_verify_tm_lib_hndl_mac(tm_ctl_PTR);

	dp_aging.dp_ut_aging_indexes_arr = (uint16_t*)tm_malloc((sizeof(uint16_t) * ((get_tm_port_count(TM_ENV(tm_ctl_PTR)) - TM_UT_INTERNAL_USE_DP_NUM )/ TM_AGING_PROFILES)));

/*	rm_ctl_PTR = (struct rmctl*)tm_ctl_PTR->rm; */

	for (iteration = 1; iteration <= num_of_iterations; iteration++)
	{

		sprintf(test_name, "%s: iteration: %d", "aging drop profiles alloc", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
						      tm_aging_drop_profile_alloc_ut(tm_ctl_PTR, dp_params_arr, &dp_aging, 1),
							  test_name);			


		sprintf(test_name, "%s: iteration: %d", "aging drop profiles delete", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
			                  tm_aging_drop_profile_delete_ut(tm_ctl_PTR, &dp_aging, 1),
			                  test_name);			

		sprintf(test_name, "%s: iteration: %d", "aging drop profiles alloc after free", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
           			          tm_aging_drop_profile_alloc_ut(tm_ctl_PTR, dp_params_arr, &dp_aging, 1),
			                  test_name);			


		sprintf(test_name, "%s: iteration: %d", "aging drop profiles delete with intervals", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
							  tm_aging_drop_profile_delete_ut(tm_ctl_PTR, &dp_aging, 3),
							  test_name);			

		sprintf(test_name, "%s: iteration: %d", "aging drop profiles realloc deleted intervals", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
 							  tm_aging_drop_profile_alloc_ut(tm_ctl_PTR, dp_params_arr, &dp_aging, 3),
							  test_name);			

		sprintf(test_name, "%s: iteration: %d", "aging drop profiles delete", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
     			              tm_aging_drop_profile_delete_ut(tm_ctl_PTR, &dp_aging, 1),
			                  test_name);			
	}

	tm_free(dp_aging.dp_ut_aging_indexes_arr);

	return 0;
}

int tm_mixed_drop_profiles_ut(uint8_t devNum, int num_of_iterations)
{
	tm_handle         tm_hndl_PTR = 0;
	struct tm_ctl   *tm_ctl_PTR = NULL;
	char            test_name[128];
	int 			rc,iteration,i;

	struct drop_profiles_ut_stc dp_mixed = {"QUEUE_DROP_AGING_PROF", TM_NUM_QUEUE_DROP_PROF-TM_UT_INTERNAL_USE_DP_NUM, Q_LEVEL,  TM_INVAL} ;

	struct tm_drop_profile_params dp_params_arr[TM_AGING_PROFILES];

	tm_memset(dp_params_arr, 0, sizeof(dp_params_arr));

	for (i = 0; i < TM_AGING_PROFILES; i++)
	{
		dp_params_arr[i].drop_mode = DISABLED;
		dp_params_arr[i].cbtdBytes = DROP_BW_KBITS_TO_BYTES( 2000 + (i + 1) * 10000);
	}

	dp_mixed.dp_ut_aging_ranges = dp_mixed.dp_max / TM_AGING_PROFILES;

	/* first DP is reserved check whether last range is full */
	if (dp_mixed.dp_max + 1 - (dp_mixed.dp_ut_aging_ranges * TM_AGING_PROFILES) < TM_AGING_PROFILES)
		dp_mixed.dp_ut_aging_ranges--;

	dp_mixed.dp_ut_aging_indexes_arr = (uint16_t*)tm_malloc((sizeof(uint16_t) * TM_NUM_QUEUE_DROP_PROF / TM_AGING_PROFILES));
	dp_mixed.dp_ut_indexes_arr = (uint16_t*)tm_malloc(sizeof(uint16_t) * dp_mixed.dp_max);

    rc = cpssTmTestTmCtlLibInit(devNum);
    tm_ut_verify_equal_mac(0, rc, "cpssTmTestTmCtlLibInit");

    tm_hndl_PTR = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

	tm_ctl_PTR = (struct tm_ctl*)tm_hndl_PTR;
	tm_ut_verify_tm_lib_hndl_mac(tm_ctl_PTR);

/*	rm_ctl_PTR = (struct rmctl*)tm_ctl_PTR->rm; */

	for (iteration = 1; iteration <= num_of_iterations; iteration++)
	{

		sprintf(test_name, "%s: iteration: %d", "mixed drop profiles alloc", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
			                  tm_mixed_drop_profile_alloc_ut(tm_ctl_PTR, dp_params_arr, &dp_mixed),
			                  test_name);			


		sprintf(test_name, "%s: iteration: %d", "mixed drop profiles delete", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
			                  tm_mixed_drop_profile_delete_ut(tm_ctl_PTR, &dp_mixed),
			                  test_name);			

		sprintf(test_name, "%s: iteration: %d", "mixed drop profiles alloc after free", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
							  tm_mixed_drop_profile_alloc_ut(tm_ctl_PTR, dp_params_arr, &dp_mixed),
							  test_name);			

		sprintf(test_name, "%s: iteration: %d", "mixed drop profiles delete", iteration);
		tm_ut_verify_test_start(test_name);        
		tm_ut_verify_test_mac(0,
			tm_mixed_drop_profile_delete_ut(tm_ctl_PTR, &dp_mixed),
			test_name);			

	}

	tm_free(dp_mixed.dp_ut_indexes_arr);
	tm_free(dp_mixed.dp_ut_aging_indexes_arr);

	return 0;
}







 
