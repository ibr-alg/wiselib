
#ifndef INT_DICTIONARY_H
#define INT_DICTIONARY_H

namespace wiselib {
   
   /**
    */
   template<
      typename OsModel_P,
      typename Value_P,
      size_t NNN
   >
   class IntDictionary {
      public:
         typedef OsModel_P OsModel;
         typedef Value_P Value;
         typedef Value value_type;
         typedef IntDictionary<OsModel, Value, NNN> self_t;
         typedef self_t dictionary_t;
         //typedef typename Allocator::template pointer_t<self_t> self_pointer_t;
         //typedef self_t* self_pointer_t;
         
         typedef typename OsModel::size_t size_type;
         typedef value_type mapped_type;
         //typedef bitstring_static_view<Os, mapped_type> compact_mapped_type;
         typedef uint16_t key_type;
         
         enum ErrorCodes {
            SUCCESS = OsModel::SUCCESS
         };
       
         class iterator {
               iterator(typename dictionary_t::self_pointer_t dict, key_type k) : dictionary_(dict), key_(k) {
                  forward();
               }
            public:
               iterator() : dictionary_(0), key_(0) {
               }
               
               iterator(iterator& other) {
                  *this = other;
               }
               
               iterator& operator=(iterator& other) {
                  dictionary_ = other.dictionary_;
                  key_ = other.key_;
               }
               
               bool operator==(iterator& other) {
                  return (dictionary_ == other.dictionary_) && (key_ == other.key_);
               }
               
               bool operator!=(iterator& other) {
                  return !(*this == other);
               }
               
               iterator& operator++() {
                  key_++;
                  forward();
                  return *this;
               }
               
               value_type& operator*() {
                  return dictionary_->entries_[key_].value;
               }
               
            private:
               void forward() {
                  for(; key_ < NNN && dictionary_->entries_[key_].used; key_++) ;
               }
               
               typename dictionary_t::self_pointer_t dictionary_;
               key_type key_;
         };
         
      private:
         struct Entry {
            value_type value;
            bool used;
            Entry() : used(false) {
            }
         };
         Entry entries_[NNN];
         
      public:
         enum {
            NULL_KEY = (key_type)-1
         };
         
      public:
         IntDictionary() {
         }
         
         int init() {
            return SUCCESS;
         }
         
         int destruct() {
            clear();
            return SUCCESS;
         }
         
         key_type insert(mapped_type v) {
            for(key_type i=0; i<NNN; i++) {
               if(!entries_[i].used) {
                  entries_[i].value = v;
                  entries_[i].used = true;
                  return i;
               }
            }
            return NULL_KEY;
         }
         
         size_type erase(key_type k) {
            entries_[k].used = false;
            return 1;
         }
         
         void clear() {
            for(key_type i=0; i<NNN; i++) {
               entries_[i] = Entry();
            }
         }
         
         key_type find(mapped_type v) {
            for(key_type i=0; i<NNN; i++) {
               if(entries_[i].used && entries_[i].value == v) {
                  return i;
               }
            }
            
            //#ifdef PC
            //printf("[!] not found: %s\n", v.c_str());
            //for(key_type i=0; i<N; i++) {
               //if(entries_[i].used) {
                  //printf("%d %s\n", i, entries_[i].value.c_str());
               //}
            //}
            //#endif
            
            
            return NULL_KEY;
         }
         
         size_type count(key_type& k) {
            return (find(k) != NULL_KEY) ? 1 : 0;
         }
         
         mapped_type operator[](key_type k) {
            return entries_[k];
         }
         
         iterator begin() {
            return iterator(this, 0);
         }
         iterator end() {
            return iterator();
         }
         
         /*
         iterator begin() {
            return tree_.begin();
         }
         
         iterator end() {
            return tree_.end();
         }
         
         size_type size() {
            return tree_.size();
         }
         
         size_type max_size() {
            return 0;
         }
         
         bool empty() { return size() == 0; }
         */
         
   };
}

#endif // DICTIONARY_H


