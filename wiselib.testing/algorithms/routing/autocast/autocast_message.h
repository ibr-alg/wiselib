/************************************************************************
 ** This file is part of the TriSOS project.
 ** Copyright (C) 2009 University of Applied Sciences Lübeck
 ** ALL RIGHTS RESERVED.
 ************************************************************************/

 /**
	autocast message consists of:

	1. mesage id to identfiy the message, that sent from another node (1 Byte)
	2. nr. of the hash values, that have saved in buffer (1 Byte)
	3. nr. of the data unit, that have saved in buffer (1 byte)
	4. nr. of the byte for the hash value (1 byte)
	5. the hash values (0-n bytes)
	6. the data units (0-n bytes)

*/
 
#ifndef __AutoCast_Message_H__
#define __AutoCast_Message_H__

// size of payload is 125 Bytes, it can be larger (it depend on the radio interface)
#define MAX_MESSAGE_SIZE 125

#define MSG_ID 112

typedef wiselib::OSMODEL Os;

namespace wiselib
{

	template<typename OsModel_P,
            typename Radio_P,
			typename Debug_P = typename OsModel_P::Debug> 
	class AutoCast_Message
	{
	
	public:
	
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;

      	typedef typename Radio::node_id_t node_id_t;
      	typedef typename Radio::block_data_t block_data_t;
      	typedef typename Radio::size_t size_t;
      	typedef typename Radio::message_id_t message_id_t;
		
		AutoCast_Message(uint8_t nrOfbyteOfHash, node_id_t nID);
		~AutoCast_Message();

		// return the size of the payload from buffer exclusive the data position
		inline size_t payload_size()
      	{ 
			
			size_t numDU = getNrOfDataUnits();
			size_t size =0;
			// nr of hash values * nr of byte for each hash value
			size_t num = (getNrOfStaleHashes() * getTypeOfHash()) + (getNrOfDataHashes() * getTypeOfHash()); // (number of stale hashes * type hash) is added

			block_data_t* ptr_begin = getDataUnit(0) - sizeof(size_t);
			block_data_t* ptr_end = getDataUnit(numDU) - sizeof(size_t);

			// distance between the pointer of the first payload and pointer of last payload
			size = ptr_end - ptr_begin ;
			
			// plus the nr of byte for all hash values 
			return size + num ; // sizeof(size_t) is deleted ? 
		}

		inline block_data_t* getDataUnits()
        { 
			return buffer + PAYLOAD_POS + sizeof(size_t); 
		} 
     
	     inline void setDataUnits( size_t len, block_data_t *buf )
	     {
	         write<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS, len);
	         memcpy( buffer + PAYLOAD_POS + sizeof(size_t), buf, len);
	     }
		
		inline uint8_t getMessageId(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + MESSAGE_ID );
		}

		inline void setMessageId(uint8_t msgId){
			
			write<OsModel, block_data_t, uint8_t>( buffer + MESSAGE_ID, msgId );
		}
		
		inline node_id_t getSendId(){
			
			return read<OsModel, block_data_t, node_id_t>( buffer + SEND_ID );
		}

		inline void setSendId(node_id_t sendId){
			
			write<OsModel, block_data_t, node_id_t>( buffer + SEND_ID, sendId );
		}

		inline uint8_t getNrOfStaleHashes(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + NR_OF_STALE_DATA_HASHES );
		}

		inline void setNrOfStaleHashes(uint8_t nrOfStaleHashes){
			
			write<OsModel, block_data_t, uint8_t>( buffer + NR_OF_STALE_DATA_HASHES, nrOfStaleHashes );
		}
		

		inline uint8_t getNrOfDataHashes(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + NR_OF_DATA_HASHES );
		}

		inline void setNrOfDataHashes(uint8_t nrOfDataHashes){
			
			write<OsModel, block_data_t, uint8_t>( buffer + NR_OF_DATA_HASHES, nrOfDataHashes );
		}

		inline uint8_t getNrOfDataUnits(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + NR_OF_DATA_UNITS );
		}

		inline void setNrOfDataUnits(uint8_t nrOfDataUnits){
			
			write<OsModel, block_data_t, uint8_t>( buffer + NR_OF_DATA_UNITS, nrOfDataUnits );
		}
		
		inline uint8_t getMsgId(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + NEXTT_BEACON_MSG_MS );
		}

		
		inline void setMsgId(uint8_t nextBeaconMsg){
			
			write<OsModel, block_data_t, uint8_t>( buffer + NEXTT_BEACON_MSG_MS, nextBeaconMsg);
		}

		// read the number of byte of hash type in the 4. position from buffer
		inline uint8_t getTypeOfHash(){
			
			return read<OsModel, block_data_t, uint8_t>( buffer + TYPE_OF_HASH_POS );
		}
		
		// write the number of byte of hash type in the 4. position in the buffer
		inline void setTypeOfHash(uint8_t nrOfbyte){
			
			write<OsModel, block_data_t, uint8_t>( buffer + TYPE_OF_HASH_POS, nrOfbyte );
		}

 		inline size_t buffer_size()
      	{ 
			return PAYLOAD_POS + payload_size();
		}

		inline size_t addDataUnit(size_t len, block_data_t *dataUnit){

		
			
			size_t numDU = getNrOfDataUnits();
			// the pointer of the last empty cell in the buffer to save tha next data hash

			block_data_t* ptr = getDataUnit(numDU) - sizeof(size_t);
			
			// check the size of the data unit is smaller then the max message size 
			if( (ptr - buffer ) + len > MAX_MESSAGE_SIZE ){

				return 0;
			}

			//first save the size of the data unit with the pointer ptr
			write<OsModel, block_data_t, uint8_t>(ptr, len );
			ptr += sizeof(len);
			// then save the data unit after the size position
			memcpy(ptr, dataUnit, len);
			numDU++;
			setNrOfDataUnits(numDU);

			return len;			

		}

		// return the 
		inline block_data_t* getDataUnit(size_t pos){
			
			uint8_t ppsize =0;
			uint8_t num = (getNrOfDataHashes() * getTypeOfHash()) + (getNrOfStaleHashes() * getTypeOfHash());

			while(pos > 0){

				ppsize += read<OsModel, block_data_t, uint8_t>(buffer +  PAYLOAD_POS + ppsize + num) + sizeof(size_t);

				pos--;
			}

			return buffer + PAYLOAD_POS +  ppsize + sizeof(size_t) + num;
		}

		// this function adds one hash value in buffer withe the bayte size, that defines above
		inline size_t addDataHash(block_data_t *dataHash){

			// sicherheit Abfrage für die Größe des Hash Wertes????
			//

			// if already data unit saved (nr of data unit larger then 0) then rteturn 0
			// we can not saved hash values after data units
			if(getNrOfDataUnits() > 0){
				
				return 0;
			}

			size_t numDH = getNrOfDataHashes();
			size_t nrOfByte = getTypeOfHash();

			// the pointer of the last empty cell in the buffer to save tha next hash value
			block_data_t* ptr = getDataHash(numDH);

			// check the size of the data unit is smaller then the max message size 
			/*if( (ptr - buffer ) + nrOfByte > MAX_MESSAGE_SIZE ){

				return 0;
			}*/

			memcpy(ptr, dataHash, nrOfByte);
			numDH++;
			setNrOfDataHashes(numDH);

			return numDH * nrOfByte;			

		}

		// give the pointer of the desird position (id of Hash Value)
		inline block_data_t* getDataHash(size_t pos){

			if(getNrOfDataUnits() > 0 && getNrOfDataHashes() == 0){

				return 0;
			}

			uint8_t ppsize = (getTypeOfHash() * getNrOfStaleHashes()) + getTypeOfHash() * pos  ;
			
			// return the pointer of PAYLOAD_POS + (number of the byte size * pos)
			return buffer + PAYLOAD_POS +  ppsize;
		}

		// this function adds one hash value in buffer withe the bayte size, that defines above
		inline size_t addStaleDataHash(block_data_t *StaleDataHash){

			// sicherheit Abfrage für die Größe des Hash Wertes????
			//


			// if already data unit saved (nr of data unit larger then 0) then rteturn 0
			// we can not saved hash values after data units
			if(getNrOfDataUnits() > 0 || getNrOfDataHashes() > 0){
				
				return 0;
			}

			size_t numSDH = getNrOfStaleHashes();
			size_t nrOfByte = getTypeOfHash();

			// the pointer of the last empty cell in the buffer to save tha next hash value
			block_data_t* ptr = getStaleDataHash(numSDH);

			// check the size of the data unit is smaller then the max message size 
			/*if( (ptr - buffer ) + nrOfByte > MAX_MESSAGE_SIZE ){

				return 0;
			}*/

			memcpy(ptr, StaleDataHash, nrOfByte);
			numSDH++;
			setNrOfStaleHashes(numSDH);

			return numSDH * nrOfByte;			

		}

		// give the pointer of the desird position (id of Hash Value)
		inline block_data_t* getStaleDataHash(size_t pos){

			if( (getNrOfDataUnits() > 0 && getNrOfStaleHashes() == 0) || 
					getNrOfDataHashes() > 0 && getNrOfStaleHashes() == 0){

				return 0;
			}

			uint8_t ppsize = getTypeOfHash() * pos ;
			
			// return the pointer of PAYLOAD_POS + (number of the byte size * pos)
			return buffer + PAYLOAD_POS +  ppsize;
		}
	
	private:

		// the differant position of elements in the buffer
		enum data_positions{

			MESSAGE_ID = 0,
			SEND_ID = MESSAGE_ID + 1,
			NEXTT_BEACON_MSG_MS = SEND_ID + sizeof(node_id_t),
			NR_OF_STALE_DATA_HASHES = NEXTT_BEACON_MSG_MS + 1,
			NR_OF_DATA_HASHES = NR_OF_STALE_DATA_HASHES + 1,
			NR_OF_DATA_UNITS = NR_OF_DATA_HASHES + 1,
			TYPE_OF_HASH_POS = NR_OF_DATA_UNITS + 1,
			PAYLOAD_POS = TYPE_OF_HASH_POS + 1

		};

		block_data_t buffer[MAX_MESSAGE_SIZE];

	};
	
	/* constructor
		

	*/
	template<typename OsModel_P,
            typename Radio_P,
			 typename Debug_P>
	AutoCast_Message<OsModel_P, Radio_P, Debug_P>::
	AutoCast_Message(uint8_t nrOfbyteOfHash, node_id_t nID)
	{
		setMessageId(MSG_ID);
		setSendId(nID);
		setMsgId(0);
	  	setNrOfDataUnits(0);
		setNrOfDataHashes(0);
		setNrOfStaleHashes(0);
		// number of byte of hash´value, wil be one time in the 4. position in buffer
		setTypeOfHash(nrOfbyteOfHash);
	  
	}

	template<typename OsModel_P,
            typename Radio_P,
			 typename Debug_P>
	AutoCast_Message<OsModel_P, Radio_P,Debug_P>::
	~AutoCast_Message()
	{
	  
	}
	
	
}
#endif
