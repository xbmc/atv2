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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <BackRow/BackRow.h>

#import "XBMCController.h"
#import "XBMCEAGLView.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
@implementation UIWindow (limneos)
-(id)parent { return nil; }
-(void)removeFromParent {}
-(BOOL)active { return NO; } 
-(void)controlWasActivated {}
-(void)controlWasDeactivated {}
@end

@implementation UIView (limneos)
-(id)parent { return nil; }
-(BOOL)active { return NO; }
-(void)removeFromParent {}
-(void)controlWasActivated {}
-(void)controlWasDeactivated {}
@end

// notification messages
extern NSString* kBRScreenSaverActivated;
extern NSString* kBRScreenSaverDismissed;

//--------------------------------------------------------------
//--------------------------------------------------------------
@interface XBMCController (PrivateMethods)
UIWindow      *m_window;
XBMCEAGLView  *m_glView;

- (void) observeDefaultCenterStuff: (NSNotification *) notification;
- (void) disableScreenSaver;
- (void) enableScreenSaver;
@end
//
//
@implementation XBMCController
+ (XBMCController*) sharedInstance
{
  // the instance of this class is stored here
  static XBMCController *myInstance = nil;

  // check to see if an instance already exists
  if (nil == myInstance)
    myInstance  = [[[[self class] alloc] init] autorelease];

  // return the instance of this class
  return myInstance;
}

- (XBMCEAGLView*) getEGLView
{
  return m_glView;
}

- (id) init
{  
  NSLog(@"%s", __PRETTY_FUNCTION__);

  self = [super init];
  if ( !self )
    return ( nil );

  NSNotificationCenter *center;
  // first the default notification center, which is all
  // notifications that only happen inside of our program
  center = [NSNotificationCenter defaultCenter];
  [center addObserver: self
    selector: @selector(observeDefaultCenterStuff:)
    name: nil
    object: nil];

  m_window = [[UIWindow alloc] initWithFrame:[BRWindow interfaceFrame]];
  m_glView = [[XBMCEAGLView alloc] initWithFrame:m_window.bounds];
  [m_window addSubview:m_glView];

  return self;
}

- (void)dealloc
{
  NSLog(@"%s", __PRETTY_FUNCTION__);

  [m_glView stopAnimation];
  [m_glView release];
  [m_window release];

  NSNotificationCenter *center;
  // take us off the default center for our app
  center = [NSNotificationCenter defaultCenter];
  [center removeObserver: self];

  [super dealloc];
}

- (void)controlWasActivated
{
  NSLog(@"%s", __PRETTY_FUNCTION__);
  
  [super controlWasActivated];

  [m_window makeKeyAndVisible];
  [[[[BRWindow windowList] objectAtIndex:0] content] addControl: m_window];

  [m_glView startAnimation];
}

- (void)controlWasDeactivated
{
  NSLog(@"%s", __PRETTY_FUNCTION__);


  [m_glView stopAnimation];

  [[[[BRWindow windowList] objectAtIndex:0] content] _removeControl: m_window];
  [m_window resignKeyWindow];

  [super controlWasDeactivated];
}

- (BOOL) recreateOnReselect
{ 
  NSLog(@"%s", __PRETTY_FUNCTION__);
  return YES;
}

- (BOOL)brEventAction:(id)action
{
  NSLog(@"%s", __PRETTY_FUNCTION__);

  return [super brEventAction:action];
}

#pragma mark -
#pragma mark private helper methods
//
- (void)observeDefaultCenterStuff: (NSNotification *) notification
{
  //NSLog(@"default: %@", [notification name]);

  //if ([notification name] == kBRScreenSaverActivated)
  //  [m_glView stopAnimation];
  
  //if ([notification name] == kBRScreenSaverDismissed)
  //  [m_glView startAnimation];
}

- (void) disableScreenSaver
{
  /*
  NSLog(@"%s", __PRETTY_FUNCTION__);
  //store screen saver state and disable it
  //!!BRSettingsFacade setScreenSaverEnabled does change the plist, but does _not_ seem to work
  m_screensaverTimeout = [[BRSettingsFacade singleton] screenSaverTimeout];
  [[BRSettingsFacade singleton] setScreenSaverTimeout:-1];
  [[BRSettingsFacade singleton] flushDiskChanges];
  */
}

- (void) enableScreenSaver
{
  /*
  NSLog(@"%s", __PRETTY_FUNCTION__);
  //reset screen saver to user settings
  [[BRSettingsFacade singleton] setScreenSaverTimeout: m_screensaverTimeout];
  [[BRSettingsFacade singleton] flushDiskChanges];
  */
}

@end
