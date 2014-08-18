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

#ifndef __MQTTSN_FLEX_STATIC_STRING__
#define __MQTTSN_FLEX_STATIC_STRING__

namespace wiselib
{
    /**
    *\brief Topic management class
    */
    template<unsigned int Size_P>
    class FlexStaticString
    {
        public:
            /**
            * \brief Constructor
            */
            FlexStaticString();

            /**
             * \brief Constructor with const char* string argument
             * \param string - string to be assigned to buffer
             */
            FlexStaticString( const char* string );

            /**
             * \brief Assignment operator
             * \param string - string to be assigned to buffer
             */
            void operator= ( const char* string );

            /**
             * \brief Comparison operator
             * \param flex - reference to object of type FlexStaticString to be compared
             * \return true if objects have same buffers, false if objects have different buffers
             */
            bool operator== ( const FlexStaticString& flex );

            /**
             * \brief Comparison operator
             * \param string - string to be compared
             * \return true if string and buffer are equal, false if are not equal
             */
            bool operator== ( const char* string );

            /**
             * \brief Relational operator
             * \param flex - reference to object of type FlexStaticString to be compared
             * \return true if objects have same buffers, false if objects have different buffers
             */
            bool operator!= ( const FlexStaticString& flex );

            /**
             * \brief Relational operator
             * \param string - string to be compared
             * \return true if string and buffer are equal, false if are not equal
             */
            bool operator!= ( const char* string );

            /**
             * \brief Subscript operator
             * \param index - index of buffer to return
             * \return character indicated by index
             */
            char operator[] ( uint8_t index ) const;

            /**
             * \brief Returns pointer to buffer
             * \return pointer to buffer
             */
            const char* c_str();

            /**
             * \brief Checks if buffer is empty
             * \return true if buffer is empty, false if buffer is not empty
             */
            bool is_empty() const;

            /**
             * \brief Returns length of string in buffer
             * \return length of string in buffer
             */
            uint8_t length() const;

            /**
             * \brief Sets certain char of buffer_ member
             * \param index - index of buffer_
             * \param character - character to be set
             */
            void set_char( uint8_t index, char character );

        private:
            /**
             * \brief Array of chars
             */
            char buffer_[Size_P+1];
    };

    template<unsigned int Size_P>
    FlexStaticString<Size_P>::FlexStaticString()
    {
        memset( buffer_, 0, Size_P+1);
    }

    template<unsigned int Size_P>
    FlexStaticString<Size_P>::FlexStaticString( const char *string )
    {
        if ( 0 != string )
        {
            memset( buffer_, 0, Size_P+1);
            memcpy( buffer_, string, Size_P );
            buffer_[Size_P] = '\0';
        }
    }

    template<unsigned int Size_P>
    void FlexStaticString<Size_P>::operator= ( const char* string )
    {
        if ( 0 != string )
        {
            uint8_t length = strlen( string );
            if ( length <= Size_P )
            {
                memset( buffer_, 0, Size_P+1 );
                memcpy( buffer_, string, length );
                buffer_[length] = '\0';
            }
        }
    }

    template<unsigned int Size_P>
    bool FlexStaticString<Size_P>::operator== ( const FlexStaticString& flex )
    {
        bool result = true;

        for( uint8_t i = 0; i < length()-1 ; ++i )
        {
            if( flex[i] != buffer_[i] )
            {
                result = false;
                break;
            }
        }

        return result;
    }

    template<unsigned int Size_P>
    bool FlexStaticString<Size_P>::operator== ( const char* string )
    {
        bool result = true;

        if ( 0 != string)
        {
            for( uint8_t i = 0; i < length()-1 ; ++i )
            {
                if( string[i] != buffer_[i] )
                {
                    result = false;
                    break;
                }
            }
        }

        return result;
    }

    template<unsigned int Size_P>
    bool FlexStaticString<Size_P>::operator!= ( const FlexStaticString& flex )
    {
        return !( operator==( flex ) );
    }

    template<unsigned int Size_P>
    bool FlexStaticString<Size_P>::operator!= ( const char* string )
    {
        return !( operator==( string ) );
    }


    template<unsigned int Size_P>
    char FlexStaticString<Size_P>::operator[] ( uint8_t index ) const
    {
        return buffer_[index];
    }

    template<unsigned int Size_P>
    const char* FlexStaticString<Size_P>::c_str()
    {
        return buffer_;
    }

    template<unsigned int Size_P>
        uint8_t FlexStaticString<Size_P>::length() const
        {
            uint8_t length = 0;

            if ( 0 != buffer_)
            {
                while ( '\0' != buffer_[length])
                {
                    length++;
                }
            }

            return length + 1;
        }

    template<unsigned int Size_P>
    bool FlexStaticString<Size_P>::is_empty() const
    {
        bool is_empty = true;

        if ( 0 != buffer_[0])
        {
            is_empty = false;
        }

        return is_empty;
    }

    template<unsigned int Size_P>
    void FlexStaticString<Size_P>::set_char( uint8_t index, char character )
    {
        buffer_[index] = character;
        buffer_[index+1] = '\n';
    }

}

#endif //__MQTTSN_FLEX_STATIC_STRING__
