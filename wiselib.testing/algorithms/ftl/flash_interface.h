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

#ifndef FLASH_INTERFACE_H
#define FLASH_INTERFACE_H

namespace wiselib
{

	template < 
		typename OsModel_P,
		typename BlockMemory_P
	>

	class FlashInterface
	{
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef BlockMemory_P BlockMemory;
			typedef typename OsModel::size_t address_t;

			
			enum {
				BLOCK_SIZE = BlockMemory::PAGE_SIZE,
				BUFFER_SIZE = BLOCK_SIZE,
				NO_ADDRESS = BlockMemory::NO_ADDRESS
			};
			
			enum {
				SUCCESS = BlockMemory::SUCCESS,
				ERR_UNSPEC = BlockMemory::ERR_UNSPEC
			};

			enum {
				PAGE_SIZE = BlockMemory::PAGE_SIZE, // 32 PAGE_SIZE is the smallest amount of data which is read or written at a time.
				PAGES_PER_SECTOR = BlockMemory::PAGES_PER_SECTOR, // 4 PAGES_PER_SECTOR is the number of pages in a sector.
				SECTOR_SIZE = BlockMemory::PAGE_SIZE*PAGES_PER_SECTOR, // 128 SECTOR_SIZE is the smallest amount of data which is erased at a time.
				NO_SECTOR = BlockMemory::NO_SECTOR,// 4 SECTOR_NO is the number of sectors in a memory
				MEMORY_SIZE = BlockMemory::NO_SECTOR * BlockMemory::SECTOR_SIZE // 512 SECTOR_NO is the number of sectors in a memory
			};


			int read(block_data_t* buffer, address_t a, address_t size=1)
			{
				if(block_memory.read(a*PAGE_SIZE,PAGE_SIZE,buffer)==true)
					return SUCCESS;
				else
					return ERR_UNSPEC;
			}
		
			int init()
			{
				if(block_memory.init()==true)
					return SUCCESS;
				else
					return ERR_UNSPEC;
			}
		
			int init(int FlashType)
			{
				if(block_memory.init(FlashType)==true)
					return SUCCESS;
				else
					return ERR_UNSPEC;
			}

			int wipe()
			{
				if(block_memory.erase()==true)	
					return SUCCESS;
				else
					return ERR_UNSPEC;
			}			

			int erase(int sector)
			{
				if(block_memory.erase(sector)==true)
					return SUCCESS;	
				else
					return ERR_UNSPEC;
			}


			int write(block_data_t* buffer, address_t a, address_t size=1)
			{
				if(block_memory.write(a*PAGE_SIZE,PAGE_SIZE,buffer)==true)	
					return SUCCESS;		
				else
					return ERR_UNSPEC;	
			}
			private:
			BlockMemory block_memory;
	};

}

#endif
