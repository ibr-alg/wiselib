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

/*
 * File:   aggregationmsg.h
 * Author: Koninis
 *
 * Created on January 22, 2011, 1:16 PM
 */

#ifndef GREEDY_PARTITION_H
#define	GREEDY_PARTITION_H

#include "util/pstl/vector_static.h"

#define MAX_ITEMS 65

namespace wiselib {

    /**
     * Class that wraps the aggregate value and hides the specific aggregate
     * function. Changing the aggregate class we can use the same implementation
     * of the aggregation protocol with different combine functions.
     */
    template
    <typename OsModel_P, typename AggregateValue_P>
    class greedy_partition {
    public:
        typedef OsModel_P OsModel;
        typedef typename OsModel::block_data_t block_data_t;
        typedef typename OsModel::Radio::node_id_t node_id_t;
        typedef typename OsModel::Radio::size_t size_t;

        typedef AggregateValue_P value_t;
        typedef greedy_partition<OsModel,AggregateValue_P> self_t;

        struct item {
            value_t value;
            uint32_t timeStampMillis;
        };

        typedef wiselib::vector_static<OsModel, struct item, MAX_ITEMS> items_vector_t;
        typedef typename items_vector_t::iterator items_iterator_t;


        greedy_partition() {
        	Avalue = 0;
                timeMillis = 0;
                source = 0;
                k = 10;
                n = 60;
//            value = 0;
        }

        greedy_partition(uint32_t K,uint32_t N) :
                Avalue(0),
                timeMillis(0),
                source(0),
                k(K),
                n(N)
        {}

        void clear() {
            items.clear();
        }

        greedy_partition(block_data_t * buffer) {
            Avalue = read<OsModel, block_data_t, value_t>(buffer);
        }

        void writeTo(uint8_t *buffer) {
            write<OsModel, block_data_t, value_t> (buffer, Avalue);
        }

        void set_value (value_t v) {
            Avalue = v;
        }

        value_t get() {
            value_t result = 0;
            for ( items_iterator_t
                    it = items.begin();
                    it != items.end();
                    it++) {
                if ( result < it->value ) {
                    result = it->value;
                }
            }

            return result;
        }

        void put(value_t value, uint32_t timeStamp) {
            uint32_t index = ((timeStamp*1000)/n)%k;

            if (items.size() < k) {
                struct item new_item;
                new_item.value = value;
                new_item.timeStampMillis = timeStamp;
//                printf("[%d x %d] ",value,timeStamp);
                items.push_back(new_item);
                return;
            }

            if ( timeStamp == 163 || timeStamp == 193) {
//                printf("[%d x %d] \n",value,timeStamp);
//                printf("[%d x %d y %d] \n",index,items.at(index).timeStampMillis,items.at(index).value);

            }
            if ( (timeStamp - items.at(index).timeStampMillis) >= n
                    || value >= items.at(index).value) {
                items.at(index).timeStampMillis = timeStamp;
                items.at(index).value = value;
            }
        }
        
//        aggregate_base& operator+(aggregate_base &rhs) {
//            aggregate_base result;

//            result.set_value((this->get() > rhs.get()) ? this->get() : rhs.get());

//            result.set_value(value + rhs.get());
//            return self_t();
//        }

        self_t combine(self_t &rhs) {
            self_t result;
            result.set_value((this->get() < rhs.get()) ? this->get() : rhs.get());

//            result.set_value(this->get() + rhs.get());
            return result;
        }

        size_t size() {
            return sizeof(value_t);
	}

	void set(uint32_t kk, uint32_t nn) {
		k = kk;
		n = nn;
	}

    private:
        value_t Avalue;
        node_id_t source;
        uint32_t timeMillis;

        items_vector_t items;
        //algorithms parameters
        uint32_t k;
        uint32_t n;

    };

}

#endif
