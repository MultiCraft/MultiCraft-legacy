//
//	SDiOSVersion.h
//	SDVersion [https://github.com/sebyddd/SDVersion]
//
//	The MIT License (MIT)
//	Copyright (c) 2014-2017 Sebastian Dobrincu
//	Copyright (c) 2019-2020 MultiCraft Development Team
//

#pragma once

#import <sys/utsname.h>
#import <UIKit/UIKit.h>

#define SDVersion SDiOSVersion

typedef NS_ENUM(NSInteger, DeviceVersion){
	Simulator             = 1,

	// iPhone
	iPhone5S              = 104,
	iPhone6               = 105,
	iPhone6Plus           = 106,
	iPhone6S              = 107,
	iPhone6SPlus          = 108,
	iPhoneSE              = 109,
	iPhone7               = 110,
	iPhone7Plus           = 111,
	iPhone8               = 112,
	iPhone8Plus           = 113,
	iPhoneX               = 114,
	iPhoneXS              = 115,
	iPhoneXR              = 116,
	iPhoneXSMax           = 117,
	iPhone11              = 118,
	iPhone11Pro           = 119,
	iPhone11ProMax        = 120,
	iPhoneSE2Gen          = 121,
	iPhone12Mini          = 122,
	iPhone12              = 123,
	iPhone12Pro           = 124,
	iPhone12ProMax        = 125,
	iPhone13Mini          = 122,
	iPhone13              = 123,
	iPhone13Pro           = 124,
	iPhone13ProMax        = 125,

	// iPad
	iPadAir               = 205,
	iPadMini2             = 206,
	iPadAir2              = 207,
	iPadMini3             = 208,
	iPadMini4             = 209,
	iPadPro12Dot9Inch     = 210,
	iPadPro9Dot7Inch      = 211,
	iPad5                 = 212,
	iPadPro12Dot9Inch2Gen = 213,
	iPadPro10Dot5Inch     = 214,
	iPad6                 = 215,
	iPadPro11Inch         = 216,
	iPadPro12Dot9Inch3Gen = 217,
	iPadMini5             = 218,
	iPadMini6             = 219,
	iPadAir3              = 219,
	iPad7                 = 220,
	iPadPro11Inch2Gen     = 216,
	iPadPro12Dot9Inch4Gen = 217,
	iPad8                 = 218,
	iPadAir4              = 219,

	// iPod Touch
	iPodTouch6Gen         = 302,
	iPodTouch7Gen         = 303
};

@interface SDiOSVersion : NSObject
+ (DeviceVersion)deviceVersion;
@end

@implementation SDiOSVersion

+ (NSDictionary*)deviceNamesByCode
{
	static NSDictionary *deviceNamesByCode = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		deviceNamesByCode = @{
			// Simulator
			@"i386"       : @(Simulator),
			@"x86_64"     : @(Simulator),

			// iPhone
			@"iPhone6,1"  : @(iPhone5S),
			@"iPhone6,2"  : @(iPhone5S),
			@"iPhone7,2"  : @(iPhone6),
			@"iPhone7,1"  : @(iPhone6Plus),
			@"iPhone8,1"  : @(iPhone6S),
			@"iPhone8,2"  : @(iPhone6SPlus),
			@"iPhone8,4"  : @(iPhoneSE),
			@"iPhone12,8" : @(iPhoneSE2Gen),
			@"iPhone9,1"  : @(iPhone7),
			@"iPhone9,3"  : @(iPhone7),
			@"iPhone9,2"  : @(iPhone7Plus),
			@"iPhone9,4"  : @(iPhone7Plus),
			@"iPhone10,1" : @(iPhone8),
			@"iPhone10,4" : @(iPhone8),
			@"iPhone10,2" : @(iPhone8Plus),
			@"iPhone10,5" : @(iPhone8Plus),
			@"iPhone10,3" : @(iPhoneX),
			@"iPhone10,6" : @(iPhoneX),
			@"iPhone11,8" : @(iPhoneXR),
			@"iPhone11,2" : @(iPhoneXS),
			@"iPhone11,4" : @(iPhoneXSMax),
			@"iPhone11,6" : @(iPhoneXSMax),
			@"iPhone12,1" : @(iPhone11),
			@"iPhone12,3" : @(iPhone11Pro),
			@"iPhone12,5" : @(iPhone11ProMax),
			@"iPhone13,1" : @(iPhone12Mini),
			@"iPhone13,2" : @(iPhone12),
			@"iPhone13,3" : @(iPhone12Pro),
			@"iPhone13,4" : @(iPhone12ProMax),
			@"iPhone14,4" : @(iPhone13Mini),
			@"iPhone14,5" : @(iPhone13),
			@"iPhone14,2" : @(iPhone13Pro),
			@"iPhone14,3" : @(iPhone13ProMax),

			// iPad
			@"iPad6,11" : @(iPad5),
			@"iPad6,12" : @(iPad5),
			@"iPad7,5"  : @(iPad6),
			@"iPad7,6"  : @(iPad6),
			@"iPad7,11" : @(iPad7),
			@"iPad7,12" : @(iPad7),
			@"iPad11,6" : @(iPad8),
			@"iPad11,7" : @(iPad8),

			@"iPad4,1"  : @(iPadAir),
			@"iPad4,2"  : @(iPadAir),
			@"iPad4,3"  : @(iPadAir),
			@"iPad5,3"  : @(iPadAir2),
			@"iPad5,4"  : @(iPadAir2),
			@"iPad11,3" : @(iPadAir3),
			@"iPad11,4" : @(iPadAir3),
			@"iPad13,1" : @(iPadAir4),
			@"iPad13,2" : @(iPadAir4),

			@"iPad4,4"  : @(iPadMini2),
			@"iPad4,5"  : @(iPadMini2),
			@"iPad4,6"  : @(iPadMini2),
			@"iPad4,7"  : @(iPadMini3),
			@"iPad4,8"  : @(iPadMini3),
			@"iPad4,9"  : @(iPadMini3),
			@"iPad5,1"  : @(iPadMini4),
			@"iPad5,2"  : @(iPadMini4),
			@"iPad11,1" : @(iPadMini5),
			@"iPad11,2" : @(iPadMini5),
			@"iPad14,1" : @(iPadMini6),
			@"iPad14,2" : @(iPadMini6),

			@"iPad6,3"  : @(iPadPro9Dot7Inch),
			@"iPad6,4"  : @(iPadPro9Dot7Inch),
			@"iPad7,3"  : @(iPadPro10Dot5Inch),
			@"iPad7,4"  : @(iPadPro10Dot5Inch),
			@"iPad8,1"  : @(iPadPro11Inch),
			@"iPad8,2"  : @(iPadPro11Inch),
			@"iPad8,3"  : @(iPadPro11Inch),
			@"iPad8,4"  : @(iPadPro11Inch),
			@"iPad8,9"  : @(iPadPro11Inch2Gen),
			@"iPad8,10" : @(iPadPro11Inch2Gen),
			@"iPad6,7"  : @(iPadPro12Dot9Inch),
			@"iPad6,8"  : @(iPadPro12Dot9Inch),
			@"iPad7,1"  : @(iPadPro12Dot9Inch2Gen),
			@"iPad7,2"  : @(iPadPro12Dot9Inch2Gen),
			@"iPad8,5"  : @(iPadPro12Dot9Inch3Gen),
			@"iPad8,6"  : @(iPadPro12Dot9Inch3Gen),
			@"iPad8,7"  : @(iPadPro12Dot9Inch3Gen),
			@"iPad8,8"  : @(iPadPro12Dot9Inch3Gen),
			@"iPad8,11" : @(iPadPro12Dot9Inch4Gen),
			@"iPad8,12" : @(iPadPro12Dot9Inch4Gen),

			// iPod
			@"iPod7,1" : @(iPodTouch6Gen),
			@"iPod9,1" : @(iPodTouch7Gen)};
	});
	return deviceNamesByCode;
}

+ (DeviceVersion)deviceVersion
{
	struct utsname systemInfo;
	uname(&systemInfo);
	NSString *code = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
	DeviceVersion version = (DeviceVersion)[self.deviceNamesByCode[code] integerValue];
	return version;
}
@end

#define SDVersion4Inch		(([SDVersion deviceVersion] == iPhone5S) || ([SDVersion deviceVersion] == iPhoneSE) || \
							([SDVersion deviceVersion] == iPodTouch6Gen) || ([SDVersion deviceVersion] == iPodTouch7Gen))

#define SDVersion4and7Inch	(([SDVersion deviceVersion] == iPhone6) || ([SDVersion deviceVersion] == iPhone6S) || ([SDVersion deviceVersion] == iPhone7) || ([SDVersion deviceVersion] == iPhone8) || ([SDVersion deviceVersion] == iPhoneSE2Gen))

#define SDVersion5and5Inch	(([SDVersion deviceVersion] == iPhone6Plus) || ([SDVersion deviceVersion] == iPhone6SPlus) || ([SDVersion deviceVersion] == iPhone7Plus) || ([SDVersion deviceVersion] == iPhone8Plus) || ([SDVersion deviceVersion] == iPhone12Mini) || ([SDVersion deviceVersion] == iPhone13Mini))

#define SDVersion5and8Inch	(([SDVersion deviceVersion] == iPhoneX) || ([SDVersion deviceVersion] == iPhoneXS) || ([SDVersion deviceVersion] == iPhone11Pro))

#define SDVersion6and1Inch	(([SDVersion deviceVersion] == iPhoneXR) || ([SDVersion deviceVersion] == iPhone11) || ([SDVersion deviceVersion] == iPhone12) || ([SDVersion deviceVersion] == iPhone12Pro) || ([SDVersion deviceVersion] == iPhone13) || ([SDVersion deviceVersion] == iPhone13Pro))

#define SDVersion6and5Inch	(([SDVersion deviceVersion] == iPhoneXSMax) || ([SDVersion deviceVersion] == iPhone11ProMax) || ([SDVersion deviceVersion] == iPhone12ProMax) || ([SDVersion deviceVersion] == iPhone13ProMax))

#define SDVersion7and9Inch	(([SDVersion deviceVersion] == iPadMini2) || ([SDVersion deviceVersion] == iPadMini3) || ([SDVersion deviceVersion] == iPadMini4) || ([SDVersion deviceVersion] == iPadMini5) || ([SDVersion deviceVersion] == iPadMini6))

#define SDVersioniPhone12Series	(([SDVersion deviceVersion] == iPhone12Mini) || ([SDVersion deviceVersion] == iPhone12) || ([SDVersion deviceVersion] == iPhone12Pro) || ([SDVersion deviceVersion] == iPhone12ProMax) || \
								([SDVersion deviceVersion] == iPhone13Mini) || ([SDVersion deviceVersion] == iPhone13) || ([SDVersion deviceVersion] == iPhone13Pro) || ([SDVersion deviceVersion] == iPhone13ProMax))

//#define SDVersion11Inch		(([SDVersion deviceVersion] == iPadPro11Inch) || ([SDVersion deviceVersion] == iPadPro11Inch2Gen) || ([SDVersion deviceVersion] == iPadAir4))

//#define SDVersion12and9Inch	(([SDVersion deviceVersion] == iPadPro12Dot9Inch) || ([SDVersion deviceVersion] == iPadPro12Dot9Inch2Gen) || ([SDVersion deviceVersion] == iPadPro12Dot9Inch3Gen) || ([SDVersion deviceVersion] == iPadPro12Dot9Inch4Gen))
