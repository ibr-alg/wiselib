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
#ifndef WISELIB_PSTL_STRING
#define WISELIB_PSTL_STRING
#define MAX_STRING_LENGTH 64

namespace wiselib {

    char *mystrncpy(char *target1, const char *source1, int no) {
        char *tempt = target1;
        for (int n = 0; n < no;) {
            *target1 = *source1;
            target1++;
            source1++;
            n++;
        }

        *target1 = '\0';
        return tempt;
    }

    int mystrncmp(const char* s1, const char* s2, uint8_t n)
    {
        unsigned char uc1, uc2;
        if (n == 0) return 0;
        while (n-- > 0 && *s1 == *s2)
        {
            if ( n == 0 || *s1 == '\0') return 0;

            s1++;
            s2++;
        }
        uc1 = (*(unsigned char *) s1);
        uc2 = (*(unsigned char *) s2);
        return ((uc1 < uc2) ? -1 : (uc1 > uc2));
    }

    char *mystrchr(const char *s, int c)
    {
        while (*s != '\0' && *s != (char)c) s++;
        return ( (*s == c) ? (char *) s : NULL );
    }

    int mystrcspn (const char *s1, const char *s2)
    {
        const char *sc1;
        for (sc1 = s1; *sc1 != '\0'; sc1++)
          if (mystrchr(s2, (int)*sc1) != NULL)
              return (sc1 - s1);
        return sc1 - s1;
    }

    class StaticString {
    private:
        unsigned char length_;
        char buffer_[MAX_STRING_LENGTH];

        char max(const char a, const char b){
            if(a>=b)
                return a;
            else return b;
        }

        char min(const char a, const char b){
            if(a<=b)
                return a;
            else return b;
        }

    public:

        StaticString() {
            length_ = 0;
        }



        StaticString(const char* value)
        {
            length_ = strlen(value);
            memcpy( buffer_, value, length_ + 1 );
        }

        StaticString(char *buffer, char buffer_size) {
            length_ = buffer_size;
            memcpy( buffer_, buffer, length_ + 1 );
        }

        char operator[] (int pos) const {
            return (pos >= length_) ? NULL : buffer_[pos];
        }

        StaticString& append(const char *other) {
            int length = strlen(other);
            if (length_ + length + 1 <= MAX_STRING_LENGTH) {
//                memcpy( buffer_[length_], other, length );

                mystrncpy(&(buffer_[length_]), other, length);
//                buffer_[length_ - 1] = '\0';
                length_ += length;
            }

            return *this;
        }

        StaticString& append(StaticString& other) {
            return append(other.c_str());
        }

        char* c_str() {
            return buffer_;
        }
        
        int size() const{
            return length_;
        }

        int length() const{
            return length_;
        }

        inline bool operator == (const StaticString& s)
        {
            if(s.length()!=length_)
                return false;
            for(int i = 0; i<length_;i++){
                if(buffer_[i]!=s[i])
                    return false;
            }
            return true;
        }

        inline bool operator == (const StaticString* s){
            return operator ==(*s);
        }

        inline bool operator!= (const StaticString& s)
        {
            return (!operator==(s));
        }
        inline bool operator!= (const StaticString* s)
        {
            return (!operator==(s));
        }

        inline bool operator < (const StaticString& s){
            for(int i = 0; i<min(length_,s.length());i++){
                if(buffer_[i]<s[i])
                    return true;
                if(buffer_[i]>s[i])
                    return false;
            }
            return length_<=s.length();
        }

        inline bool operator > (const StaticString& s){
            for(int i = 0; i<min(length_,s.length());i++){
                if(buffer_[i]>s[i])
                    return true;
                if(buffer_[i]<s[i])
                    return false;
            }
            return length_>=s.length();
        }

        inline bool operator < (const StaticString* s){
            return operator <(*s);
        }

        inline bool operator > (const StaticString* s){
            return operator <(*s);
        }
        inline bool operator >= (const StaticString& s){
            return !(operator <(s));
        }
        inline bool operator >= (const StaticString* s){
            return operator >=(*s);
        }
        inline bool operator <= (const StaticString& s){
            return !(operator >(s));
        }
        inline bool operator <= (const StaticString* s){
            return operator <=(*s);
        }
        inline void operator = (const char* s)
        {
            length_ = strlen(s);
            memcpy( buffer_, s, length_ + 1 );
        }


    };

}

#endif
