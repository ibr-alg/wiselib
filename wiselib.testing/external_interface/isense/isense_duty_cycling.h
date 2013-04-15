/* 
 * File:   dutycycling.h
 * Author: dante
 *
 * Created on August 11, 2012, 6:18 PM
 */

#ifndef ISENSEDUTYCYCLING_H
#define	ISENSEDUTYCYCLING_H

#include "external_interface/isense/isense_types.h"
#include <isense/os.h>
#include <isense/timeout_handler.h>
#include <isense/sleep_handler.h>
#include <isense/time.h>


namespace wiselib {

    template <typename OsModel_P>
    class iSenseDutyCycling : public isense::SleepHandler {
    public:

        typedef OsModel_P OsModel;
        typedef iSenseDutyCycling<OsModel> self_type;
        typedef self_type* self_pointer_t;

        enum ErrorCodes {
            SUCCESS = OsModel::SUCCESS,
            ERR_UNSPEC = OsModel::ERR_UNSPEC
        };

        iSenseDutyCycling(isense::Os& os)
        : os_(os) {
            os_.allow_doze(false);
            os_.add_sleep_handler(this);
        };

        ~iSenseDutyCycling() {
        };

        /**
         * Called before going to sleep.
         * @return true if the node can go sleep.
         */
        bool stand_by() {
#ifdef DEBUG_DUTY_CYCLING
            os_.debug("duty:sleep");
#endif
            return true;
        }

        /**
         * Called when waking up.
         * @param state previous state.
         */
        void wake_up(bool state) {
#ifdef DEBUG_DUTY_CYCLING
            os_.debug("duty:wake");
#endif                     
        }

        void sleep() {
            os_.allow_sleep(true);
        }

        void wake() {
            os_.allow_sleep(false);
        }

    private:

        isense::Os& os_;

        // --------------------------------------------------------------------
    };


}




#endif	/* iSenseDutyCycling_H */


