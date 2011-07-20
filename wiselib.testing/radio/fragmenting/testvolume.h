/* 
 * File:   testvolume.h
 * Author: amaxilatis
 *
 * Created on August 2, 2010, 2:32 PM
 */

#ifndef _TESTVOLUME_H
#define	_TESTVOLUME_H

#include "util/delegates/delegate.hpp"
#include <iostream>
#include <string.h>
//#define TIME_DELAY 2000
//#define COUNTER_MAX 1000

namespace wiselib {

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    class testvolume {
    public:

        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        
        typedef Debug_P Debug;
        typedef Timer_P Timer;

        typedef testvolume<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;



 

        void enable(void);
        void disable(void);


   
        void receive(node_id_t receiver, size_t len, block_data_t *data);

      
        void send_messages(void * node) {
            int BIG_MESSAGE = 30000;

            //if (counter_*150>BIG_MESSAGE) return;
            if (counter_==10) return;

            int items = counter_*100-1;
            uint8_t payload[2*items+2];
            payload[0]=items%256;
            payload[1]=items/256;
            for (int i=0;i<items;i++) {
                payload[2+i*2] = i%256;
                payload[2+i*2+1] = i/256;
            }

             debug().debug("Application::values [");
             for (int i=0;i<items;i++){
             //    debug().debug(" %d",payload[2+i*2]+payload[2+i*2+1]*256);

             }
             debug().debug("]\n");


            debug().debug("Application::send node= %d dest= %d  message_size=%d\n",radio().id(),node,2*items);



            radio().send(/*Radio_t::BROADCAST_ADDRESS*/ (node_id_t)node ,2*(items+1)*sizeof(uint8_t),payload);

            counter_++;
            timer().template set_timer<self_t, &self_t::send_messages > (
                    3000, this, (void*) node);

        }

        ///@name Construction / Destruction
        ///@{

        testvolume()

        {
        }

        ~testvolume() {
        }
        ///@}



        void init (Radio& radio , Timer& timer , Debug& debug){
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };
    private:
        int callback_id_;
        int counter_;



        Radio * radio_;
        Timer * timer_;
        Debug * debug_;


        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }
        
    };

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testvolume<OsModel_P, Radio_P, Timer_P, Debug_P>::
    enable(void) {

        
        radio().enable_radio();
        debug().debug( "Booting up");
        debug().debug( ".");

        callback_id_ = radio().template reg_recv_callback<self_t,
                &self_t::receive > (this);
        debug().debug( ".");
        counter_=1;
        debug().debug( ".");

        debug().debug( "OK!\n");


        if (radio().id() == 9) {

            send_messages((void*)2);
            //send_messages((void*)1);
            



        }

    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testvolume<OsModel_P, Radio_P, Timer_P, Debug_P>::
    disable(void) {
        
        
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testvolume<OsModel_P, Radio_P, Timer_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {

        if (from==radio().id())
            return;
        else{
             uint16_t count = data[0]+data[1]*256;
             debug().debug("Application::receive count= %d\n",count);

             uint8_t * values = new uint8_t[count*2];

             memcpy(values,data+2,count*2);
             uint16_t prev=values[0]+values[1]*256,current;
             bool ok=true;

             for (int i=1;i<count;i++){
                 
                 current = values[i*2]+values[i*2+1]*256;
                 if(prev+1!=current){
                    debug().debug("Application::receive error\n");
                    ok=false;
                 }
                 prev=current;

             }
             if (ok){
                debug().debug("Application::receive status= ok count= %d\n",count);
             }


        }

    }


}

#endif	/* _TESTVOLUME_H */

