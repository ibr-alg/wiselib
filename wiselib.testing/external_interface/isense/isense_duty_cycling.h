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
    class iSenseDutyCycling {
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
            os_.allow_sleep(false);
            os_.allow_doze(false);
        };

        ~iSenseDutyCycling() {
        };

        void sleep() {
#ifdef DEBUG_DUTY_CYCLING
            os_.debug("duty:sleep");
#endif
            os_.allow_sleep(true);
            os_.allow_doze(true);
        }

        void wake() {
#ifdef DEBUG_DUTY_CYCLING
            os_.debug("duty:wake");
#endif
            os_.allow_sleep(false);
            os_.allow_doze(false);
        }

    private:

        isense::Os& os_;

        // --------------------------------------------------------------------
    };


}




#endif	/* iSenseDutyCycling_H */


