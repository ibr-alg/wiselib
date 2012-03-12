/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

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

