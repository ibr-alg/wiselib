/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

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


class FTL
{

	
	// Page and Sector States
	enum
	{
		FREE = 0,		// Page is free
		PAGE = 1,		// Page stores a page
		HEADER = 2,		// Page stores a header
		DEAD = 3,		// Page is dead
		UNKNOWN = 4,		// status of Page is unknown
		TO_ERASE = 5		// Page has to be erased
	};

	// FTL PAGE Tags
	typedef struct
	{
		int lookup;		// Storage location of a particular block/page
		int logical_state;	// logical information of the page
		int physical_state;	// phyiscal information of the page						
	} FTLPageTag;

	public:

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

	// Required for status of garbage collector
	enum 	{
		ON = 1,			// Garbage collector is on
		OFF = 0			// Garbage collector is off
	};

	enum { 
		BLOCK_SIZE = 512 // BLOCK_SIZE is fixed to 512
	};

	enum { 
		NO_ADDRESS = 0 // NO_ADDRESS is assigned as 0
	};

	// Required for selecting the flash
	enum
	{
		NEWFLASH = 1,		// Select new flash
		OLDFLASH = 2		// Select existing flash
	};
	
	

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
		garbagecollector=OFF;				// Set garbage collection to OFF
		FlashMemory_.init();				// Initialse Flash memory and setup the flags
		for(i=0;i<SECTOR_NO*PAGE_NO;i++)
		{
			ftlpagetag_[i].physical_state=UNKNOWN;
			ftlpagetag_[i].logical_state=UNKNOWN;
			wipe();
		}

		// Setup the header
		headerpointer=0;
		datapointer=PAGE_NO - 1;
		sectorpointer=0;
		rsectorpointer=SECTOR_NO-1;		
		header[0]=0x55;
		header[1]=0xaa;
		header[PAGE_SIZE-2]=0x55;
		header[PAGE_SIZE-1]=0xaa;
		count=2;
		return SUCCESS;
	}
	

	
	int init(int FlashType)
	{
		if(FlashType == NEWFLASH)				// For new flash   i.e. The Flash doesnt contain any data
			init();
		else if(FlashType == OLDFLASH)				// For old flash   i.e. The flash already has data
		{
			int i,j,k,logicalpage,physicalpage;		// In case of old flash, the lookup table must be rebuilt
			block_data_t buffer[PAGE_SIZE];			// by using the headers that are stored in the flash


			for(i=0;i<SECTOR_NO;i++)			// Search for the reserve sector
			{
				FlashMemory_.read(buffer,i*PAGE_NO);
				if(buffer[0]==0xff && buffer[1]==0xff  && buffer[2]==0xff  && buffer[3]==0xff )
				{	
					rsectorpointer=i;
					break;
				}	
			}

			// Setup FTL tags
			for(i=0;i<SECTOR_NO*PAGE_NO;i++)
			{
				ftlpagetag_[i].physical_state=UNKNOWN;
				ftlpagetag_[i].logical_state=UNKNOWN;
				ftlpagetag_[i].lookup=0;
			}

			// Turn off garbage collector and setup header
			garbagecollector=OFF;
			header[0]=0x55;
			header[1]=0xaa;
			header[PAGE_SIZE-2]=0x55;
			header[PAGE_SIZE-1]=0xaa;
			count=2;
			for(i=0;i<PAGE_NO;i++)
			{
				ftlpagetag_[i+rsectorpointer*PAGE_NO].physical_state=FREE;
			}

			// Search for header content and rebuilt the lookup table
			for(i=rsectorpointer+1;i<SECTOR_NO;i++)
			{
				for(j=0;j<PAGE_NO;j++)
				{
					FlashMemory_.read(buffer,i*PAGE_NO+(j));
					if(buffer[0]==0x55 && buffer[1] == 0xaa)					// Found a header
					{
						for(k=2;k<PAGE_SIZE;k++)
						{
							if(buffer[k]==0xff)						// Found Free space
								break;							// Hence Quit this header

							else if(buffer[k]==PAGE)					// Found a PAGE
							{								// Store the page information
								logicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+(buffer[k+3]);
								physicalpage=(buffer[k+4]<<16)+(buffer[k+5]<<8)+(buffer[k+6]);
								ftlpagetag_[logicalpage].lookup=physicalpage;
								ftlpagetag_[logicalpage].logical_state=PAGE;
								ftlpagetag_[physicalpage].physical_state=PAGE;
								k=k+6;
							}	
							else if(buffer[k]==DEAD)					// Found a DEAD PAGE
							{								// Store the information for 
															// Bad block remapping
								physicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+(buffer[k+3]);
								ftlpagetag_[physicalpage].physical_state=DEAD;
								k=k+3;
							}
							else if(buffer[k]==TO_ERASE)					// Found a TO_ERASE
							{
								logicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+buffer[k+3];
								ftlpagetag_[logicalpage].logical_state=FREE;
								k=k+3;
							}
						}
					}
					else
						break;									// Found free space		
				}											// Quit sector
			}

			for(i=0;i<=rsectorpointer;i++)
			{
				for(j=0;j<PAGE_NO;j++)
				{
					FlashMemory_.read(buffer,i*PAGE_NO+(j));
					if(buffer[0]==0x55 && buffer[1] == 0xaa)
					{
						for(k=2;k<PAGE_SIZE;k++)
						{
							if(buffer[k]==0xff)						// Found Free space
								break;							// Hence Quit this header

							else if(buffer[k]==PAGE)					// Found a PAGE
							{								// Store the page information

								logicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+buffer[k+3];
								physicalpage=(buffer[k+4]<<16)+(buffer[k+5]<<8)+buffer[k+6];
								ftlpagetag_[logicalpage].lookup=physicalpage;
								ftlpagetag_[logicalpage].logical_state=PAGE;
								ftlpagetag_[physicalpage].physical_state=PAGE;
								k=k+6;
							}	
							else if(buffer[k]==DEAD)					// Found a DEAD PAGE
							{								// Store the information for 
															// Bad block remapping
								physicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+buffer[k+3];
								ftlpagetag_[physicalpage].physical_state=DEAD;
								k=k+3;
							}	
							else if(buffer[k]==TO_ERASE)					// Found a TO_ERASE
							{
								logicalpage=(buffer[k+1]<<16)+(buffer[k+2]<<8)+buffer[k+3];
								ftlpagetag_[logicalpage].logical_state=FREE;
								k=k+3;
							}
						}
					}
					else
						break;									// Found free space
				}											// Quit Sector
			}
			
			
		// Setup different pointers
		headerpointer=0;
		datapointer=PAGE_NO-2;
		sectorpointer=rsectorpointer;

		// Turn on garbage collector for collecting garbage
		garbagecollector=ON;
		collect_garbage();
		garbagecollector=OFF;

		}		
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
		for(i=0;i<blocks*2;i++)							// Split into two blocks since PAGE_SIZE is 256
			if(read_(buffer+i*PAGE_SIZE,(2*addr+i)*PAGE_SIZE)!=SUCCESS)
				return ERR_UNSPEC;

		return SUCCESS;
	}

	int read_(block_data_t* buffer, address_t addr)
	{
		// Check alignment
		if(addr%PAGE_SIZE!=0)							
			return ERR_UNSPEC;

		if(ftlpagetag_[addr/PAGE_SIZE].logical_state == FREE)
			for(int i=0;i<PAGE_SIZE;i++)
				buffer[i]=0xff;
		else if(ftlpagetag_[addr/PAGE_SIZE].logical_state == PAGE)			// Check if the block is page or not
			FlashMemory_.read(buffer,ftlpagetag_[addr/PAGE_SIZE].lookup);
		else
			return ERR_UNSPEC;
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
		if(write_(buffer,addr,blocks)!=SUCCESS)
			return ERR_UNSPEC;
		garbagecollector=ON;
		collect_garbage();
		garbagecollector=OFF;
		return SUCCESS;
	}

	int write_(block_data_t* buffer, address_t addr, address_t blocks)
	{
		address_t i=0;
		for(i=0;i<blocks*2;i++)
			if(write_(buffer+i*PAGE_SIZE,(2*addr+i)*PAGE_SIZE)!=SUCCESS)		// Splitting into two blocks internally
				return ERR_UNSPEC;

		return SUCCESS;
	}

	int write_(block_data_t* buffer, address_t addr)
	{	

		int i;
		int write_success = ERR_UNSPEC;
		
		// Check for alignment	
		if(addr%PAGE_SIZE!=0)
			return ERR_UNSPEC;
		for(i=0;i<5&&write_success!=SUCCESS;i++)
		{
			if(ftlpagetag_[sectorpointer*PAGE_NO+datapointer].physical_state==FREE)
			{
				// setup up a TO_ERASE tag if the page is used
				if(ftlpagetag_[addr/PAGE_SIZE].logical_state==PAGE && (!(garbagecollector==ON)))
				{
					ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].physical_state = TO_ERASE;
					ftlpagetag_[addr/PAGE_SIZE].logical_state = FREE;
				}

				// Check if the sector is full or not
				if(datapointer<=headerpointer)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;
					FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
	
				// Write the block
				write_success = FlashMemory_.write(buffer,(sectorpointer*PAGE_NO+datapointer));
				if(write_success == SUCCESS)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+datapointer].physical_state=PAGE;	
					ftlpagetag_[addr/PAGE_SIZE].logical_state=PAGE;		
					ftlpagetag_[addr/PAGE_SIZE].lookup=sectorpointer*PAGE_NO+datapointer;
					
					// Store the lookup information in form of a 7-byte header
					header[count]=PAGE;
					count++;
					header[count]=((addr/PAGE_SIZE)>>16)&0xff;
					count++;
					header[count]=((addr/PAGE_SIZE)>>8)&0xff;
					count++;
					header[count]=(addr/PAGE_SIZE)&0xff;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer)>>16)&0xff;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer)>>8)&0xff;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer))&0xff;
					count++;
					datapointer--;				
				}
				else	// The block is dead
				{	// Store the dead block information in a header
					ftlpagetag_[sectorpointer*PAGE_NO+datapointer].physical_state=DEAD;	
					header[count]=DEAD;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer)>>16)&0xff;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer)>>8)&0xff;
					count++;
					header[count]=((sectorpointer*PAGE_NO+datapointer))&0xff;
					count++;
					datapointer--;				
				}				

				//Check if the sector is full or not
				if(datapointer<=headerpointer)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;
					FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
	
				// Check if the header is full or not
				if(count>=PAGE_SIZE-7)
				{			
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;								
					FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
					headerpointer++;
					count=2;
				}

				// Check if the sector is full or not
				if(datapointer<=headerpointer)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;
					FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
				
			}			
			else
			{	
				// Remap the bad block
				datapointer--;
				if(datapointer<=headerpointer)
				{
					ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;
					FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
					headerpointer=0;
					datapointer=PAGE_NO-1;
					sectorpointer=(sectorpointer+1)%SECTOR_NO;
					count=2;					
				}
			}
		}
	
		// Unable to write due large number of bad blocks
		if(i==5 && write_success ==SUCCESS)
			return ERR_UNSPEC;

		return SUCCESS;
	}


	/**Setup the TO_ERASE tag for the pages
	 * @param addr The number of the first block to be erased.
	 * @param blocks The number of blocks to erase. (blocks==0 allowed)
	 * @return SUCCESS iff the block was erased successfully
	 */
	int erase(address_t addr, address_t blocks = 1)
	{
		int i;
		for(i=0;i<2*blocks;i++)
			erase_((2*addr+i)*PAGE_SIZE);	
		return SUCCESS;
	}

	int erase_(address_t addr)
	{
		// Check alignment
		if(addr%PAGE_SIZE!=0)
			return ERR_UNSPEC;
		
		if(ftlpagetag_[addr/PAGE_SIZE].logical_state==PAGE)
		{
			// Make the PAGE free
			ftlpagetag_[ftlpagetag_[addr/PAGE_SIZE].lookup].physical_state = TO_ERASE;
			ftlpagetag_[addr/PAGE_SIZE].logical_state = FREE;
			header[count]=TO_ERASE;
			count++;
			header[count]=((addr/PAGE_SIZE)>>16)&0xff;
			count++;				
			header[count]=((addr/PAGE_SIZE)>>8)&0xff;
			count++;				
			header[count]=((addr/PAGE_SIZE))&0xff;
			count++;				
			
			// Check if the sector is full or not
			if(datapointer<=headerpointer)
			{
				ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;
				FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
				headerpointer=0;
				datapointer=PAGE_NO-1;
				sectorpointer=(sectorpointer+1)%SECTOR_NO;
				count=2;					
			}
	
			// Check if the header is full or not
			if(count>=PAGE_SIZE-5)
			{			
				ftlpagetag_[sectorpointer*PAGE_NO+headerpointer].physical_state=HEADER;								
				FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
				headerpointer++;
				count=2;
			}
		}
		return SUCCESS;
	}
	

	/**
	 * Erase a particular sector,
	 * param sector: select the particlar sector to be erased
	 * @return SUCCESS iff the memory was wiped successfully
	 */
	int erase_sector(int sector)	
	{
		int i;

		for(i=0;i<PAGE_NO;i++)
		{
			ftlpagetag_[sector*PAGE_NO+i].physical_state=FREE;
		}	
		if(FlashMemory_.erase(sector)==true)
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
			ftlpagetag_[i].physical_state = FREE;
			ftlpagetag_[i].logical_state = FREE;		
		}	
		if(FlashMemory_.wipe()==true)
			return SUCCESS;
		else
			return ERR_UNSPEC;
	}


	/**
	 *@return size of the block memory device in blocks
	 */
	address_t size()
	{
		return FlashMemory_.page_size();
	}


	/**
	 * Uninitialse the flash memory,
	 * @return SUCCESS iff the memory has been uninitialsed
	 */

	int uninit()
	{
		FlashMemory_.write(header,sectorpointer*PAGE_NO+headerpointer);
		FlashMemory_.flush();
		return SUCCESS;
	}


	/**
	 * Garbabge Collector,
	 * for internal use only
	 */

	void collect_garbage()
	{	
		int i,j;			
		block_data_t buffer[PAGE_SIZE];

		// garbage collector will only activate if all the sector expect the reserve sector are used
		if((rsectorpointer==sectorpointer))
		{
			rsectorpointer=(rsectorpointer+1);
			
			// Re-allocate only the block pages only and
			// get rid of TO_ERASE pages
			for(i=rsectorpointer;i<SECTOR_NO;i++)
			{
				for(j=0;j<SECTOR_NO*PAGE_NO;j++)
					if((ftlpagetag_[j].lookup/PAGE_NO)==i)
						if(ftlpagetag_[j].logical_state==PAGE)
						{
							read_(buffer,j*PAGE_SIZE);
							write_(buffer,j*PAGE_SIZE);
						}
				erase_sector(i);
			}
			

			// Re-allocate only the block pages only and
			// get rid of TO_ERASE pages
			for(i=0;i<(rsectorpointer-1);i++)
			{
				for(j=0;j<SECTOR_NO*PAGE_NO;j++)
					if((ftlpagetag_[j].lookup/PAGE_NO)==i)
						if(ftlpagetag_[j].logical_state==PAGE)
						{
							read_(buffer,j*PAGE_SIZE);
							write_(buffer,j*PAGE_SIZE);
						}
				erase_sector(i);
			}

			rsectorpointer=rsectorpointer-2;
			if(rsectorpointer>=0)
				rsectorpointer=rsectorpointer%SECTOR_NO;
			else
				rsectorpointer=rsectorpointer+SECTOR_NO;
		}
	}

	private:
	FlashMemory_P FlashMemory_;			// object of block memory
	FTLPageTag ftlpagetag_[SECTOR_NO*PAGE_NO];	// Page tages for FTL
	block_data_t header[PAGE_SIZE];			// header to store the locations of blocks stored in flash
	int count;					// count the size of header
	int headerpointer;				// Points to the headers in a particular sector
	int datapointer;				// Points to the data in a particular sector
	int sectorpointer;				// Points the sector to which write has to take place
	int rsectorpointer;				// Points to the reserve sector used for garbage collection
	int garbagecollector;				// Flag used to check if garbage collection is taking place or not
	Os::Debug::self_pointer_t debug_;
};

}

#endif

