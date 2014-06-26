#ifndef __RAMFLASHMEMORY_H__
#define __RAMFLASHMEMORY_H__

#include <external_interface/external_interface.h>


typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
typedef Os::size_t address_t;

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
int NO_SECTOR_P = 4,
int PAGE_SIZE_P = 256,
int PAGES_PER_SECTOR_P=512
>

class RAMFlashMemory
{

	enum {
		PAGE_SIZE = PAGE_SIZE_P, // PAGE_SIZE is the smallest amount of data which is read or written at a time.

	};

	enum {
		PAGES_PER_SECTOR = PAGES_PER_SECTOR_P, // PAGES_PER_SECTOR is the number of pages in a sector.

	};
			
	enum { 
		SECTOR_SIZE = PAGE_SIZE_P*PAGES_PER_SECTOR_P // SECTOR_SIZE is the smallest amount of data which is erased at a time.
	};

	enum { 
		NO_SECTOR = NO_SECTOR_P// SECTOR_NO is the number of sectors in a memory
	};

	enum { 
		MEMORY_SIZE = NO_SECTOR * SECTOR_SIZE // SECTOR_NO is the number of sectors in a memory
	};


	public:


	//----------------------------------------------------------------------------
	/** Erases the whole Flash memory. Returns after erase is completed.
	 * Sets all bits in flash to 1.
	 * @return true if flash was erased successfully, false otherwise
	 */
	bool erase()				
	{
		int i;
		for(i=0;i<MEMORY_SIZE;i++)
			memory_data[i]=0xff;
		for(i=0;i<NO_SECTOR*PAGES_PER_SECTOR;i++)
			erased[i]=true;
		return true;
	}		

	//----------------------------------------------------------------------------
	/** Erases one sector of the Flash memory. Returns after erase is completed. 
	 * Sets all bits in the referenced flash page to 1.
	 * 
	 * @return true if page was erased successfully, false otherwise
	 * 
	 * @param page The page to erase
	 */
	bool erase(address_t sector)
	{
		int i;
		for(i=0;i<SECTOR_SIZE;i++)
			memory_data[sector*SECTOR_SIZE+i]=0xff;
		for(i=0;i<PAGES_PER_SECTOR;i++)
			erased[sector*PAGES_PER_SECTOR+i]=true;
		return true;
	}
		
	//----------------------------------------------------------------------------
	/** Write data into the flash memory.  
	 * 
	 * @param start_address flash address to start writing at
	 * @param byte_count write length in bytes, must be <= 256
	 * @param data Pointer to the buffer that contains the data to be written to flash
	 * 
	 * @return true if write was successful and data was written, false otherwise
	 * 
	 * @note Data can only be written to area erased before, since data cannot be 'overwritten'
	 * @note Each write access to the flash must not affect multiple pages
	 * @note Because flash operations take long, the function should not be called from the interrupt context.
	 */
	bool write(address_t start_address, address_t byte_count, block_data_t* data)
	{
		int i;

		//Check alignment
		if(start_address%PAGE_SIZE!=0)
			return false;

		if(erased[start_address/PAGE_SIZE] != true)
			return false;
		else
		{
		for(i=0;i<byte_count;i++)
			memory_data[start_address+i]=data[i];
		erased[start_address/PAGE_SIZE]=false;
		}		
		return true;
	}
		
		
	//----------------------------------------------------------------------------
	/** Read flash.
	 *  
	 * @param start_address flash address to start reading from
	 * @param byte_count read length in bytes
	 * @param data Pointer to the buffer the data is written to
	 *
	 * @return true if read was successful and data contain valid data, false otherwise
	  *
	 */	
	bool read(address_t start_address, address_t byte_count, block_data_t* data)
	{
		int i;

		//Check alignment
		if(start_address%PAGE_SIZE!=0)
			return false;

		for(i=0;i<PAGE_SIZE;i++)
			data[i]=memory_data[start_address+i];
		return true;
	}
		
	//----------------------------------------------------------------------------
	/** Returns the size of the flash pages.
	 *  
	 * @return size of each page. For the Jennic flash, the page size is 08000h or 32 kbytes
	 *
	 */	
	address_t page_size() 
	{
		return PAGE_SIZE;
	}
		
	//----------------------------------------------------------------------------
	/**
	 */
	//Initialise the flash memory.
	void init()
	{
		int i;
		erase();	
		//i=0;
		//fstream fread("data.txt",ios::in);
		//while(fread!=NULL)
		//{
		//	fread>>data[i];
		//	i++;
		//}
		//fread.close();		
	}

	private:
	block_data_t memory_data[MEMORY_SIZE];				// store the flash memory
	bool erased[NO_SECTOR*PAGES_PER_SECTOR];			
	
};

}
#endif




