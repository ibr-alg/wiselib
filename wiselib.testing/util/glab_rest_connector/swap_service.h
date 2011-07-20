/* 
 * File:   ssd_data_container.h
 * Author: maxpagel
 *
 * Created on 27. Dezember 2010, 16:29
 */

#ifndef _SSD_DATA_CONTAINER_H
#define	_SSD_DATA_CONTAINER_H

#include "external_interface/default_return_values.h"
#include "util/wisebed_node_api/command_types.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/list_static.h"
#include "util/pstl/static_string.h"
#include "util/delegates/delegate.hpp"
#include "config_testing.h"
#include <stdint.h>
#include "swap_request_message.h"
#include "swap_response_message.h"
#include "request_types.h"
#include "util/pstl/pair.h"
#include "util/wisebed_node_api/sensors/sensor_controller.h"

namespace wiselib{

   template<typename OsModel_P,
            typename Uart_P,
            typename SensorControler_P,
            typename Debug_P = typename OsModel_P::Debug,
            typename Timer_P = typename OsModel_P::Timer>
   class SwapService{
      
      typedef OsModel_P OsModel;
      typedef Uart_P Uart;
      typedef Timer_P Timer;
      typedef Debug_P Debug;
      typedef SensorControler_P SensorControler;
      typedef SwapService<OsModel,Uart,SensorControler,Debug,Timer> self_type;
      typedef SwapRequestMessage<OsModel> request_t;
      typedef SwapResponseMessage<OsModel> response_t;
      typedef typename Uart::block_data_t block_data_t;
      typedef typename Uart::size_t size_t;
      typedef wiselib::vector_static<OsModel,StaticString,4> meta_data_list_t;
//      typedef wiselib::MapStaticVector<OsModel,uint8_t,StaticString,10 >  sensor_meta_data_map_t;
      typedef wiselib::MapStaticVector<OsModel,uint8_t,meta_data_list_t,2 >  sensor_meta_data_map_t;
//      typedef wiselib::MapStaticVector<OsModel,StaticString,long,3 >  subscriptions_map_t;
      
//      typedef wiselib::MapStaticVector<OsModel, int, pair<StaticString, delegate0<char*> >, 10> sensor_map_t;
//      typedef wiselib::SensorController<OsModel, sensor_map_t, StaticString> sensor_controller_t;


   public:

       SwapService()
        :uart_(0),
                timer_(0),
                debug_(0)
        {

        }
  // --------------------------------------------------------------------
//       typename subscriptions_map_t::reference subscriptions()
//       {
//         return &subscriptions_;
//       }
// --------------------------------------------------------------------
       StaticString& node_meta_data()
       {
           return &node_Meta_data_;
       }
// --------------------------------------------------------------------
       void set_node_meta_dat(const char* str)
       {
           node_Meta_data_ = str;
       }
// --------------------------------------------------------------------
       typename sensor_meta_data_map_t::reference sensor_meta_data()
       {
           return &sensor_meta_data_;
       }
// --------------------------------------------------------------------
		void addSensorMetaData(uint8_t id, StaticString data)
		{
                    if(sensor_meta_data_[id].size()<3)
                    {
                        #ifdef SWAP_SERVICE_DEBUG
                        debug_->debug("adding sendor meta data");
                        #endif
                        sensor_meta_data_[id].push_back(data);
                    }
		}
// --------------------------------------------------------------------
//       typename sensor_list_t::reference sensors()
//       {
//           return &sensors_;
//       }
// --------------------------------------------------------------------
       void timeout_push(void*)
       {

       }
// --------------------------------------------------------------------
       template<class T, void (T::*TMethod)(self_type*)>
       int subscribe(int interval,T *obj_pnt)
       {
           timer_->template set_timer<self_type, &self_type::timer_elapsed>(interval, this, 0);
           return 0;
       }
// --------------------------------------------------------------------
       void timer_elapsed(void* ){

       }
// --------------------------------------------------------------------
       void init(Uart& uart,SensorControler& sensControl,Debug& debug, Timer& timer)
       {
           uart_ = &uart;
           sensor_controler_ = &sensControl;
           timer_ = &timer;
           debug_ = &debug;
           uart_->template reg_read_callback<self_type,
                         &self_type::message_received>( this );
       }
// --------------------------------------------------------------------
       void message_received( size_t length, block_data_t *data )
       {
           #ifdef SWAP_SERVICE_DEBUG
           debug_->debug("swap service received message(%d), type(%d)\n",data[0],data[2]);
           #endif
           request_t *request = (request_t*) data;
           if(request->command_type() == SSD_REST_REQUEST)
           {               
               block_data_t temp;
               uart_->new_packet();
               temp = (block_data_t) SSD_REST_RESPONSE;
               write_escape_packet(1,&temp);
               temp = (block_data_t) request->request_id();
               write_escape_packet(1,&temp);
               
               switch ( request->request_type())
               {
                   case GET_SENSORS:
                   {
                       temp = (block_data_t) DefaultReturnValues<OsModel>::SUCCESS;
                       write_escape_packet(1,&temp);
                       StaticString sensors = sensor_controler_->sensors();
                       #ifdef SWAP_SERVICE_DEBUG
                       debug_->debug("sending sensorids string lkength = %d", sensors.length());
                       debug_->debug(sensors.c_str());
                       #endif
                       temp = (block_data_t) sensors.length();
                       write_escape_packet(1,&temp);
                       write_escape_packet(sensors);
                   }
                   break;
                   case GET_NODE_META_DATA:
                   {
                       #ifdef SWAP_SERVICE_DEBUG
                       debug_->debug("sending node meta data");
                       #endif
                       temp = (block_data_t) DefaultReturnValues<OsModel>::SUCCESS;
                       write_escape_packet(1,&temp);                       
                       temp = (block_data_t) node_Meta_data_.length();
                       write_escape_packet(1,&temp);
                       write_escape_packet(node_Meta_data_);
                   }
                   break;
                   case POST_SENSOR_META_DATA:
                   {
                       #ifdef SWAP_SERVICE_DEBUG
                       debug_->debug("received sensor meta data for sensor %d",*request->request_opts());
                       #endif
                        if(sensor_controler_->has_sensor(*request->request_opts()))
                       {                           
                           block_data_t* request_opts = request->request_opts();
                           request_opts++;
                           block_data_t* size = request_opts;
                           request_opts++;
                           StaticString meta_data = StaticString((char*)request_opts,(char)*size);
                           #ifdef SWAP_SERVICE_DEBUG
                           debug_->debug(meta_data.c_str());
                           #endif
                           meta_data.append("\n");
                           addSensorMetaData(*request->request_opts(),meta_data);

                           temp = (block_data_t) DefaultReturnValues<OsModel>::SUCCESS;
                           write_escape_packet(1,&temp);
                           temp = (block_data_t) 0;
                           write_escape_packet(1,&temp);
                       }else
                       {
                           #ifdef SWAP_SERVICE_DEBUG
                           debug_->debug("sensor not found");
                           #endif
                           temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_UNSPEC;
                           write_escape_packet(1,&temp);
                       }
                   }
                   break;
                   case GET_SENSOR_META_DATA:
                   {

                       if(sensor_controler_->has_sensor(*request->request_opts()))
                       {
                           #ifdef SWAP_SERVICE_DEBUG
                           debug_->debug("sending sensor meta data from sensor %d",*request->request_opts());
                           #endif
                           temp = (block_data_t) DefaultReturnValues<OsModel>::SUCCESS;
                           write_escape_packet(1,&temp);
                           int size = 0;
                           for(typename meta_data_list_t::iterator it = sensor_meta_data_[*request->request_opts()].begin(),
                            end = sensor_meta_data_[*request->request_opts()].end();it!=end;++it)
                           {
                               size+=it->length();
                           }
                           temp = (block_data_t) size;
                           write_escape_packet(1,&temp);
                           for(typename meta_data_list_t::iterator it = sensor_meta_data_[*request->request_opts()].begin(),
                            end = sensor_meta_data_[*request->request_opts()].end();it!=end;++it)
                           {
                               StaticString& smd = (*it);
                               write_escape_packet(smd);
                               debug_->debug(smd.c_str());
                           }

                       }else
                       {
                           temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_UNSPEC;
                           write_escape_packet(1,&temp);
                       }


                   }
                   break;
                   case GET_SENSOR_VALUE:
                   {
                       #ifdef SWAP_SERVICE_DEBUG
                   	   debug_->debug("requesting sensor with id %d",(uint8_t)*request->request_opts());
                           #endif
                       if(sensor_controler_->has_sensor((uint8_t)*request->request_opts()))
                       {
                           #ifdef SWAP_SERVICE_DEBUG
                           debug_->debug("sending senor value for sensor %d\n",*request->request_opts());
                           #endif
                           temp = (block_data_t) DefaultReturnValues<OsModel>::SUCCESS;
                           write_escape_packet(1,&temp);
                           char *value = sensor_controler_->value((uint8_t)*request->request_opts());
                           #ifdef SWAP_SERVICE_DEBUG
                           debug_->debug("value = %d %d %d\n",value[0],value[1],value[2]);
                           debug_->debug("length %d", 2+((size_t)value[1]));
                           #endif

                           temp = (block_data_t) (2+((size_t)value[1]));
                           
                           write_escape_packet(1,&temp);                           

                           write_escape_packet(2+((size_t)value[1]),(block_data_t*)value);
                       }
                       else
                       {
                           temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_UNSPEC;
                           write_escape_packet(1,&temp);
                           temp = (block_data_t) 0;
                           write_escape_packet(1,&temp);
                       }
                   }
                   break;
                   case SUBSCRIBE:
                   {
                       temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_NOTIMPL;
                       write_escape_packet(1,&temp);
                       temp = (block_data_t) 0;
                       write_escape_packet(1,&temp);
                   }
                   break;
                   case CANCEL_SUBSCRIPTION:
                   {
                       temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_NOTIMPL;
                       write_escape_packet(1,&temp);
                       temp = (block_data_t) 0;
                       write_escape_packet(1,&temp);

                   }
                   break;
                   default:
                       temp = (block_data_t) DefaultReturnValues<OsModel>::ERR_NOTIMPL;
                       write_escape_packet(1,&temp);
                       temp = (block_data_t) 0;
                       write_escape_packet(1,&temp);
                   break;
               }
               uart_->end_packet();
               
           }
       }
// --------------------------------------------------------------------
   private:
       enum { DLE = 0x10, STX = 0x02, ETX = 0x03 };

       void write_escape_packet( size_t len,block_data_t* buffer)
       {
            

            for (size_t i = 0; i < len; i++) {
                //DLE characters must be sent twice.
                if ((uint8_t) buffer[i] == DLE)
                {
                    block_data_t temp = (block_data_t) DLE;
                    uart_->write(1,&temp);
                }

                uart_->write(1, &buffer[i]);
            }
        }
       void write_escape_packet(StaticString& buffer)
       {
           write_escape_packet(buffer.length(),(block_data_t*)(buffer.c_str()));
        }
       void write_excape_packet(const char* str)
       {
           StaticString stStr = str;
           write_escape_packet(stStr);
       }      
       
       sensor_meta_data_map_t sensor_meta_data_;
//       subscriptions_map_t subscriptions_;
       StaticString node_Meta_data_;
       Uart* uart_;
       SensorControler* sensor_controler_;
       Timer* timer_;
       Debug* debug_;


   };

}

#endif	/* _SSD_DATA_CONTAINER_H */

