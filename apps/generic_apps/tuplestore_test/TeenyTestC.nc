
#include <Timer.h>
#include "defs.h"
#include "tl_objs.h"
#include <printf.h>
#include "TupleSpace.h"

#define DEBUG 0

module TeenyTestC {
	uses {
		interface Boot;
		//interface Receive;
		//interface AMSend;
		interface Timer<TMilli> as Timer0;
		interface Timer<TMilli> as Timer1;
		interface Timer<TMilli> as Timer2;
		//interface Packet;
		//interface SplitControl as RadioControl;

		interface TupleSpace as TS;
		interface TeenyLIMESystem;

		interface SplitControl as PrintfControl;
		interface PrintfFlush;

		interface McuSleep;
		//interface SplitControl as PhysSleep;

		//interface LowPowerListening as LPL;

		interface Tuning;
	}
}

implementation {
	
	#define true 1
	#define false 0

	#define START_INSERT_INTERVAL 1000
	#define START_FIND_INTERVAL 5000
	#define DISABLE_RADIO_INTERVAL 100


	#ifdef FLASH_SYNC_TIME
flahs sync time should not be defined for proper energy measurement!
	#endif


	bool first_receive;
	//uint8_t rdf_buffer_[1024];
	bool receiving;
	uint16_t nextpos;
	uint16_t tuples;
	//NeighborTuple<uint16_t> neighborTuple;
	
		//uint8_t buf_s[120];
		//uint8_t buf_p[120];
		//uint8_t buf_o[120];


	void initialize_db() {
	}

	void waste_energy();

	void write_uint16(uint8_t* p, uint16_t v) {
		p[0] = v & 0xff;
		p[1] = (v >> 8);
	}

	uint16_t read_uint16(uint8_t* p) {
		return p[0] | ((uint16_t)p[1] << 8);
	}

	event void Tuning.setDone(uint8_t key, uint16_t value) {
	}

	event void Boot.booted() {
		#if DEBUG
			call PrintfControl.start();
			printf("boot\n");
			call PrintfFlush.flush();
		#endif

		//dbg("boot, i guess.");
		// init
		//call Timer2.startPeriodic(10000);
		//call PhysSleep.stop();
		

		//call LPL.setLocalSleepInterval(1000);
		//call LPL.setLocalDutyCycle();

		call Tuning.set(KEY_RADIO_CONTROL, RADIO_OFF);
		//call RadioControl.stop();
		//call McuSleep.sleep();

		call Timer0.startPeriodic(1000);
	}

	void reboot() {
		//lastpos = 0;
		first_receive = true;
		tuples = 0;
		nextpos = 0;
	}

	event void PrintfFlush.flushDone(error_t error) {
	}

	event void Timer0.fired() {
		tuple<uint8_t[120], uint8_t[120], uint8_t[120]> t;
		TLOpId_t inId;
		int i;

		call Tuning.set(KEY_RADIO_CONTROL, RADIO_OFF);

		#if DEBUG
			printf("ins xstart\n");
		#endif // DEBUG
		//call PrintfFlush.flush();

		//waste_energy();

		//printf("WAT\n"); //call PrintfFlush.flush();
		
	/*
	for(i=0; i<20; i++) {
	*/
		#if DEBUG
			printf("i=%d\n", (int)i);
		#endif

		t.expireIn = TIME_UNDEFINED;
		t.type = 1;
		t.flags = 0;
		//printf("sss\n"); //call PrintfFlush.flush();

		strcpy((char*)t.value0, "<http://spitfire-project.eu/sensor_stimulus/silver_expansion>");
		t.match_types[0] = MATCH_ACTUAL;

		//printf("ppp\n"); //call PrintfFlush.flush();

		strcpy((char*)t.value1, "<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>");
		t.match_types[1] = MATCH_ACTUAL;
		
		//printf("ooo\n"); //call PrintfFlush.flush();
		
		strcpy((char*)t.value2, "<http://spitfire-project.eu/property/Temperature>");
		t.match_types[2] = MATCH_ACTUAL;

		//printf("out\n"); //call PrintfFlush.flush();
		//t = newTuple(
			//actualField(buf_s),
			//actualField(buf_p),
			//actualField(buf_o));

		call TS.out(&inId, FALSE, TL_LOCAL, RAM_TS, (tuple*)&t);
/*
	}
*/
#if DEBUG
	printf("ins end\n");
	call PrintfFlush.flush();
#endif // DEBUG
	}

	void waste_energy() {
		int blah = 5;
		int i = 0;
		int j = 7;
		for(i = 0; i < 1000; i++) {
			//for(j = 0; j < 1000; j++) {
				blah = ((j*blah / (1 + i)) << 3) + 700;
			//}
		}
		printf("%d", blah);
	}

	event void Timer1.fired() {
	}

	event void Timer2.fired() {
		//dbg("<3");
		//printf("heartbeat\n");
		//call PrintfFlush.flush();
	}

	event void PrintfControl.startDone(error_t err) {
		//printf("start is done btw.\n");
		//call PrintfFlush.flush();
	}

	event void PrintfControl.stopDone(error_t err) {
	}


#if 0
	event void AMSend.sendDone(message_t* msg, error_t err) {
	}

	//event void PhysSleep.startDone(error_t err) {
	//}

	//event void PhysSleep.stopDone(error_t err) {
	//}
	event void RadioControl.startDone(error_t err) {
	}

	event void RadioControl.stopDone(error_t err) {
	}
	//message_t packet;

	event message_t* Receive.receive(message_t* msg, void* payload, uint8_t len) {
		uint8_t *data;
		uint16_t pos;
		uint8_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
		//uint8_t p[100];
		uint8_t *p;
		return msg;

		//if(!receiving) { return msg; }
		//data = (uint8_t*)payload;
		
		////printf("rcv l=%d %02x %02x %02x %02x", (int)len, (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
		
		//if(data[0] == 0x99 && data[1] == (EXP_NR & 0xff)) {	
			//if(first_receive) {
				//first_receive = false;
				//initialize_db();
			//}

			//pos = read_uint16(data + 2);
			//if(pos != 0 && pos != nextpos) {
				//write_uint16(ack + 2, pos);
				//// TODO
			//p = (uint8_t*)(call Packet.getPayload(&packet, NULL)); //, 4));
			////	[>p = (uint8_t*)(<] call Packet.getPayload(&packet, p);
				//p[0] = ack[0]; p[1] = ack[1];
				//p[2] = ack[2]; p[3] = ack[3];
				//call AMSend.send(AM_BROADCAST_ADDR, &packet, 4);
				//return msg;
			//}
			////lastpos = pos;
			//nextpos = pos + len - 4;

			//if(len == 4) {
				//receiving = false;
				//nextpos = 0;
				//call Timer0.startOneShot(START_INSERT_INTERVAL);
				//call Timer1.startOneShot(DISABLE_RADIO_INTERVAL);
			//}
			//else {
				//memcpy(rdf_buffer_ + pos, data + 4, len - 4);
			//}

			////uint8_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
			//write_uint16(ack + 2, pos);
			//p = (uint8_t*)(call Packet.getPayload(&packet, NULL)); //, 4));
				////p = (uint8_t*)( call Packet.getPayload(&packet, p);
			//p[0] = ack[0]; p[1] = ack[1];
			//p[2] = ack[2]; p[3] = ack[3];
			//call AMSend.send(AM_BROADCAST_ADDR, &packet, 4);
		//}
		//else if(data[0] == 0xbb && data[1] == (EXP_NR & 0xff)) {
			//reboot();
		//}
		//else {
		//}

		//return msg;
	}
#endif

	event void TS.tupleReady(TLOpId_t operationId, TupleIterator *iterator) {
#if DEBUG
		printf("ready\n");
		call PrintfFlush.flush();
#endif // DEBUG
	}

	tuple<uint16_t> neighborTuple;
	event tuple* TeenyLIMESystem.reifyNeighborTuple() {
		return (tuple *)&neighborTuple;
	}
 event void TS.reifyCapabilityTuple(tuple* ct) {
  }


  event void TS.operationCompleted(uint8_t completionCode,
                                   TLOpId_t operationId,
                                   TLTarget_t target,
                   TLTupleSpace_t ts,
                                   tuple* returningTuple) {
#if DEBUG
	  printf("compl\n");
	  call PrintfFlush.flush();
#endif // DEBUG
	
  }


}

	
// vim: set ft=cpp:

