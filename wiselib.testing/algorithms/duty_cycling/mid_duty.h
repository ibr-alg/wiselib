/* 
 * File:   dutycycling.h
 * Author: dante
 *
 * Created on August 11, 2012, 6:18 PM
 */

#ifndef MIDISENSEDUTYCYCLING_H
#define	MIDISENSEDUTYCYCLING_H

#include "external_interface/external_interface_testing.h"
#include <isense/os.h>


#define DEBUG_DUTY_CYCLING


typedef wiselib::OSMODEL Os;


namespace wiselib {

    class MidDutyCycling : public isense::SleepHandler {
    public:

        typedef Os::Clock Clock;

        typedef MidDutyCycling self_type;
        typedef self_type* self_pointer_t;

        /**
         * Constructor.
         */
        MidDutyCycling() {
        };

        /**
         * Desctructor.
         */
        ~MidDutyCycling() {
        };

        /**
         * Initialize the dutyCycling app.
         * @param timer the timer instance
         * @param duty the dutyCycling instance
         */
        void init(Os::Timer& timer, Os::DutyCycling& duty) {
            timer_ = &timer;
            duty_ = &duty;

            sleep_period_ = 0;
            wake_up_period_ = 1000;
            duty_->sleep();
            timer_->set_timer<MidDutyCycling, &MidDutyCycling::change_sleep > (100, this, (void *) 1);
        }

        //        /**
        //         * Starts Cycling.
        //         */
        //        void enable() {
        //            timer_->set_timer<MidDutyCycling, &MidDutyCycling::change_sleep > (10000, this, (void *) 1);
        //        }

        //        /**
        //         * Stops Cycling.
        //         */
        //        void disable() {
        //            sleep_period_ = 0;
        //            wake_up_period_ = 0;
        //        }

        /**
         * Set the rate of duty cycling.
         * @param period period of a cycle in millis.
         * @param rate the rate of operation inside a cycle in percentage (0 no operation, 100 always on).
         */
        void set_rate(uint16_t period, uint8_t rate) {
            wake_up_period_ = (period * rate) / 100;
            sleep_period_ = period - wake_up_period_;
        }

        /**
         * Called before going to sleep.
         * @return true if the node can go sleep.
         */
        bool stand_by() {
            return true;
        }

        /**
         * Called when waking up.
         * @param state previous state.
         */
        void wake_up(bool state) {
        }

        /**
         * Called to change the state of the node.
         * @param action 0 sends the node to sleep, 1 wakes the node up.
         */
        void change_sleep(void * action) {
            if ((action == (void*) 0)) {
                duty_->sleep();
            } else {
                duty_->wake();
                timer_->set_timer<MidDutyCycling, &MidDutyCycling::change_sleep > (wake_up_period_ + sleep_period_, this, (void *) 1);
                timer_->set_timer<MidDutyCycling, &MidDutyCycling::change_sleep > (wake_up_period_, this, (void *) 0);
            }
        }

        // --------------------------------------------------------------------

    private:
        //the time in millis the node should remain alive
        uint16_t wake_up_period_;
        //the time the node is asleep
        uint16_t sleep_period_;

        Os::Timer::self_pointer_t timer_;
        Os::DutyCycling::self_pointer_t duty_;
    };
}




#endif	/* iSenseDutyCycling_H */


