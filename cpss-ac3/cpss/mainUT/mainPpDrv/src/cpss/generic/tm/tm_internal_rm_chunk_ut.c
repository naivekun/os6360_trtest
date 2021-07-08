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


#include <cpss/generic/tm/tm_internal_ut_defs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <stdio.h>
#include "rm_free.h"
#include "rm_chunk.h"
#include "tm_core_types.h"
#include "tm_ctl.h"
#include "rm_ctl.h"


/* 
open this flag for old chunk implementation 
#define OLD_CHUNK_IMP 
*/ 

#ifdef OLD_CHUNK_IMP
    #define rm_allocate_chunk rm_alloc_chunk 
    #define rm_release_chunk rm_free_chunk  
    #define rm_expand_chunk rm_alloc_chunk_by_index 

static int rm_dump_chunk(rmctl_t hndl, enum rm_level lvl)
{
    return 0;
}

#endif


/* get, release */
static int rm_ut_chunk_list_scenario_1(rmctl_t rm_hndl, enum tm_level level, uint32_t count, uint32_t *arr_size, uint32_t *idxs_arr)
{
    uint32_t    i;
    int         rc;
    char		msg[128];

    /* verify all cuncks can be allocated */
    for (i = 0; i < count; i++) 
    {
        if (arr_size[i]	== 0)
            continue;
			
        rc = rm_allocate_chunk(rm_hndl, level, arr_size[i], &idxs_arr[i]); 
        if (rc) 
        {
            rm_dump_chunk(rm_hndl, level);
        }
        sprintf(msg, "rm_allocate_chunk: count: %d, size: %ld", i, arr_size[i]); 
        tm_ut_verify_equal_mac(0, rc, msg);
    }

    /* verify all cuncks can be released */
    for (i = 0; i < count; i++) 
    {
		if (arr_size[i]	== 0)
			continue;
		
        rc = rm_release_chunk(rm_hndl, level, arr_size[i], idxs_arr[i]); 
        sprintf(msg, "rm_release_chunk: count: %d, size: %ld idx: %ld", i, arr_size[i], idxs_arr[i]);        
        tm_ut_verify_equal_mac(0, rc, msg);
    }

    return 0;
}

/* expand right */
static int rm_ut_chunk_list_scenario_2(rmctl_t rm_hndl, enum tm_level level, uint32_t count, uint32_t *arr_size, uint32_t *idxs_arr)
{
    uint32_t    i,j;
    int         rc;
    uint32_t    expnd_idx;
    char		msg[64];    

    /* verify expand right */

    for (i = 0; i < count; i++) 
    {
		if (arr_size[i]	== 0)
			continue;

        rc = rm_allocate_chunk(rm_hndl, level, arr_size[i], &idxs_arr[i]); 
        sprintf(msg, "rm_allocate_chunk: count: %d, size: %ld", i, arr_size[i]);        
        tm_ut_verify_equal_mac(0, rc, msg);
    }

    /* released odd chuncks */
    for (i = 0; i < count - 1; i+=2) 
    {
		if (arr_size[i]	== 0)
			continue;
		
        rc = rm_release_chunk(rm_hndl, level, arr_size[i+1], idxs_arr[i+1]); 
        sprintf(msg, "rm_release_chunk: count: %d, size: %ld, idx: %ld", i, arr_size[i], idxs_arr[i]);                
        tm_ut_verify_equal_mac(0, rc, msg);
    }

    /* expand even chuncks */
    for (i = 0; i < count - 1; i+=2) 
    {
		if (arr_size[i] == 0)
			continue;
			
		if (arr_size[i+1] == 0)
			continue;
				
        expnd_idx = idxs_arr[i] + arr_size[i];
        for (j = 0; j < arr_size[i + 1]; j++)         
		{            
            rc = rm_expand_chunk_right(rm_hndl, level, expnd_idx);             
			sprintf(msg, "rm_expand_chunk: idx: %ld, size: %ld",  expnd_idx, arr_size[i]);        			            
			tm_ut_verify_equal_mac(0, rc, msg);
			expnd_idx++;
			arr_size[i]++;            
        }

		/* after expanding arr_size[i] by arr_size[i+1] */
        arr_size[i+1] = 0;
    }

    /* released chuncks */
	for (i = 0; i < count; i++) 
	{
		if (arr_size[i]	== 0)
			continue;

		rc = rm_release_chunk(rm_hndl, level, arr_size[i], idxs_arr[i]); 
		sprintf(msg, "rm_release_chunk: count: %d, size: %ld, idx: %ld", i, arr_size[i], idxs_arr[i]);                        
		tm_ut_verify_equal_mac(0, rc, 0);
	}

    return 0;
}


static int rm_ut_chunk_list_sum_sizes(uint32_t arr_size, uint32_t *sizes_arr)
{
    uint32_t i;
    uint32_t sum = 0;
    for (i = 0; i < arr_size; i++) 
        sum += sizes_arr[i]; 

    return sum;
}

static void rm_ut_chunk_list_print_sizes(uint32_t arr_size, uint32_t *sizes_arr)
{
    uint32_t i;
    for (i = 0; i < arr_size; i++) 
    {
		tm_printf("%ld ", sizes_arr[i]);
		if((i+1)%20 == 0)
			tm_printf("\n", sizes_arr[i]);		
	}	
	
	tm_printf("\n");			
}



static int rm_ut_chunk_list_fill_sizes(uint32_t total_nodes, uint32_t arr_size, uint32_t *sizes_arr)
{
    uint32_t portion, i;

    tm_memset(sizes_arr, 0, arr_size * sizeof(uint32_t));

    for (i = 0; i < arr_size && total_nodes; i += 2) 
    {
        portion = total_nodes / (arr_size - i);

		if (portion == 0)
			portion = (total_nodes % (arr_size - i));

        sizes_arr[i] = portion;
        total_nodes -= portion;
    }

    if (total_nodes)
    {
        for (i = 0; i < arr_size - 1 && total_nodes; i +=2) 
        {
            portion = total_nodes / (arr_size - i);

			if (portion == 0)
				portion = (total_nodes % (arr_size - i));

            sizes_arr[i+1] = portion;
            total_nodes -= portion;
        }
    }

    if (total_nodes) /* add to first element */
    {
        sizes_arr[0] += total_nodes;
    }

    return 0;
}

int tm_rm_ut_chunk_list(uint8_t devNum, int num_of_iterations)
{
    tm_handle       tm_hndl_PTR = 0;
    struct tm_ctl   *tm_ctl_PTR = NULL;

    int             arr_size = 50;
    uint32_t        idxs_arr[100];
    uint32_t        sizes_arr[100]; 

    uint32_t        levels_arr[4] = {A_LEVEL, B_LEVEL, C_LEVEL, Q_LEVEL};
    char*           levels_names[4] = {"A_LVL", "B_LVL", "C_LVL", "Q_LVL"};
    uint32_t        levels_sizes_arr[4],tmp;
    char            test_name[64];
    int 			i,sum,j,k,rc,iteration;

    uint32_t        chunk_index, chunk_size;


	rc = cpssTmTestTmCtlLibInit(devNum);
	tm_ut_verify_equal_mac(0, rc, "cpssTmTestTmCtlLibInit");

	tm_hndl_PTR = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

    /* 
        Assuming Next code has run it tm_lib_open->rm_open 

	    ctl->rm_free_nodes[RM_A_LVL]=rm_new_chunk(0,total_a_nodes,NULL);
	    ctl->rm_free_nodes[RM_B_LVL]=rm_new_chunk(0,total_b_nodes,NULL);
	    ctl->rm_free_nodes[RM_C_LVL]=rm_new_chunk(0,total_c_nodes,NULL);
	        special case - patching H/W bug  - exclude queue 0  from configuration
	    ctl->rm_free_nodes[RM_Q_LVL]=rm_new_chunk(1,total_queues-1,NULL); 
    */

    tm_ctl_PTR = (struct tm_ctl*)tm_hndl_PTR;
	tm_ut_verify_tm_lib_hndl_mac(tm_ctl_PTR);


    levels_sizes_arr[0] = tm_ctl_PTR->tm_total_a_nodes; 
    levels_sizes_arr[1] = tm_ctl_PTR->tm_total_b_nodes;
    levels_sizes_arr[2] = tm_ctl_PTR->tm_total_c_nodes;
    levels_sizes_arr[3] = tm_ctl_PTR->tm_total_queues-1;

	for (iteration = 1; iteration <= num_of_iterations; iteration++)
	{
        if (tm_ut_debug_flag)
        {
            tm_printf("rm_ut_chunk_list: %d\n", iteration); 
        }
		for (i = 0; i < 4; i++) 
		{
			rm_ut_chunk_list_fill_sizes(levels_sizes_arr[i], arr_size, sizes_arr);

			if (iteration%2 == 0) /* sort lower to bigger*/
			{
				for(j = 0; j < arr_size; j++) 
				{
					for(k = 0; k < arr_size-1; k++) 				
					{
						if (sizes_arr[k] > sizes_arr[k+1])
						{
							tmp = sizes_arr[k];
							sizes_arr[k] = sizes_arr[k+1];
							sizes_arr[k+1] = tmp;
						}
					}
				}
			}
			
			else if (iteration%3 == 0) /* sort bigger to lower*/
			{
				for(j = 0; j < arr_size; j++) 
				{
					for(k = 0; k < arr_size-1; k++) 				
					{
						if (sizes_arr[k] < sizes_arr[k+1])
						{
							tmp = sizes_arr[k];
							sizes_arr[k] = sizes_arr[k+1];
							sizes_arr[k+1] = tmp;
						}
					}
				}
			}
			
						
			sum = rm_ut_chunk_list_sum_sizes(arr_size, sizes_arr);

			if (tm_ut_debug_flag)
			{
				tm_printf("%s: total_nodes: %d, total sizes_arr: %ld sizes:\n", levels_names[i], levels_sizes_arr[i], sum); 				
				rm_ut_chunk_list_print_sizes(arr_size, sizes_arr);				
			}
				
			/* verify sizes_arr */
			sum = rm_ut_chunk_list_sum_sizes(arr_size, sizes_arr);
			tm_ut_verify_equal_mac(levels_sizes_arr[i], sum, "0: wrong sizes");

			if (tm_ut_debug_flag)
				tm_printf("%s: total nodes: %d\n", levels_names[i], sum); 

			sprintf(test_name, "%s: %s", "rm chunk list basic", levels_names[i]); 
			tm_ut_verify_test_start(test_name);
			tm_ut_verify_test_mac(0,
								  rm_ut_chunk_list_scenario_1(tm_ctl_PTR->rm, levels_arr[i], arr_size, sizes_arr, idxs_arr),
								  test_name);

			sum = rm_ut_chunk_list_sum_sizes(arr_size, sizes_arr);
			tm_ut_verify_equal_mac(levels_sizes_arr[i], sum, "1: wrong sizes");

			sprintf(test_name, "%s: %s", "rm chunk list expand right", levels_names[i]); 
			tm_ut_verify_test_start(test_name);
			tm_ut_verify_test_mac(0,
								  rm_ut_chunk_list_scenario_2(tm_ctl_PTR->rm, levels_arr[i], arr_size, sizes_arr, idxs_arr),
								  test_name);

			sprintf(test_name, "%s: %s", "rm chunk list basic", levels_names[i]); 
			tm_ut_verify_test_start(test_name);
			sum = rm_ut_chunk_list_sum_sizes(arr_size, sizes_arr);
			tm_ut_verify_equal_mac(levels_sizes_arr[i], sum, "2: wrong sizes");

			tm_ut_verify_test_mac(0,
								  rm_ut_chunk_list_scenario_1(tm_ctl_PTR->rm, levels_arr[i], arr_size, sizes_arr, idxs_arr),
								  test_name);

		}
		if (tm_ut_debug_flag)
			tm_printf("\n");
	}


    if (tm_ut_debug_flag)
    {
    	tm_printf(" test merging chunk ,initial state \n"); 
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	rm_allocate_chunk(tm_ctl_PTR->rm, Q_LEVEL,102,&tmp);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,100);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" case 1 :release two chunks inside occupied range\n"); 

    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,10);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,20,60);
      
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	tm_printf(" release two chunks , one  shoud be merged left, other should be merged right\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,20);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,50);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" release chunk , shoud be merged left & right\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,20,30);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" restore initial  state\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,0);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,20,80);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,101);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	rm_allocate_chunk(tm_ctl_PTR->rm, Q_LEVEL,102,&tmp);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,100);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);


    	tm_printf(" case 2 : release two chunks inside occupied range (chunk sizes are opposite to case 1)\n"); 

    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL, 20,10);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,60);

    	
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	tm_printf(" release two chunks , one  shoud be merged left, other should be merged right\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,30);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,50);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" release chunk , shoud be merged left & right\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,40);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" restore initial  state\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,10,0);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,30,70);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,101);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	rm_allocate_chunk(tm_ctl_PTR->rm, Q_LEVEL,102,&tmp);
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,100);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);


    	tm_printf(" test merging chunk , wrong chunk size  \n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,1,101);
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);
    	tm_printf(" attempt to add free range 105 from position 0, while exist chunk starting from position 100, should be error...\n"); 
    	rc = rm_release_chunk(tm_ctl_PTR->rm, Q_LEVEL,105,0);
    	if (rc==0)
    	{
            tm_printf("\t%s, line: %d error : free space inside chunk not recognized\n",__FILE__, __LINE__) ;
    		return 1; 
    	}
    	tm_printf(" error  recognized, release cancelled.\n"); 
    	rm_dump_chunk(tm_ctl_PTR->rm, Q_LEVEL);

    	tm_printf(" test merging chunk , new chunk should be biggest (case of :next chunk is bigger \n"); 
    	rm_allocate_chunk(tm_ctl_PTR->rm, A_LEVEL,1200,&tmp);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,200,0);
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,600,400);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	tm_printf(" release range between 200::400, two edge chunks should be merged and new chunk should be biggest.\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,200,200);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	/* restore initial - return  complementary chunk */ 
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,200,1000);
    	
    	tm_printf(" test merging chunk , new chunk should be biggest (case of : previous chunk is bigger) \n"); 

    	rm_allocate_chunk(tm_ctl_PTR->rm, A_LEVEL,1200,&tmp);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,600,0);
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,200,800);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);
    	tm_printf(" release range between 600::800, two edge chunks should be merged and new chunk should be biggest.\n"); 
    	rm_release_chunk(tm_ctl_PTR->rm, A_LEVEL,200,600);
    	rm_dump_chunk(tm_ctl_PTR->rm, A_LEVEL);

    	if (get_chunk_data(tm_ctl_PTR->rm, A_LEVEL, 0, &chunk_index , &chunk_size ))
    	{
            tm_printf("\t%s, line: %d error : error reading content of first chunk \n",__FILE__, __LINE__) ;
    		return 1; 
    	}
    	tm_ut_verify_equal_mac(1000, chunk_size, " unexpected size of the first chunk ");
    }

    return 0;
}



