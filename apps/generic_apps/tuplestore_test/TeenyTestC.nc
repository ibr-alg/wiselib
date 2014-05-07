
#include <Timer.h>
#include "defs.h"
#include "tl_objs.h"
#include <printf.h>
#include "TupleSpace.h"

////
//#define DEBUG 1
//#define MODE_INSERT 1
//#define MODE_FIND 0
//#define MODE_ERASE 0
////


#define INSERTS_AT_ONCE 4
#define FINDS_AT_ONCE 1

#define INSERT_CALLS 3
#define FIND_CALLS 100
#define MAX_TUPLES (INSERTS_AT_ONCE * INSERT_CALLS)

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

	//#define START_INSERT_INTERVAL 1000
	//#define START_FIND_INTERVAL 5000
	//#define DISABLE_RADIO_INTERVAL 100

	#ifdef FLASH_SYNC_TIME
flahs sync time should not be defined for proper energy measurement!
	#endif


	bool first_receive;
	//uint8_t rdf_buffer_[1024];
	bool receiving;
	uint16_t nextpos;
	uint16_t tuples;
	int calls;
	//int erasepos;
	//NeighborTuple<uint16_t> neighborTuple;
	
		//uint8_t buf_s[120];
		//uint8_t buf_p[120];
		//uint8_t buf_o[120];


	void initialize_db() {
	}

	void waste_energy();
	void insert_some();
	void insert_fill();
	void erase(int);
	void find(int);

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


		call Tuning.set(KEY_RADIO_CONTROL, RADIO_OFF);
		calls = 0;

		#if MODE_FIND
			insert_fill();
			call Timer0.startPeriodic(1000);
		
		#elif MODE_INSERT
			call Timer0.startPeriodic(1000);

		#elif MODE_ERASE
			insert_fill();
			call Timer0.startPeriodic(1000);

		#endif
			
	}

	void reboot() {
		//lastpos = 0;
		first_receive = true;
		tuples = 0;
		nextpos = 0;
	}

	event void PrintfFlush.flushDone(error_t error) {
	}

	// "random" permutation chosen by fair dice roll (tm) ;)
	int rnd[MAX_TUPLES] = { 10, 3, 4, 5, 11, 7, 0, 2, 6, 8, 9, 1 };


	event void Timer0.fired() {
		calls++;

		//call Tuning.set(KEY_RADIO_CONTROL, RADIO_OFF);
		#if MODE_INSERT
			insert_some();

			if(calls >= INSERT_CALLS) { call Timer0.stop(); }

		#elif MODE_FIND
			find(rnd[(calls - 1) % MAX_TUPLES]);
			if(calls >= FIND_CALLS) { call Timer0.stop(); }

		#elif MODE_ERASE
			erase(rnd[calls - 1]);
			if(calls >= MAX_TUPLES) { call Timer0.stop(); }
		#endif

	}


	void insert_some() {
		tuple<uint8_t[120], uint8_t[120], uint8_t[120]> t;
		TLOpId_t inId;
		int i;

		#if DEBUG
			printf("ins\n");
			call PrintfFlush.flush();
		#endif

		for(i=0; i<INSERTS_AT_ONCE; i++) {
			t.expireIn = TIME_UNDEFINED;
			t.type = 1;
			t.flags = 0;

			strcpy((char*)t.value0, "<http://spitfire-project.eu/sensor_stimulus/silver_expansion>");
			t.match_types[0] = MATCH_ACTUAL;

			strcpy((char*)t.value1, "<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>");
			t.match_types[1] = MATCH_ACTUAL;
			
			strcpy((char*)t.value2, "<http://spitfire-project.eu/property/Temperature>");
			t.match_types[2] = MATCH_ACTUAL;

			call TS.out(&inId, FALSE, TL_LOCAL, RAM_TS, (tuple*)&t);
		}

	}

	void insert_fill() {
		tuple<uint8_t[120], uint8_t[120], uint8_t[120]> t;
		TLOpId_t inId;
		int i;

		#if DEBUG
			printf("fill\n");
			call PrintfFlush.flush();
		#endif

		for(i=0; i<MAX_TUPLES; i++) {
			t.expireIn = TIME_UNDEFINED;
			t.type = 1;
			t.flags = 0;

			strcpy((char*)t.value0, "<http://spitfire-project.eu/sensor_stimulus/silver_expansion>");
			t.value0[28] += i;
			t.match_types[0] = MATCH_ACTUAL;

			strcpy((char*)t.value1, "<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>");
			t.value1[28] += i;
			t.match_types[1] = MATCH_ACTUAL;
			
			strcpy((char*)t.value2, "<http://spitfire-project.eu/property/Temperature>");
			t.value2[28] += i;
			t.match_types[2] = MATCH_ACTUAL;

			call TS.out(&inId, FALSE, TL_LOCAL, RAM_TS, (tuple*)&t);
		}
	}

	void find(int i) {
		tuple<uint8_t[120], uint8_t[120], uint8_t[120]> t;
		TLOpId_t inId;
		int spo;
		int j = 0;

		strcpy((char*)t.value0, "<http://spitfire-project.eu/sensor_stimulus/silver_expansion>");
		t.value0[28] += i;

		strcpy((char*)t.value1, "<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>");
		t.value1[28] += i;

		strcpy((char*)t.value2, "<http://spitfire-project.eu/property/Temperature>");
		t.value2[28] += i;

		t.expireIn = TIME_UNDEFINED;
		t.type = 1;
		t.flags = 0;


		for(j = 0; j < FINDS_AT_ONCE; j++) {
			spo = (unsigned)rand() % 3;

			t.match_types[0] = MATCH_ACTUAL;
			t.match_types[1] = MATCH_ACTUAL;
			t.match_types[2] = MATCH_ACTUAL;
			t.match_types[spo] = MATCH_DONT_CARE;
		
			#if DEBUG
				printf("find j=%d i=%d spo=%d\n", (int)j, (int)i, (int)spo);
			#endif

			// Construct the matching tuple
			// spo == 0 --> (* ssn:isProxyFor spit:Temperature)
			// spo == 1 --> (ss:silver_exp * spit:Temperature)
			// spo == 2 --> (ss:silver_exp ssn:isProxyFor *)

			call TS.rd(&inId, FALSE, TL_LOCAL, RAM_TS, (tuple *)&t);
		}

		#if DEBUG
			call PrintfFlush.flush();
		#endif
	}


	void erase(int i) {
		tuple<uint8_t[120], uint8_t[120], uint8_t[120]> t;
		TLOpId_t inId;
		int spo = (unsigned)rand() % 3;
	
		#if DEBUG
			printf("erase i=%d spo=%d\n", (int)i, (int)spo);
			call PrintfFlush.flush();
		#endif

		// Construct the matching tuple
		// spo == 0 --> (* ssn:isProxyFor spit:Temperature)
		// spo == 1 --> (ss:silver_exp * spit:Temperature)
		// spo == 2 --> (ss:silver_exp ssn:isProxyFor *)

		t.expireIn = TIME_UNDEFINED;
		t.type = 1;
		t.flags = 0;

		if(spo == 0) {
			t.match_types[0] = MATCH_DONT_CARE;
		}
		else {
			strcpy((char*)t.value0, "<http://spitfire-project.eu/sensor_stimulus/silver_expansion>");
			t.value0[28] += i;
			t.match_types[0] = MATCH_ACTUAL;
		}

		if(spo == 1) {
			t.match_types[1] = MATCH_DONT_CARE;
		}
		else {
			strcpy((char*)t.value1, "<http://purl.oclc.org/NET/ssnx/ssn#isProxyFor>");
			t.value1[28] += i;
			t.match_types[1] = MATCH_ACTUAL;
		}
		
		if(spo == 2) {
			t.match_types[2] = MATCH_DONT_CARE;
		}
		else {
			strcpy((char*)t.value2, "<http://spitfire-project.eu/property/Temperature>");
			t.value2[28] += i;
			t.match_types[2] = MATCH_ACTUAL;
		}

		call TS.in(&inId, FALSE, TL_LOCAL, RAM_TS, (tuple *)&t);

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
		//call TS.nextTuple(operationId, iterator);
		//call TS.nextTuple(operationId, iterator);
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

