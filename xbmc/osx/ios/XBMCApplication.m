//
//  OpenEGLDemoAppliance.m
//
//  Created by Edgar Hucek on 12/06/10.
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

#import "XBMCApplication.h"
#import "XBMCController.h"

@implementation XBMCApplicationDelegate

@synthesize window;
@synthesize xbmcController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  //NSLog(@"%s", __PRETTY_FUNCTION__);
  [self.window addSubview:self.xbmcController.view];
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  [self.xbmcController startAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  [self.xbmcController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  [self.xbmcController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
  // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
  // Handle any foreground procedures not related to animation here.
}

/*
- (void)applicationDidFinishLaunching:(UIApplication *)application { 
  xbmcController = [[XBMCController alloc]initWithNibName:@"xbmcController" bundle:nil];
	
  NSLog(@"%s", __PRETTY_FUNCTION__);
  [window addSubview:[xbmcController view]];
  [window makeKeyAndVisible];
}
*/

- (void)dealloc
{
    [xbmcController release];
    [window release];
	
    [super dealloc];
}
@end

int main(int argc, char *argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];	

  // Block SIGPIPE
  // SIGPIPE repeatably kills us, turn it off
  {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);
  }
  
  //NSLog(@"%s", __PRETTY_FUNCTION__);

  @try
  {
    
    UIApplicationMain(argc, argv, nil, nil);
  
  } 
  @catch (id theException) 
  {
    NSLog(@"%@", theException);
  }
  @finally 
  {
    NSLog(@"This always happens.");
  }
    
  [pool release];
	
  return noErr;

}
