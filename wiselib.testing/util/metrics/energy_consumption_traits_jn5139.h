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
#ifndef __UTIL_METRICS_ENERGY_CONSUMPTION_JN5139_TRAITS_H
#define __UTIL_METRICS_ENERGY_CONSUMPTION_JN5139_TRAITS_H

/** Energy Consumption Traits for Jennic JN5139
    * 
    *  Data is based on Jennic Appication Note "JN-AN-1001 Calculating
    *  JN5139/JN5148 Power Consumption". See
    *   http://www.jennic.com/files/support_files/JN-AN-1001-Power-Estimation.pdf
    *  for direct download, or alternatively
    *   http://www.jennic.com/support/support_files/jn-an-1001_calculating_jn5139_jn5148_power_consumption
    */
   struct EnergyConsumptionTraitsJennic5139
   {
      /** MAC header in Jennic radio. See page 1 in JN-AN-1001; assuming 16bit
       *  addresses.
       */
      static const int MESSAGE_HEADER_SIZE = 13;
      
      /** Supposed to be multiplied with number of transmitted bytes.</br>
       *  </br>
       *  As written in JN-AN-1001:</br>
       *    Data Frame Transmission Period (ms) = (HeaderSize + PayloadSize) x 8 / 250</br>
       *  </br>
       *  This is multiplied with the 38mA as given on page 4.</br>
       *  </br>
       *  Finally, the result is divided by (3600. * 1000.) to get mAh
       *  (instead of mAms).
       */
      static const double TX_MULTIPLIER = ((8. / 250.) * 38.) / (3600. * 1000.);
      
      /** Supposed to be multiplied with number of transmitted bytes.</br>
       *  </br>
       *  As written in JN-AN-1001:</br>
       *    Data Frame Transmission Period (ms) = (HeaderSize + PayloadSize) x 8 / 250</br>
       *  </br>
       *  This is multiplied with the 37mA as given on page 4.</br>
       *  </br>
       *  Finally, the result is divided by (3600. * 1000.) to get mAh
       *  (instead of mAms).
       */
      static const double RX_MULTIPLIER = ((8. / 250.) * 37.) / (3600. * 1000.);
      
      /** Supposed to be multiplied with the active time given in ms.</br>
       *  </br>
       *  The time is multiplied with the CPU active value as given on page 4
       *  from JN-AN-1001.</br>
       *  CPU active = 2.85 + 0.285 per MHz = 2.85 + 32*0.285 = 11.97mA
       *  (CPU active with Wireless Receiver active would be 27.3mA)
       *  </br>
       *  Finally, the result is divided by (3600. * 1000.) to get mAh
       *  (instead of mAms).
       */
      static const double ACTIVE_MULTIPLIER = 11.97 / (3600. * 1000.);
      
      /** Supposed to be multiplied with the idle time given in ms.</br>
       *  </br>
       *  The time is multiplied with the CPU idle value as given on page 4
       *  from JN-AN-1001.</br>
       *  CPU idle with RAM held = 0.0024mA
       *  </br>
       *  Finally, the result is divided by (3600. * 1000.) to get mAh
       *  (instead of mAms).
       */
      static const double IDLE_MULTIPLIER = 0.0024 / (3600. * 1000.);
   };

#endif