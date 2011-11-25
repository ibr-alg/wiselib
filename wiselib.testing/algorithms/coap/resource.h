/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef RESOURCE_H
#define	RESOURCE_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"
#include "util/pstl/pair.h"

namespace wiselib
{
template<typename String_P = StaticString>
class ResourceController
{
public:
    typedef String_P String;
    typedef delegate1<char *, uint8_t> my_delegate_t;
    template<class T, char* (T::*TMethod) (uint8_t)>
    void reg_callback(T *obj_pnt)
    {
        del_ = my_delegate_t::template from_method<T, TMethod>(obj_pnt);
    }
    char* value(uint8_t par)
    {
        if(del_)
        {
            return del_(par);
        }
    }
    void init()
    {
        is_set_ = false;
    }
    void reg_resource(String name, uint8_t sensor_index, uint8_t resource_len, uint8_t content_type)
    {
        name_ = name;
        is_set_ = true;
        sensor_index_ = sensor_index;
        resource_len_ = resource_len;
        content_type_ = content_type;
    }
    char* name()
    {
        return name_.c_str();
    }
    uint8_t name_length()
    {
        return name_.length();
    }
    uint8_t sensor_index()
    {
        return sensor_index_;
    }
    uint8_t resource_len()
    {
        return resource_len_;
    }
    bool is_set()
    {
        return is_set_;
    }
    uint8_t content_type()
    {
        return content_type_;
    }
private:
    my_delegate_t del_;
    String name_;
    uint8_t sensor_index_;
    uint8_t resource_len_;
    bool is_set_;
    uint8_t content_type_;
};
}
#endif
