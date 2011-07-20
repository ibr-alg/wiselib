#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <external_interface/external_interface_testing.h>

#define ASSERT(_X) if (!(_X)) {std::cout<<"*** ASSERTION ("<<#_X<<") FAILED AT "<<__FILE__<<":"<<__LINE__<<" ***"<<std::endl; std::cin.get();}

typedef wiselib::OSMODEL Os;
typedef Os::Timer Timer;

#ifdef SHAWN
typedef Os::TxRadio Radio;
#else
typedef Os::ExtendedRadio Radio;
#endif

enum error_code_t
{
	ecSuccess,
	ecBufferTooShort
};

typedef Radio::node_id_t nodeid_t;

#define ARRSIZE(_a) (sizeof(_a)/sizeof((_a)[0]))

//This struct's size must be a multiple of 4 in order for serialization to work.
struct topology_record_t
{
	nodeid_t nodeid;
	uint8_t	distance;
	bool is_leader;
	uint8_t will_be_leader;
	nodeid_t leader;
	nodeid_t parent;
};

#endif /* UTILS_H_ */
