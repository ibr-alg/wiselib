
const char* rdf[][3] = {
	#include "incontextsensing_short.cpp"
	{ 0, 0, 0 }
};

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80

#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)

/*
 * MIN(?v) MEAN(?v) MAX(?v) {
 *    ?sens <http://purl.oclc.org/NET/ssnx/ssn#observedProperty> <http://spitfire-project.eu/property/Temperature> .
 *    ?sens <http://www.loa-cnr.it/ontologies/DUL.owl#hasValue> ?v .
 * }
 * 
<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>          4d0f60b4
<http://spitfire-project.eu/property/Temperature>            b23860b3
<http://purl.oclc.org/NET/ssnx/ssn#observedProperty>         bf26b82e
 */
enum { Q = Communicator::MESSAGE_ID_QUERY, OP = Communicator::MESSAGE_ID_OPERATOR };
enum { OP_ROOT = 0 };
enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5 };
enum { QID = 1 };
block_data_t op100[] = { OP, QID, 100, 'a', OP_ROOT, BIN(010101), BIN(0), BIN(0), BIN(0), 3, MIN | AGAIN, AVG | AGAIN, MAX };
block_data_t op90[]  = { OP, QID,  90, 'j', LEFT | 100, BIN(010000), BIN(0), BIN(0), BIN(0), LEFT_COL(0) | RIGHT_COL(0) };
block_data_t op80[]  = { OP, QID,  80, 'g', RIGHT | 90, BIN(010011), BIN(0), BIN(0), BIN(0), BIN(010), 0x4d, 0x0f, 0x60, 0xb4 };
block_data_t op70[]  = { OP, QID,  70, 'g', LEFT | 90, BIN(11), BIN(0), BIN(0), BIN(0), BIN(110), 0xbf, 0x26, 0xb8, 0x2e, 0xb2, 0x38, 0x60, 0xb3 };
block_data_t cmd[]   = { Q, QID, 4 };



