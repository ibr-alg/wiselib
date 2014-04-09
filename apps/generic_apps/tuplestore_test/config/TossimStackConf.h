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
 * Filename: TossimStackConf.h
 * $Id: TeenyLimeC.nc 1153 2010-05-10 12:56:02Z sguna $
 * 
 * Description: Definition of constants for test suite.
 *
 * Author(s): Stefan Guna <guna@disi.unitn.it>
 */

#ifndef MSGQUEUE_H
#define MSGQUEUE_H

// The max number of TOS messages waiting to be transmitted
#define TOSSIM_QUEUE_SIZE 10

// The duty cycle of radios the node needs to talk to
#define REMOTE_LPL_INTERVAL 100

// The maximum number of retries to guarantee message delivery
#define MAX_MSG_RETRIES 4


#endif
