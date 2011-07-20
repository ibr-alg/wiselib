/******************************************************************************
Copyright 2009, Freie Universitaet Berlin (FUB). All rights reserved.

These sources were developed at the Freie Universitaet Berlin, Computer Systems
and Telematics group (http://cst.mi.fu-berlin.de).
-------------------------------------------------------------------------------
This file is part of FeuerWare.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

FeuerWare is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see http://www.gnu.org/licenses/ .
--------------------------------------------------------------------------------
For further information and questions please use the web site
	http://scatterweb.mi.fu-berlin.de
and the mailinglist (subscription via web site)
	scatterweb@lists.spline.inf.fu-berlin.de
*******************************************************************************/

#ifndef PROJECTCONF_H_
#define PROJECTCONF_H_

/**
 * @ defgroup	<name>		<title>
 * @ ingroup		<name>
 *
 * @ {
 */

/**
 * @file
 * @brief
 *
 * @author      Freie Universität Berlin, Computer Systems & Telematics, FeuerWhere project
 * @author		baar
 * @version     $Revision: 944 $
 *
 * @note		$Id: project-conf.h 944 2009-03-31 13:50:23Z hillebra $
 */

#define	FEUERWARE_CONF_ENABLE_FAT					0
#define FEUERWARE_CONF_ENABLE_HAL					1
#define FEUERWARE_CONF_ENABLE_SYSMON				0

#define TRACELOG_CONF_NUM_ENTRIES					0
#define SYSLOG_CONF_DEFAULT_LEVEL					0x3f

#define CMD_LEVEL									0xFF

/** @ } */
#endif /* PROJECTCONF_H_ */
