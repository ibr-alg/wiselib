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
#include "util/pstl/vector_static.h"

namespace wiselib
{

	template<	typename Os_P,
				typename Radio_P,
				typename Data_P,
				typename Debug_P>
	class SerializableDataType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Data_P Data;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataType<Os, Radio, Data, Debug> self_type;
		SerializableDataType( void )
		{
			data = NULL;
		}
		SerializableDataType( const self_type& t )
		{
			*this = t;
		}
		SerializableDataType( const Data& d )
		{
			data = d;
		}
		SerializableDataType( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			write<Os, block_data_t, Data>( buff + offset, data );
			return buff;
		}
		self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			data = read<Os, block_data_t, Data>( buff + offset );
			return *this;
		}
		size_t get_serial_size()
		{
			return sizeof( Data );
		}
		self_type& operator=( const self_type& t )
		{
			data = t.data;
			return *this;
		}
		void set_data( const Data& d )
		{
			data = d;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		Data get_data()
		{
			return data;
		}
		Data* get_data_ref()
		{
			return &data;
		}
		void debug( Debug& debug )
		{
			debug.debug( "%x - ( size %i )", data, get_serial_size() );
		}
	private:
		Data data;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData_P,
				int VECTOR_SIZE,
				typename Debug_P>
	class SerializableDataListType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData_P SerializableData;
		typedef Debug_P Debug;
		typedef wiselib::vector_static <Os, SerializableData, VECTOR_SIZE> SerializableList;
		typedef typename Os::size_t list_size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename SerializableList::iterator iterator;
		typedef SerializableDataListType<Os, Radio, SerializableData, VECTOR_SIZE, Debug> self_type;
		SerializableDataListType( void )
		{
		}
		SerializableDataListType( const self_type& t )
		{
			*this = t;
		}
		SerializableDataListType( const SerializableList& dl )
		{
			data_list = dl;
		}
		SerializableDataListType( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_LIST_SIZE_POS = 0;
			size_t SERIALIZABLE_DATA_LIST_POS = SERIALIZABLE_DATA_LIST_SIZE_POS + sizeof( list_size_t );
			size_t SERIALIZABLE_DATA_POS = SERIALIZABLE_DATA_LIST_POS;
			list_size_t len = data_list.size();
			wiselib::write<Os, block_data_t, list_size_t>( buff + SERIALIZABLE_DATA_LIST_SIZE_POS + offset, len );
			for ( iterator i = data_list.begin(); i != data_list.end(); ++i )
			{
				i->serialize( buff, SERIALIZABLE_DATA_POS + offset );
				SERIALIZABLE_DATA_POS = SERIALIZABLE_DATA_POS + i->get_serial_size();
			}
			return buff;
		}
		SerializableDataListType de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			data_list.clear();
			size_t SERIALIZABLE_DATA_LIST_SIZE_POS = 0;
			size_t SERIALIZABLE_DATA_LIST_POS = SERIALIZABLE_DATA_LIST_SIZE_POS + sizeof( list_size_t );
			size_t SERIALIZABLE_DATA_POS = SERIALIZABLE_DATA_LIST_POS;
			list_size_t len = read<Os, block_data_t, list_size_t>( buff + SERIALIZABLE_DATA_LIST_SIZE_POS + offset);
			for ( list_size_t i = 0; i < len; ++i )
			{
				data_list.push_back( SerializableData().de_serialize( buff, SERIALIZABLE_DATA_POS + offset ) );
				SERIALIZABLE_DATA_POS = SERIALIZABLE_DATA_POS + SerializableData().de_serialize( buff, SERIALIZABLE_DATA_POS + offset ).get_serial_size();
			}
			return *this;
		}
		size_t get_serial_size( void )
		{
			size_t len = 0;
			for ( iterator i = data_list.begin(); i != data_list.end(); ++i )
			{
				len = len + i->get_serial_size();
			}
			return len;
		}
		self_type& operator=( const self_type& t )
		{
			data_list = t.data_list;
			return *this;
		}
		void set_data( const SerializableList& dl )
		{
			data_list = dl;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableList get_data( void )
		{
			return data_list;
		}
		SerializableList* get_data_ref()
		{
			return &data_list;
		}
		void debug( Debug& debug )
		{
			for ( iterator i = data_list.begin(); i != data_list.end(); ++i )
			{
				i->debug( debug );
			}
		}
	private:
		SerializableList data_list;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename Debug_P >
	class SerializableDataSetType2
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType2<Os, Radio, SerializableData1, SerializableData2, Debug> self_type;
		SerializableDataSetType2()
		{
		}
		SerializableDataSetType2( const SerializableData1& d1, const SerializableData2& d2 )
		{
			data1 = d1;
			data2 = d2;
		}
		SerializableDataSetType2( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType2( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType2( block_data_t* buff1, block_data_t* buff2, size_t offset1 = 0, size_t offset2 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			return SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2 )
		{
			data1 = d1;
			data2 = d2;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, size_t offset1 = 0, size_t offset2 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename Debug_P >
	class SerializableDataSetType3
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType3<Os, Radio, SerializableData1, SerializableData2, SerializableData3, Debug> self_type;
		SerializableDataSetType3()
		{
		}
		SerializableDataSetType3( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
		}
		SerializableDataSetType3( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType3( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType3( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			return SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename Debug_P >
	class SerializableDataSetType4
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType4<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, Debug> self_type;
		SerializableDataSetType4()
		{
		}
		SerializableDataSetType4( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
		}
		SerializableDataSetType4( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType4( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType4( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			return SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename Debug_P >
	class SerializableDataSetType5
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType5<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, Debug> self_type;
		SerializableDataSetType5()
		{
		}
		SerializableDataSetType5( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
		}
		SerializableDataSetType5( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType5( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType5( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			return SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename SerializableData6_P,
				typename Debug_P >
	class SerializableDataSetType6
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef SerializableData6_P SerializableData6;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType6<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, SerializableData6, Debug> self_type;
		SerializableDataSetType6()
		{
		}
		SerializableDataSetType6( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5, const SerializableData6& d6 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
		}
		SerializableDataSetType6( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType6( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType6( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			data6.serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data6.de_serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			return SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			data6 = t.data6;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5, const SerializableData6& d6 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline void set_data6( const SerializableData6& d )
		{
			data6 = d;
		}
		inline void set_data6( block_data_t* buff, size_t offset = 0)
		{
			data6 = SerializableData6().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData6 get_data6()
		{
			return data6;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline SerializableData6* get_data_ref6()
		{
			return &data6;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
			data6.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
		SerializableData6 data6;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename SerializableData6_P,
				typename SerializableData7_P,
				typename Debug_P >
	class SerializableDataSetType7
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef SerializableData6_P SerializableData6;
		typedef SerializableData7_P SerializableData7;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType7<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, SerializableData6, SerializableData7, Debug> self_type;
		SerializableDataSetType7()
		{
		}
		SerializableDataSetType7( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
		}
		SerializableDataSetType7( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType7( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType7( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			data6.serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			data7.serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data6.de_serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			data7.de_serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			return SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			data6 = t.data6;
			data7 = t.data7;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline void set_data6( const SerializableData6& d )
		{
			data6 = d;
		}
		inline void set_data6( block_data_t* buff, size_t offset = 0)
		{
			data6 = SerializableData6().de_serialize( buff, offset );
		}
		inline void set_data7( const SerializableData7& d )
		{
			data7 = d;
		}
		inline void set_data7( block_data_t* buff, size_t offset = 0)
		{
			data7 = SerializableData7().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData6 get_data6()
		{
			return data6;
		}
		inline SerializableData7 get_data7()
		{
			return data7;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline SerializableData6* get_data_ref6()
		{
			return &data6;
		}
		inline SerializableData7* get_data_ref7()
		{
			return &data7;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
			data6.debug( debug );
			data7.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
		SerializableData6 data6;
		SerializableData7 data7;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename SerializableData6_P,
				typename SerializableData7_P,
				typename SerializableData8_P,
				typename Debug_P >
	class SerializableDataSetType8
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef SerializableData6_P SerializableData6;
		typedef SerializableData7_P SerializableData7;
		typedef SerializableData8_P SerializableData8;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType8<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, SerializableData6, SerializableData7, SerializableData8, Debug> self_type;
		SerializableDataSetType8()
		{
		}
		SerializableDataSetType8( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
		}
		SerializableDataSetType8( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType8( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType8( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			data6.serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			data7.serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			data8.serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data6.de_serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			data7.de_serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			data8.de_serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			return SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			data6 = t.data6;
			data7 = t.data7;
			data8 = t.data8;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline void set_data6( const SerializableData6& d )
		{
			data6 = d;
		}
		inline void set_data6( block_data_t* buff, size_t offset = 0)
		{
			data6 = SerializableData6().de_serialize( buff, offset );
		}
		inline void set_data7( const SerializableData7& d )
		{
			data7 = d;
		}
		inline void set_data7( block_data_t* buff, size_t offset = 0)
		{
			data7 = SerializableData7().de_serialize( buff, offset );
		}
		inline void set_data8( const SerializableData8& d )
		{
			data8 = d;
		}
		inline void set_data8( block_data_t* buff, size_t offset = 0)
		{
			data8 = SerializableData8().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData6 get_data6()
		{
			return data6;
		}
		inline SerializableData7 get_data7()
		{
			return data7;
		}
		inline SerializableData8 get_data8()
		{
			return data8;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline SerializableData6* get_data_ref6()
		{
			return &data6;
		}
		inline SerializableData7* get_data_ref7()
		{
			return &data7;
		}
		inline SerializableData8* get_data_ref8()
		{
			return &data8;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
			data6.debug( debug );
			data7.debug( debug );
			data8.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
		SerializableData6 data6;
		SerializableData7 data7;
		SerializableData8 data8;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename SerializableData6_P,
				typename SerializableData7_P,
				typename SerializableData8_P,
				typename SerializableData9_P,
				typename Debug_P >
	class SerializableDataSetType9
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef SerializableData6_P SerializableData6;
		typedef SerializableData7_P SerializableData7;
		typedef SerializableData8_P SerializableData8;
		typedef SerializableData9_P SerializableData9;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType9<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, SerializableData6, SerializableData7, SerializableData8, SerializableData9, Debug> self_type;
		SerializableDataSetType9()
		{
		}
		SerializableDataSetType9( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8, const SerializableData9& d9 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
			data9 = d9;
		}
		SerializableDataSetType9( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType9( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType9( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, block_data_t* buff9, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0, size_t offset9 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
			data9 = SerializableData9().de_serialize( buff9, offset9 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			data6.serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			data7.serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			data8.serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			data9.serialize( buff, SERIALIZABLE_DATA_9_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data6.de_serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			data7.de_serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			data8.de_serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			data9.de_serialize( buff, SERIALIZABLE_DATA_9_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			return SERIALIZABLE_DATA_9_POS + data9.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			data6 = t.data6;
			data7 = t.data7;
			data8 = t.data8;
			data9 = t.data9;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8, const SerializableData9& d9 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
			data9 = d9;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, block_data_t* buff9, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0, size_t offset9 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
			data9 = SerializableData9().de_serialize( buff9, offset9 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline void set_data6( const SerializableData6& d )
		{
			data6 = d;
		}
		inline void set_data6( block_data_t* buff, size_t offset = 0)
		{
			data6 = SerializableData6().de_serialize( buff, offset );
		}
		inline void set_data7( const SerializableData7& d )
		{
			data7 = d;
		}
		inline void set_data7( block_data_t* buff, size_t offset = 0)
		{
			data7 = SerializableData7().de_serialize( buff, offset );
		}
		inline void set_data8( const SerializableData8& d )
		{
			data8 = d;
		}
		inline void set_data8( block_data_t* buff, size_t offset = 0)
		{
			data8 = SerializableData8().de_serialize( buff, offset );
		}
		inline void set_data9( const SerializableData9& d )
		{
			data9 = d;
		}
		inline void set_data9( block_data_t* buff, size_t offset = 0)
		{
			data9 = SerializableData9().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData6 get_data6()
		{
			return data6;
		}
		inline SerializableData7 get_data7()
		{
			return data7;
		}
		inline SerializableData8 get_data8()
		{
			return data8;
		}
		inline SerializableData9 get_data9()
		{
			return data9;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline SerializableData6* get_data_ref6()
		{
			return &data6;
		}
		inline SerializableData7* get_data_ref7()
		{
			return &data7;
		}
		inline SerializableData8* get_data_ref8()
		{
			return &data8;
		}
		inline SerializableData9* get_data_ref9()
		{
			return &data9;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
			data6.debug( debug );
			data7.debug( debug );
			data8.debug( debug );
			data9.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
		SerializableData6 data6;
		SerializableData7 data7;
		SerializableData8 data8;
		SerializableData9 data9;
	};

	template<	typename Os_P,
				typename Radio_P,
				typename SerializableData1_P,
				typename SerializableData2_P,
				typename SerializableData3_P,
				typename SerializableData4_P,
				typename SerializableData5_P,
				typename SerializableData6_P,
				typename SerializableData7_P,
				typename SerializableData8_P,
				typename SerializableData9_P,
				typename SerializableData10_P,
				typename Debug_P >
	class SerializableDataSetType10
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef SerializableData1_P SerializableData1;
		typedef SerializableData2_P SerializableData2;
		typedef SerializableData3_P SerializableData3;
		typedef SerializableData4_P SerializableData4;
		typedef SerializableData5_P SerializableData5;
		typedef SerializableData6_P SerializableData6;
		typedef SerializableData7_P SerializableData7;
		typedef SerializableData8_P SerializableData8;
		typedef SerializableData9_P SerializableData9;
		typedef SerializableData10_P SerializableData10;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef SerializableDataSetType10<Os, Radio, SerializableData1, SerializableData2, SerializableData3, SerializableData4, SerializableData5, SerializableData6, SerializableData7, SerializableData8, SerializableData9, SerializableData10, Debug> self_type;
		SerializableDataSetType10()
		{
		}
		SerializableDataSetType10( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData1& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8, const SerializableData9& d9, const SerializableData10& d10 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
			data9 = d9;
			data10 = d10;
		}
		SerializableDataSetType10( const self_type& d )
		{
			*this = d;
		}
		SerializableDataSetType10( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		SerializableDataSetType10( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, block_data_t* buff9, block_data_t* buff10, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0, size_t offset9 = 0, size_t offset10 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
			data9 = SerializableData9().de_serialize( buff9, offset9 );
			data10 = SerializableData10().de_serialize( buff10, offset10 );
		}
		inline block_data_t* serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			size_t SERIALIZABLE_DATA_10_POS = SERIALIZABLE_DATA_9_POS + data9.get_serial_size();
			data1.serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			data2.serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			data3.serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			data4.serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			data5.serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			data6.serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			data7.serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			data8.serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			data9.serialize( buff, SERIALIZABLE_DATA_9_POS + offset );
			data10.serialize( buff, SERIALIZABLE_DATA_10_POS + offset );
			return buff;
		}
		inline self_type de_serialize( block_data_t* buff, size_t offset = 0 )
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			data1.de_serialize( buff, SERIALIZABLE_DATA_1_POS + offset );
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			data2.de_serialize( buff, SERIALIZABLE_DATA_2_POS + offset );
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			data3.de_serialize( buff, SERIALIZABLE_DATA_3_POS + offset );
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			data4.de_serialize( buff, SERIALIZABLE_DATA_4_POS + offset );
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			data5.de_serialize( buff, SERIALIZABLE_DATA_5_POS + offset );
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			data6.de_serialize( buff, SERIALIZABLE_DATA_6_POS + offset );
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			data7.de_serialize( buff, SERIALIZABLE_DATA_7_POS + offset );
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			data8.de_serialize( buff, SERIALIZABLE_DATA_8_POS + offset );
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			data9.de_serialize( buff, SERIALIZABLE_DATA_9_POS + offset );
			size_t SERIALIZABLE_DATA_10_POS = SERIALIZABLE_DATA_9_POS + data9.get_serial_size();
			data10.de_serialize( buff, SERIALIZABLE_DATA_10_POS + offset );
			return *this;
		}
		inline size_t get_serial_size()
		{
			size_t SERIALIZABLE_DATA_1_POS = 0;
			size_t SERIALIZABLE_DATA_2_POS = SERIALIZABLE_DATA_1_POS + data1.get_serial_size();
			size_t SERIALIZABLE_DATA_3_POS = SERIALIZABLE_DATA_2_POS + data2.get_serial_size();
			size_t SERIALIZABLE_DATA_4_POS = SERIALIZABLE_DATA_3_POS + data3.get_serial_size();
			size_t SERIALIZABLE_DATA_5_POS = SERIALIZABLE_DATA_4_POS + data4.get_serial_size();
			size_t SERIALIZABLE_DATA_6_POS = SERIALIZABLE_DATA_5_POS + data5.get_serial_size();
			size_t SERIALIZABLE_DATA_7_POS = SERIALIZABLE_DATA_6_POS + data6.get_serial_size();
			size_t SERIALIZABLE_DATA_8_POS = SERIALIZABLE_DATA_7_POS + data7.get_serial_size();
			size_t SERIALIZABLE_DATA_9_POS = SERIALIZABLE_DATA_8_POS + data8.get_serial_size();
			size_t SERIALIZABLE_DATA_10_POS = SERIALIZABLE_DATA_9_POS + data9.get_serial_size();
			return SERIALIZABLE_DATA_10_POS + data10.get_serial_size();
		}
		inline self_type& operator=( const self_type& t )
		{
			data1 = t.data1;
			data2 = t.data2;
			data3 = t.data3;
			data4 = t.data4;
			data5 = t.data5;
			data6 = t.data6;
			data7 = t.data7;
			data8 = t.data8;
			data9 = t.data9;
			data10 = t.data10;
			return *this;
		}
		void set_data( const self_type& d )
		{
			*this = d;
		}
		void set_data( const SerializableData1& d1, const SerializableData2& d2, const SerializableData3& d3, const SerializableData4& d4, const SerializableData5& d5, const SerializableData6& d6, const SerializableData7& d7, const SerializableData8& d8, const SerializableData9& d9, const SerializableData10& d10 )
		{
			data1 = d1;
			data2 = d2;
			data3 = d3;
			data4 = d4;
			data5 = d5;
			data6 = d6;
			data7 = d7;
			data8 = d8;
			data9 = d9;
			data10 = d10;
		}
		void set_data( block_data_t* buff, size_t offset = 0 )
		{
			de_serialize( buff, offset );
		}
		void set_data( block_data_t* buff1, block_data_t* buff2, block_data_t* buff3, block_data_t* buff4, block_data_t* buff5, block_data_t* buff6, block_data_t* buff7, block_data_t* buff8, block_data_t* buff9, block_data_t* buff10, size_t offset1 = 0, size_t offset2 = 0, size_t offset3 = 0, size_t offset4 = 0, size_t offset5 = 0, size_t offset6 = 0, size_t offset7 = 0, size_t offset8 = 0, size_t offset9 = 0, size_t offset10 = 0 )
		{
			data1 = SerializableData1().de_serialize( buff1, offset1 );
			data2 = SerializableData2().de_serialize( buff2, offset2 );
			data3 = SerializableData3().de_serialize( buff3, offset3 );
			data4 = SerializableData4().de_serialize( buff4, offset4 );
			data5 = SerializableData5().de_serialize( buff5, offset5 );
			data6 = SerializableData6().de_serialize( buff6, offset6 );
			data7 = SerializableData7().de_serialize( buff7, offset7 );
			data8 = SerializableData8().de_serialize( buff8, offset8 );
			data9 = SerializableData9().de_serialize( buff9, offset9 );
			data10 = SerializableData10().de_serialize( buff10, offset10 );
		}
		inline void set_data1( const SerializableData1& d )
		{
			data1 = d;
		}
		inline void set_data1( block_data_t* buff, size_t offset = 0)
		{
			data1 = SerializableData1().de_serialize( buff, offset );
		}
		inline void set_data2( const SerializableData2& d )
		{
			data2 = d;
		}
		inline void set_data2( block_data_t* buff, size_t offset = 0)
		{
			data2 = SerializableData2().de_serialize( buff, offset );
		}
		inline void set_data3( const SerializableData3& d )
		{
			data3 = d;
		}
		inline void set_data3( block_data_t* buff, size_t offset = 0)
		{
			data3 = SerializableData3().de_serialize( buff, offset );
		}
		inline void set_data4( const SerializableData4& d )
		{
			data4 = d;
		}
		inline void set_data4( block_data_t* buff, size_t offset = 0)
		{
			data4 = SerializableData4().de_serialize( buff, offset );
		}
		inline void set_data5( const SerializableData5& d )
		{
			data5 = d;
		}
		inline void set_data5( block_data_t* buff, size_t offset = 0)
		{
			data5 = SerializableData5().de_serialize( buff, offset );
		}
		inline void set_data6( const SerializableData6& d )
		{
			data6 = d;
		}
		inline void set_data6( block_data_t* buff, size_t offset = 0)
		{
			data6 = SerializableData6().de_serialize( buff, offset );
		}
		inline void set_data7( const SerializableData7& d )
		{
			data7 = d;
		}
		inline void set_data7( block_data_t* buff, size_t offset = 0)
		{
			data7 = SerializableData7().de_serialize( buff, offset );
		}
		inline void set_data8( const SerializableData8& d )
		{
			data8 = d;
		}
		inline void set_data8( block_data_t* buff, size_t offset = 0)
		{
			data8 = SerializableData8().de_serialize( buff, offset );
		}
		inline void set_data9( const SerializableData9& d )
		{
			data9 = d;
		}
		inline void set_data9( block_data_t* buff, size_t offset = 0)
		{
			data9 = SerializableData9().de_serialize( buff, offset );
		}
		inline void set_data10( const SerializableData10& d )
		{
			data10 = d;
		}
		inline void set_data10( block_data_t* buff, size_t offset = 0)
		{
			data10 = SerializableData10().de_serialize( buff, offset );
		}
		inline SerializableData1 get_data1()
		{
			return data1;
		}
		inline SerializableData2 get_data2()
		{
			return data2;
		}
		inline SerializableData3 get_data3()
		{
			return data3;
		}
		inline SerializableData4 get_data4()
		{
			return data4;
		}
		inline SerializableData5 get_data5()
		{
			return data5;
		}
		inline SerializableData6 get_data6()
		{
			return data6;
		}
		inline SerializableData7 get_data7()
		{
			return data7;
		}
		inline SerializableData8 get_data8()
		{
			return data8;
		}
		inline SerializableData9 get_data9()
		{
			return data9;
		}
		inline SerializableData10 get_data10()
		{
			return data10;
		}
		inline SerializableData1* get_data_ref1()
		{
			return &data1;
		}
		inline SerializableData2* get_data_ref2()
		{
			return &data2;
		}
		inline SerializableData3* get_data_ref3()
		{
			return &data3;
		}
		inline SerializableData4* get_data_ref4()
		{
			return &data4;
		}
		inline SerializableData5* get_data_ref5()
		{
			return &data5;
		}
		inline SerializableData6* get_data_ref6()
		{
			return &data6;
		}
		inline SerializableData7* get_data_ref7()
		{
			return &data7;
		}
		inline SerializableData8* get_data_ref8()
		{
			return &data8;
		}
		inline SerializableData9* get_data_ref9()
		{
			return &data9;
		}
		inline SerializableData10* get_data_ref10()
		{
			return &data10;
		}
		inline void debug( Debug& debug )
		{
			data1.debug( debug );
			data2.debug( debug );
			data3.debug( debug );
			data4.debug( debug );
			data5.debug( debug );
			data6.debug( debug );
			data7.debug( debug );
			data8.debug( debug );
			data9.debug( debug );
			data10.debug( debug );
		}
	private:
		SerializableData1 data1;
		SerializableData2 data2;
		SerializableData3 data3;
		SerializableData4 data4;
		SerializableData5 data5;
		SerializableData6 data6;
		SerializableData7 data7;
		SerializableData8 data8;
		SerializableData9 data9;
		SerializableData10 data10;
	};

}
