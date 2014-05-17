

#define LEFT 0
#define RIGHT 0x80
#define AGAIN 0x80
#define LEFT_COL(X) ((X) << 4)
#define RIGHT_COL(X) ((X) & 0x0f)
#define COL(L, R) (LEFT_COL(L) | RIGHT_COL(R))

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

block_data_t op1[] = { OP, QID, 1, 'g', LEFT | 10, BIN(111111), 0, 0, 0, BIN(000) };
block_data_t op10[] = { OP, QID, 10, 'c', OP_ROOT, BIN(111111), 0, 0, 0 };
block_data_t cmd[] = { Q, QID, 2 };

OpInfo g_query[] = {
	{ sizeof(op1), op1 }, { sizeof(op10), op10 },
	{ sizeof(cmd), cmd }, { 0, 0 }
};

