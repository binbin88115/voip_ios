/*
 Copyright (c) 2014-2015, GoBelieve
 All rights reserved.
 
 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */
#include <arpa/inet.h>
#import "VOIPSession.h"
#import "VOIPService.h"
#import "stun.h"

#define VOIP_PORT 20002
#define STUN_SERVER  @"stun.counterpath.net"

@interface VOIPSession()

@property(nonatomic, assign) int dialCount;
@property(nonatomic, assign) time_t dialBeginTimestamp;
@property(nonatomic) NSTimer *dialTimer;

@property(nonatomic, assign) time_t acceptTimestamp;
@property(nonatomic) NSTimer *acceptTimer;

@property(nonatomic, assign) time_t refuseTimestamp;
@property(nonatomic) NSTimer *refuseTimer;

@property(atomic, assign) StunAddress4 mappedAddr;
@property(atomic, assign) NatType natType;
@property(nonatomic) BOOL hairpin;

@end

@implementation VOIPSession

-(id)init {
    self = [super init];
    if (self) {
        self.state = VOIP_ACCEPTING;
        
        self.voipPort = VOIP_PORT;
        self.stunServer = STUN_SERVER;
    }
    return self;
}

-(void)holePunch {
    self.natType = StunTypeUnknown;
    self.hairpin = NO;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        StunAddress4 addr;
        BOOL hairpin = NO;
        NatType stype = [self mapNatAddress:&addr hairpin:&hairpin];
        dispatch_async(dispatch_get_main_queue(), ^{
            self.natType = stype;
            self.mappedAddr = addr;
            
            
            if (self.localNatMap == nil) {
                self.localNatMap = [[NatPortMap alloc] init];
                self.localNatMap.ip = self.mappedAddr.addr;
                self.localNatMap.port = self.mappedAddr.port;
                
                //self.localNatMap.localIP = [self getPrimaryIP];
                //self.localNatMap.localPort = [Config instance].voipPort;
            }
            
        });
    });
}



#define VERBOSE false
-(NatType)mapNatAddress:(StunAddress4*)eaddr hairpin:(BOOL*)ph{
    int fd = -1;
    StunAddress4 mappedAddr;
    StunAddress4 stunServerAddr;
    NSString *stunServer = self.stunServer;
    stunParseServerName( (char*)[stunServer UTF8String], stunServerAddr);
    
    NSLog(@"nat mapping...");
    bool presPort = false, hairpin = false;
    NatType stype = stunNatType( stunServerAddr, VERBOSE, &presPort, &hairpin,
                                0, NULL);
    
    NSLog(@"nat type:%d", stype);
    *ph = hairpin;
    
    BOOL isOpen = NO;
    switch (stype)
    {
        case StunTypeFailure:
            break;
        case StunTypeUnknown:
            break;
        case StunTypeBlocked:
            break;
            
        case StunTypeOpen:
        case StunTypeFirewall:
            //todo get local address
        case StunTypeIndependentFilter:
        case StunTypeDependentFilter:
        case StunTypePortDependedFilter:
            isOpen = YES;
            break;
        case StunTypeDependentMapping:
            break;
        default:
            break;
    }
    
    
    if (!isOpen) {
        return stype;
    }
    for (int i = 0; i < 8; i++) {
        fd = stunOpenSocket(stunServerAddr, &mappedAddr, self.voipPort, NULL, VERBOSE);
        if (fd == -1) {
            continue;
        }
        break;
    }
    if (fd != -1) {
        close(fd);
        struct in_addr addr;
        addr.s_addr = htonl(mappedAddr.addr);
        NSLog(@"mapped address:%s:%d", inet_ntoa(addr), mappedAddr.port);
        *eaddr = mappedAddr;
    } else {
        NSLog(@"map nat address fail");
    }
    return stype;
}


- (void)sendDial {
    NSLog(@"dial...");
    VOIPControl *ctl = [[VOIPControl alloc] init];
    ctl.sender = self.currentUID;
    ctl.receiver = self.peerUID;
    ctl.cmd = VOIP_COMMAND_DIAL;
    ctl.dialCount = self.dialCount + 1;
    BOOL r = [[VOIPService instance] sendVOIPControl:ctl];
    if (r) {
        self.dialCount = self.dialCount + 1;
    } else {
        NSLog(@"dial fail");
    }
    
    time_t now = time(NULL);
    if (now - self.dialBeginTimestamp >= 60) {
        NSLog(@"dial timeout");
        
        //ondialtimeout
        [self.delegate onDialTimeout];
    }
}

-(void)sendControlCommand:(enum VOIPCommand)cmd {
    VOIPControl *ctl = [[VOIPControl alloc] init];
    ctl.sender = self.currentUID;
    ctl.receiver = self.peerUID;
    ctl.cmd = cmd;
    [[VOIPService instance] sendVOIPControl:ctl];
}

-(void)sendRefused {
    [self sendControlCommand:VOIP_COMMAND_REFUSED];
}

-(void)sendTalking:(int64_t)receiver {
    VOIPControl *ctl = [[VOIPControl alloc] init];
    ctl.sender = self.currentUID;
    ctl.receiver = receiver;
    ctl.cmd = VOIP_COMMAND_TALKING;
    [[VOIPService instance] sendVOIPControl:ctl];
}

-(void)sendReset {
    [self sendControlCommand:VOIP_COMMAND_RESET];
}

-(void)sendConnected {
    VOIPControl *ctl = [[VOIPControl alloc] init];
    ctl.sender = self.currentUID;
    ctl.receiver = self.peerUID;
    ctl.cmd = VOIP_COMMAND_CONNECTED;
    ctl.natMap = self.localNatMap;
    
    if (self.relayIP.length > 0) {
        in_addr_t addr = inet_addr([self.relayIP UTF8String]);
        ctl.relayIP = ntohl(addr);
    }
    
    [[VOIPService instance] sendVOIPControl:ctl];
}

-(void)sendDialAccept {
    VOIPControl *ctl = [[VOIPControl alloc] init];
    ctl.sender = self.currentUID;
    ctl.receiver = self.peerUID;
    ctl.cmd = VOIP_COMMAND_ACCEPT;
    ctl.natMap = self.localNatMap;
    
    [[VOIPService instance] sendVOIPControl:ctl];
    
    time_t now = time(NULL);
    if (now - self.acceptTimestamp >= 10) {
        NSLog(@"accept timeout");
        [self.acceptTimer invalidate];

        //onaccepttimeout
        [self.delegate onAcceptTimeout];
    }
}

-(void)sendDialRefuse {
    [self sendControlCommand:VOIP_COMMAND_REFUSE];
    
    time_t now = time(NULL);
    if (now - self.refuseTimestamp > 10) {
        NSLog(@"refuse timeout");
        [self.refuseTimer invalidate];
        
        VOIPSession *voip = self;
        voip.state = VOIP_REFUSED;

        [self.delegate onRefuseFinished];
    }
}

-(void)sendHangUp {
    NSLog(@"send hang up");
    [self sendControlCommand:VOIP_COMMAND_HANG_UP];
}

#pragma mark - VOIPObserver
-(void)onVOIPControl:(VOIPControl*)ctl {
    VOIPSession *voip = self;
    
    if (ctl.sender != self.peerUID) {
        [self sendTalking:ctl.sender];
        return;
    }
    NSLog(@"voip state:%d command:%d", voip.state, ctl.cmd);
    
    if (voip.state == VOIP_DIALING) {
        if (ctl.cmd == VOIP_COMMAND_ACCEPT) {
            self.peerNatMap = ctl.natMap;
            
            if (self.localNatMap == nil) {
                self.localNatMap = [[NatPortMap alloc] init];
            }
            
            if (self.relayIP == nil) {
                self.relayIP = [VOIPService instance].relayIP;
            }
            
            [self sendConnected];
            voip.state = VOIP_CONNECTED;
            [self.dialTimer invalidate];
            self.dialTimer = nil;

            //onconnected
            [self.delegate onConnected];
        } else if (ctl.cmd == VOIP_COMMAND_REFUSE) {
            voip.state = VOIP_REFUSED;
            
            [self sendRefused];
            
            [self.dialTimer invalidate];
            self.dialTimer = nil;

            //onrefuse
            [self.delegate onRefuse];
            
        } else if (ctl.cmd == VOIP_COMMAND_TALKING) {
            voip.state = VOIP_SHUTDOWN;
            
            [self.dialTimer invalidate];
            self.dialTimer = nil;
            
            [self.delegate onTalking];
        }
    } else if (voip.state == VOIP_ACCEPTING) {
        if (ctl.cmd == VOIP_COMMAND_HANG_UP) {
            voip.state = VOIP_HANGED_UP;
            //onhangup
            [self.delegate onHangUp];
        }
    } else if (voip.state == VOIP_ACCEPTED) {
        if (ctl.cmd == VOIP_COMMAND_CONNECTED) {
            NSLog(@"called voip connected");

            self.peerNatMap = ctl.natMap;
            if (ctl.relayIP > 0) {
                in_addr addr;
                addr.s_addr = htonl(ctl.relayIP);
                char buff[64] = {0};
                const char *str = inet_ntop(AF_INET, &addr, buff, 64);
                self.relayIP = [NSString stringWithUTF8String:str];
            } else {
                self.relayIP = [VOIPService instance].relayIP;
            }
            
            [self.acceptTimer invalidate];
            voip.state = VOIP_CONNECTED;
            
            //onconnected
            [self.delegate onConnected];

        }
    } else if (voip.state == VOIP_CONNECTED) {
        if (ctl.cmd == VOIP_COMMAND_HANG_UP) {
            voip.state = VOIP_HANGED_UP;

            //onhangup
            [self.delegate onHangUp];
        } else if (ctl.cmd == VOIP_COMMAND_ACCEPT) {
            [self sendConnected];
        }
    } else if (voip.state == VOIP_REFUSING) {
        if (ctl.cmd == VOIP_COMMAND_REFUSED) {
            NSLog(@"refuse finished");
            voip.state = VOIP_REFUSED;
            //onRefuseFinished
            [self.delegate onRefuseFinished];
        }
    }
}


-(void)dial {
    self.state = VOIP_DIALING;
    
    self.dialBeginTimestamp = time(NULL);
    [self sendDial];
    self.dialTimer = [NSTimer scheduledTimerWithTimeInterval: 1
                                                      target:self
                                                    selector:@selector(sendDial)
                                                    userInfo:nil
                                                     repeats:YES];
}

-(void)accept {
    VOIPSession *voip = self;
    voip.state = VOIP_ACCEPTED;
    
    if (self.localNatMap == nil) {
        self.localNatMap = [[NatPortMap alloc] init];
    }
    
    self.acceptTimestamp = time(NULL);
    self.acceptTimer = [NSTimer scheduledTimerWithTimeInterval: 1
                                                        target:self
                                                      selector:@selector(sendDialAccept)
                                                      userInfo:nil
                                                       repeats:YES];
    [self sendDialAccept];
    


}
-(void)refuse {
    self.state = VOIP_REFUSING;
    
    self.refuseTimestamp = time(NULL);
    self.refuseTimer = [NSTimer scheduledTimerWithTimeInterval: 1
                                                        target:self
                                                      selector:@selector(sendDialRefuse)
                                                      userInfo:nil
                                                       repeats:YES];
    
    [self sendDialRefuse];

}

-(void)hangUp {
    VOIPSession *voip = self;
    if (voip.state == VOIP_DIALING ) {
        [self.dialTimer invalidate];
        self.dialTimer = nil;

        [self sendHangUp];
        voip.state = VOIP_HANGED_UP;
    } else if (voip.state == VOIP_CONNECTED) {
        [self sendHangUp];
        voip.state = VOIP_HANGED_UP;
    }else {
        NSLog(@"invalid voip state:%d", voip.state);
    }
}


@end
