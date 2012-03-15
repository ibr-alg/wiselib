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
#include "external_interface/trisos/trisos_os.h"

extern "C" {
/* */
#include <util/delay.h>
/* Contiki */
#include "contiki.h"
/* fwtrisos  */
#include "fwtrisos/src/hw/encoder.h"
#include "fwtrisos/src/hw/led.h"
#include "fwtrisos/src/hw/com.h"
#include "fwtrisos/src/hw/display.h"
#include "fwtrisos/src/hw/twi_master.h"
#include "fwtrisos/src/hw/rf/rf.h"
#include "fwtrisos/src/hw/rf/tat.h"
#include "fwtrisos/src/gui/fonts.h"
#include "fwtrisos/src/gui/graphics.h"
#include "fwtrisos/src/sys/io.h"
#include "fwtrisos/src/sys/sys.h"
#include "fwtrisos/src/fs/hal_fs.h"
#include "fwtrisos/src/fs/tfs.h"
}

PROCINIT(&etimer_process);

/* Hardware initialization */
void
init_lowlevel(void)
{
	com_init();
	io_init();
	disp_init(true);

	led_init();

	encoder_init();

	com_config_baudrate(COMBR_38400);

	rf_init();
	
	tat_set_operating_channel( 24 );
	
	twi_master_init();
	sys_init();
	tfs_init();
	sei();

	_delay_ms(200);

}

int main()
{
  	/* Initialize hardware */
  	init_lowlevel();
	 
	/* Clock */
	clock_init();

	/* Process subsystem */
	process_init();	

	/* Register initial processes */
	procinit_init();

	io_config_stdout_target(IO_OUT_BOTH);

	wiselib::wiselib_trisos_init();
	
	/* Main scheduler loop */
	while(1) {
		process_run();
	} 

	return 0;	
}
