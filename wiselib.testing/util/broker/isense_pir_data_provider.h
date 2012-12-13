
#ifndef ISENSE_PIR_DATA_PROVIDER_H
#define ISENSE_PIR_DATA_PROVIDER_H

#include <isense/modules/security_module/pir_sensor.h>
#include <isense/util/get_os.h>
#include <util/pstl/string_utils.h>
#include <isense/timeout_handler.h>
#include <isense/protocols/ip/coap/coap_resource.h>
#include <isense/timeout_handler.h>
#include <isense/task.h>

namespace wiselib {

	typedef enum{
		on = 1,
		off = 2
	} pirState;

	template<
		typename OsModel_P,
		typename Broker_P,
		typename Timer_P = typename OsModel_P::Timer
	>
	class IsensePirDataProvider : public isense::SensorHandler, public isense::TimeoutHandler, isense::Task {
		public:
			typedef OsModel_P OsModel;
			typedef Broker_P Broker;
			typedef Timer_P Timer;
			
			typedef typename Broker::bitmask_t bitmask_t;
			typedef typename Broker_P::TupleStore TupleStore;
			typedef typename TupleStore::Tuple Tuple;
			typedef typename Broker::column_mask_t column_mask_t;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename Broker::iterator iterator;
			
			
			enum {
				COL_SUBJECT = Broker::COL_SUBJECT,
				COL_PREDICATE = Broker::COL_PREDICATE,
				COL_OBJECT = Broker::COL_OBJECT,
				COL_BITMASK = Broker::COL_BITMASK
			};

			IsensePirDataProvider() {
				uri_hasValue = "<http://spitfire-project.eu/ontology/ns/value>";
				uri_date = "<http://purl.org/dc/terms/date>";
//				uri_type = "rdf:type";
//				uri_sensor = "ssn:Sensor";
//				uri_observedProperty = "ssn:observedProperty";
//				uri_uomInUse = "ssn:uomInUse";
			}
			
			void init(
					char* uri,
					char* docname_base,
					bitmask_t node_mask,
//					char* observedProperty,
					typename Broker::self_pointer_t broker,
					//typename Sensor::self_pointer_t sensor,
					//typename Debug_P::self_pointer_t debug,
					typename Timer::self_pointer_t timer,
					isense::PirSensor *sensor
					)
			{
				timer_ = timer;
				broker_ = broker;
				sensor_ = sensor;
				node_mask_ = node_mask;
				
				sensor_->set_sensor_handler(this);
				sensor_->set_pir_sensor_int_interval( 1000 );
				sensor_->enable();
				
				//uri_hasValue = "hv"; //<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>";
				//uri_date = "<http://purl.org/dc/terms/date>";
				//uri_type = "rdf:type";
				//uri_sensor = "ssn:Sensor";
				//uri_observedProperty = "ssn:observedProperty";
				//uri_uomInUse = "ssn:uomInUse";

				me_ = uri;
//				observed_property_ = observedProperty;
//				uom_ = "<http://gradcelsius.de>";

				char *s = alloc_strcat(docname_base, "/_intrinsic"); //postfix_intrinsic_);
				mask_intrinsic_ = broker_->create_document(s);

				s = alloc_strcat(docname_base, "/_minimal"); //postfix_minimal_);
				mask_minimal_ = broker_->create_document(s);

				s = alloc_strcat(docname_base, ""); //postfix_complete_);
				mask_complete_ = broker_->create_document(s);

				//sensor_->template register_sensor_callback<self_type, &self_type::on_sens > (this);
				//time_ = 0;

				//debug_->debug("Init of sensor complete %s\n", uri);

				timeout_id_ = 0xff;

//				add_static_info();
			}
			
			void setResource( isense::ip_stack::Resource* resource ){
				resource_ = resource;
			}

		private:
			
			void add_static_info() {
				/*
				   intrinsic:

				   <http://spitfire-project.eu/sensor/SENSORNAME> ns0:type <http://purl.oclc.org/NET/ssnx/ssn#Sensor> ;
												   ns1:observedProperty <http://spitfire-project.eu/property/Temperature> ;
												   ns2:uomInUse <http://spitfire-project.eu/uom/Centigrade> ;
												   ns3:hasValue "10.2" ;

				 */
				bitmask_t ic = mask_intrinsic_ | mask_complete_;
				insert_tuple(me_, uri_type, uri_sensor, ic);
				insert_tuple(me_, uri_observedProperty, observed_property_, ic);
				insert_tuple(me_, uri_uomInUse, uom_, ic);

			}
			
			//void on_time(void*) {
				//if(current_epoch_activity_ != last_epoch_activity_) {
					//update();
				//}
				
				//last_epoch_activity_ = current_epoch_activity_;
			//}
			
			void handle_sensor() {
//				GET_OS.fatal("KLIrrr");
//				current_epoch_activity_ = !this_is_the_end;
//
//				if(current_epoch_activity_ != last_epoch_activity_) {
//					update();
//				}
//
//				last_epoch_activity_ = current_epoch_activity_;
				if( timeout_id_ == 0xff ){
					/*
					 * Update resource( new notification )
					 */
//					update( true );
//					resource_->update( resource_ );
					GET_OS.add_task( this, (void*)on );
					timeout_id_ = GET_OS.add_timeout_in( 5000, this, NULL );

				} else {
					/*
					 * Prolong timeout
					 */
					GET_OS.remove_timeout( timeout_id_, this );
					timeout_id_ = GET_OS.add_timeout_in( 5000, this, NULL );
				}
			}
			
			void execute( void* userdata ){
				if( userdata == (void*)on ){
					update( true );
				} else if( userdata == (void*)off ){
					update( false );
				}
				resource_->update( resource_ );
			}

			void timeout( void* userdata ){
				/*
				 * No pir event within the last n seconds, send free notification
				 *
				 * TODO: maybe put this to task
				 */
				timeout_id_ = 0xff;
//				update( false );
//				resource_->update( resource_ );
				GET_OS.add_task( this, (void*)off );
			}

			TupleStore& tuple_store() { return broker_->tuple_store(); }
			
			void insert_tuple(char *s, char *p, char *o, bitmask_t bm) {
				Tuple t;
				t.set(s, p, o, bm);
				tuple_store().insert(t);
			}
		
			void replace_tuple(Tuple& t, column_mask_t query_columns) {
				bitmask_t bm = 0;
				iterator iter = tuple_store().begin(&t, query_columns);
				if(iter != tuple_store().end()) {
					bm = iter->bitmask();
					tuple_store().erase(iter);
				}
				t.set_bitmask(t.bitmask() | bm);
				tuple_store().insert(t);
			}
			
			void update( bool value ) {
				Tuple t;
				
				t.set(me_, uri_hasValue, const_cast<char*>(value ? "\"1\"" : "\"0\""), mask_intrinsic_ | mask_minimal_ | mask_complete_ | node_mask_);
				replace_tuple(t, (1 << COL_SUBJECT) | (1 << COL_PREDICATE));
				
				//t.set(me_, uri_date, date, mask_intrinsic_ | mask_minimal_ | mask_complete_);
				//replace_tuple(t, (1 << COL_SUBJECT) | (1 << COL_PREDICATE));
			}
			
			bool current_epoch_activity_, last_epoch_activity_;
        char *me_, *observed_property_, *uom_;
        char *uri_hasValue, *uri_date;
        char *uri_type, *uri_observedProperty, *uri_uomInUse, *uri_sensor;
			typename Timer::self_pointer_t timer_;
			typename Broker::self_pointer_t broker_;
					isense::PirSensor *sensor_;
			bitmask_t mask_intrinsic_, mask_minimal_, mask_complete_, node_mask_;

			uint8 timeout_id_;
			isense::ip_stack::Resource* resource_;
	};
	
}

#endif // ISENSE_PIR_DATA_PROVIDER_H


