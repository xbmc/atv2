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

#ifndef _XBMC_mutex_h
#define _XBMC_mutex_h

#include <pthread.h>

/* Functions to provide thread synchronization primitives

	These are independent of the other SDL routines.
*/

/* Synchronization functions which can time out return this value
   if they time out.
*/
#define XBMC_MUTEX_TIMEDOUT	1

/* This is the timeout value which corresponds to never time out */
#define XBMC_MUTEX_MAXWAIT	(~(Uint32)0)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Mutex functions                                               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* The SDL mutex structure, defined in XBMC_mutex.c */
#if !defined(_WIN32)
  #if !PTHREAD_MUTEX_RECURSIVE && \
      !PTHREAD_MUTEX_RECURSIVE_NP
  #define FAKE_RECURSIVE_MUTEX
  #endif
#endif

typedef struct XBMC_mutex {
#if defined(_WIN32)
	HANDLE id;
#else
    pthread_mutex_t id;
    #if defined(FAKE_RECURSIVE_MUTEX)
      int recursive;
      pthread_t owner;
    #endif
#endif
} XBMC_mutex;

/* Create a mutex, initialized unlocked */
XBMC_mutex *XBMC_CreateMutex(void);

/* Lock the mutex  (Returns 0, or -1 on error) */
#define XBMC_LockMutex(m)	XBMC_mutexP(m)
int XBMC_mutexP(XBMC_mutex *mutex);

/* Unlock the mutex  (Returns 0, or -1 on error)
   It is an error to unlock a mutex that has not been locked by
   the current thread, and doing so results in undefined behavior.
 */
#define XBMC_UnlockMutex(m)	XBMC_mutexV(m)
int XBMC_mutexV(XBMC_mutex *mutex);

/* Destroy a mutex */
void XBMC_DestroyMutex(XBMC_mutex *mutex);

#endif /* _XBMC_mutex_h */
