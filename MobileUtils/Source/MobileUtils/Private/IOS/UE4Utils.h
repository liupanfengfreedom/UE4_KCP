//
//  UE4Utils.h
//  KXUnboundedLife
//
//  Created by atme on 2019/5/24.
//  Copyright © 2019年 atme. All rights reserved.
//

#pragma once

#import <UIKit/UIKit.h>

@interface UE4Utils : NSObject

+ (void)messageCallback:(int)message jsonString:(const char *)json_uc;

@end
