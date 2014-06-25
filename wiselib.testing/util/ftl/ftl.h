#ifndef __FTL_H__
#define __FTL_H__


#include "ramflashmemory.h"
#include <external_interface/external_interface.h>

typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;


namespace wiselib
{
	
template < 
typename OsModel_P, 
int SECTOR_P = 4,
int PAGE_SIZE_P = 256,
int PAGES_PER_SECTOR_P=512
>
class Flash
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
		VFlash_.init();
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
		{
			LookupTable[i]=0;
			ValidPage[i]=0;
			FreePage[i]=1;		
		}
		headerpointer=0;
		datapointer=PAGE_NO - 1;
		sectorpointer=0;
		header[0]=0x55;
		header[1]=0xaa;
		header[PAGE_SIZE-2]=0x55;
		header[PAGE_SIZE-1]=0xaa;
		count=2;
		//displayerased();
		return SUCCESS;
	}


	int read(block_data_t* buffer, int logical_sector, int logical_page)
	{
		if(ValidPage[LookupTable[logical_sector*PAGE_NO+logical_page]]==1)
			VFlash_.read(buffer, LookupTable[logical_sector*PAGE_NO+logical_page]/PAGE_NO, LookupTable[logical_sector*PAGE_NO+logical_page]%PAGE_NO);
		else
			return ERR_UNSPEC;
		//displayerased();
		return SUCCESS;
	}


	int write(block_data_t* buffer, int logical_sector, int logical_page)
	{

			if(FreePage[sectorpointer*PAGE_NO+datapointer]==1)
			{
				VFlash_.write(buffer,sectorpointer,datapointer);
				FreePage[sectorpointer*PAGE_NO+datapointer]=0;	
				ValidPage[sectorpointer*PAGE_NO+datapointer]=1;
				LookupTable[logical_sector*PAGE_NO+logical_page]=sectorpointer*PAGE_NO+datapointer;
				header[count]=logical_sector*PAGE_NO+logical_page;
				count++;
				header[count]=sectorpointer*PAGE_NO+datapointer;
				count++;
				datapointer--;

		
			if(datapointer<=headerpointer)
				{
					FreePage[sectorpointer*PAGE_NO+headerpointer]=0;
					VFlash_.write(header,sectorpointer,headerpointer);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
	

					if(count==PAGE_SIZE-2)
				{			
					FreePage[sectorpointer*PAGE_NO+headerpointer]=0;
								
					VFlash_.write(header,sectorpointer,headerpointer);
					headerpointer++;
					count=2;
				}
		
			}
		
		//displayerased();
		return SUCCESS;
	}
	
	int erase(int sector)
	{
		int i;
		VFlash_.erase(sector);
		for(i=0;i<SECTOR_NO;i++)
		{
			ValidPage[sector*SECTOR_SIZE+i]=0;
			FreePage[sector*SECTOR_SIZE+i]=1;
		}	

	//	displayerased();
		return SUCCESS;
	}


	int erase()
	{
		int i;
		VFlash_.erase();
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
		{
			ValidPage[i]=0;
			FreePage[i]=1;
		}	
		//displayerased();
		return SUCCESS;
	}

	/*void displayerased()
	{
		int i,j;
		VFlash_.displayerased();
		cout<<"Valid  : ";
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
			cout<<ValidPage[i]<<" ";
		cout<<endl;
		cout<<"Free   : ";
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
			cout<<FreePage[i]<<" ";
		cout<<endl;
		
		for(i=0;i<PAGE_NO*SECTOR_NO;i++)
		{
			cout<<i <<" "<<LookupTable[i]<<endl;
		}

	}*/

	int uninit()
	{
		VFlash_.write(header,sectorpointer,headerpointer);	
		return SUCCESS;
	}


	private:
	wiselib::RAMFlashMemory <Os> VFlash_;
	int LookupTable[SECTOR_NO*PAGE_NO] ;			
	int ValidPage[SECTOR_NO*PAGE_NO] ;			
	int FreePage[SECTOR_NO*PAGE_NO] ;			
	block_data_t header[PAGE_SIZE];
	int count;					// count the size of header
	int headerpointer;				// Points to the headers
	int datapointer;				// Points to the data
	int sectorpointer;
	
};

}

#endif

