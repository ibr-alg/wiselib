/*
 * File:   aggregationmsg.h
 * Author: Koninis
 *
 * Created on January 22, 2011, 1:16 PM
 */

#ifndef AGGREGATE_H
#define	AGGREGATE_H

namespace wiselib {

    /**
     * Class that wraps the aggregate value and hides the specific aggregate
     * function. Changing the aggregate class we can use the same implementation
     * of the aggregation protocol with different combine functions.
     */
    template
    <typename OsModel_P, typename AggregateValue_P>
    class aggregate_base {
    public:
        typedef OsModel_P OsModel;
        typedef typename OsModel::block_data_t block_data_t;
        typedef typename OsModel::Radio::node_id_t node_id_t;
        typedef typename OsModel::Radio::size_t size_t;

        typedef AggregateValue_P value_t;
        typedef aggregate_base<OsModel,AggregateValue_P> self_t;

        aggregate_base() {
        	value = 0xFFFFFFFF;
//                timeStampMillis = 0;
//                source = 0;
//            value = 0;
        }

        aggregate_base(block_data_t * buffer) {
            value = read<OsModel, block_data_t, value_t>(buffer);
        }

        void set_value (value_t v) {
            value = v;
        }

        value_t get() {
            return value;
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

        void writeTo(uint8_t *buffer) {
            write<OsModel, block_data_t, value_t> (buffer, value);
        }

        size_t size() {
            return sizeof(value);
        }

    private:
        value_t value;
//        node_id_t source;
//        uint32_t timeStampMillis;
    };

}

#endif
