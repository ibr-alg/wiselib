#ifndef __VIRTSDCARD_H__
#define  __VIRTSDCARD_H__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pc_os_model.h"

namespace wiselib {

/*
 * @brief A virtual bock memory for the PC os model. It implements the block memory interface.
 * @author Dominik Krupke
 * @author Maximilian Ernestus
 * @author Marco Nikander
 * 
 * This is an implementation of a virtual block device for testing of external memory algorithms and 
 * applications. The data is stored in a array in ram; though inefficient this is acceptable for test purposes. 
 * The stats (in milliseconds) are calculated using test-data collected during testing of various SD-Cards on 
 * Arduino ÂµCs over SPI.
 * 
 * @tparam OsModel_P specifies the os model to use
 * @tparam nrOfBlocks specifies the number of blocks we want to use (default 1000)
 * @tparam blocksize the size of the blocks you want to use (deault 512)
 */

//TODO: use memcopy in read, write erase instead of for loops to clean up code a bit
/*TODO: Redo IO & IO-time calculations. Make an enum for the seperate times and values, and use the enum 
to calc the stats*/

template<typename OsModel_P, int nrOfBlocks = 1000, int blocksize = 512>
class VirtualSD {

public:

	typedef OsModel_P OsModel;
	typedef VirtualSD self_type;
	typedef self_type* self_pointer_t;
	typedef typename OsModel::block_data_t block_data_t;
	typedef typename OsModel::size_t address_t;
	
	enum {
		BLOCK_SIZE = blocksize, ///< size of block in byte (usable payload)
	};
			
	enum { 
		NO_ADDRESS = (address_t)(-1), ///< address_t value denoting an invalid address
	};
	
	enum {
		SUCCESS = OsModel::SUCCESS, 
		ERR_IO = OsModel::ERR_IO,
		ERR_NOMEM = OsModel::ERR_NOMEM,
		ERR_UNSPEC = OsModel::ERR_UNSPEC,
	};
	
	//BLOCK MEMORY CONCEPT
	
	int init(){
		static bool initialized = false;
		
		if(initialized == false) {
			memset(isWritten, false, nrOfBlocks);
			memset(memory, 0, sizeof(memory[0][0]) * nrOfBlocks * blocksize);
			initialized = true;
		}

		resetStats();
		return SUCCESS;
	}
	
	int init(typename OsModel::AppMainParameter& value){ 
	
		return init();
	}

	int read(block_data_t* buffer, address_t addr, address_t blocks = 1) {
		++ios_;
		duration_ += 2;

		for (address_t i = 0; i < blocks; i++) {
			//read(buffer + blocksize * i, addr + i);
			
			if (addr + i < 0 || addr + i >= nrOfBlocks || addr + i == NO_ADDRESS) {
				printf("OVERFLOW VIRTUAL SD.  Attempted to access block %ju\n", (uintmax_t) (addr + i));
				return ERR_UNSPEC;
			}
			++ios_;
			++blocksRead_;
			duration_ += 4;
			//else if(!isWritten[block]) std::cerr << "READING EMPTY BLOCK" << std::endl;

			for (int j = 0; j < blocksize; j++)
				(buffer + blocksize * i)[j] = memory[addr+i][j];
			//return SUCCESS;
			
			duration_ -= 2;
			--ios_;
		}
		return SUCCESS;
	}

	int write(block_data_t* buffer, address_t addr, address_t blocks = 1) { 
		++ios_;
		duration_ += 4;
		
		for (address_t i = 0; i < blocks; i++) {
			//write(buffer + blocksize * i, addr + i);
					
			++ios_;
			++blocksWritten_;
			duration_ += 8;
			
			if (addr + i < 0 || addr + i >= nrOfBlocks || addr + i == NO_ADDRESS) {
				printf("OVERFLOW VIRTUAL SD. Attempted to access block %ju\n", (uintmax_t) (addr + i));
				return ERR_UNSPEC;
			}
			for (int j = 0; j < blocksize; j++)
			{
				memory[addr + i][j] = (buffer + blocksize * i)[j];
				isWritten[addr + i] = true;
			}
			//return SUCCESS;
			
			
			
			duration_ -= 6;
			--ios_;
		}
		return SUCCESS;
	}

	int erase(address_t addr, address_t blocks = 1) {
		for(address_t i = 0; i < blocks; i++)
		{			
			if ( addr + i >= nrOfBlocks ) {
				printf("OVERFLOW VIRTUAL SD. Attempted to access block %ju\n", (uintmax_t) (addr + i));
				return ERR_UNSPEC;
			}
			
			for(address_t j = 0; j < blocksize; j++)
				memory[i + addr][j] = 0;
		}

		return SUCCESS;
	}
	
	address_t size() {
		return nrOfBlocks;
	}

	





	//EXTRA FUNCTIONALITY

	/*
	 * Resets all the counters that keep track of the IO's
	 */
	void resetStats() {
		blocksWritten_ = 0;
		blocksRead_ = 0;
		ios_ = 0;
		duration_ = 0;
	}

	/*
	 * Prints usage statistics about the virtual sd card to the console.
	 */
	void printStats() {
		printf("Blocks Written: %d \n", blocksWritten_);
		printf("Blocks Read: %d\n", blocksRead_ );
		printf("IOs: %d\n", ios_);
		printf("Duration: %d\n", duration_);
		printf("AvgIO: %d\n", duration_ / ios_);
	}

	/*
	 * Prints out a graphviz graph displaying every single byte.
	 * Non-null bytes are colored red so it is easier to see what parts are used.
	 * @param fromBlock The number of the block where to start displaying.
	 * @param toBlock The number of the block where to stop displaying.
	 */
	void printGraphBytes(int fromBlock, int toBlock, FILE* f)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		fprintf(f, "digraph BM { \n\t node [shape=none, margin=0]; \n");
		fprintf(f, "\t sdcard [label=<");
		printHTMLTableBytes(fromBlock, toBlock, f);
		fprintf(f, ">];\n");
		fprintf(f, "}\n");
	}

	/*
	 * Prints out a graphiz graph displaying the specified blocks.
	 * Used blocks are marked red.
	 * @param fromBlock The number of the block where to start displaying.
	 * @param toBlock The number of the block where to stop displaying.
	 */
	void printGraphBlocks(int fromBlock, int toBlock, FILE* f)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;

		fprintf(f, "digraph BM {\n\t node [shape=none, margin=0];\n");
		fprintf(f, "\t sdcard [label=<");
		printHTMLTableBlocks(fromBlock, toBlock, f);
		fprintf(f, ">]\n");
		fprintf(f, "}\n");
	}

	/*
	 * Helper for printGraphBytes
	 */
	void printHTMLTableBytes(int fromBlock, int toBlock, FILE* f)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		fprintf(f, "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">");
		for(int block = fromBlock; block <= toBlock; block++)
		{
			fprintf(f, "\n\t\t<tr>\n");
			for(int j = 0; j < blocksize; j++)
			{
				fprintf(f, "\t\t\t<td bgcolor=\"%s\">%3d</td>\n", ((int)memory[block][j] == 0 ? "#FFFFFF" : "#FF0000"), (int)memory[block][j]);
				//std::cout << "\t\t\t<td bgcolor=\"" << ((int)memory[block][j] == 0 ? "#FFFFFF" : "#FF0000") << "\">" << " " << "</td>\n";
			}
			fprintf(f, "\t\t</tr>\n");
		}
		fprintf(f, "</table>");
	}

	/*
	 * Helper for printGraphBlocks
	 */
	void printHTMLTableBlocks(int fromBlock, int toBlock, FILE* f)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		fprintf(f, "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">");
		fprintf(f, "\n\t\t<tr>\n");
		for(int block = fromBlock; block <= toBlock; block++)
		{
			fprintf(f, "\t\t\t<td bgcolor=\"%s\">%d</td>\n", (!isWritten[block] ? "#FFFFFF" : "#FF0000"), isWritten[block]);
			//std::cout << "\t\t\t<td bgcolor=\"" << ((int)memory[block][j] == 0 ? "#FFFFFF" : "#FF0000") << "\">" << " " << "</td>\n";

		}
		fprintf(f, "\t\t</tr>\n");
		fprintf(f, "</table>");
	}

	void printASCIIOutputBytes(int fromBlock, int toBlock)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		for(int block = fromBlock; block <= toBlock; block++)
		{
			for(int j = 0; j < 20; j++)
			{
				printf("%3d|", ((int)memory[block][j]));
			}
			printf("\n");
		}
	}

	void printASCIIOutputBlocks(int fromBlock, int toBlock)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		for(int block = fromBlock; block <= toBlock; block++)
		{
			printf("|%s", isWritten[block] ? "1" : " ");
		}
		printf("|\n");
	}

	void printGNUPLOTOutputBytes(address_t fromBlock, address_t toBlock, FILE* f)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;
		for(address_t block = fromBlock; block < toBlock; ++block)
			{
				for(int j = 0; j < blocksize; j++)
				{
//					fprintf(f, "%s ", ((int)memory[block][j] == 0 ? "1" : "0"));
					fprintf(f, "%d ", ((int)memory[block][j]));
				}
				fprintf(f, "\n");
			}
	}

	typedef int (*colorfunc)(address_t, int, int);

	void printGNUPLOTOutputBytes(address_t fromBlock, address_t toBlock, FILE* f, colorfunc colorize)
	{
		if(fromBlock < 0 || toBlock > nrOfBlocks) return;

		for(address_t block = fromBlock; block < toBlock; ++block)
			{
				for(int j = 0; j < blocksize; j++)
				{
//					fprintf(f, "%s ", ((int)memory[block][j] == 0 ? "1" : "0"));
					fprintf(f, "%d ", colorize(block, j, (int)memory[block][j]));
				}
				fprintf(f, "\n");
			}
	}

	void reset()
	{
		for (address_t i = 0; i < nrOfBlocks; i++)
			isWritten[i] = false;
		resetStats();

		erase(0, nrOfBlocks);
	}

	void dumpToFile(FILE* f)
	{
		for(int i = 0; i < nrOfBlocks; ++i)
		{
			fwrite(memory[i], blocksize, 1, f);
		}
	}

private:
	block_data_t memory[nrOfBlocks][blocksize];
	bool isWritten[nrOfBlocks];
	int blocksWritten_;
	int blocksRead_;
	int ios_;
	int duration_;

};
} //NAMESPACE
#endif // __VIRTSDCARD_H__
