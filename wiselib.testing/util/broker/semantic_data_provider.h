/*
 * File:   SemanticDataProvider.h
 * Author: maxpagel
 *
 * Created on 19. April 2012, 13:42
 */

#ifndef _SEMANTICDATAPROVIDER_H
#define _SEMANTICDATAPROVIDER_H

#include <util/delegates/delegate.hpp>
#include <util/pstl/string_utils.h>
#include <isense/protocols/ip/coap/coap_resource.h>
#include <isense/timeout_handler.h>
#include <isense/util/get_os.h>
#include <isense/task.h>

namespace wiselib
{

    template<typename OsModel_P,
        typename Broker_P,
        typename Sensor_P,
        typename Debug_P = typename OsModel_P::Debug,
        typename Timer_P = typename OsModel_P::Timer
    >
    class SemanticDataProvider : public isense::TimeoutHandler, public isense::Task
    {
        typedef Broker_P Broker;
        typedef Sensor_P Sensor;
        typedef OsModel_P Os;
        typedef Timer_P Timer;

        typedef typename Broker::bitmask_t bitmask_t;

        typedef typename Broker_P::TupleStore TupleStore;
        typedef typename Broker_P::TupleStore TupleStore_t;
        typedef typename Broker_P::TupleStore tuple_store_t;
        typename TupleStore_t::self_pointer_t tuple_store_;
        //typename TempSensor_t::self_pointer_t temp_sensor_;
        typedef SemanticDataProvider<Os, Broker, Sensor> self_type;

        //typedef iSenseTemperatureCallbackSensor<Os> TempSensor_t; //TODO generalize
        typedef typename TupleStore_t::Tuple tuple_t;
        typedef typename Os::block_data_t block_data_t;

    public:

        enum {
            COL_SUBJECT = Broker::COL_SUBJECT,
            COL_PREDICATE = Broker::COL_PREDICATE,
            COL_OBJECT = Broker::COL_OBJECT,
            COL_BITMASK = Broker::COL_BITMASK
        };

        SemanticDataProvider()
        {
			uri_hasValue = "<http://spitfire-project.eu/ontology/ns/value>";
            uri_date = "<http://purl.org/dc/terms/date>";
            time_ = 0;
            busy_ = false;
            
        }

        void add_tuple(char* s, char* p, char* o, bitmask_t mask) {
            //debug_->debug("add tpl (%s %s %s %x)\n", s, p, o, mask);
            tuple_t t;
            t.set(COL_SUBJECT, (block_data_t*)s);
            t.set(COL_PREDICATE, (block_data_t*)p);
            t.set(COL_OBJECT, (block_data_t*)o);
            t.set_bitmask(mask);
            tuple_store_->insert(t);
        }


        void init(
                char* uri,
                char* docname_base,
                bitmask_t mask_node,
//                char* observedProperty,
                typename Broker::self_pointer_t broker,
                typename Sensor::self_pointer_t sensor,
                typename Debug_P::self_pointer_t debug,
                typename Timer::self_pointer_t timer
                )
        {
            timer_ = timer;
            debug_ = debug;
            broker_ = broker;
            tuple_store_ = &broker->tuple_store();
            sensor_ = sensor;
            sensor_->enable();

            type_ = 0;
            resource_ = NULL;
            timeout_id_ = 0xff;
            time_ = 0;
            busy_ = false;

            me_ = uri;

            mask_node_ = mask_node;
            char *s = alloc_strcat(docname_base, postfix_intrinsic_);
            mask_intrinsic_ = broker_->create_document(s);

            s = alloc_strcat(docname_base, postfix_minimal_);
            mask_minimal_ = broker_->create_document(s);

//            s = alloc_strcat(docname_base, postfix_complete_);
            s = alloc_strcat(docname_base, "");
            mask_complete_ = broker_->create_document(s);

            sensor_->template register_sensor_callback<self_type, &self_type::on_sens > (this);
            time_ = 0;
        }

        void on_sens(typename Sensor::value_t v)
        {
//        	debug_->debug("sensing %s %d",observed_property_, v);
            if(!busy_){
                last_measurement_ = v;
//                timer_->template set_timer<self_type,
//                     &self_type::update> (0, this, 0);
                if( resource_ != NULL && type_ != 0 ){
                	if( type_ == 1 /*light*/ && timeout_id_ == 0xff){
                		timeout_id_ = GET_OS.add_timeout_in( 5000, this, NULL );
                	} else if( type_ == 2 )/*temp*/{
                		/*temperature*/
//                		GET_OS.fatal("/*temperature*/, %i", v);
//                		update();
//						resource_->update( resource_ );
                		GET_OS.add_task( this, NULL );
                	}
                }
            }
        }

        void execute( void* userdata ){
        	update();
        	resource_->update( resource_ );
        }

        void timeout( void* userdata ){
//        	GET_OS.fatal("/*light*/");
        	timeout_id_ = 0xff;
        	GET_OS.add_task( this, NULL );
//        	update();
//        	resource_->update( resource_ );
        }

        void setResourceAndType( isense::ip_stack::Resource* resource, uint8 type ){
        	resource_ = resource;
        	type_ = type;
        }

        void update()
        {
            debug_->debug("<sensor update>");
//        	debug_->debug("beginMEM: %u", mem->mem_free());
        	const size_t buffer_size = 32;
            char date[buffer_size];
            char data[buffer_size];
//             debug_->debug("updating TS");
            busy_ = true;
            //int_to_string(time_, date, buffer_size);
            //int_to_string(last_measurement_, data, buffer_size);
            snprintf(date, buffer_size, "%d", time_);
            snprintf(data, buffer_size, "\"%d\"", last_measurement_);
            
            tuple_t t;
            bitmask_t old_bitmask_hasvalue, old_bitmask_date;

            // Erase hasValue tuple for this sensor
            // ( $me :hasValue * * )

            t.set(COL_SUBJECT, (block_data_t*)me_);
            t.set(COL_PREDICATE, (block_data_t*)uri_hasValue);

            typename tuple_store_t::iterator it_end = tuple_store_->end( );
//            debug_->debug("1");
            typename tuple_store_t::iterator iter = tuple_store_->begin(&t, (1 << COL_SUBJECT) | (1 << COL_PREDICATE));
//            debug_->debug("2");
            if(iter != tuple_store_->end()) {
            	//debug_->debug("2a %u", mem->mem_free());
//            	debug_->debug("2a");
            	old_bitmask_hasvalue = iter->bitmask(); //Broker::get_bitmask(*iter);
//            	debug_->debug("2b");
                tuple_t t;
                t.set(0, iter->get(0));
                t.set(1, iter->get(1));
                t.set(2, iter->get(2));
                t.set(3, iter->get(3));
                // debug_->debug( "erasing s %s p %s o %s m %s \n",t[0].c_str( ), t[1].c_str( ), t[2].c_str( ), t[3].c_str( ) );
//                debug_->debug("pre erase");
         //XXX
                tuple_store_->erase(iter);
            }
            else {
                old_bitmask_hasvalue = 0;
            }
            t.set(COL_SUBJECT, (block_data_t*)me_);
            t.set(COL_PREDICATE, (block_data_t*)uri_date);
//            debug_->debug("3");
            iter = tuple_store_->begin(&t, (1 << COL_SUBJECT) | (1 << COL_PREDICATE));
//            debug_->debug("4");
            if(iter != tuple_store_->end()) {
                old_bitmask_date = iter->bitmask(); //Broker::get_bitmask(*iter);
        //XXX
                tuple_store_->erase(iter);
            }
            else {
                old_bitmask_date = 0;
            }

            // Insert new hasValue tuple
            // ( $me :hasValue * new_bitmask )

            t.set(COL_SUBJECT, (block_data_t*)me_);
            t.set(COL_PREDICATE, (block_data_t*)uri_hasValue);
            t.set(COL_OBJECT, (block_data_t*)data);
            t.set_bitmask(old_bitmask_hasvalue | mask_intrinsic_ | mask_minimal_ | mask_complete_ | mask_node_);

            tuple_store_->insert(t);

            time_++;
            busy_ = false;
            debug_->debug("</sensor update>");
        }

        typename Sensor::self_pointer_t sensor_;
    private:
        //typename iSenseTemperatureCallbackSensor<Os>::self_pointer_t sensor_;
        typename Broker::self_pointer_t broker_;
        typename Timer::self_pointer_t timer_;
        typename Debug_P::self_pointer_t debug_;
        static char *postfix_intrinsic_,
                     *postfix_minimal_,
                     *postfix_complete_;
        bitmask_t mask_intrinsic_,
                  mask_minimal_,
                  mask_complete_,
                  mask_node_;
        char *me_, *observed_property_, *uom_;
        char *uri_hasValue, *uri_date;
        char *uri_type, *uri_observedProperty, *uri_uomInUse, *uri_sensor;
        uint16_t time_;
        typename Sensor::value_t last_measurement_;
        bool busy_;
        isense::ip_stack::Resource* resource_;
        uint8 type_;
        uint8 timeout_id_;
    };


    template<typename OsModel_P,
    typename Broker_P,
    typename Sensor_P,
    typename Debug_P,
    typename Timer_P
    >
    char* SemanticDataProvider<OsModel_P, Broker_P, Sensor_P, Debug_P,Timer_P>::postfix_intrinsic_ = "/_intrinsic";

    template<typename OsModel_P,
    typename Broker_P,
    typename Sensor_P,
    typename Debug_P,
    typename Timer_P
    >
    char* SemanticDataProvider<OsModel_P, Broker_P, Sensor_P, Debug_P,Timer_P >::postfix_minimal_ = "/_minimal";

    template<typename OsModel_P,
    typename Broker_P,
    typename Sensor_P,
    typename Debug_P,
    typename Timer_P
    >
    char* SemanticDataProvider<OsModel_P, Broker_P, Sensor_P, Debug_P,Timer_P >::postfix_complete_ = "/_complete";
}

#endif	/* _SEMANTICDATAPROVIDER_H */

/* vim: set ts=4 sw=4 tw=78 expandtab :*/
