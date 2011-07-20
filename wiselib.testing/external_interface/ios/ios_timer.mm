#import "ios_timer.h"

@implementation iOsTimerController


- (id) initWithTime: (NSTimeInterval)someSeconds andDelegate: (ios_timer_delegate_t) aDelegate andUserData: (void*) aData {
   if ( (self = [super init]) != nil ) {
      delegate_ = aDelegate;
      userData_ = aData;
      seconds_ = someSeconds;
   }
   return self;
}

- (void) fireDelegate: (NSTimer*) theTimer {
#ifdef IOS_TIMER_DEBUG
      NSLog(@"IOS_TIMER_DEBUG: Fire!!!");
#endif  
   delegate_( userData_ );
}

- (void) startTimer {
#ifdef IOS_TIMER_DEBUG
      NSLog(@"IOS_TIMER_DEBUG: The bomb has been planted!");
#endif 
   [NSTimer scheduledTimerWithTimeInterval:seconds_
         target:self
         selector:@selector(fireDelegate:)
         userInfo: nil
         repeats:NO];
}

- (void) dealloc {
   userData_ = nil;
   seconds_ = nil;
   
   [super dealloc];
}

@end
