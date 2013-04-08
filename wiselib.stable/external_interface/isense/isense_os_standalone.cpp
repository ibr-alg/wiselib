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
#include "external_interface/isense/isense_math.h"
#include <isense/application.h>
#include <isense/os.h>

void application_main( isense::Os& );
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
namespace wiselib
{
   class iSenseWiselibApplication: public isense::Application
   {
   public:
      iSenseWiselibApplication(isense::Os &os)
         : isense::Application( os ),
            os_( os )
      {}

      virtual void boot ( void )
      { application_main( os_ ); }
      // --------------------------------------------------------------------
      virtual uint16 application_id( void )
      { return 0; }
      // --------------------------------------------------------------------
      virtual uint8 software_revision (void)
      { return 0; }
   private:
      isense::Os &os_;
   };
}
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

#ifndef WISELIB_ALLOW_SLEEP
   #define WISELIB_ALLOW_SLEEP false
#endif

#ifndef WISELIB_ALLOW_DOZE
   #define WISELIB_ALLOW_DOZE false
#endif

isense::Application* application_factory( isense::Os& os )
{
   wiselib::iSenseWiselibApplication *app =
      new wiselib::iSenseWiselibApplication( os );

#if WISELIB_ALLOW_SLEEP
   #warning "------------- ALLOWING NODE SLEEP! --------------"
#endif
#if WISELIB_ALLOW_DOZE
   #warning "------------- ALLOWING NODE DOZE! --------------"
#endif

   os.allow_sleep( WISELIB_ALLOW_SLEEP );
   os.allow_doze( WISELIB_ALLOW_DOZE );

   return app;
};
