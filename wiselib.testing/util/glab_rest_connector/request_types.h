/* 
 * File:   request_types.h
 * Author: maxpagel
 *
 * Created on 28. Dezember 2010, 10:30
 */

#ifndef _REQUEST_TYPES_H
#define	_REQUEST_TYPES_H
enum RequestTypes
{
    GET_SENSORS          = 0,
    GET_NODE_META_DATA   = 1,
    GET_SENSOR_VALUE     = 2,
    GET_SENSOR_META_DATA = 3,
    SUBSCRIBE            = 4,
    CANCEL_SUBSCRIPTION  = 5,
    POST_SENSOR_META_DATA = 6

};

#endif	/* _REQUEST_TYPES_H */

