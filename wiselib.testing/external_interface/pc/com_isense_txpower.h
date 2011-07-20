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
#ifndef CONNECTOR_ISENSE_TXPOWER_H
#define CONNECTOR_ISENSE_TXPOWER_H

namespace wiselib
{

	/** \brief This class implements the \ref txpower_concept
	 *
	 *  iSense implementation of the \ref txpower_concept "ComIsenseTxPower Concept" ...
	 */
	template<typename OsModel_P>
	class ComIsenseTxPower {
	public:
		ComIsenseTxPower();
		ComIsenseTxPower(ComIsenseTxPower const &);

		ComIsenseTxPower &operator=(ComIsenseTxPower const &);
		bool operator==(ComIsenseTxPower) const;
		bool operator!=(ComIsenseTxPower) const;
		bool operator<=(ComIsenseTxPower) const;
		bool operator>=(ComIsenseTxPower) const;
		bool operator<(ComIsenseTxPower) const;
		bool operator>(ComIsenseTxPower) const;
		ComIsenseTxPower operator++();
		ComIsenseTxPower operator++(int);
		ComIsenseTxPower operator--();
		ComIsenseTxPower operator--(int);

		static ComIsenseTxPower from_ratio(int);
		void set_ratio(int);
		int to_ratio() const;
		static ComIsenseTxPower from_dB(int);
		void set_dB(int);
		int to_dB() const;

		static ComIsenseTxPower const MIN;
		static ComIsenseTxPower const MAX;

	private:
		ComIsenseTxPower(int);
		int value_;
	};

	/// static MIN value for signal power
	template<typename OsModel_P> ComIsenseTxPower<OsModel_P> const ComIsenseTxPower<OsModel_P>::
		MIN=-30;

	/// static MAX value for signal power
	template<typename OsModel_P> ComIsenseTxPower<OsModel_P> const ComIsenseTxPower<OsModel_P>::
		MAX=0;

	template<typename OsModel_P>
	inline ComIsenseTxPower<OsModel_P>::ComIsenseTxPower(int v):
		value_(v){}

	template<typename OsModel_P>
	inline ComIsenseTxPower<OsModel_P>::ComIsenseTxPower():
		value_(0)
	{}

	template<typename OsModel_P>
	inline ComIsenseTxPower<OsModel_P> &ComIsenseTxPower<OsModel_P>::operator=(ComIsenseTxPower<OsModel_P> const &p) {
		value_=p.value_;
		return *this;
	}

	template<typename OsModel_P>
	inline ComIsenseTxPower<OsModel_P>::ComIsenseTxPower(ComIsenseTxPower const &power):
	  value_( power.value_ ){}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator==(ComIsenseTxPower p) const {
	   return value_==p.value_;
	}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator!=(ComIsenseTxPower p) const {
		return value_!=p.value_;
	}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator<=(ComIsenseTxPower p) const {
		return value_<=p.value_;
	}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator>=(ComIsenseTxPower p) const {
		return value_>=p.value_;
	}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator<(ComIsenseTxPower p) const {
		return value_<p.value_;
	}

	template<typename OsModel_P>
	inline bool ComIsenseTxPower<OsModel_P>::operator>(ComIsenseTxPower p) const {
		return value_>p.value_;
	}

	template<typename OsModel_P>
	ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::operator++(){
		value_+=6;
		return *this;
	}

	template<typename OsModel_P>
	ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::operator++(int){
		ComIsenseTxPower p=*this;
		value_+=6;
		return p;
	}

	template<typename OsModel_P>
	ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::operator--(){
		value_-=6;
		return *this;
	}

	template<typename OsModel_P>
	ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::operator--(int){
		ComIsenseTxPower p=*this;
		value_-=6;
		return p;
	}

	/** Transforms an arbitrary input ratio to ComIsenseTxPower object with corresponding radio power set.
	 *
	 * \param ratio is the input ratio that shall be transformed
	 * \return ComIsenseTxPower object that has ratio preset
	 */
	template<typename OsModel_P> ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::
		from_ratio( int ratio )
	{
		if(ratio<=1)
			return MIN;
		else if(ratio<=4)
			return ComIsenseTxPower(-24);
		else if(ratio<=16)
			return ComIsenseTxPower(-18);
		else if(ratio<=63)
			return ComIsenseTxPower(-12);
		else if(ratio<=251)
			return ComIsenseTxPower(-6);
		else
			return MAX;
   }

	/** \brief This method sets the dB value by a given ratio.
	 *
	 * Here everything below 1 corresponds to dB=-30 and everything above 1000 corresponds to dB=0,
	 * all other values in between these correspond by powers of 2 to the accurate dB values, each.
	 * Mapping is done by next higher power.
	 * \param ratio is integer
	 * \return void
	 */
	template<typename OsModel_P> void ComIsenseTxPower<OsModel_P>::
		set_ratio( int ratio )
   {
		if(ratio<=1)
			value_=-30;
		else if(ratio<=4)
			value_=-24;
		else if(ratio<=16)
			value_=-18;
		else if(ratio<=63)
			value_=-12;
		else if(ratio<=251)
			value_=-6;
		else
			value_=0;
	}

	/** Gives ratio that corresponds to set dB value of object.
	 *
	 * \return dB value
	 */
	template<typename OsModel_P> int ComIsenseTxPower<OsModel_P>::
		to_ratio() const
	{
		switch(value_){
		case 0:
			return 1000;
		case -6:
			return 251;
		case -12:
			return 63;
		case -18:
			return 16;
		case -24:
			return 4;
		default:
			return 1;
	   }
	}

	/** Transforms an arbitrary input dB integer value to valid value in set {-30,-24,..,0}
	 *
	 * \param db is the input dB value that shall be transformed
	 * \return ComIsenseTxPower object that has db preset
	 */
	template<typename OsModel_P> ComIsenseTxPower<OsModel_P> ComIsenseTxPower<OsModel_P>::
		from_dB( int db )
	{
		if(db<=-30)
			return MIN;
		else if(db<=-24)
			return ComIsenseTxPower(-24);
		else if(db<=-18)
			return ComIsenseTxPower(-18);
		else if(db<=-12)
			return ComIsenseTxPower(-12);
		else if(db<=-6)
			return ComIsenseTxPower(-6);
		else
			return MAX;
	}

	/** \brief set transmission power to value in interval [-30,0] in value-6 steps
	 *
	 * this method sets the transmission power
	 * if dB value is not within allowed range or of allowed value, the input value
	 * is changed to a valid value that meets all criteria
	 * \param dB value, any integer allowed
	 * \return void
	 */
	template<typename OsModel_P> void ComIsenseTxPower<OsModel_P>::
		set_dB( int db )
	{
	   if(db<=-30)
		   value_=-30;
	   else if(db<=-24)
		   value_=-24;
	   else if(db<=-18)
		   value_=-18;
	   else if(db<=-12)
		   value_=-12;
	   else if(db<=-6)
		   value_=-6;
	   else
		   value_=0;
	}

	/** Gives ratio that corresponds to set dB value of object.
	 *
	 * \return dB value
	 */
	template<typename OsModel_P> inline int ComIsenseTxPower<OsModel_P>::
		to_dB() const
	{
		return value_;
	}
}
#endif
