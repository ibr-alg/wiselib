
/*
 * AVG(?v) {
 *    ?sens <http://spitfire-project.eu/ontology/ns/obs> <http://140.203.155.101:8182/ld4s/resource/property/temperature> .
 *    ?sens <http://spitfire-project.eu/ontology/ns/out> ?ov .
 *    ?ov <http://spitfire-project.eu/ontology/ns/value> ?v .
 * }
 *
 
<http://example.org/sensor_042_1>
1581490174 000000005e439ffe 94 67 159 254

<http://spitfire-project.eu/ontology/ns/obs>
1171510393 0000000045d3d479 69 211 212 121 

"20.4"
103903868 000000000631727c 6 49 114 124 

_:b1
1626935914 0000000060f9126a 96 249 18 106 

 */

//#define LEFT 0
//#define RIGHT 0x80
//#define AGAIN 0x80
//#define LEFT_COL(X) ((X) << 4)
//#define RIGHT_COL(X) ((X) & 0x0f)
//#define COL(L, R) (LEFT_COL(L) | RIGHT_COL(R))

enum {
   //QID = 1,
   Q = Communicator::MESSAGE_ID_QUERY,
   OP = Communicator::MESSAGE_ID_OPERATOR,
   OP_ROOT = 0,
};
enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5 };

//struct OpInfo {
	//int len;
	//block_data_t *op;
//};


// obs, temperature
block_data_t op1[] = { OP, QID, 1, 'g', LEFT | 10, BIN(11), 0, 0, 0, BIN(110), 0x45, 0xd3, 0xd4, 0x79, 0x22, 0xfa, 0x4f, 0xb4 };
// out
block_data_t op2[] = { OP, QID, 2, 'g', RIGHT | 10, BIN(110011), 0, 0, 0, BIN(010), 0x4f, 0x2f, 0xfb, 0x4b};
// value
block_data_t op3[] = { OP, QID, 3, 'g', RIGHT | 20, BIN(100011), 0, 0, 0, BIN(010), 0x59, 0xd4, 0x2a, 0x48};
block_data_t op10[] = { OP, QID, 10, 'j', LEFT | 20, BIN(110000), 0, 0, 0, COL(0, 0) };
block_data_t op20[] = { OP, QID, 20, 'j', LEFT | 30, BIN(100000), 0, 0, 0, COL(0, 0) };
block_data_t op30[] = { OP, QID, 30, 'a', OP_ROOT, BIN(10), 0, 0, 0, 1, AVG };
block_data_t cmd[] = { Q, QID, 6 };

OpInfo g_query[] = {
	{ sizeof(op1), op1 }, { sizeof(op2), op2 },
	{ sizeof(op3), op3 }, { sizeof(op10), op10 },
	{ sizeof(op20), op20 }, { sizeof(op30), op30 },
	{ sizeof(cmd), cmd }, { 0, 0 }
};


