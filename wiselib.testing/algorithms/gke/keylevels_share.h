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

#ifndef _KEYLEVELS_SHARE_H
#define _KEYLEVELS_SHARE_H

#define COMMON_SEED 0
#ifndef SHAWN
  #define KEYSHARE_SIZE 16
  #define KEYPOOL_SIZE 250
#else
  #warning tutaj czary zeby sie z pliku czytalo
  #define KEYSHARE_SIZE _getFileKeyshareSize()
  #define KEYPOOL_SIZE _getFileKeypoolSize()
#endif

#define KEY_LEVELS 3
#define SMALL_KEY_SIZE 16
#define KEY_INFO_SIZE 3 //key index, key level, key value


#include "util/pstl/map_static_vector.h"
#include "keylevels_main.h"

#ifdef SHAWN
#include <string>
#include "sys/simulation/simulation_controller.h"
#include "sys/processor.h"
#include "external_interface/shawn/shawn_types.h"
#include "sys/taggings/basic_tags.h"
#include "sys/taggings/group_tag.h"
using namespace shawn;

#endif

namespace wiselib {

typedef wiselib::OSMODEL Os;
typedef Os::Rand Random;

typedef struct p {
	uint16_t key_index;
	uint16_t key_level;
	uint8_t key_value[SMALL_KEY_SIZE];
} key;

typedef struct ki {
	uint16_t key_index;
	uint16_t key_level;
} key_info;

typedef struct sk {
	uint8_t key_data[GROUP_KEY_SIZE];
	bool have_key;
	Random *random_;

	void leader_init(Random* random) {
		//random->srand(0);
		init(random);
		have_key = true;
		for (int i = 0; i < GROUP_KEY_SIZE; i++) {
			key_data[i] = (uint8_t) (*random)() % 256;
			//key_data[i] = (uint8_t) 170;
		}
	}

	void init(Random* random) {
		random_ = random;
		have_key = false;
	}

	void set_key(uint8_t* key_address) {
		memcpy(&key_data, key_address, GROUP_KEY_SIZE);
		have_key = true;
	}
} group_key;

template<typename OsModel_P, typename Radio_P,
typename Debug_P = typename OsModel_P::Debug>
class KeyShare {

public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;

	//typedef typename wiselib::set_static<OsModel, key, KEYSHARE_SIZE> keyshare_t;
	typedef typename Radio::node_id_t node_id_t;
	typedef typename wiselib::MapStaticVector<OsModel, node_id_t, key, NODES_MAX> trusted_links_t;

	enum ReturnValues {
		SUCCESS = OsModel::SUCCESS,
		ERR_UNSPEC = OsModel::ERR_UNSPEC
	};

	KeyShare() 
	{ 
#ifdef SHAWN
	// PTB czytanie danych z pliku
	if(!(keyshare = (key*) malloc(KEYSHARE_SIZE * sizeof(key))))
	{
		fprintf(0, "No memory for keyshare!\n");
		exit(-1);
	}
#endif
	}



	void variation_on_SDBMHash(uint8_t* data, unsigned int len) {
		uint8_t hash = 0;
		int i = 0;

#ifdef HASH_DEBUG
		debug_->debug("[KLS] {%d} HASH: in: ",radio_->id());
		print_key_value(data,len);
		debug_->debug("\n");
#endif
		for(unsigned int j=0;j<len;j++) {
			i=j;
			hash = data[i] + (hash << 6) + (hash << 16) - hash;
			(++i)%=len;
			hash = data[i] + (hash << 6) + (hash << 16) - hash;
			data[j] = hash;
		}
#ifdef KEYLEVELS_SHARE_DEBUG
		debug_->debug("[KLS] {%d} HASH: out: ",radio_->id());
		print_key_value(data,len);
		debug_->debug("\n");
#endif
	}

	int init(Radio& radio, Debug& debug, Random& random) {
		radio_ = &radio;
		debug_ = &debug;
		random_ = &random;
		//#ifdef SHAWN
		//	readKeysFromXML();
		//#else
		fillKeyshareWithFakeKeys();
		fillKeyshareWithKeys(random_);
//		fillKeyshareWithTestKeys(random_);
		//#endif
		return SUCCESS;
	}

	void fillKeyshareWithKeys(Random* random) {
		fillKeyshareWithKeyIndexesAndLevels(random);
		fillKeyshareWithKeyValues(random);
#ifdef KEYLEVELS_SHARE_DEBUG
		listKeyshare();
#endif
	}

	void fillKeyshareWithKeyIndexesAndLevels(Random* random) {
		bool already;
		random->srand(radio_->id());
		uint16_t key_index, key_level;
		for (unsigned int i = 0; i < KEYSHARE_SIZE; i++) {
			already = true;
			while (already) {
				key_index = (*random)() % KEYPOOL_SIZE;
				already = false;
				if (owns_key(key_index)) {
					already = true;
				}
				//for(unsigned int j=0; j<i; j++){
				//	if (keyshare[j].key_index == key_index){
				//		already = true;
				//		break;
				//	}
				//}
			}
			key_level = (*random)() % KEY_LEVELS;
			keyshare[i].key_index = key_index;
			keyshare[i].key_level = key_level;
		}
	}

	void fillKeyshareWithKeyValues(Random* random) {

		random->srand(COMMON_SEED);
		uint8_t temp_key[SMALL_KEY_SIZE];
		int i=0; //key pool iterator
		int k=0; //key value iterator
		int l=0; //key level iterator
		for (i = 0; i < KEYPOOL_SIZE; i++) {
			for(k=0;k<SMALL_KEY_SIZE;k++) {
				temp_key[k] = (*random)();
			}
#ifdef KEYLEVELS_SHARE_DEBUG
			debug_->debug("[KLS] {%d} pool key[%d] = ", radio_->id(), i);
			print_key_value(temp_key, SMALL_KEY_SIZE);
			debug_->debug("\n");
#endif
			if (owns_key(i)) {
				for(k=0;k<SMALL_KEY_SIZE;k++) {
					get_key(i)->key_value[k]=temp_key[k];
				}
				for(l=0; l<get_key(i)->key_level; l++) {
#ifdef HASH_DEBUG
					debug_->debug("[KLS] {%d} pool key[%d] level %d = ",radio_->id(),i,l);
					print_key_value(get_key(i)->key_value,SMALL_KEY_SIZE);
					debug_->debug("\n");
#endif
					variation_on_SDBMHash(get_key(i)->key_value,SMALL_KEY_SIZE);
				}
#ifdef HASH_DEBUG
				debug_->debug("[KLS] {%d} pool key[%d] level %d = ",radio_->id(),i,get_key(i)->key_level);
				print_key_value(get_key(i)->key_value,SMALL_KEY_SIZE);
				debug_->debug("\n");
#endif
			}
		}
	}

	void fillKeyshareWithTestKeyIndexesAndLevels() {
#ifndef SHAWN
		if(radio_->id() == 3261)
		{
			keyshare[0].key_index = 1;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 2;
			keyshare[1].key_level = 1;
		}else if(radio_->id() == 3234)
		{
			keyshare[0].key_index = 2;
			keyshare[0].key_level = 4;
			keyshare[1].key_index = 3;
			keyshare[1].key_level = 0;
		}else if(radio_->id() == 0x1c2c)
		{
			keyshare[0].key_index = 5;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 3;
			keyshare[1].key_level = 0;
		}
		return;
#endif
#ifdef SHAWN
		switch (radio_->id()) {
			case 0:
			keyshare[0].key_index = 1;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 2;
			keyshare[1].key_level = 7;
			break;
			case 1:
			keyshare[0].key_index = 2;
			keyshare[0].key_level = 4;
			keyshare[1].key_index = 3;
			keyshare[1].key_level = 0;
			break;
			case 2:
			keyshare[0].key_index = 5;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 3;
			keyshare[1].key_level = 0;
			break;
			case 3:
			keyshare[0].key_index = 5;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 6;
			keyshare[1].key_level = 0;
			break;
			case 4:
			keyshare[0].key_index = 2;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 8;
			keyshare[1].key_level = 0;
			break;
			case 5:
			keyshare[0].key_index = 2;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 9;
			keyshare[1].key_level = 0;
			break;
			case 6:
			keyshare[0].key_index = 6;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 10;
			keyshare[1].key_level = 0;
			break;
			case 7:
			keyshare[0].key_index = 4;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 3;
			keyshare[1].key_level = 0;
			break;
			case 8:
			keyshare[0].key_index = 4;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 5;
			keyshare[1].key_level = 0;
			break;
			case 9:
			keyshare[0].key_index = 1;
			keyshare[0].key_level = 0;
			keyshare[1].key_index = 11;
			keyshare[1].key_level = 0;
			break;
		}
#endif

	}

	void fillKeyshareWithFakeKeys() {

		for (unsigned int i = 0; i < KEYSHARE_SIZE; i++) {
			uint16_t key_index, key_level;
			bool already = true;

			while (already) {
				key_index = random() % KEYPOOL_SIZE;
				already = false;

				for (unsigned int j = 0; j < i; j++) {
					if (keyshare[j].key_index == key_index) {
						already = true;
						break;
					}
				}
			}

			key_level = random() % KEY_LEVELS;
			keyshare[i].key_index = key_index;
			keyshare[i].key_level = key_level;
		}
	}

	void fillKeyshareWithTestKeys(Random* random)
	{
		fillKeyshareWithTestKeyIndexesAndLevels();
		fillKeyshareWithKeyValues(random);
	}

#ifndef SHAWN
	int random() { return 1; }
#endif

	unsigned int get_keyshare_size() { return KEYSHARE_SIZE; }

	void listKeyshare() 
	{
		for (unsigned int i = 0; i < KEYSHARE_SIZE; i++) 
		{
			debug_->debug("[KLS] Key: (%d,%d): ", keyshare[i].key_index,keyshare[i].key_level);
			print_key(keyshare[i].key_value,SMALL_KEY_SIZE);
			debug_->debug("\n");
		}
	}

	key* get_key(uint16_t key_index)
	{
		for (unsigned int i = 0; i < KEYSHARE_SIZE; i++) {
			if (keyshare[i].key_index == key_index) {
				return &keyshare[i];
			}
		}
		return NULL;
	}

	bool owns_key(uint16_t key_index)
	{
		return get_key(key_index) != NULL;
	}

	bool trusted_link_exists(node_id_t node)
	{
		return trusted_links_.find(node) != trusted_links_.end();
	}

	void put_trusted_link(node_id_t node, key link)
	{
#ifdef KEYLEVELS_SHARE_DEBUG
		debug_->debug("[KLS] {%d} put_trusted_link: ",radio_->id());
		print_key_value(link.key_value,SMALL_KEY_SIZE);
		debug_->debug("\n");
#endif
		trusted_links_[node] = link;

	}

	key* get_key_info(node_id_t node) 
	{
		if (trusted_link_exists(node)) {
			return &trusted_links_[node];
		}
		return NULL;
	}

	void print_key(uint8_t* key, int size) 
	{
		for (int i = 0; i < size; i++, key++) {
			debug_->debug("%d", *key);
		}
	}

	void print_key_value(uint8_t* value, uint8_t size) 
	{
			for (int i = 0; i < size; i++) {
				debug_->debug("%d:", value[i]);
			}
			debug_->debug("\n");
	}

	void print_all_key_info(key* k) 
	{
			debug_->debug("[KLS] {%d} KEY: (%d,%d) ", radio_->id(), k->key_index,
					k->key_level);
			print_key_value(k->key_value, SMALL_KEY_SIZE);
	}

#ifdef SHAWN
	void readKeysFromXML()
	{
		ShawnOs& os_ = radio_->os();
		const shawn::GroupTag *gt = NULL;
		int _lvl, _id, keyRdy = 0;
		std::string _kstr;

		shawn::ConstTagHandle th = os_.proc->owner().find_tag("keyset");
		if(th != NULL)
		{
			gt = dynamic_cast<const shawn::GroupTag*>(th.get());
		} else { }
		if(gt != NULL)
		{
			shawn::TagContainer::tag_iterator ti = gt->begin_tags();
			unsigned int i = 0;
			while(ti != gt->end_tags())
			{
				shawn::TagHandle tth = ti->second;
				const shawn::GroupTag* ggt = dynamic_cast<const shawn::GroupTag*>(tth.get());
				if(ggt != NULL)
				{
					shawn::TagContainer::tag_iterator tti = ggt->begin_tags();
					while(tti != ggt->end_tags())
					{
						TagHandle ttth = tti->second;
						if(ttth->name() == "key")
						{
							_kstr = ttth->encoded_content().c_str();
							keyRdy++;
						}
						else if(ttth->name() == "level")
						{
							_lvl = atoi(ttth->encoded_content().c_str());
							keyRdy++;
						}
						else if(ttth->name() == "id")
						{
							_id = atoi(ttth->encoded_content().c_str());
							keyRdy++;
						}
						else
						{}
						tti++;
					}
					if(keyRdy == 3)
					{
						if(i < KEYSHARE_SIZE)
						{
							keyshare[i].key_index = _id;
							keyshare[i].key_level = _lvl;
							i++;
						}
					}
					keyRdy = 0;
				}
				ti++;
			}
		}
	}
#endif

// PTB czytanie danych z pliku
#ifndef SHAWN
	key keyshare[KEYSHARE_SIZE];
#else
	key* keyshare;
#endif

private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	typename Random::self_pointer_t random_;


	trusted_links_t trusted_links_;
};

} /* namespace wiselib */

#endif  /* _KEYLEVELS_SHARE_H */
