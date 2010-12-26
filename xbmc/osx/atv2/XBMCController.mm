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
#import "xbmcclientwrapper.h"
#import "XBMCDebugHelpers.h"

typedef enum {
  // for originator kBREventOriginatorRemote
  kBREventRemoteActionMenu = 1,
  kBREventRemoteActionMenuHold,
  kBREventRemoteActionUp,
  kBREventRemoteActionDown,
  kBREventRemoteActionPlay,
  kBREventRemoteActionLeft,
  kBREventRemoteActionRight,

  kBREventRemoteActionPlayHold = 20,

  // Gestures, for originator kBREventOriginatorGesture
  kBREventRemoteActionTap = 30,
  kBREventRemoteActionSwipeLeft,
  kBREventRemoteActionSwipeRight,
  kBREventRemoteActionSwipeUp,
  kBREventRemoteActionSwipeDown,

  // Custom remote actions for old remote actions
  kBREventRemoteActionHoldLeft = 0xfeed0001,
  kBREventRemoteActionHoldRight,
  kBREventRemoteActionHoldUp,
  kBREventRemoteActionHoldDown,
} BREventRemoteAction;

XBMCController *g_xbmcController;

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
/*
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
*/

- (void) initDisplayLink
{
  [m_glView initDisplayLink];
}
- (void) deinitDisplayLink
{
  [m_glView deinitDisplayLink];
}
- (double) getDisplayLinkFPS
{
  return [m_glView getDisplayLinkFPS];
}
- (void) setFramebuffer
{
  [m_glView setFramebuffer];
}
- (bool) presentFramebuffer
{
  return [m_glView presentFramebuffer];
}
- (CGSize) getScreenSize
{
  CGSize screensize;

  screensize.width  = [BRWindow interfaceFrame].size.width;
  screensize.height = [BRWindow interfaceFrame].size.height;

  //NSLog(@"%s UpdateResolutions width=%f, height=%f", 
	//	__PRETTY_FUNCTION__, screensize.width, screensize.height);

  return screensize;
}


- (id) init
{  
  //NSLog(@"%s", __PRETTY_FUNCTION__);

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

  g_xbmcController = self;

  bool use_universal = NO;
  NSString *client_address= @"localhost";
  mp_xbmclient = [[XBMCClientWrapper alloc] initWithUniversalMode:use_universal serverAddress:client_address];
  return self;
}

- (void)dealloc
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);

	[mp_xbmclient release];

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
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  
  [super controlWasActivated];

  [m_window makeKeyAndVisible];
  [[[[BRWindow windowList] objectAtIndex:0] content] addControl: m_window];

  [m_glView startAnimation];
}

- (void)controlWasDeactivated
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);


  [m_glView stopAnimation];

  [[[[BRWindow windowList] objectAtIndex:0] content] _removeControl: m_window];
  [m_window resignKeyWindow];

  [super controlWasDeactivated];
}

- (BOOL) recreateOnReselect
{ 
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  return YES;
}

- (eATVClientEvent) ATVClientEventFromBREvent:(BREvent*) f_event
{
  int remoteAction = [f_event remoteAction];
  //BOOL downEvent = [f_event value];
  //DLOG(@"got action %i %@", remoteAction, (downEvent)? @"pressed":@"released");
  
  switch (remoteAction)
  {
    case kBREventRemoteActionUp:
    case 65676:  // tap up
      if([f_event value] == 1)
        return ATV_BUTTON_UP;
      else
        return ATV_BUTTON_UP_RELEASE;
    case kBREventRemoteActionDown:
    case 65677:  // tap down
      if([f_event value] == 1)
        return ATV_BUTTON_DOWN;
      else
        return ATV_BUTTON_DOWN_RELEASE;
    case kBREventRemoteActionLeft:
    case 65675:  // tap left
      if([f_event value] == 1)
        return ATV_BUTTON_LEFT;
      else
        return ATV_BUTTON_LEFT_RELEASE;
    case 786612: // hold left
      if([f_event value] == 1)
        return ATV_LEARNED_REWIND;
      else
        return ATV_LEARNED_REWIND_RELEASE;
    case kBREventRemoteActionRight:
    case 65674:  // tap right
      if ([f_event value] == 1)
        return ATV_BUTTON_RIGHT;
      else
        return ATV_BUTTON_RIGHT_RELEASE;
    case 786611: // hold right
      if ([f_event value] == 1)
        return ATV_LEARNED_FORWARD;
      else
        return ATV_LEARNED_FORWARD_RELEASE;
    case kBREventRemoteActionPlay:
    case 65673:  // tap play
      return ATV_BUTTON_PLAY;
    case kBREventRemoteActionPlayHold:
    case 65668:  // hold play
      return ATV_BUTTON_PLAY_H;
    case kBREventRemoteActionMenu:
    case 65670:  // menu
      return ATV_BUTTON_MENU;
    case kBREventRemoteActionMenuHold:
    case 786496: // hold menu
      return ATV_BUTTON_MENU_H;
    case 786608: //learned play
      return ATV_LEARNED_PLAY;
    case 786609: //learned pause
      return ATV_LEARNED_PAUSE;
    case 786615: //learned stop
      return ATV_LEARNED_STOP;
    case 786613: //learned nexxt
      return ATV_LEARNED_NEXT;
    case 786614: //learned previous
      return ATV_LEARNED_PREVIOUS;
    case 786630: //learned enter, like go into something
      return ATV_LEARNED_ENTER;
    case 786631: //learned return, like go back
      return ATV_LEARNED_RETURN;
    case 786637:
      return ATV_ALUMINIUM_PLAY;
    default:
      ELOG(@"XBMCPureController: Unknown button press remoteAction = %i", remoteAction);
      return ATV_INVALID_BUTTON;
  }
}

- (BOOL)brEventAction:(BREvent*)event
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);

	if ([m_glView isAnimating])
  {
    eATVClientEvent xbmcclient_event = [self ATVClientEventFromBREvent:event];
    
    if ( xbmcclient_event == ATV_INVALID_BUTTON )
    {
      return NO;
    } 
    else
    {
      [mp_xbmclient handleEvent:xbmcclient_event];
      return TRUE;
    }
	}
  else
  {
		return [super brEventAction:event];
	}
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
