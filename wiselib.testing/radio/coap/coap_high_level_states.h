/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef COAP_HIGH_LEVEL_STATES_H_
#define COAP_HIGH_LEVEL_STATES_H_

#include "util/pstl/vector_static.h"

namespace wiselib {

static const int COAP_MAX_HL_STATES = 10;
static const int COAP_MAX_STATE_RESOURCES = 10;

// TODO STRING type implementation for high level states
// TODO proper debugging

// --------------------------------------------------------------------------
namespace HighLevelCreationType {
	enum CreationType {
		INTEGER,
		FLOAT,
		STRING
	};
}
typedef HighLevelCreationType::CreationType creation_type_t;
// --------------------------------------------------------------------------
namespace HighLevelQueryType {
	enum QueryType {
		STATE,
		STATE_NUMBER,
		STATE_DESCRIPTION
	};
}
typedef HighLevelQueryType::QueryType query_type_t;
// --------------------------------------------------------------------------
template<typename T>
struct number_state
{
	T lower_bound;
	T upper_bound;
	const char* name;

	char* to_json();
	bool contains(T sensor_value)
	{
		return sensor_value >= lower_bound && sensor_value < upper_bound;
	}
};
// --------------------------------------------------------------------------
template<>
struct number_state<const char*>
{
	// TODO this should probably be a vector of strings
	const char* value;
	const char* name;

	bool contains(const char* sensor_value)
	{
		return strcmp(sensor_value, value);
	}

	char* to_json() {
		char* result = new char[256];
		char str[6] = "{str:";
		char s[5] = ",s:'";
		char c[3] = "'}";

		strcat(result, str);
		strcat(result, value);
		strcat(result, s);
		strcat(result, name);
		strcat(result, c);

		return result;
	}
};
// --------------------------------------------------------------------------
template<typename Os_Model, typename T>
struct number_state_resource
{
	creation_type_t type;
	const char* path;
	vector_static<Os_Model, number_state<T>, COAP_MAX_HL_STATES> states;

	char* to_json();
	const char* get_state(T sensor_value);
	int get_state_number(T sensor_value);
};
// --------------------------------------------------------------------------
template<typename Os_Model, typename T>
const char* number_state_resource<Os_Model, T >::get_state(T sensor_value)
{
	for (size_t i=0; i<states.size(); i++)
	{
		number_state<T> curr = states.at(i);
		if ( curr.contains(sensor_value) )
		{
			return curr.name;
		}
	}
	return "undefined";
}
template<typename Os_Model, typename T>
int number_state_resource<Os_Model, T >::get_state_number(T sensor_value)
{
	for (size_t i=0; i<states.size(); i++)
	{
		number_state<T> curr = states.at(i);
		if ( curr.contains(sensor_value) )
		{
			return i;
		}
	}
	return -1;
}
// --------------------------------------------------------------------------
template<typename Os_Model, typename T>
char* number_state_resource<Os_Model, T>::to_json()
{
	uint16_t size = 32 * states.size() + 23 + 64;
	char* result = new char[size];
	char p[5] = "{p:'";
	char n[9] = "', num:[";
	char c[3] = "]}";

	strcat(result, p);
	strcat(result, path);
	strcat(result, n);

	for (size_t i=0; i<states.size(); i++)
	{
		char* json = states.at(i).to_json();
		strcat(result, json);
	}
	strcat(result, c);

	return result;
}
// --------------------------------------------------------------------------
template<>
char* number_state<uint16_t>::to_json() {
	char* result = new char[32];
	char l[4] = "{l:";
	char h[4] = ",h:";
	char s[5] = ",s:'";
	char c[3] = "'}";

	char low_char[8];
	snprintf(low_char,8,"%d",lower_bound);

	char up_char[8];
	snprintf(up_char,8,"%d",upper_bound);

	strcat(result, l);
	strcat(result, low_char);
	strcat(result, h);
	strcat(result, up_char);
	strcat(result, s);
	strcat(result, name);
	strcat(result, c);

	return result;
}
// --------------------------------------------------------------------------
template<>
char* number_state<float>::to_json() {
	char* result = new char[32];
	char l[4] = "{l:";
	char h[4] = ",h:";
	char s[5] = ",s:'";
	char c[3] = "'}";

	char low_char[8];
	snprintf(low_char,8,"%f",lower_bound);

	char up_char[8];
	snprintf(up_char,8,"%f",upper_bound);

	strcat(result, l);
	strcat(result, low_char);
	strcat(result, h);
	strcat(result, up_char);
	strcat(result, s);
	strcat(result, name);
	strcat(result, c);

	return result;
}

}



#endif /* COAP_HIGH_LEVEL_STATES_H_ */
