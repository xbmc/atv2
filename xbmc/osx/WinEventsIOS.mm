/*
*      Copyright (C) 2005-2008 Team XBMC
*      http://www.xbmc.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*  http://www.gnu.org/copyleft/gpl.html
*
*/

#define BOOL XBMC_BOOL 
#include "system.h"
#include "WinEvents.h"
#include "WinEventsIOS.h"
#include "Application.h"
#include "XBMC_vkeys.h"
#include "MouseStat.h"
#include "WindowingFactory.h"
#undef BOOL

#import <UIKit/UIKit.h>

PHANDLE_EVENT_FUNC CWinEventsBase::m_pEventFunc = NULL;

XBMC_Event g_newEvent;
bool g_bNewEvent = false;

//NSLock *lockMessage = [[NSLock alloc] init];

void CWinEventsIOS::MessagePush(XBMC_Event *newEvent)
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  
  //if([lockMessage tryLock]) {
    memcpy(&g_newEvent, newEvent, sizeof(XBMC_Event));
    g_bNewEvent = true;
    //[lockMessage unlock];
  //}
}

bool CWinEventsIOS::MessagePump()
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  
  //if([lockMessage tryLock]) {
    if(g_bNewEvent) {
      g_application.OnEvent(g_newEvent);
      g_bNewEvent = false;
      //[lockMessage unlock];
      return true;
    }
  //}
  
  return false;
}
