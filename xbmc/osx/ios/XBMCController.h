//  OpenEGLDemoController.h
//
//  Created by Scott Davilla and Thomas Cool on 10/20/10.
/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <AudioToolbox/AudioToolbox.h>

#import "XBMC_events.h"
#include "XBMC_keysym.h"

@interface XBMCController : UIViewController
{
  int m_screensaverTimeout;
	
  /* Touch handling */
  CGPoint firstTouch;
  CGPoint lastTouch;
  CGSize screensize;
	
  UIInterfaceOrientation orientation;

  XBMC_Event lastEvent;
}
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property CGPoint firstTouch;
@property CGPoint lastTouch;
@property CGSize screensize;
@property XBMC_Event lastEvent;

// message from which our instance is obtained
- (void)pauseAnimation;
- (void)resumeAnimation;
- (void)startAnimation;
- (void)stopAnimation;
- (void) sendKey: (XBMCKey) key;
- (void) observeDefaultCenterStuff: (NSNotification *) notification;
- (void) initDisplayLink;
- (void) deinitDisplayLink;
- (double) getDisplayLinkFPS;
- (void) setFramebuffer;
- (bool) presentFramebuffer;
- (CGSize) getScreenSize;
- (UIInterfaceOrientation) getOrientation;
- (void)createGestureRecognizers;
- (void) disableSystemSleep;
- (void) enableSystemSleep;
- (void) disableScreenSaver;
- (void) enableScreenSaver;
@end

extern XBMCController *g_xbmcController;
