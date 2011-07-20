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

#ifndef __ALGORITHMS_SECURE_HDL_CLUSTERING_MESSAGE__
#define __ALGORITHMS_SECURE_HDL_CLUSTERING_MESSAGE__

#include "securehdlclusteringtypes.h"

namespace wiselib {
    template<typename OsModel_P,
            typename Radio_P>
    class SecureHdlMessage {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::size_t size_t;
        inline SecureHdlMessage() {}
        inline uint8_t current_layer() {
            return read<OsModel, block_data_t, uint8_t>(buffer+CURRENT_LAYER_POS);
        }
        inline void set_current_layer(uint8_t layer) {
            write<OsModel, block_data_t, uint8_t>(buffer+CURRENT_LAYER_POS, layer);
        }
        inline uint8_t key_size() {
            return sizeof(HKCelement_t);
        }
        inline HKCelement_t* key() {
	    return (HKCelement_t*) &(buffer[HKC_ELEMENT_POS]);
        }
        inline void set_key(HKCelement_t *key) {
            memcpy(buffer+HKC_ELEMENT_POS, key, key_size());
        }
        inline size_t buffer_size() {
            return HKC_ELEMENT_POS+sizeof(HKCelement_t);
        }
        
    private:
        enum data_positions {
            CURRENT_LAYER_POS   = 0,
            HKC_ELEMENT_POS     = 1
        };
        uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
    };
}

#endif
