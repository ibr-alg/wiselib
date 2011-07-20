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
#include "external_interface/lorien/lorien_os.h"
#include "external_interface/lorien/lorien_timer.h"
#include "external_interface/lorien/lorien_radio.h"

//NOTE: This is written in raw Lorien Component Assembly, which is bad (function names and record attributes are not stable)


void application_main( Component& );

namespace wiselib
{
//    LorienOsModel lorien_os;
   
   
   // -----------------------------------------------------------------------
   /* TODO: Find a better solution for the following:
    * This forces the msp430-g++ to create contructor and destructor in the
    * ctor/dtor-section. When this is done, you can call tr = tree_routing_t()
    * locally in a method. Otherwise, when this global dummy is not created
    * (and thus no ctor/dtor), tr = tree_routing_t() fails.
    */
   //tree_routing_t dummy;
   // -----------------------------------------------------------------------
   static unsigned int send( Component *comp, unsigned char *data, size_t len, unsigned int port )
   {
      //((LXState*)comp->state)->routing.send( 0, len, data );
      return OK;
   }
   // -----------------------------------------------------------------------
   static unsigned int sendTo( Component *comp, unsigned char *data, size_t len, unsigned int id, unsigned int port )
   {
      //((LXState*)comp->state)->routing.send( id, len, data );
      return OK;
   }
   // -----------------------------------------------------------------------
   static int reg_recv_callback( Component *comp, Component *r, unsigned int port )
   {
   	//?!
      //oc_regReceptacle(comp, (char*)"IReceiver_Int", sizeof(IReceiver_Int));
      //comp -> kernel -> bind(comp, r, (char*)"IReceiver_Int", NULL);

      // TODO: When Wiselib receives message, call r. See contiki_radio of how this works.
      // TODO: This method can only be called *once*. Quick: Introduce state and return error
      //       when registered second time. Long-Term: See sontiki_radio

      return OK;
   }
   // -----------------------------------------------------------------------
   static int unreg_recv_callback( Component *comp, Component *r, unsigned int port )
   {
   	//?!
      //comp -> kernel -> unbind(comp -> kernel -> getBinding(comp, (char*)"IReceiver_Int"));
      //oc_delReceptacle(comp, (char*)"IReceiver_Int");

      return OK;
   }
   // -----------------------------------------------------------------------
   static unsigned int get_id( Component *comp)
   {
   	return ((IMXRadio*) ((LXState*) comp -> state) -> radio -> userRecp) -> getID(((LXState*) comp -> state) -> radio -> ifHostComponent);
   }
   // -----------------------------------------------------------------------
   static int set_id( Component *comp, int id )
   {
      //NI

      return OK;
   }
   // -----------------------------------------------------------------------
   static const char* get_id_type( Component *comp)
   {
      return "int";
   }
   // -----------------------------------------------------------------------
   static int enable( Component *comp)
   {
      //((LXState*) comp -> state) -> routing.enable();
      return OK;
   }
   // -----------------------------------------------------------------------
   static int disable( Component *comp)
   {
      //((LXState*) comp -> state) -> routing.disable();
      return OK;
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   static int timer_elapse(Component *comp, void *tid, void *ptr_o )
   {
      if ( ((LXState*) comp -> state) -> lorien_timer )
         ((LXState*) comp -> state) -> lorien_timer -> timer_elapsed(ptr_o);
      return OK;
   }
   // -----------------------------------------------------------------------
   static int osa_recv( Component *comp, unsigned char *data, size_t len, unsigned int from_id, unsigned int port )
   {
      if ( ((LXState*) comp -> state) -> lorien_radio )
         ((LXState*) comp -> state) -> lorien_radio -> received(data, len, from_id);
      return OK;
   }
   // -----------------------------------------------------------------------
   static int start(Component *comp)
   {
      ((LXState*) comp -> state) -> lorien_timer = 0;
      ((LXState*) comp -> state) -> lorien_radio = 0;

      //register with the radio
      ((IMXRadio*) ((LXState*) comp -> state) -> radio -> userRecp) -> registerCallback(((LXState*) comp -> state) -> radio -> ifHostComponent, comp, WISELIB_PORT);
      
      application_main( *comp );

      return OK;
   }
   // -----------------------------------------------------------------------
   static int stop(Component *comp)
   {
      //unregister with the radio
      ((IMXRadio*) ((LXState*) comp -> state) -> radio -> userRecp) -> unregisterCallback(((LXState*) comp -> state) -> radio -> ifHostComponent, comp, WISELIB_PORT);

      return OK;
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   extern "C" int construct( Component *comp )
   {
      int nerror = OK;
      /* set up my interfaces */
      IMXRadio *ir = NULL;
      IHWControl *ihc = NULL;
      ILife *il = NULL;
      ITimerCallback *itc = NULL;
      IMXRadioCallback *irc = NULL;
      
      LXState *st = (LXState*) malloc(sizeof(LXState));
      
      if (st == NULL)
      	return OUT_OF_MEMORY;
      
      oc_regState(comp, st);
      
      if ((nerror = oc_regInterfaces(comp, 5, "IMXRadio", NULL, &ir, sizeof(IMXRadio),
      														"IHWControl", NULL, &ihc, sizeof(IHWControl),
      														"ILife", NULL, &il, sizeof(ILife),
      														"ITimerCallback", NULL, &itc, sizeof(ITimerCallback),
      														"IMXRadioCallback", NULL, &irc, sizeof(IMXRadioCallback))) != OK)
      														return nerror;

      ir->send = send;
      ir->sendTo = sendTo;
      ir->registerCallback = reg_recv_callback;
      ir->unregisterCallback = unreg_recv_callback;
      ir->getID = get_id;

      irc -> recv = osa_recv;

      itc -> timeout = timer_elapse;

      ihc -> enable = enable;
      ihc -> disable = disable;
      
      il -> start = start;
		il -> stop = stop;

      if ((nerror = oc_regReceptacles(comp, 3,
                        "IMXRadio", NULL, &st -> radio, sizeof(IMXRadio),
                        "ITimer", NULL, &st -> timer, sizeof(ITimer),
                        "IPort", NULL, &st -> port, sizeof(IPort))) != OK)
      {
         return nerror;
      }

      return OK;
   }
   // -----------------------------------------------------------------------
   extern "C" int destruct( Component *comp )
   {
      return OK;
   }

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void* operator new ( size_t size )
{
   return malloc( size );
}
//---------------------------------------------------------------------------
void operator delete ( void *pointer )
{
   free( pointer );
}

