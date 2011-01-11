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
#undef BOOL

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "XBMCEAGLView.h"
#import "XBMCApplication.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
@interface XBMCEAGLView (PrivateMethods)
- (void) deleteFramebuffer;
@end

@implementation XBMCEAGLView
@synthesize firstTouch;
@synthesize lastTouch;

@dynamic context;

// You must implement this method
+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

/*
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch* touch = [[event touchesForView:self] anyObject];
  firstTouch = [touch locationInView:self];
  lastTouch = [touch locationInView:self];
  
  NSLog(@"%s touchesBegan x=%d, y=%d ", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y);
  
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));
  
  newEvent.type = XBMC_MOUSEBUTTONDOWN;
  newEvent.button.type = XBMC_MOUSEBUTTONDOWN;
  newEvent.button.button = XBMC_BUTTON_LEFT;
  newEvent.button.state = XBMC_PRESSED;
  newEvent.button.x = lastTouch.x;
  newEvent.button.y = lastTouch.y;
  g_application.OnEvent(newEvent);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch *touch = [touches anyObject];
  lastTouch = [touch locationInView:self];
  
  NSLog(@"%s touchesMoved x=%d, y=%d ", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y);
  
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));
  
  newEvent.type = XBMC_MOUSEBUTTONDOWN;
  newEvent.motion.x = lastTouch.x;
  newEvent.motion.y = lastTouch.y;
  newEvent.motion.state = 0;
  
  g_application.OnEvent(newEvent);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  UITouch *touch = [touches anyObject];
  lastTouch = [touch locationInView:self];
  
  NSLog(@"%s touchesEnded x=%d, y=%d ", __PRETTY_FUNCTION__, lastTouch.x, lastTouch.y);
  
  // use double tapp as right mouse key
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));
  
  newEvent.type = XBMC_MOUSEBUTTONUP;
  if([touch tapCount] == 2) {
    newEvent.button.button = XBMC_BUTTON_RIGHT;
    newEvent.button.type = XBMC_MOUSEBUTTONUP;
    newEvent.button.state = XBMC_PRESSED;
    newEvent.button.x = lastTouch.x;
    newEvent.button.y = lastTouch.y;
    g_application.OnEvent(newEvent);
  } else {
    newEvent.button.button = XBMC_BUTTON_LEFT;  
  }
  newEvent.button.type = XBMC_MOUSEBUTTONUP;
  newEvent.button.state = XBMC_RELEASED;
  newEvent.button.x = lastTouch.x;
  newEvent.button.y = lastTouch.y;
  g_application.OnEvent(newEvent);  
}
*/

- (id)initWithCoder:(NSCoder*)coder
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
	
  self = [super initWithCoder:coder];
  if (self)
  {
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
									  //[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
									  //kEAGLColorFormatRGB565,
									  kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
									  nil];
	/*  
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
								[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
								kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
								nil];
	*/
  }
	
  return self;
}

//--------------------------------------------------------------
- (void) dealloc
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  [self deleteFramebuffer];    
  [context release];
  
  [super dealloc];
}

//--------------------------------------------------------------
- (EAGLContext *)context
{
  return context;
}
//--------------------------------------------------------------
- (void)setContext:(EAGLContext *)newContext
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  if (context != newContext)
  {
    [self deleteFramebuffer];
    
    [context release];
    context = [newContext retain];
    
    [EAGLContext setCurrentContext:nil];
  }
}

//--------------------------------------------------------------
- (void)createFramebuffer
{
  if (context && !defaultFramebuffer)
  {
    //NSLog(@"%s", __PRETTY_FUNCTION__);
    [EAGLContext setCurrentContext:context];
    
    // Create default framebuffer object.
    glGenFramebuffers(1, &defaultFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    // Create color render buffer and allocate backing store.
    glGenRenderbuffers(1, &colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
  }
}
//--------------------------------------------------------------
- (void) deleteFramebuffer
{
  if (context)
  {
    //NSLog(@"%s", __PRETTY_FUNCTION__);
    [EAGLContext setCurrentContext:context];
    
    if (defaultFramebuffer)
    {
      glDeleteFramebuffers(1, &defaultFramebuffer);
      defaultFramebuffer = 0;
    }
    
    if (colorRenderbuffer)
    {
      glDeleteRenderbuffers(1, &colorRenderbuffer);
      colorRenderbuffer = 0;
    }
  }
}
//--------------------------------------------------------------
- (void) setFramebuffer
{

  if (context)
  {
	//NSLog(@"%s", __PRETTY_FUNCTION__);
	[EAGLContext setCurrentContext:context];
	  
	if (!defaultFramebuffer)
	  [self createFramebuffer];
	  
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glScissor(0, 0, framebufferWidth, framebufferHeight);
  }

}
//--------------------------------------------------------------
- (BOOL) presentFramebuffer
{
  bool success = FALSE;
  
  if (context)
  {
    //NSLog(@"%s", __PRETTY_FUNCTION__);
   
    if ([EAGLContext currentContext] != context)
      [EAGLContext setCurrentContext:context];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    success = [context presentRenderbuffer:GL_RENDERBUFFER];
  }
  
  return success;
}
//--------------------------------------------------------------
- (void)layoutSubviews
{
    //NSLog(@"%s", __PRETTY_FUNCTION__);

    // The framebuffer will be re-created at the beginning of the next setFramebuffer method call.
    [self deleteFramebuffer];
}

@end
