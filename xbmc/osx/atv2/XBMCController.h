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

#import <Foundation/Foundation.h>
#import <BackRow/BackRow.h>
#import "XBMCEAGLView.h"

@interface XBMCController : BRController
{
  int padding[16];  // credit is due here to SapphireCompatibilityClasses!!
        
  int m_screensaverTimeout;
  BRController *m_controller;
}
// message from which our instance is obtained
+ (XBMCController*) sharedInstance;
- (XBMCEAGLView*) getEGLView;
@end

