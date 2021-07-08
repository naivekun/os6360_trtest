#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTask.h>
#include <cpssCommon/cpssPresteraDefs.h>

#define UNUSED_PARAMETER(x) x = x

#if defined  CPSS_USE_MUTEX_PROFILER   

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

#include <gtOs/gtOsSemDefs.h>

#define GT_PRINT_DEBUG_MSG(_counterName)  printf("Granular locking violation counter %s  incremented. \nTo debug put breakpoint in  file gtMutexProfiler.c line %d \n",#_counterName,  __LINE__)

extern CPSS_OS_MUTEX   prvCpssApiLockDevicelessMtxZeroLevel;


#if  defined(WIN32)  
#define MUTEX_ARRAY_SIZE  userspaceMutexes.allocated
#define MUTEX_ARRAY_LIST(_index)  userspaceMutexes.list[_index]
#define MUTEX_TYPE    GT_WIN32_MUTEX 
#elif defined(LINUX) 
#define MUTEX_ARRAY_SIZE semaphores.allocated
#define MUTEX_ARRAY_LIST(_index)  semaphores.list[_index]
#define MUTEX_TYPE    _V2L_semSTC 
#endif

extern GT_BOOL  cpssInitSystemIsDone;

static GT_U32  violationGlobalAfterDeviceSpecificCount = 0;              /* global after device specific*/
static GT_U32  violationRxTxConfigurationMixCount = 0;            /*rx/tx mixed with configuration*/
static GT_U32  violationZeroLevelCount = 0;            /* zero level violation*/
static GT_U32  violationUnprotectedDbAccessCount = 0;            /*access to device data base is not protected by mutex*/




#endif



#if  defined(WIN32)
extern OS_OBJECT_LIST_STC userspaceMutexes ;
#elif (defined(LINUX)&&defined (V2L_PTHREADS))
extern OS_OBJECT_LIST_STC semaphores;
#endif


/*******************************************************************************
* osMutexSetGlAttributes
*
* DESCRIPTION:
*       Set granular locking attributes to mutex. Such as type of the mutex related to the granular locking
*       and device number.
*
* INPUTS:
*       mtxid    - mutex id
*       isZeroLevel - GT_TRUE if the mutex is for zero level protection ,GT_FALSE otherwise
*       isDeviceOriented - GT_TRUE if the mutex is per device protection ,GT_FALSE otherwise
*       isRxTxFunctionality - GT_TRUE if the mutex is per rx/tx functionality protection ,GT_FALSE otherwise
*       devNum - device number,relevant only if isDeviceOriented is GT_TRUE
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*      
*
*******************************************************************************/

GT_STATUS osMutexSetGlAttributes
( 
    IN GT_MUTEX                           mtxId,
    IN GT_BOOL                            isZeroLevel,
    IN GT_BOOL                            isDeviceOriented,
    IN GT_BOOL                            isRxTxFunctionality,
    IN GT_U8                              devNum
)
{

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    CHECK_MTX(mtxId);

    if(isZeroLevel == GT_TRUE)
    {
        mtx->header.glType = GRANULAR_LOCKING_TYPE_ZERO_LEVEL;
    }
    else if(isDeviceOriented == GT_TRUE)
    {
        if(isRxTxFunctionality == GT_TRUE)
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX;
        }
        else
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION;
        }

        mtx->header.devNum = devNum;

    }
    else /*deviceOriented == GT_FALSE*/
    {
        if(isRxTxFunctionality == GT_TRUE)
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX;
        }
        else
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION;
        }
    }
#endif

    return GT_OK;
}
/*******************************************************************************
* osMutexLockGlValidation
*
* DESCRIPTION:
*       This function check that locking mutex does not violate granular locking convention
*
* INPUTS:
*       mtxid    - mutex id
*
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK - validation succeeded  
*       GT_FAIL -  mutex id is not ok
*
* COMMENTS:
*      The rules of granular locking :
*           1. When thread take zero level mutex, this thread can not take additional mutexes until this mutex is unlocked
*           2. Thread can not take non device oriented mutex after it took device oriented mutex
*           3. Thread can not mix rx/tx mutexes with configuration mutexes.
*
*******************************************************************************/
GT_STATUS osMutexLockGlValidation
(
    IN GT_MUTEX mtxid
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)


    int i;
    MUTEX_TYPE *mtxIterator;
    MUTEX_TYPE *zeroLevel;

    GT_BOOL ownedBySameThread;


    CHECK_MTX(mtxid);


    switch(mtx->header.glType)
    {   
    case GRANULAR_LOCKING_TYPE_ZERO_LEVEL:

        break;

    case GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION:      
        /*check zero level violation*/

        {
        
           zeroLevel =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(prvCpssApiLockDevicelessMtxZeroLevel);
           
#if  defined(WIN32) 
            ownedBySameThread = ((GT_U32)zeroLevel->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
            ownedBySameThread = ( pthread_equal(zeroLevel->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif
            if(ownedBySameThread)
            {
                violationZeroLevelCount++;
                GT_PRINT_DEBUG_MSG(violationZeroLevelCount);
            }
        }

        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)    
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);

            if (!mtxIterator || mtxIterator->header.type == 0)
                continue;

#if  defined(WIN32) 
              if (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE)
             {
                    continue;
             }
            ownedBySameThread = ((GT_U32)mtxIterator->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
            /*skip non mutex and non granular locking*/
             if( (mtxIterator->header.type != SEMTYPE_MUTEX)||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE))
             {
                    continue;
             }
             
            ownedBySameThread = ( pthread_equal(mtxIterator->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

            if(ownedBySameThread == GT_FALSE)
            {
                continue;
            }

            /*check that currently there is no per device mutex that is taken by the same thread*/
            if( mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION)
            {
                violationGlobalAfterDeviceSpecificCount++;
                GT_PRINT_DEBUG_MSG(violationGlobalAfterDeviceSpecificCount);
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if( (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
                
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }

        }
        break;

    case GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX:      

   
        {
        
           zeroLevel =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(prvCpssApiLockDevicelessMtxZeroLevel);
           
#if  defined(WIN32) 
            ownedBySameThread = ((GT_U32)zeroLevel->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
            ownedBySameThread = ( pthread_equal(zeroLevel->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif
            if(ownedBySameThread)
            {
                violationZeroLevelCount++;
                GT_PRINT_DEBUG_MSG(violationZeroLevelCount);
            }
        }


         for (i = 1; i < MUTEX_ARRAY_SIZE; i++)    
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);

            if (!mtxIterator || mtxIterator->header.type == 0)
                continue;

#if  defined(WIN32) 
             if (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE)
             {
                    continue;
             }
             
            ownedBySameThread = ((GT_U32)mtxIterator->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
              /*skip non mutex and non granular locking*/
             if( (mtxIterator->header.type != SEMTYPE_MUTEX)||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE))
             {
                    continue;
             }
            ownedBySameThread = ( pthread_equal(mtxIterator->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

            if(ownedBySameThread == GT_FALSE)
            {
                continue;
            }


            /*check that currently there is no per device mutex that is taken by the same thread*/
            if( mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX)
            {
                violationGlobalAfterDeviceSpecificCount++;
                GT_PRINT_DEBUG_MSG(violationGlobalAfterDeviceSpecificCount);
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }

        }
        break;

    case GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION:

        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)    
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);

            if (!mtxIterator || mtxIterator->header.type == 0)
                continue;

#if  defined(WIN32) 

             if (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE)
             {
                    continue;
             }
             
            ownedBySameThread = ((GT_U32)mtxIterator->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
              /*skip non mutex and non granular locking*/
             if( (mtxIterator->header.type != SEMTYPE_MUTEX)||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE))
             {
                    continue;
             }
            ownedBySameThread = ( pthread_equal(mtxIterator->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

            if(ownedBySameThread == GT_FALSE)
            {
                continue;
            }


            /*check that same thread is not taken rx/tx functionality mutex*/
            if((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
 
        }

            /*check zero level violation*/

            {
        
           zeroLevel =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(prvCpssApiLockDevicelessMtxZeroLevel);
           
#if  defined(WIN32) 
            ownedBySameThread = ((GT_U32)zeroLevel->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
            ownedBySameThread = ( pthread_equal(zeroLevel->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif
            if(ownedBySameThread)
            {
                violationZeroLevelCount++;
                GT_PRINT_DEBUG_MSG(violationZeroLevelCount);
            }
        }

        break;


    case GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX:

         for (i = 1; i < MUTEX_ARRAY_SIZE; i++)    
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);

            if (!mtxIterator || mtxIterator->header.type == 0)
                continue;

#if  defined(WIN32) 

             if (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE)
             {
                    continue;
             }
             
            ownedBySameThread = ((GT_U32)mtxIterator->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)   
                /*skip non mutex and non granular locking*/
             if( (mtxIterator->header.type != SEMTYPE_MUTEX)||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_NONE))
             {
                    continue;
             }
             
            ownedBySameThread = ( pthread_equal(mtxIterator->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

            if(ownedBySameThread == GT_FALSE)
            {
                continue;
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if ((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
            
        }

        /*check zero level violation*/

            {
        
           zeroLevel =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(prvCpssApiLockDevicelessMtxZeroLevel);
           
#if  defined(WIN32) 
            ownedBySameThread = ((GT_U32)zeroLevel->mtx.OwningThread == GetCurrentThreadId());         
#elif defined(LINUX)    
            ownedBySameThread = ( pthread_equal(zeroLevel->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif
            if(ownedBySameThread)
            {
                violationZeroLevelCount++;
                GT_PRINT_DEBUG_MSG(violationZeroLevelCount);
            }
        }

        break;


    default:
        break;
    }

#endif

    return GT_OK;

}

/*******************************************************************************
* osCheckUnprotectedPerDeviceDbAccess
*
* DESCRIPTION:
*      This function checks that device related mutex is locked when accessing device
*       related software database
*
* INPUTS:
*       devNum              - device mumber
*       functionName    - name of the function that is being checked
*
* OUTPUTS:
*       None
*
* RETURNS:
*      NONE
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID osCheckUnprotectedPerDeviceDbAccess
( 
    IN GT_U32                                 devNum,
    IN const char *                           functionName
)
{

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)


    int m;
    GT_BOOL dbProtected = GT_FALSE;
    GT_BOOL devicelessMutexLocked = GT_FALSE;
    GT_BOOL ownedBySameThread;
#if  defined(WIN32)  
    GT_WIN32_MUTEX *mtx;
#elif defined(LINUX) 
    _V2L_semSTC *mtx;
#endif

    /*start count only after system is initialized*/
    if(cpssInitSystemIsDone == GT_FALSE)
    {
        return ;
    }

     for (m = 1; m< MUTEX_ARRAY_SIZE; m++)    
    {
        mtx = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(m);

        if (!mtx || mtx->header.type == 0)
            continue;

#if  defined(WIN32) 
            /*skip non granular locking*/
             if (mtx->header.glType == GRANULAR_LOCKING_TYPE_NONE)
             {
                    continue;
             }
            ownedBySameThread = ((GT_U32)mtx->mtx.OwningThread == GetCurrentThreadId());      
#elif defined(LINUX)
            /*skip non mutex and non granular locking*/
             if( (mtx->header.type != SEMTYPE_MUTEX)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_NONE))
             {
                    continue;
             }
            ownedBySameThread = ( pthread_equal(mtx->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

           if(ownedBySameThread == GT_FALSE)
            {
                continue;
            }

        if( (mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
        {
            if (mtx->header.devNum == devNum)
            {
                dbProtected = GT_TRUE;
                break;
            }
        }

        if( (mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX))
        {
                devicelessMutexLocked = GT_TRUE;  
        }
    }

    if((dbProtected == GT_FALSE)&&(devicelessMutexLocked == GT_TRUE))
    { 
        /*Need to check that general mutex is locked in order to negate cases where macro is called directly from enhnaced UT of from LUA.
                    Those cases are not intresting*/

        /*Filter some function because those are known cases*/
        if(cpssOsStrCmp("deviceIsPrestera",functionName)!=0)
        {
                printf("Unprotected db access in function %s\n",functionName);
                violationUnprotectedDbAccessCount++;
                GT_PRINT_DEBUG_MSG(violationUnprotectedDbAccessCount);     
        }
                    


    }
#endif
    
}

/*******************************************************************************
* osMutexCountersStatistic
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
* INPUTS:
*       None
*      
*
* OUTPUTS:
*       violationGlobalAfterDeviceSpecificCountPtr -Get the value of violationGlobalAfterDeviceSpecificCount
*       violationRxTxConfigurationMixCountPtr -Get the value of violationRxTxConfigurationMixCount
*       violationZeroLevelCountPtr - Get the value of violationZeroLevelCount
*       violationUnprotectedDbAccessCountPtr - Get the value of violationUnprotectedDbAccessCount
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*      GT_NOT_SUPPORTED -   in case  that granular locking feature flag is not raised during the compilation
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexCountersStatistic
(
    OUT GT_U32  * violationGlobalAfterDeviceSpecificCountPtr,
    OUT GT_U32  * violationRxTxConfigurationMixCountPtr,
    OUT GT_U32  * violationZeroLevelCountPtr,
    OUT GT_U32  * violationUnprotectedDbAccessCountPtr
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    *violationGlobalAfterDeviceSpecificCountPtr = violationGlobalAfterDeviceSpecificCount;
    *violationRxTxConfigurationMixCountPtr  = violationRxTxConfigurationMixCount;
    *violationZeroLevelCountPtr = violationZeroLevelCount;
    *violationUnprotectedDbAccessCountPtr  = violationUnprotectedDbAccessCount;
    return GT_OK;
#else
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountPtr);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountPtr);
    UNUSED_PARAMETER(violationZeroLevelCountPtr);    
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountPtr);
    return GT_NOT_IMPLEMENTED;
#endif

   
}

/*******************************************************************************
* osMutexErrorsCreate
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
*
* INPUTS:
*       violationGlobalAfterDeviceSpecificCountError -if GT_TRUE then create global after device specific error
*       violationRxTxConfigurationMixCountError -if GT_TRUE then create Rx/Tx configuration Mix error
*       violationZeroLevelCountError -if GT_TRUE then create zero level error
*       violationUnprotectedDbAccessCountError - if GT_TRUE then create unprotected db access error
* OUTPUTS:
*   None
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexErrorsCreate
(
    IN GT_BOOL   violationGlobalAfterDeviceSpecificCountError,
    IN GT_BOOL   violationRxTxConfigurationMixCountError,
    IN GT_BOOL   violationZeroLevelCountError,
    IN GT_BOOL   violationUnprotectedDbAccessCountError
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    if(violationGlobalAfterDeviceSpecificCountError == GT_TRUE)
    {   
       /*take device oriented mutex*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*take non - device oriented mutex*/
       CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release non - device oriented mutex*/
       CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release device oriented mutex*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);  
    }

    if(violationRxTxConfigurationMixCountError == GT_TRUE)
    {   
       /*take device oriented mutex - configuration*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /*take device oriented mutex - rx-tx*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
       /*release device oriented mutex - rx-tx*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
       /*release device oriented mutex - configuration*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);   
    }

    if(violationZeroLevelCountError == GT_TRUE)
    {   
       /*take zero level mutex*/ 
       CPSS_ZERO_LEVEL_API_LOCK_MAC
       /*take device oriented mutex */
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release device oriented mutex */
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release zero level mutex*/ 
       CPSS_ZERO_LEVEL_API_UNLOCK_MAC
    }

    if(violationUnprotectedDbAccessCountError == GT_TRUE)
    { 
       /*take non - device oriented mutex*/
       CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        
 #ifdef  WIN32   
       osCheckUnprotectedPerDeviceDbAccess(10,__FUNCTION__);
 #else
       osCheckUnprotectedPerDeviceDbAccess(10,__func__);
 #endif
 
       /*release non - device oriented mutex*/
       CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    return GT_OK;
#else
    return GT_NOT_IMPLEMENTED;
#endif
   
}

#else /*defined  CPSS_USE_MUTEX_PROFILER   */
/*******************************************************************************
* osMutexCountersStatistic
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
* INPUTS:
*       None
*      
*
* OUTPUTS:
*       violationGlobalAfterDeviceSpecificCountPtr -Get the value of violationGlobalAfterDeviceSpecificCount
*       violationRxTxConfigurationMixCountPtr -Get the value of violationRxTxConfigurationMixCount
*       violationZeroLevelCountPtr - Get the value of violationZeroLevelCount
*       violationUnprotectedDbAccessCountPtr - Get the value of violationUnprotectedDbAccessCount
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*      GT_NOT_SUPPORTED -   in case  that granular locking feature flag is not raised during the compilation
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexCountersStatistic
(
    OUT GT_U32  * violationGlobalAfterDeviceSpecificCountPtr,
    OUT GT_U32  * violationRxTxConfigurationMixCountPtr,
    OUT GT_U32  * violationZeroLevelCountPtr,
    OUT GT_U32  * violationUnprotectedDbAccessCountPtr
)
{       
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountPtr);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountPtr);
    UNUSED_PARAMETER(violationZeroLevelCountPtr);    
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountPtr);
    return GT_NOT_SUPPORTED;
   
}

/*******************************************************************************
* osMutexErrorsCreate
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
*
* INPUTS:
*       violationGlobalAfterDeviceSpecificCountError -if GT_TRUE then create global after device specific error
*       violationRxTxConfigurationMixCountError -if GT_TRUE then create Rx/Tx configuration Mix error
*       violationZeroLevelCountError -if GT_TRUE then create zero level error
*       violationUnprotectedDbAccessCountError - if GT_TRUE then create unprotected db access error
* OUTPUTS:
*   None
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexErrorsCreate
(
    IN GT_BOOL   violationGlobalAfterDeviceSpecificCountError,
    IN GT_BOOL   violationRxTxConfigurationMixCountError,
    IN GT_BOOL   violationZeroLevelCountError,
    IN GT_BOOL   violationUnprotectedDbAccessCountError
)
{
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountError);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountError);
    UNUSED_PARAMETER(violationZeroLevelCountError);    
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountError);
    
    return GT_NOT_SUPPORTED;
}

#endif /*else - defined  CPSS_USE_MUTEX_PROFILER   */
