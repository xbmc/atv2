/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifndef _XBMC_cond_h
#define _XBMC_cond_h

#include <stdint.h>
#include "XBMC_mutex.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Condition variable functions                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* The SDL condition variable structure, defined in XBMC_cond.c */
typedef struct XBMC_cond
{
	pthread_cond_t cond;
} XBMC_cond;

/* Create a condition variable */
XBMC_cond *XBMC_CreateCond(void);

/* Destroy a condition variable */
void XBMC_DestroyCond(XBMC_cond *cond);

/* Restart one of the threads that are waiting on the condition variable,
   returns 0 or -1 on error.
 */
int XBMC_CondSignal(XBMC_cond *cond);

/* Restart all threads that are waiting on the condition variable,
   returns 0 or -1 on error.
 */
int XBMC_CondBroadcast(XBMC_cond *cond);

/* Wait on the condition variable, unlocking the provided mutex.
   The mutex must be locked before entering this function!
   The mutex is re-locked once the condition variable is signaled.
   Returns 0 when it is signaled, or -1 on error.
 */
int XBMC_CondWait(XBMC_cond *cond, XBMC_mutex *mut);

/* Waits for at most 'ms' milliseconds, and returns 0 if the condition
   variable is signaled, XBMC_MUTEX_TIMEDOUT if the condition is not
   signaled in the allotted time, and -1 on error.
   On some platforms this function is implemented by looping with a delay
   of 1 ms, and so should be avoided if possible.
*/
int XBMC_CondWaitTimeout(XBMC_cond *cond, XBMC_mutex *mutex, uint32_t ms);

#endif /* _XBMC_cond_h */
