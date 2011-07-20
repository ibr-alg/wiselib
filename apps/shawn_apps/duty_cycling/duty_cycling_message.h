/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004,2005 by  SwarmNet (www.swarmnet.de)             **
 **                         and SWARMS   (www.swarms.de)               **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the GNU General Public License, version 2.      **
 ************************************************************************/
#include "../buildfiles/_legacyapps_enable_cmake.h"
#ifdef ENABLE_DUTY_CYCLING


#ifndef __SHAWN_LEGACYAPPS_DUTY_CYCLING_MESSAGE_H__
#define __SHAWN_LEGACYAPPS_DUTY_CYCLING_MESSAGE_H__


#include "sys/message.h"

namespace duty_cycling
{

   class DutyCyclingMessage
      : public shawn::Message
   {
   public:
      DutyCyclingMessage( double );
      virtual ~DutyCyclingMessage();

      double activity( void ) const throw();

   private:

      double activity_;
   };

}

#endif
#endif
/*-----------------------------------------------------------------------
 * Source  $Source: /cvs/shawn/shawn/tubsapps/duty_cycling/duty_cycling_message.h,v $
 * Version $Revision: 1.8 $
 * Date    $Date: 2005/08/05 10:00:52 $
 *-----------------------------------------------------------------------
 * $Log: duty_cycling_message.h,v $
 * Revision 1.8  2005/08/05 10:00:52  ali
 * 2005 copyright notice
 *
 *-----------------------------------------------------------------------*/
