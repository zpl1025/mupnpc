//
//  CGUpnpAvController.h
//  CyberLink for C
//
//  Created by Satoshi Konno on 08/06/25.
//  Copyright 2008 Satoshi Konno. All rights reserved.
//

#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

#if !defined(CG_NOUSE_CLINKC_FRAMEWORK)
#import <CyberLink/UPnP.h>
#else
#import <UPnP.h>
#endif

@class CGUpnpAvServer;
@class CGUpnpAvObject;

/**
 * The CGUpnpControlPoint class is a wrapper class for CgUpnpControlPoint of CyberLink for C to 
 * program using only Objective-C directly on MacOSX.
 * Currently, the only basic methods are wrapped to control UPnP devices.
 */
@interface CGUpnpAvController : CGUpnpControlPoint 
{
}
/**
 * Activate some background threads of the control point such as SSDP and 
 * HTTP servers to listen messages and events of UPnP. You must call this 
 * method before you can actually use a control point.
 *
 * @return TRUE if successful; otherwise FALSE
 *
 */
- (NSArray *)servers;
- (CGUpnpAvServer *)serverForUDN:(NSString *)aUdn;
- (CGUpnpAvServer *)serverForFriendlyName:(NSString *)aFriendlyName;
- (CGUpnpAvServer *)serverForPath:(NSString *)aPath;
- (NSArray *)browseWithTitlePath:(NSString *)aServerAndTitlePath;
- (CGUpnpAvObject *)objectForTitlePath:(NSString *)aServerAndTitlePath;
- (CGUpnpAvObject *)objectForIndexPath:(NSIndexPath *)aServerAndTitleIndexPath;
@end