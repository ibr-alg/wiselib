
#include <Timer.h>
#include "defs.h"
#include "tl_objs.h"
#include "TupleSpace.h"

module TeenyTestC {
	uses {
		interface Boot;
		interface Receive;
		interface AMSend;
		interface Timer<TMilli> as Timer0;
		interface Timer<TMilli> as Timer1;
		interface Packet;
		interface SplitControl as RadioControl;

		interface TupleSpace as TS;
		interface TeenyLIMESystem;
	}
}

implementation {
	
	#define true 1
	#define false 0

	#define START_INSERT_INTERVAL 1000
	#define START_FIND_INTERVAL 5000
	#define DISABLE_RADIO_INTERVAL 100

	bool first_receive;
	uint8_t rdf_buffer_[1024];
	bool receiving;
	uint16_t nextpos;
	uint16_t tuples;
	//NeighborTuple<uint16_t> neighborTuple;
	

	void initialize_db() {
	}

	void write_uint16(uint8_t* p, uint16_t v) {
		p[0] = v & 0xff;
		p[1] = (v >> 8);
	}

	uint16_t read_uint16(uint8_t* p) {
		return p[0] | ((uint16_t)p[1] << 8);
	}


	event void Boot.booted() {
		dbg("boot, i guess.");
		// init
		call RadioControl.start();
	}

	void reboot() {
		//lastpos = 0;
		first_receive = true;
		tuples = 0;
		nextpos = 0;
	}

	event void Timer0.fired() {
	}

	event void Timer1.fired() {
	}

	event void AMSend.sendDone(message_t* msg, error_t err) {
	}

	event void RadioControl.startDone(error_t err) {
	}

	event void RadioControl.stopDone(error_t err) {
	}

	message_t packet;

	event message_t* Receive.receive(message_t* msg, void* payload, uint8_t len) {
		uint8_t *data;
		uint16_t pos;
		uint8_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
		//uint8_t p[100];
		uint8_t *p;

		if(!receiving) { return msg; }
		data = (uint8_t*)payload;
		
		dbg("l=%d %02x %02x %02x %02x", (int)len, (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
		
		if(data[0] == 0x99 && data[1] == (EXP_NR & 0xff)) {	
			if(first_receive) {
				first_receive = false;
				initialize_db();
			}

			pos = read_uint16(data + 2);
			if(pos != 0 && pos != nextpos) {
				write_uint16(ack + 2, pos);
				// TODO
			p = (uint8_t*)(call Packet.getPayload(&packet, NULL)); //, 4));
			//	/*p = (uint8_t*)(*/ call Packet.getPayload(&packet, p);
				p[0] = ack[0]; p[1] = ack[1];
				p[2] = ack[2]; p[3] = ack[3];
				call AMSend.send(AM_BROADCAST_ADDR, &packet, 4);
				return msg;
			}
			//lastpos = pos;
			nextpos = pos + len - 4;

			if(len == 4) {
				receiving = false;
				nextpos = 0;
				call Timer0.startOneShot(START_INSERT_INTERVAL);
				call Timer1.startOneShot(DISABLE_RADIO_INTERVAL);
			}
			else {
				memcpy(rdf_buffer_ + pos, data + 4, len - 4);
			}

			//uint8_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
			write_uint16(ack + 2, pos);
			p = (uint8_t*)(call Packet.getPayload(&packet, NULL)); //, 4));
				//p = (uint8_t*)( call Packet.getPayload(&packet, p);
			p[0] = ack[0]; p[1] = ack[1];
			p[2] = ack[2]; p[3] = ack[3];
			call AMSend.send(AM_BROADCAST_ADDR, &packet, 4);
		}
		else if(data[0] == 0xbb && data[1] == (EXP_NR & 0xff)) {
			reboot();
		}
		else {
		}

		return msg;
	}

	event void TS.tupleReady(TLOpId_t operationId, TupleIterator *iterator) {
	}

	event tuple* TeenyLIMESystem.reifyNeighborTuple() {
		//return (tuple *)&neighborTuple
		return 0;
	}
 event void TS.reifyCapabilityTuple(tuple* ct) {
  }


  event void TS.operationCompleted(uint8_t completionCode,
                                   TLOpId_t operationId,
                                   TLTarget_t target,
                   TLTupleSpace_t ts,
                                   tuple* returningTuple) {
  }


}

	
// vim: set ft=cpp:

