/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoTmConfig.h
*
* @brief Initialization functions for the TM.
*
* @version   2.
********************************************************************************
*/
#ifndef __gtAppDemoTmConfigh
#define __gtAppDemoTmConfigh


/**
* @internal appDemoTmGeneral48PortsInit function
* @endinternal
*
* @brief   Implements general 48 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmGeneral48PortsInit
(
    IN  GT_U8   dev
);


/**
* @internal appDemoTmGeneral4PortsInit function
* @endinternal
*
* @brief   Implements general 4 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmGeneral4PortsInit
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario2Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario2Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario3Init function
* @endinternal
*
* @brief   Implements scenario 3 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node.
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario3Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario4Init function
* @endinternal
*
* @brief   Implements scenario 4 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node and wred mode on queue0 in each A node
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario4Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario5Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario5Init
(
    IN  GT_U8   dev
);
/**
* @internal appDemoTmScenario6Init function
* @endinternal
*
* @brief   Implements scenario 6 initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario6Init
(
    IN  GT_U8   dev
);


/**
* @internal appDemoTmScenario7Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario7Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario8Init function
* @endinternal
*
* @brief   Implements scenario 8 initialization for specific device.
*         In this scenario there are shpaers for every (VLAN,TC) pair
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario8Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmScenario9Init function
* @endinternal
*
* @brief   Scenario for demonstration of PFC. There is one C-node for one Q-node, including nodes
*         in between
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario9Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmRev4ScenarioInit function
* @endinternal
*
* @brief   Scenario for demonstration of PFC. There is one C-node for one Q-node, including nodes
*         in between
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmRev4ScenarioInit
(
    IN  GT_U8   dev
);


/**
* @internal appDemoTmStressScenarioInit function
* @endinternal
*
* @brief   Implements strss test
*
* @param[in] dev                      -   device number
* @param[in] mode                     -   select tree configuration  -  implementation nodes in  .c file
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmStressScenarioInit
(
    IN  GT_U8   dev,
    IN  GT_U8   mode
);


/**
* @internal appDemoTmScenario20Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
*         port 64 - 40G  -> C -> B -> A -> 252 Queues
*         no shaping , no drop
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmScenario20Init
(
    IN  GT_U8   dev
);

/**
* @internal appDemoTmLibInitOnly function
* @endinternal
*
* @brief   Implements only cpssTmCtlLibInit() - default init configuration for TM
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoTmLibInitOnly
(
    IN  GT_U8   dev
);

/* get total Queues per port for TM scenarios 1-9 */
extern GT_U32 appDemoTmGetTotalQueuesPerPort(void);

#endif /*  __gtAppDemoTmConfigh */


