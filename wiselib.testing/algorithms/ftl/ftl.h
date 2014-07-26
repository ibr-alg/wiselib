/*
 * File:   ftl.h
 * Author: Sachin
 *
 */

#ifndef __FTL_H__
#define __FTL_H__


#include <external_interface/external_interface.h>
#include <algorithms/block_memory/ram_flash_memory.h>

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;


namespace wiselib
{
	
template < 
typename OsModel_P, 
typename FlashMemory_P
>

class Flash
{

	// Page and Sector States
	enum
	{
		FREE = 0,
		PAGE = 1,
		HEADER = 2,
		DEAD = 3,
		UNKNOWN = 4
	};

	// FTL PAGE Tags
	typedef struct
	{
		int lookup;
		int version;			
		int state;						
	} FTLPageTag;

	// FTL SECTOR Tags
	typedef struct
	{
		int state;						
	} FTLSectorTag;

	enum {
		PAGE_SIZE = FlashMemory_P::PAGE_SIZE, // PAGE_SIZE is the smallest amount of data which is read or written at a time.
		PAGE_NO = FlashMemory_P::PAGES_PER_SECTOR, // PAGE_NO is the number of pages in a sector.
		SECTOR_SIZE = FlashMemory_P::SECTOR_SIZE, // SECTOR_SIZE is the smallest amount of data which is erased at a time.
		SECTOR_NO = FlashMemory_P::NO_SECTOR, // SECTOR_NO is the number of sectors in a memory
		MEMORY_SIZE = FlashMemory_P::MEMORY_SIZE // MEMORY_SIZE is the size of the memory
	};


	enum {
		SUCCESS = OsModel_P::SUCCESS, 
		ERR_UNSPEC = OsModel_P::ERR_UNSPEC,
	};

	public:


	/* 
	 * Setup / initialization needed to get the block memory device initialized. This 
	 * method can also be used to reset the state of the block memory object to its original
	 * state, but without erasing any of the data, whether on the block memroy device or
	 * in volatile memory.
	 * @return SUCCESS iff the block memory device was initialized successfully
	 */
	int init()
	{
		int i;
		FlashMemory_.init();
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
		{
			ftlpagetag_[i].state=UNKNOWN;
			wipe();
		}
		headerpointer=0;
		datapointer=PAGE_NO - 1;
		sectorpointer=0;
		header[0]=0x55;
		header[1]=0xaa;
		header[PAGE_SIZE-2]=0x55;
		header[PAGE_SIZE-1]=0xaa;
		count=2;
		return SUCCESS;
	}
			
	/**
	* Init method allowing initialization of block memory device using the FacetProvider
	* @return SUCCESS iff the block memeory device was initialized successfully
	*/
	int init(typename Os::AppMainParameter& value)
	{
		init();		
		return SUCCESS;
	}

	/**
	 * Reads data from multiple sucessive blocks at once and stores it into a buffer.
	 * @param buffer The buffer to store the data into. It is assumed to be of lenth length*blocksize
	 * @param addr The number of the block where to start reading from.
	 * @param blocks the number of blocks to read. (blocks==0 allowed)
	 * @return SUCCESS iff the block was read successfully
	 */
	int read(block_data_t* buffer, address_t addr, address_t blocks = 1)
	{	
		address_t i=0;
		for(i=0;i<blocks*2;i++)
			if(read_(buffer+i*PAGE_SIZE,(2*addr+i)*PAGE_SIZE)!=SUCCESS)
				return ERR_UNSPEC;

		return SUCCESS;
	}

	int read_(block_data_t* buffer, address_t addr)
	{

		if(addr%PAGE_SIZE!=0)
			return ERR_UNSPEC;

		if(ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].state == PAGE)
			FlashMemory_.read(ftlpagetag_[addr/PAGE_SIZE].lookup*PAGE_SIZE, PAGE_SIZE, buffer);
		else if(ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].state == FREE)
			for(int i=0;i<PAGE_SIZE;i++)
				buffer[i]=0xff;
		else
			return ERR_UNSPEC;
		//debug_->debug("1: %d %d\n",addr/PAGE_SIZE,ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].state);

		return SUCCESS;
	}

	/**
	 * Writes data from a big array into multiple successive blocks.
	 * @param buffer The array of data to be written. The array is assumed to be of size blocks*blocksize.
	 * @param addr The number of the block where to start writing the data.
	 * @param blocks The number of blocks to write into. (blocks==0 allowed)
	 * @return SUCCESS iff the block was written successfully
	 */
	int write(block_data_t* buffer, address_t addr, address_t blocks = 1)
	{
		address_t i=0;
		for(i=0;i<blocks*2;i++)
			if(write_(buffer+i*PAGE_SIZE,(2*addr+i)*PAGE_SIZE)!=SUCCESS)
				return ERR_UNSPEC;

		return SUCCESS;
	}

			

	int write_(block_data_t* buffer, address_t addr)
	{	
		int i;
		bool write_success = false;
		if(addr%PAGE_SIZE!=0)
			return ERR_UNSPEC;
		for(i=0;i<5&&write_success==false;i++)
		{
			if(ftlpagetag_[sectorpointer*PAGE_NO+datapointer].state==FREE)
			{
				write_success = FlashMemory_.write((sectorpointer*PAGE_NO+datapointer)*PAGE_SIZE,PAGE_SIZE,buffer);
				if(write_success == true)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+datapointer].state=PAGE;	
					ftlpagetag_[addr/PAGE_SIZE].lookup=sectorpointer*PAGE_NO+datapointer;
	
					header[count]=PAGE;
					count++;
					header[count]=addr/PAGE_SIZE;
					count++;
					header[count]=sectorpointer*PAGE_NO+datapointer;
					count++;
					datapointer--;			
					header[count]=ftlpagetag_[addr/PAGE_SIZE].version;
					count++;				
					ftlpagetag_[addr/PAGE_SIZE].version=(ftlpagetag_[addr/PAGE_SIZE].version+1)%4;
				}
				else							
				{	
					ftlpagetag_[sectorpointer*PAGE_NO+datapointer].state=DEAD;	

					header[count]=DEAD;
					count++;
					header[count]=DEAD;
					count++;
					header[count]=sectorpointer*PAGE_NO+datapointer;
					count++;
					datapointer--;			
					header[count]=DEAD;
					count++;				
				}				

		
				if(datapointer<=headerpointer)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].state=HEADER;
					FlashMemory_.write(sectorpointer*SECTOR_SIZE+headerpointer*PAGE_SIZE,PAGE_SIZE,header);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
	

				if(count>=PAGE_SIZE-5)
				{			
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].state=HEADER;								
					FlashMemory_.write(sectorpointer*SECTOR_SIZE+headerpointer*PAGE_SIZE,PAGE_SIZE,header);
					headerpointer++;
					count=2;
				}
				
			}
		//debug_->debug("1:%d %d %d\n",i,ftlpagetag_[addr/PAGE_SIZE].lookup,ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].state);
		}
	
		if(i==5 && write_success ==false)
			return ERR_UNSPEC;
		//debug_->debug("2: %d %d %d\n",addr/PAGE_SIZE,sectorpointer*PAGE_NO+datapointer,lookup[addr/PAGE_SIZE]);
		//print();
		return SUCCESS;
	}


	/**
	 * @param addr The number of the first block to be erased.
	 * @param blocks The number of blocks to erase. (blocks==0 allowed)
	 * @return SUCCESS iff the block was erased successfully
	 */
	//int erase(address_t addr, address_t blocks = 1)
	int erase(int sector)	
	{
		int i;
		for(i=0;i<SECTOR_NO;i++)
		{
			ftlpagetag_[sector*SECTOR_SIZE+i].state=FREE;
			ftlpagetag_[sector*SECTOR_SIZE+i].version=0;
		}	
		if(FlashMemory_.erase()==true)
			return SUCCESS;
		else
			return ERR_UNSPEC;
	}
			
	/**
	 * Erase the whole disk,
	 * @return SUCCESS iff the memory was wiped successfully
	 */
	int wipe() 
	{ 
		int i;
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
		{
			ftlpagetag_[i].state = FREE;		
		}	
		if(FlashMemory_.erase()==true)
			return SUCCESS;
		else
			return ERR_UNSPEC;
	}

	/*void print()
	{
		int i;
		for(i=0;i<PAGE_NO;i++)
			debug_->debug("%d %d\n",i,lookup[i]);
	}*/

	/**
	 *@return size of the block memory device in blocks
	 */
	address_t size()
	{
		return FlashMemory_.page_size();
	}



	int uninit()
	{
		FlashMemory_.write(sectorpointer*SECTOR_SIZE+headerpointer*PAGE_SIZE,PAGE_SIZE,header);
		return SUCCESS;
	}


	private:
	FlashMemory_P FlashMemory_;
	FTLPageTag ftlpagetag_[SECTOR_NO*PAGE_NO];
	FTLSectorTag ftlsectortag_[SECTOR_NO];
	block_data_t header[PAGE_SIZE];
	int count;					// count the size of header
	int headerpointer;				// Points to the headers
	int datapointer;				// Points to the data
	int sectorpointer;
	Os::Debug::self_pointer_t debug_;
};

}

#endif

