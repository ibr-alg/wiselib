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
#ifndef CONNECTOR_LORIEN_DEBUGOUTPUT_H
#define CONNECTOR_LORIEN_DEBUGOUTPUT_H

#include <stdarg.h>
#include <stdio.h>

extern "C" {
#include "lorien.h"
}

namespace wiselib
{

   /** \brief Lorien Implementation of \ref debug_concept "Debug Concept".
    *
    *  \ingroup debug_concept
    *  \ingroup lorien_facets
    *
    *  Lorien implementation of the \ref debug_concept "Debug Concept" ...
    */
   template<typename OsModel_P>
   class LorienDebug
   {
      public:
         typedef OsModel_P OsModel;

         typedef LorienDebug<OsModel> self_type;
         typedef self_type* self_pointer_t;
         // -----------------------------------------------------------------
         void init( Component *comp )
         {
            comp_ = comp;
         }
         
         
			/**
			* C++ version 0.4 char* style "itoa":
			* Written by Lukás Chmela
			* Released under GPLv3.
			* sourced from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
			*/
			char* op_ltoa(long value, char* result, int base)
				{
				// check that the base if valid
				if (base < 2 || base > 36)
					{
					*result = '\0';
					return result;
					}

				char* ptr = result, *ptr1 = result, tmp_char;
				long tmp_value;

				do
					{
					tmp_value = value;
					value /= base;
					*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
					} while ( value );

				// Apply negative sign
				if (tmp_value < 0) *ptr++ = '-';
				*ptr-- = '\0';
				while(ptr1 < ptr)
					{
					tmp_char = *ptr;
					*ptr--= *ptr1;
					*ptr1++ = tmp_char;
					}
				return result;
				}

			char* op_ultoa(unsigned long value, char* result, int base)
				{
				// check that the base if valid
				if (base < 2 || base > 36)
					{
					*result = '\0';
					return result;
					}

				char* ptr = result, *ptr1 = result, tmp_char;
				unsigned long tmp_value;

				do
					{
					tmp_value = value;
					value /= base;
					*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
					} while ( value );

				// Apply negative sign
				*ptr-- = '\0';
				while(ptr1 < ptr)
					{
					tmp_char = *ptr;
					*ptr--= *ptr1;
					*ptr1++ = tmp_char;
					}
				return result;
				}
         
         #define BUFSIZE 1024
         // -----------------------------------------------------------------
         void debug( const char *format, ... )
         {
         //below code taken from Lorien's stdout library component
			//TODO: check we don't exceed BUFSIZE in the below procedure
			char finalLine[BUFSIZE];
			va_list vars;
			memset(finalLine, '\0', BUFSIZE);
	
			va_start(vars, format);
			//vsprintf(finalLine, format, vars); //this is generally too expensive to have in fundamentals
			char *fm;
			while ((fm = strchr(format, '%')) != NULL)
				{
				strncpy(&finalLine[strlen(finalLine)], format, fm - format);
		
				if (fm[1] == 'u')
					{
					unsigned int ui = va_arg(vars, unsigned int);
					op_ultoa(ui, &finalLine[strlen(finalLine)], 10);
					}
					else if ((fm[1] == 'i') || (fm[1] == 'd'))
					{
					int i = va_arg(vars, int);
					op_ltoa(i, &finalLine[strlen(finalLine)], 10);
					}
					else if (fm[1] == 'l')
					{
					if (fm[2] == 'u')
						{
						unsigned long ul = va_arg(vars, unsigned long);
						op_ultoa(ul, &finalLine[strlen(finalLine)], 10);
						fm ++;
						}
						else
						{
						long ul = va_arg(vars, long);
						op_ltoa(ul, &finalLine[strlen(finalLine)], 10);
						}
					}
					else if (fm[1] == 'p')
					{
					unsigned int u = va_arg(vars, unsigned int);
					finalLine[strlen(finalLine)] = '0';
					finalLine[strlen(finalLine)] = 'x';
					op_ultoa(u, &finalLine[strlen(finalLine)], 16);
					}
					else if (fm[1] == 's')
					{
					char * s = va_arg(vars, char *);
					strcpy(&finalLine[strlen(finalLine)], s);
					}
					else
					{
					if (fm[1] != '%')
						va_arg(vars, void*);
					finalLine[strlen(finalLine)] = fm[1];
					}
		
				format = fm + 2;
				}
			va_end(vars);
	
			strcpy(&finalLine[strlen(finalLine)], format);
	
			((IPort*) ((LXState*) comp_ -> state) -> port -> userRecp) -> send(((LXState*) comp_ -> state) -> port -> ifHostComponent, (unsigned char*) finalLine, strlen(finalLine));
         
            /*
            va_list fmtargs;
            char buffer[1024];
            va_start( fmtargs, msg );
            vsnprintf( buffer, sizeof(buffer) - 1, msg, fmtargs );
            va_end( fmtargs );
            printf( "%s", buffer );
            */
         }

      private:
         Component *comp_;
   };
}

#endif
