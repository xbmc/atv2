/*
 *      Copyright (C) 2005-2010 Team XBMC
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
#if defined(__APPLE__)
#if defined(__arm__)
  #import <Foundation/Foundation.h>
#else
  #import <Cocoa/Cocoa.h>
#endif

#import "iOS_Utils.h"
#import "XBMCApplication.h"

CCocoaAutoPool::CCocoaAutoPool()
{
  m_opaque_pool = [[NSAutoreleasePool alloc] init];
}
CCocoaAutoPool::~CCocoaAutoPool()
{
  [(NSAutoreleasePool*)m_opaque_pool release];
}

void* Create_AutoReleasePool(void)
{
  // Create an autorelease pool (necessary to call Obj-C code from non-Obj-C code)
  return [[NSAutoreleasePool alloc] init];
}

void  Destroy_AutoReleasePool(void *aPool)
{
  [(NSAutoreleasePool*)aPool release];
}

int  GetFrappBundlePath(char* path, uint32_t *bufsize)
{
  NSString *pathname;
	
  //pathname = [[NSBundle bundleForClass:[XBMCApplicationDelegate class]] pathForResource:@"XBMC" ofType:nil];

  pathname = [[NSBundle mainBundle] executablePath];

  strcpy(path, [pathname UTF8String]);
  *bufsize = strlen(path);
  *bufsize -= 4;
  path[*bufsize] = 0;
  
  NSLog(@"%s executable_path %s", __PRETTY_FUNCTION__, path);
  
  return 0;
}

#endif
