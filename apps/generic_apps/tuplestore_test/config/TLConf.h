/*
 * This file is part of TeenyLIME version 1.0 - a wireless sensor networks 
 * programming abstraction replacing 1-hop broadcast, the basic 
 * communication abstraction, with 1-hop data sharing.
 * 
 * Copyright (C) 2009 - 2011 by University of Trento, FBK-irst, 
 * Matteo Ceriotti, Stefan Guna, Luca Mottola, Amy L. Murphy, 
 * Gian Pietro Picco.
 * 
 * TeenyLIME is free software: you can redistribute it and/or modify it under 
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * TeenyLIME is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License 
 * along with TeenyLIME. If not, see <http://www.gnu.org/licenses/>.
 *
 * For more information on TeenyLIME or to contact the TeenyLIME team,
 * Matteo Ceriotti, Paolo Costa, Stefan Guna, Luca Mottola, Amy L. Murphy, 
 * Gian Pietro Picco, please visit <http://teenylime.sourceforge.net/>.
 *
 *
 * Filename: TLConf.h 
 * $Id: TeenyLimeC.nc 1153 2010-05-10 12:56:02Z sguna $
 * 
 * Description: Definition of constants for test suite.
 *
 * Author(s): Luca Mottola <luca@sics.se>
 */

#ifndef TLCONF_H
#define TLCONF_H

#define MAX_REACTIONS 4

// Max number of distributed operations pending... this must be
// less than or equal to the number of outgoing messages in the queue
#define MAX_PENDING_OPS 6  

// Max number of flash operation pending.
#define PERSIST_QUEUE_SIZE 5 

// The duration of a epoch, which also determines the reaction refresh,
// and node tuple refresh
#define EPOCH 30000

//tahusaotehu

// The number of EPOCHs before removing remote information 
// (reactions and neighbor tuples) 
#define REMOTE_LOST_REFRESH 3

// Timeout for remote query operations
#define REMOTE_OP_TIMEOUT 30000

// Max number of neighbor in the TeenyLIME system
// (be careful: these are stored as tuples in the main tuple space)
#define MAX_NEIGHBORS 9

// Number of slabs to be used by the memory allocator.
#define SLABS_NUM 10
#define PSLABS_NUM 12

// Size of a single slab. Must be an even number.
#define SLAB_SIZE 150

// Size for the usage bitmap of a single slab. Must be an even number.
// A maximum of 2^SLAB_BITMAP_SIZE tuples can be accomodated in the slab.
#define SLAB_BITMAP_SIZE 8

// Enables Flash-based tuple space if enabled
/* #define FLASH_SYNC_TIME 10000 */

// Slab allocator parameters for Flash-based tuple space
#define PSLAB_BITMAP_SIZE 16
#define PSLAB_SIZE 4096

#endif
