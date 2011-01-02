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

//hack around problem with xbmc's typedef int BOOL
// and obj-c's typedef unsigned char BOOL
#define BOOL XBMC_BOOL 
#include <sys/resource.h>
#include <signal.h>

#include "system.h"
#include "AdvancedSettings.h"
#include "FileItem.h"
#include "Application.h"
#include "MouseStat.h"
#include "WindowingFactory.h"
#include "VideoReferenceClock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "Util.h"
#include "WinEventsIOS.h"
#undef BOOL

#import "XBMCEAGLView.h"

#import <BackRow/BackRow.h>

#import "XBMCController.h"
#import "XBMCApplication.h"
#import "xbmcclientwrapper.h"
#import "XBMCDebugHelpers.h"
#import "iOSUtils.h"

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

// notification messages
extern NSString* kBRScreenSaverActivated;
extern NSString* kBRScreenSaverDismissed;

//--------------------------------------------------------------
//

@interface XBMCController ()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
@end

@implementation XBMCController
@synthesize animating, context, displayLink;
@synthesize firstTouch;
@synthesize lastTouch;
@synthesize lastEvent;

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  if(interfaceOrientation == UIInterfaceOrientationLandscapeLeft)
    return YES;
  else {
    return NO;
  }

}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
  orientation = toInterfaceOrientation;
}

- (UIInterfaceOrientation) getOrientation
{
	return orientation;
}

-(void)sendKey:(uint16_t) key
{
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  newEvent.key.keysym.unicode = key;

  newEvent.type = XBMC_KEYDOWN;
  CWinEventsIOS::MessagePush(&newEvent);

  newEvent.type = XBMC_KEYUP;
  CWinEventsIOS::MessagePush(&newEvent);
}

#define kMinimumGestureLength  100
#define kMaximumVariance   10

-(bool)handleSwipe {

  //CGFloat deltaX = fabsf(firstTouch.x - lastTouch.x);
  //CGFloat deltaY = fabsf(firstTouch.y - lastTouch.y);

  CGFloat deltaX = firstTouch.x - lastTouch.x;
  CGFloat deltaY = firstTouch.y - lastTouch.y;

  if ( (deltaX > kMinimumGestureLength) && ( deltaY < kMaximumVariance) ) 
  {
    //[self handleSwipeLeft];
    [self sendKey:8];
    NSLog(@"%s Swipe left", __PRETTY_FUNCTION__);
  }
  else if ( (deltaX*(-1) > kMinimumGestureLength) && ( deltaY < kMaximumVariance) ) 
  {
    [self sendKey:9];
    //[self handleSwipeRight];
    NSLog(@"%s Swipe right", __PRETTY_FUNCTION__);
  }
  else if ( (deltaY > kMinimumGestureLength) && ( deltaX < kMaximumVariance) ) 
  {
    [self sendKey:'c'];
    NSLog(@"%s Swipe up", __PRETTY_FUNCTION__);
  }
  else if( (deltaY*(-1) > kMinimumGestureLength) && ( deltaX < kMaximumVariance) ) 
  {
    [self sendKey:'o' ];
    NSLog(@"%s Swipe down", __PRETTY_FUNCTION__);
  }

  NSLog(@"%s handleSwipe x=%f, y=%f", __PRETTY_FUNCTION__, deltaX, deltaY);

  return false;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch* touch = [[event touchesForView:self.view] anyObject];
  firstTouch = [touch locationInView:self.view];
  lastTouch = [touch locationInView:self.view];

  //NSLog(@"%s touchesBegan x=%d, y=%d count=%d", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y, touch.tapCount);

  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  newEvent.type = XBMC_MOUSEBUTTONDOWN;
  newEvent.button.type = XBMC_MOUSEBUTTONDOWN;
  newEvent.button.x = lastTouch.x;
  newEvent.button.y = lastTouch.y;

  if ([touch tapCount] == 2) {
    newEvent.button.button = XBMC_BUTTON_RIGHT;
  } else {
    newEvent.button.button = XBMC_BUTTON_LEFT;
  }
  
  CWinEventsIOS::MessagePush(&newEvent);
  /* Store the tap action for later */
  memcpy(&lastEvent, &newEvent, sizeof(XBMC_Event));
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch *touch = [touches anyObject];
  lastTouch = [touch locationInView:self.view];

  //NSLog(@"%s touchesMoved x=%d, y=%d count=%d", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y, touch.tapCount);

  static int nCount = 0;
  
  if(nCount == 4) {
  
    XBMC_Event newEvent;
    memcpy(&newEvent, &lastEvent, sizeof(XBMC_Event));

    newEvent.motion.x = lastTouch.x;
    newEvent.motion.y = lastTouch.y;
    //newEvent.motion.state = 0;

    CWinEventsIOS::MessagePush(&newEvent);
    
    nCount = 0;
    
  } else {
    
    nCount++;
  
  }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch *touch = [touches anyObject];
  lastTouch = [touch locationInView:self.view];
  
  //NSLog(@"%s touchesEnded x=%d, y=%d ", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y);

  //[self handleSwipe];

  XBMC_Event newEvent;
  memcpy(&newEvent, &lastEvent, sizeof(XBMC_Event));

  newEvent.type = XBMC_MOUSEBUTTONUP;
  newEvent.button.type = XBMC_MOUSEBUTTONUP;
  newEvent.button.x = lastTouch.x;
  newEvent.button.y = lastTouch.y;
  CWinEventsIOS::MessagePush(&newEvent);

  memset(&lastEvent, 0x0, sizeof(XBMC_Event));         
}

- (void)awakeFromNib
{ 
  NSLog(@"%s", __PRETTY_FUNCTION__);

  NSNotificationCenter *center;
  // first the default notification center, which is all
  // notifications that only happen inside of our program
  center = [NSNotificationCenter defaultCenter];
  [center addObserver: self
    selector: @selector(observeDefaultCenterStuff:)
    name: nil
    object: nil];
	
  CWinEventsIOS::Init();

  EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	
  if (!aContext)
    NSLog(@"Failed to create ES context");
  else if (![EAGLContext setCurrentContext:aContext])
	NSLog(@"Failed to set ES context current");
	
  self.context = aContext;
  [aContext release];
	
  [(XBMCEAGLView *)self.view setContext:context];
  [(XBMCEAGLView *)self.view createFramebuffer];
  [(XBMCEAGLView *)self.view setFramebuffer];

  animating = FALSE;
  self.displayLink = nil;
	
  g_xbmcController = self;
  
  bool use_universal = NO;
  NSString *client_address= @"localhost";
  mp_xbmclient = [[XBMCClientWrapper alloc] initWithUniversalMode:use_universal serverAddress:client_address];
}

- (void)dealloc
{
  NSLog(@"%s", __PRETTY_FUNCTION__);

	[mp_xbmclient release];

  NSNotificationCenter *center;
  // take us off the default center for our app
  center = [NSNotificationCenter defaultCenter];
  [center removeObserver: self];

  // Tear down context.
  if ([EAGLContext currentContext] == context)
	  [EAGLContext setCurrentContext:nil];
  self.context = nil;

  CWinEventsIOS::DeInit();

  [context release];	
	
  [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
  NSLog(@"%s", __PRETTY_FUNCTION__);

  [self startAnimation];
	
  [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{  
  NSLog(@"%s", __PRETTY_FUNCTION__);
  
  [self stopAnimation];
	
  [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
  NSLog(@"%s", __PRETTY_FUNCTION__);
  
	[super viewDidUnload];
	
  // Tear down context.
  if ([EAGLContext currentContext] == context)
      [EAGLContext setCurrentContext:nil];
	self.context = nil;
}

- (void)viewDidLoad
{
  [super viewDidLoad];
  
  self.view.multipleTouchEnabled=YES;  
}

- (void)presentFramebuffer
{
	
	[(XBMCEAGLView *)self.view presentFramebuffer];
}

- (void)setFramebuffer
{
	
	[(XBMCEAGLView *)self.view setFramebuffer];
}

- (void)startAnimation
{
  NSLog(@"%s", __PRETTY_FUNCTION__);
  
	if (!animating)
	{
		animating = TRUE;
		// kick off an animation thread
		animationThreadLock = [[NSConditionLock alloc] initWithCondition: FALSE];
		animationThread = [[NSThread alloc] initWithTarget:self 
												  selector:@selector(runAnimation:) 
													object:animationThreadLock];
		[animationThread start];
		
		[self initDisplayLink];
	}
	
}

- (void)stopAnimation
{
  NSLog(@"%s", __PRETTY_FUNCTION__);

	if (animating)
	{
		[self deinitDisplayLink];
		animating = FALSE;
		g_application.Stop();
		// wait for animation thread to die
		if ([animationThread isFinished] == NO)
			[animationThreadLock lockWhenCondition:TRUE];
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;		
	}
}

//--------------------------------------------------------------
- (void) runAnimation:(id) arg
{
	NSLog(@"%s", __PRETTY_FUNCTION__);
  CCocoaAutoPool outerpool;
	
	//[NSThread setThreadPriority:1]
	// Changing to SCHED_RR is safe under OSX, you don't need elevated privileges and the
	// OSX scheduler will monitor SCHED_RR threads and drop to SCHED_OTHER if it detects
	// the thread running away. OSX automatically does this with the CoreAudio audio
	// device handler thread.
	int32_t result;
	thread_extended_policy_data_t theFixedPolicy;
	
	// make thread fixed, set to 'true' for a non-fixed thread
	theFixedPolicy.timeshare = false;
	result = thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_EXTENDED_POLICY, 
							   (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);
	
	int policy;
	struct sched_param param;
	result = pthread_getschedparam(pthread_self(), &policy, &param );
	// change from default SCHED_OTHER to SCHED_RR
	policy = SCHED_RR;
	result = pthread_setschedparam(pthread_self(), policy, &param );
	
	// signal we are alive
	NSConditionLock* myLock = arg;
	[myLock lock];
	
#ifdef _DEBUG
    g_advancedSettings.m_logLevel     = LOG_LEVEL_DEBUG;
    g_advancedSettings.m_logLevelHint = LOG_LEVEL_DEBUG;
#else
    g_advancedSettings.m_logLevel     = LOG_LEVEL_NORMAL;
    g_advancedSettings.m_logLevelHint = LOG_LEVEL_NORMAL;
#endif
	
	// Prevent child processes from becoming zombies on exit if not waited upon. See also Util::Command
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDWAIT;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);
  
	setlocale(LC_NUMERIC, "C");
	g_advancedSettings.Initialize();
	
	g_advancedSettings.m_startFullScreen = true;
  g_application.SetStandAlone(true);
	
	g_application.Preflight();
	if (g_application.Create())
	{
		try
		{
			while (animating)
			{
        CCocoaAutoPool innerpool;
				g_application.Run();
			}
			g_Windowing.DestroyWindow();
		}
		catch(...)
		{
			NSLog(@"%sException caught on main loop. Exiting", __PRETTY_FUNCTION__);
		}
	}
	else
	{
		NSLog(@"%sUnable to create application", __PRETTY_FUNCTION__);
	}
	
	// signal we are dead
	[myLock unlockWithCondition:TRUE];
	
	NSLog(@"%s:exit", __PRETTY_FUNCTION__);
}

//--------------------------------------------------------------
- (void) runDisplayLink;
{
	//NSLog(@"%s", __PRETTY_FUNCTION__);
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	displayFPS = 1.0 / [displayLink duration];
	if (animationThread && [animationThread isExecuting] == YES)
	{
		if (g_VideoReferenceClock)
			g_VideoReferenceClock.VblankHandler(CurrentHostCounter(), displayFPS);
	}
	[pool release];
}
//--------------------------------------------------------------
- (void) initDisplayLink
{
	NSLog(@"%s", __PRETTY_FUNCTION__);
	CADisplayLink *aDisplayLink = [NSClassFromString(@"CADisplayLink") 
				   displayLinkWithTarget:self
				   selector:@selector(runDisplayLink)];
	[aDisplayLink setFrameInterval:1];
	[aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	self.displayLink = aDisplayLink;
	displayFPS = 60;
}
//--------------------------------------------------------------
- (void) deinitDisplayLink
{
	NSLog(@"%s", __PRETTY_FUNCTION__);
	[displayLink invalidate];
	displayLink = nil;
}
//--------------------------------------------------------------
- (double) getDisplayLinkFPS;
{
	NSLog(@"%s:displayFPS(%f)", __PRETTY_FUNCTION__, displayFPS);
	return displayFPS;
}
- (CGSize) getScreenSize
{
  CGSize screensize;

  if (orientation == UIInterfaceOrientationPortrait)
  {
    screensize.width  = [[UIScreen mainScreen] bounds].size.width;
    screensize.height = [[UIScreen mainScreen] bounds].size.height;
  }
  else
  {
    screensize.height = [[UIScreen mainScreen] bounds].size.width;
    screensize.width  = [[UIScreen mainScreen] bounds].size.height;
  }
  return screensize;
}

- (BOOL) recreateOnReselect
{ 
  NSLog(@"%s", __PRETTY_FUNCTION__);
  return YES;
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
    // Release any cached data, images, etc. that aren't in use.
}

- (eATVClientEvent) ATVClientEventFromBREvent:(BREvent*) f_event
{
  BOOL downEvent = [f_event value];
  int remoteAction = [f_event remoteAction];
  DLOG(@"got action %i %@", remoteAction, (downEvent)? @"pressed":@"released");
  
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
  NSLog(@"%s", __PRETTY_FUNCTION__);

  if (animating)
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
    // TODO 
    //return [super brEventAction:event];
	return FALSE;
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
