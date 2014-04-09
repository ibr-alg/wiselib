
namespace concepts {

	/**
	 */
	class TupleSerializer_concept {
		public:
			typedef ... self_type;
			typedef ... block_data_t;
			typedef ... size_type;
			typedef ... TupleStoreT;
			typedef typename TupleStoreT::Dictionary Dictionary;
			typedef delegateX<void, self_type&> write_callback_t;

			enum { SUCCESS = ..., ERR_UNSPEC = ... };

			int init(block_data_t* buffer, size_type buffer_size, write_callback_t write_callback, Dictionary& dictionary);

			int init();

			/**
			 * Write a single tuple.
			 */
			void write_tuple(typename TupleStore::iterator& it);

			/**
			 * Force calling of write_callback() and clearing of the buffer.
			 */
			void flush();

			// These shall only be called from within write_callback

			/**
			 * Access to the buffer into which serialization data has been
			 * written (the buffer argument given to init() or set_buffer).
			 */
			block_data_t* buffer();

			/**
			 * Amount of data that has been written to buffer so far.
			 * It should always hold that 0 <= buffer_used() <= buffer_size();
			 */
			size_type buffer_used();

			/**
			 * Total buffer size, changed by init() or set_buffer().
			 * It should always hold that 0 <= buffer_used() <= buffer_size();
			 */
			size_type buffer_size();

			/**
			 * Set a new buffer.
			 */
			void set_buffer(block_data_t* buffer, size_type buffer_size);

			/**
			 * Discard data to the buffer such that buffer_used() == 0 will
			 * hold after this call and buffer() will be left unchanged.
			 */
			void reuse_buffer();
	}

}

