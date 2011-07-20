
#ifndef __ALGORITHMS_SECURE_DFS_CLUSTERING_MESSAGE__
#define __ALGORITHMS_SECURE_DFS_CLUSTERING_MESSAGE__

#include "algorithms/crypto/ecc.h"
#include "util/serialization/simple_types.h"

namespace wiselib {
    template<typename OsModel_P,
            typename Radio_P>
    class SecureDFSMessage {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::size_t size_t;

	enum msg_id_t {
		NEIGHBOR_DISCOVERY   = 0,
		NEIGHBOR_REPLY       = 1,
		JOIN_REQUEST         = 2,
		JOIN_DENY            = 3,
		RESUME               = 4,
		RESUME2              = 5,
		RESUME3              = 6,
		CK_SEND              = 7,
		CKSEND2              = 8,
		CKSEND3              = 9
	};

        inline SecureDFSMessage() {}

	inline void set_msg_id(uint8_t id) {
            write<OsModel, block_data_t, uint8_t>(buffer+MSGID_POS, id);
	}
	inline uint8_t msg_id() {
            return read<OsModel, block_data_t, uint8_t>(buffer+MSGID_POS);
	}

	inline void set_sid(uint16_t sid) {
            write<OsModel, block_data_t, uint16_t>(buffer+SID_POS, sid);
	}
	inline uint16_t sid() {
            return read<OsModel, block_data_t, uint16_t>(buffer+SID_POS);
	}

        inline uint8_t key_size() {
            return sizeof(PubKey)/2;
        }
        inline PubKey* key() {
		PubKey *ret=new PubKey();
		for(int i=0; i<21; i++) {
			ret->W.x.val[i]=ret->W.y.val[i]=0;
		}
		memcpy(ret->W.x.val+21, buffer+KEY_POS, 21);
		memcpy(ret->W.y.val+21, buffer+KEY_POS+21, 21);
		return ret;
        }
        inline void set_key(PubKey *key) {
		memcpy(buffer+KEY_POS, key->W.x.val+21, 21);
		memcpy(buffer+KEY_POS+21, key->W.y.val+21, 21);
        }
        inline size_t buffer_size() {
            return KEY_POS+key_size();
        }
        
    private:
        enum data_positions {
		MSGID_POS = 0,
		SID_POS   = 1,
		KEY_POS   = 3
        };
        uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
    };
}

#endif
