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
 * @brief platform undependent wrapper interface to  platform dependent os functions
 *
* @file tm_os_interface.h
*
* $Revision: 2.0 $
 */
#ifndef SCHED_OS_INTERFACE_H
#define SCHED_OS_INTERFACE_H


/* */
void *schedMalloc(unsigned int size);
void  schedFree(void *ptr);
void *schedMemSet(void *s, int c, unsigned int n);
void *schedMemCpy(void *dest, const void *src, unsigned int n);


double schedRound(double x);
float  schedRoundF(float x);
int    schedCeil(float x);
float  schedFabsFl(float x);
double schedFabsDb(double x);
int    schedAbs(int x);



#define TM_MAX_STR_SIZE	1024

int schedPrintf(const char *format, ...);


#endif   /* TM_OS_INTERFACE_H */

