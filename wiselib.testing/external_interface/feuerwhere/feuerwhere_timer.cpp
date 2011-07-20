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
#include "feuerwhere_timer.h"
//#include <stdio.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(fmt, args...)		printf("utmr: " fmt "\n", ##args)
#else
#define PRINTF(fmt, args...)
#endif

namespace wiselib
{

static const int MAX_REGISTERED_TIMER = 10;
// -----------------------------------------------------------------------
static FeuerwhereTimerItem timer_items[MAX_REGISTERED_TIMER];
// -----------------------------------------------------------------------
void init_timer( void )
{
   for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
     timer_items[i].cb = feuerwhere_timer_delegate_t();
}

/**
 * @brief Describes a message object which can be sent between threads.
 *
 * User can set type and one of content.ptr and content.value. (content is a union)
 * The meaning of type and the content fields is totally up to the user,
 * the corresponding fields are never read by the kernel.
 *
 */
/*
typedef struct msg {
    uint16_t sender_pid;    ///< PID of sending thread. Will be filled in by msg_send
    uint16_t type;          ///< Type field.
    union {
        char *ptr;          ///< pointer content field
        unsigned int value; ///< value content field
    } content;
    unsigned int flags;     ///< Flags field. Internally used by send/receive functions
} msg;
*/
void feuerwhere_timer_thread(void)
{
    msg m;

    while(1) {
      do {
          msg_receive(&m);
      } while( m.type != MSG_TIMER );

      FeuerwhereTimerItem *t = (FeuerwhereTimerItem*)m.content.ptr;
      t->cb(t->ptr);

      ///printf ("In timer thread for %s\n",(char*)t->ptr );
      // finally, clear cb entry so that it can be re-used
      t->cb = feuerwhere_timer_delegate_t();
  }
}
// -----------------------------------------------------------------------

FeuerwhereTimerItem* get_feuerwhere_timer_item( void )
{
   for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
      if ( !timer_items[i].cb )
      {
         return &timer_items[i];
      }

   return 0;
}

}
