#ifndef IOS_CLOCK_H
#define IOS_CLOCK_H

#include "external_interface/ios/ios_system.h"

namespace wiselib
{
   template<typename OsModel_P>
   class iOsClockModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef iOsClockModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef double time_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
       enum {
           CLOCKS_PER_SECOND = 1000
       };
       // --------------------------------------------------------------------
       iOsClockModel( iOsSystem& system )
       : system_(system)
       {
       }
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      time_t time()
      {
         return (double)[[NSDate date] timeIntervalSince1970];
      }
      // --------------------------------------------------------------------
      uint32_t microseconds( time_t time )
      {
          return 0;
      }
      // --------------------------------------------------------------------
      uint16_t milliseconds( time_t time )
      {
         return (uint16_t)(time * 1000);
      }
      // --------------------------------------------------------------------
      uint32_t seconds( time_t time )
      {
         return (uint32_t)time;
      }

   private:
       iOsSystem& system()
       { return system_; }
       // --------------------------------------------------------------------
       iOsSystem& system_;
   };
}

#endif
