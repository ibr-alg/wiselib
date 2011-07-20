/************************************************************************
 ** This file is part of the TriSOS project.
 ** Copyright (C) 2009 University of Applied Sciences Lübeck
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
/**
	DataUnit consists of:

	1. node id to identfiy the node, that the sent the message (1 Byte)
	2. mesage id to identfiy the message, that sent from another node (1 Byte)
	3. maxLifeTime: how long time should be saved the message by received node (1 Byte)
	4. length of the payload wil be saved in the buffer, one position before the payload (1 Byte)
	5. payload: the new data, that we want to send to another node (0 - n Bytes)

*/
 
#ifndef __DATAUINT_H__
#define __DATAUINT_H__

// size of payload is 20 Bytes, it can be larger (it depend on the radio interface)
#define MAX_PAYLOAD_SIZE 20

typedef wiselib::OSMODEL Os;

namespace wiselib
{

	template<typename OsModel_P,
            typename Radio_P> 
	class DataUnit 
	{
	
	public:

		typedef OsModel_P OsModel;
      	typedef Radio_P Radio;

		typedef DataUnit<OsModel, Radio_P> self_type;

		typedef typename Radio::block_data_t block_data_t;
		typedef uint8_t hashValue_t;
		typedef DataUnit<Os, Radio> du_t;

		typedef typename Radio::size_t size_t;

		typedef uint16_t hash_Value_t;
		typedef int32_t maxLifeTimeId_t;

		DataUnit();
		~DataUnit();

		// get the payload from buffer by position PAYLOAD_POS (=4)
		inline block_data_t* getPayload()
        { 
			return buffer + PAYLOAD_POS; 
		} 

		// to get the hash value id from buffer by position HASH_VALUE_POS (=1)
		inline hash_Value_t getHashValue()
		{
			
			return read<OsModel, block_data_t, hash_Value_t>( buffer + HASH_VALUE_POS );
		}

		// to give the size of payload of saved data from buffer by position PAYLOAD_LENGTH_POS (=3)
		inline size_t payload_size()
      	{ 
			return read<OsModel, block_data_t, size_t>(buffer + PAYLOAD_LENGTH_POS); 
		}

		// give the real total size of the buffer (HDCDataUnit) inclusive payload
		inline size_t buffer_size()
      	{ 
			return PAYLOAD_POS + payload_size(); 
		
		}

		enum state_t{
			VALID = 0,   // the time of the data unit is valid
			INVALID = 1, // the time of the data unit is invalid
			STALE = 2	 // the data unit have to be saved in "staleDataUnits" set
		};

	protected:

		// define the positions of all single data, that we would to save  
		enum data_positions{

			TYPE_OF_DATA_UNIT = 0,
			NODE_ID_POS = TYPE_OF_DATA_UNIT + 1 , // 1. position in buffer (size= 1 Byte)
			HASH_VALUE_POS = NODE_ID_POS + 1, // 2. position in buffer (size = sizeof(hash_Value_t) Byte) 
			MAXLIFETIME_POS = HASH_VALUE_POS + sizeof(hash_Value_t), // 3. position in buffer (size = sizeof(maxLifeTimeId_t) Bytes)
			PAYLOAD_LENGTH_POS = MAXLIFETIME_POS + sizeof(maxLifeTimeId_t), // 4. position in buffer (size = 1 Byte)
			// 5. position in buffer (size = 1 to MAX_PAYLOAD_SIZE), one position after payload length position
			PAYLOAD_POS = PAYLOAD_LENGTH_POS + sizeof(size_t) 

		}; 
		
		block_data_t buffer[PAYLOAD_POS + MAX_PAYLOAD_SIZE];

	
	};
	
	// constructor
	template<typename OsModel_P,
            typename Radio_P>
	DataUnit<OsModel_P, Radio_P>::
	DataUnit()
	{
		
	}
	
	//destructor
	template<typename OsModel_P,
            typename Radio_P>
	DataUnit<OsModel_P, Radio_P>::
	~DataUnit()
	{
	}

}
#endif
