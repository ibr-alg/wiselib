// vim: set noexpandtab ts=4 sw=4:

#ifndef IOS_TIMER_H
#define IOS_TIMER_H

#include "util/delegates/delegate.hpp"
#include "ios_system.h"
#import <Foundation/Foundation.h>

typedef delegate1<void, void*> ios_timer_delegate_t;

@interface iOsTimerController : NSObject {
   ios_timer_delegate_t delegate_;
   void* userData_;
   NSTimeInterval seconds_;
}

- (id) initWithTime: (NSTimeInterval)seconds andDelegate: (ios_timer_delegate_t) aDelegate andUserData: (void*) aData;
- (void) startTimer;
- (void) fireDelegate: (NSTimer*)theTimer;

@end

namespace wiselib
{

   template<typename OsModel_P>
   class iOsTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef iOsTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      iOsTimerModel( iOsSystem& system )
         : system_(system)
      {
      }
      // --------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata )
      {     
         NSTimeInterval seconds = millis/1000.0;
         iOsTimerController *timer = [[iOsTimerController alloc] initWithTime: seconds
                                             andDelegate: ios_timer_delegate_t::from_method<T, TMethod>(obj_pnt)
                                             andUserData: userdata];
         [timer startTimer];        
         [timer release];
         return SUCCESS;
      }

   private:
      iOsSystem& system()
      { return system_; }
      // --------------------------------------------------------------------
      iOsSystem& system_;
   };
   
}
#endif // IOS_TIMER_H

