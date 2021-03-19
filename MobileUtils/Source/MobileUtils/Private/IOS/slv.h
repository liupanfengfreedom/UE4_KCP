//
//  slv.h
//  slv
//
//  Created by anti on 2020/9/20.
//  Copyright © 2020 anti. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface slv : NSObject
+(int)iosadd:(int)a sec:(int)b;
+(NSString *)getIPAddress:(BOOL)preferIPv4;
+(NSDictionary *)getIPAddresses;
@end
