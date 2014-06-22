#ifndef __RAMFLASHMEMORY_H__
#define __RAMFLASHMEMORY_H__

#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;

namespace wiselib

{

/* Modelling a Flash Device with 4 sectors and 512 pages each
	The memory is organized as: 
	131,072 bytes (8 bits each)
	4 sectors (256 Kbits, 32768 bytes each),
	Sector 0:  00000h 07FFFh,
	Sector 1:  08000h 0FFFFh,
	Sector 2:  10000h 17FFFh,
	Sector 3: 18000h 1FFFFh 
	512 pages (256 bytes each)
	*/
	
template < 
typename OsModel_P, 
int SECTOR_P = 4,
int PAGE_SIZE_P = 256,
int PAGES_PER_SECTOR_P=512
>

class RAMFlashMemory
{

#ifdef ISENSE	
	enum {
		PAGE_SIZE = 8, // PAGE_SIZE is the smallest amount of data which is read or written at a time.

	};

	enum {
		PAGE_NO = 16, // PAGE_NO is the number of pages in a sector.

	};
			
	enum { 
		SECTOR_SIZE = 128 // SECTOR_SIZE is the smallest amount of data which is erased at a time.
	};

	enum { 
		SECTOR_NO = 2// SECTOR_NO is the number of sectors in a memory
	};

	enum { 
		MEMORY_SIZE = 256 // SECTOR_NO is the number of sectors in a memory
	};
#else	
	enum {
		PAGE_SIZE = PAGE_SIZE_P, // PAGE_SIZE is the smallest amount of data which is read or written at a time.

	};

	enum {
		PAGE_NO = PAGES_PER_SECTOR_P, // PAGE_NO is the number of pages in a sector.

	};
			
	enum { 
		SECTOR_SIZE = PAGE_SIZE_P*PAGES_PER_SECTOR_P // SECTOR_SIZE is the smallest amount of data which is erased at a time.
	};

	enum { 
		SECTOR_NO = SECTOR_P// SECTOR_NO is the number of sectors in a memory
	};

	enum { 
		MEMORY_SIZE = SECTOR_NO * SECTOR_SIZE // SECTOR_NO is the number of sectors in a memory
	};

#endif

	enum {
		SUCCESS = OsModel_P::SUCCESS, 
		ERR_UNSPEC = OsModel_P::ERR_UNSPEC,
	};

	public:

	//Initialise the flash memory.
	int init()
	{
		int i;

			erase();	

			i=0;
			//fstream fread("data.txt",ios::in);
			//while(fread!=NULL)
			//{
			//	fread>>data[i];
			//	i++;
			//}
			//fread.close();		
		
		return SUCCESS;
	}

	// Read 1 page
	int read(block_data_t* buffer, int sector, int page)
	{
		int i;
		for(i=0;i<PAGE_SIZE;i++)
			buffer[i]=data[sector*SECTOR_SIZE+page*PAGE_SIZE+i];
		return SUCCESS;
	}

	// write 1 page
	int write(block_data_t* buffer, int sector, int page)
	{
		int i;
		if(erased[(sector*PAGE_NO + page)] != 0)
			return ERR_UNSPEC;
		else
		{
		for(i=0;i<PAGE_SIZE;i++)
			data[sector*SECTOR_SIZE+page*PAGE_SIZE+i]=buffer[i];
		erased[(sector*PAGE_NO + page)]=1;
		}		
		return SUCCESS;
	}
	
	// erase 1 sector	
	int erase(int sector)
	{
		int i;
		for(i=0;i<SECTOR_SIZE;i++)
			data[sector*SECTOR_SIZE+i]=0xff;
		for(i=0;i<PAGE_NO;i++)
			erased[sector*PAGE_NO+i]=0;
		return SUCCESS;
	}

	// erase full chip	
	int erase()
	{
		int i;
		for(i=0;i<MEMORY_SIZE;i++)
			data[i]=0xff;
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
			erased[i]=0;
		return SUCCESS;
	}



	private:
	block_data_t data[MEMORY_SIZE];				// store the flash memory
	int erased[SECTOR_NO*PAGE_NO] ;			
	
};

}
#endif




