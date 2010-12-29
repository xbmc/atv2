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
#import <ImageIO/ImageIO.h>
#import "iOS_Utils.h"

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

CFURLRef CreateCFURLRefFromFilePath(const char *filepath)
{
  NSString *fpath = [NSString stringWithUTF8String:filepath];
  CFURLRef fileURL = (CFURLRef)[NSURL fileURLWithPath: fpath];
  [fpath release];
  
  return fileURL;
}

int  GetIOSFrameworkPath(char* path, uint32_t *pathsize)
{
  // see if we can figure out who we are
	NSString *pathname;

  // a) XBMC frappliance running under ATV2
  Class XBMCfrapp = NSClassFromString(@"XBMCAppliance");
  if (XBMCfrapp != NULL)
  {
    pathname = [[NSBundle bundleForClass:XBMCfrapp] pathForResource:@"Frameworks" ofType:@""];
    strcpy(path, [pathname UTF8String]);
    *pathsize = strlen(path);
    //NSLog(@"%s XBMC Frapp Frameworks path %s", __PRETTY_FUNCTION__, path);
    return 0;
  }

  // b) XBMC application running under IOS
  Class XBMCapp= NSClassFromString(@"XBMCApplication");
  if (XBMCfrapp != NULL)
  {
    pathname = [[NSBundle bundleForClass:XBMCapp] pathForResource:@"Frameworks" ofType:@""];
    strcpy(path, [pathname UTF8String]);
    *pathsize = strlen(path);
    //NSLog(@"%s XBMC IOS Frameworks path %s", __PRETTY_FUNCTION__, path);
    return 0;
  }

  // c) XBMC application running under OSX
  pathname = [[NSBundle mainBundle] privateFrameworksPath];
  strcpy(path, [pathname UTF8String]);
  *pathsize = strlen(path);
  //NSLog(@"%s XBMC Frameworks path %s", __PRETTY_FUNCTION__, path);

  return 0;
}

int  GetIOSExecutablePath(char* path, uint32_t *pathsize)
{
  // see if we can figure out who we are
	NSString *pathname;

  // a) XBMC frappliance running under ATV2
  Class XBMCfrapp = NSClassFromString(@"XBMCAppliance");
  if (XBMCfrapp != NULL)
  {
    pathname = [[NSBundle bundleForClass:XBMCfrapp] pathForResource:@"XBMC" ofType:@""];
    strcpy(path, [pathname UTF8String]);
    *pathsize = strlen(path);
    //NSLog(@"%s XBMC frapp executable_path %s", __PRETTY_FUNCTION__, path);
    return 0;
  }

  // b) XBMC application running under IOS
  // c) XBMC application running under OSX
  pathname = [[NSBundle mainBundle] executablePath];
  strcpy(path, [pathname UTF8String]);
  *pathsize = strlen(path);
  //NSLog(@"%s XBMC app executable_path %s", __PRETTY_FUNCTION__, path);

  return 0;
}
#endif
