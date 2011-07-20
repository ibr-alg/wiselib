/************************************************************************
 ** This file is part of the TriSOS project.
 ** Copyright (C) 2009 University of Applied Sciences Lübeck
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
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
