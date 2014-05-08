
#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80
#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)
#define COL(L, R) (LEFT_COL(L) | RIGHT_COL(R))

enum {
   QID = 1,
   Q = Communicator::MESSAGE_ID_QUERY,
   OP = Communicator::MESSAGE_ID_OPERATOR,
   OP_ROOT = 0,
};
enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5 };

struct OpInfo {
	int len;
	block_data_t *op;
};

// 1,1,'g',LEFT | 3,BIN(00000011),0,0,0,BIN(110),"<http://spitfire-project.eu/ontology/ns/obs>","<http://140.203.155.101:8182/ld4s/resource/property/light>",
// 1,2,'g',RIGHT | 3,BIN(00110011),0,0,0,BIN(010),"<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>",
// 1,3,'j',LEFT | 5,BIN(00110011),0,0,0,LEFT_COL(0) | RIGHT_COL(0),
// 1,4,'g',RIGHT | 5,BIN(00110011),0,0,0,BIN(010),"<http://spitfire-project.eu/ontology/ns/out>",
// 1,5,'j',LEFT | 7,BIN(11001111),0,0,0,LEFT_COL(0) | RIGHT_COL(0),
// 1,6,'g',RIGHT | 7,BIN(00100011),0,0,0,BIN(010),"<http://spitfire-project.eu/ontology/ns/value>",
// 1,7,'j',LEFT | 8,BIN(00001100),BIN(00000010),0,0,LEFT_COL(2) | RIGHT_COL(0),
// 1,8,'c',LEFT | 0,BIN(00001011),0,0,0,
block_data_t op0[] = {OP, QID, 1, 103, 3, 3, 0, 0, 0, 6, 69, -45, -44, 121, -45, 105, 88, 18};
block_data_t op1[] = {OP, QID, 2, 103, -125, 51, 0, 0, 0, 2, -56, -77, -12, 58};
block_data_t op2[] = {OP, QID, 3, 106, 5, 51, 0, 0, 0, 0};
block_data_t op3[] = {OP, QID, 4, 103, -123, 51, 0, 0, 0, 2, 79, 47, -5, 75};
block_data_t op4[] = {OP, QID, 5, 106, 7, -49, 0, 0, 0, 0};
block_data_t op5[] = {OP, QID, 6, 103, -121, 35, 0, 0, 0, 2, 89, -44, 42, 72};
block_data_t op6[] = {OP, QID, 7, 106, 8, 12, 2, 0, 0, 32};
block_data_t op7[] = {OP, QID, 8, 99, 0, 11, 0, 0, 0};
block_data_t q[] = { Q, QID, 8};

OpInfo g_query[] = {
	{ sizeof(op0), op0 },
	{ sizeof(op1), op1 },
	{ sizeof(op2), op2 },
	{ sizeof(op3), op3 },
	{ sizeof(op4), op4 },
	{ sizeof(op5), op5 },
	{ sizeof(op6), op6 },
	{ sizeof(op7), op7 },
	{ sizeof(q), q }, { 0, 0 } };
