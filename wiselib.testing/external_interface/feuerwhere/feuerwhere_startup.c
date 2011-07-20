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

#include <stdio.h>
#include <stdlib.h>

#include "gpioint.h"
#include "ktimer.h"
#include "benchmark.h"
#include "kernel.h"
#include "msg.h"

// sys
#include "builddate.h"
#include "configuration.h"
#include "cfg-feuerware.h"
#include "utimer.h"
#include "powermon.h"
#include "clock.h"
#include "hal-board.h"
#include "tracelog.h"
#include "sysmon.h"
#include "syslog.h"
#include "hal.h"
#include "cmdengine.h"
#include "callback-demon.h"

// drivers
#include "device-sdc.h"
#include "device-serial.h"

// network
#include "radio.h"
#include "cc1100.h"
#include "cc1100-interface.h"

#include "cc1100_spi.h"
#include "cc1100-internal.h"

#include "net.h"
#include "trans.h"
#include "mmstack.h"
#include "cfg-mmstack.h"
//#include "feuerwhere_cc1100_rahio.h"

//#define USE_NANOTRON

#ifdef USE_NANOTRON
#include "nanopan5375-interface.h"
#endif


extern void ltc4150_1_init(ltc4150 *driver);
ltc4150 ltc4150_system;

extern uintptr_t __commands_start;
extern uintptr_t __commands_end;

//extern void protocol_handler(void* msg, int msg_size, packet_info_t* packet_info);
/*---------------------------------------------------------------------------*/
/*
static int message_counter = 0;

static void protocol_handler(void* msg, int msg_size, packet_info_t* packet_info)
{
	message_counter++;
	printf("Got %i\r\n", message_counter);
	printf("Got message [%i]: %s\r\n", msg_size, (char*)msg);
	//printf("PHY SRC: %u\r\nRSSI: %u\r\nLQI: %u\r\n", packet_info->phy_src, packet_info->rssi, packet_info->lqi);
	//printf("NET SRC: %u\r\nNET DST: %u\r\n", packet_info->source, packet_info->destination);
}
*/
void board_init_drivers(void)
{	// sw reset monitor (includes tracelog, if active)
	//sysmon_init();
	//tracelog_init();

	// minimal
	gpioint_init();
	ktimer_init();
	printf("ktimer....[OK]\n");

    // timers
	clock_init();

	struct timeval t;
	t.tv_sec = time(NULL) ;
	printf("setting time \n");
	settimeofday(&t, NULL);

    printf("clock system....[OK]\n");
    utimer_init();
    printf("utimer....[OK]\n");

    // hw monitoring
    benchmark_init();
    printf("benchmark.[OK]\n");

    // Adjust ktimer with current CPU speed
    ktimer_wait(200000);
    ktimer_init_comp(get_system_speed());
    printf("ktimer fcpu....[OK]\n");

    // hal
    hal_init();
    vgpio_clear(&gpio_led_red);
    printf("hal.......[OK]\n");

    // power monitor
    ltc4150_1_init(&ltc4150_system);
    printf("pwrman....[OK]\n");

    // callback demon
	cbd_init();
	printf("cbd.......[OK]\n");
/*
	// configuration
	cfg_reset(&cfg_feuerware_spec, &cfg_feuerware);
	cfg_init(&cfg_file_store, &cfg_file_load);
	cfg_load(&cfg_feuerware_spec, &cfg_feuerware);
*/
	// logging
/*
	//syslog_init();
	printf("syslog....[OK]\n");

	// sdc
#if defined(MODULE_SYSLOG)
	if( !syslog_open_file() ) {
		syslog_set_level("file", SYSLOG_NONE);
	}
#endif
*/
	cfg_feuerware_print();

	// tracelog output in case of reset
	//if (sysmon.reset_code > SYSMON_RS_HWRESET) tracelog_dump();

	// radio stack
	//cc1100_init();
/*
	mms_init();
	mms_add_interface("cc0", 0x01FF, &radio_cc1100);
	mms_set_protocol_handler(1, protocol_handler);
	printf("mms.....[OK]\n");
*/
	// command processor

	cmd_init();
	cmd_register_commandset((struct command*)&__commands_start,
			((unsigned int)&__commands_end - (unsigned int)&__commands_start + 1) / sizeof(struct command));
	console0.state->line_handler = cmdd_pid;
	printf("cmd......[OK]\n");

	//cfg_load(&cfg_mmstack_spec, &cfg_mmstack);

	// init rand
	//int rand_seed = (int)radio_cc1100.get_address() + (int)rtc_time(NULL);
	//srand(rand_seed);

	// Init done, clear green LED
	vgpio_clear(&gpio_led_green);
	printf("# Init completed.\n");
}
 
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void feuerware_os_main();
// --------------------------------------------------------------------------
int main( void )
{

   board_init_drivers();   
   printf("FeuerWare: Startup\n");
   feuerware_os_main();

   while (1);
   return 1;
}
