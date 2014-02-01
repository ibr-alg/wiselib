
#include "ntuples.h"
#define APP_DATABASE_DEBUG 0
#define APP_DATABASE_FIND  0
#define APP_HEARTBEAT 0

extern "C" {
	#include <string.h>
}

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;

#include <util/serialization/serialization.h>
#include <util/meta.h>

#if defined(CONTIKI)
extern "C" {
	#define delete why_on_earth_do_you_guys_name_a_variable_delete
	#define DOMAIN DOMAIN_WHAT_IS_THIS_I_DONT_EVEN

	#include <contiki.h>
	#include <netstack.h>
	#include <antelope.h>
}
#endif

class App {
	public:

		#include "app_db_mixin.cpp"


		void initialize_db() {
			db_init();
			db_result result;
			result = db_query(NULL, "REMOVE RELATION rdf;");
			if(DB_ERROR(result)) { debug_->debug("eRR:%s", db_get_result_message(result)); }
			result = db_query(NULL, "CREATE RELATION rdf;");
			if(DB_ERROR(result)) { debug_->debug("eCR:%s", db_get_result_message(result)); }
			//debug_->debug("xxx");
			//relation_t *r = relation_create("rdf", DB_STORAGE);
			//debug_->debug("--- r=%p ---", r);
			result = db_query(NULL, "CREATE ATTRIBUTE s DOMAIN STRING(120) IN rdf;");
			if(DB_ERROR(result)) { debug_->debug("eCRs:%s", db_get_result_message(result)); }
			result = db_query(NULL, "CREATE ATTRIBUTE p DOMAIN STRING(120) IN rdf;");
			if(DB_ERROR(result)) { debug_->debug("eCRp:%s", db_get_result_message(result)); }
			result = db_query(NULL, "CREATE ATTRIBUTE o DOMAIN STRING(120) IN rdf;");
			if(DB_ERROR(result)) { debug_->debug("eCRo:%s", db_get_result_message(result)); }

			#if APP_DATABASE_DEBUG
				debug_->debug("db init");
			#endif
		}

		void insert_tuple(char *s, char *p, char *o) {
			#if APP_DATABASE_DEBUG
				debug_->debug("ins(%s,%s,%s)", s, p, o);
			#endif

			db_result_t result = db_query(NULL, "INSERT (\'%s\', \'%s\', \'%s\') INTO rdf;", s, p, o);

			if(DB_ERROR(result)) {
				//debug_->debug("");
				debug_->debug("eri:%s", db_get_result_message(result));
				//debug_->debug(db_get_result_message(result));
			}
		}

		size_type size() {
			relation_t *rel = relation_load("rdf");
			tuple_id_t t = relation_cardinality(rel);
			relation_release(rel);
			return t;
		}

		void find(block_data_t* s, block_data_t* p, block_data_t* o, char* out) {
			#if APP_DATABASE_DEBUG
				debug_->debug("fnd(%s,%s,%s)", s, p, o);
			#endif

			char *spo[] = { (char*)s, (char*)p, (char*)o };
			db_result result;
			static db_handle_t handle;

			// select on string equality is not supported, we have to do it
			// ourselves!
			result = db_query(&handle, "SELECT s,p,o FROM rdf;");
			if(DB_ERROR(result)) {
				debug_->debug("erF:%s", db_get_result_message(result));
				db_free(&handle);
				return;
			}

			int jj = 0;
			while(db_processing(&handle)) {
				result = db_process(&handle);
				if(result == DB_GOT_ROW) {
					// process row somehow
					// actually get out the values, for more fair comparability
					attribute_value_t value;
					bool match = true;
					for(int column = 0; column < handle.ncolumns; column++) {
						result = db_get_value(&value, &handle, column);
						if(DB_ERROR(result)) {
							debug_->debug("DBerc:%s", db_get_result_message(result));
							jj++;
							continue;
						}

						char *ss = spo[column];
						if(ss && strcmp((char*)VALUE_STRING(&value), ss) != 0) {
							//debug_->debug("xxx [%d] %s != %s", column, (char*)VALUE_STRING(&value), ss);
							match = false;
							break;
						}
					}

					if(match) {
						#if APP_DATABASE_DEBUG
							attribute_value_t vs, vp, vo;
							db_get_value(&vs, &handle, 0);
							db_get_value(&vp, &handle, 1);
							db_get_value(&vo, &handle, 2);

							debug_->debug("%d match (%s,%s,%s)", (int)jj, (char*)VALUE_STRING(&vs),
								(char*)VALUE_STRING(&vp), (char*)VALUE_STRING(&vo));
						#endif
					}
				}
				else if(result == DB_OK) {
					// DB saw an uninteresting tuple.
					// Fucks given: 0.
				}
				else if(result == DB_FINISHED) {
					// done.
					break;
				}
				else if(DB_ERROR(result)) {
					debug_->debug("DBer:%s", db_get_result_message(result));
					db_free(&handle);
					return;
				}
				else {
					debug_->debug("RESULT=%d ROW=%d OK=%d FIN=%d", (int)result, (int)DB_GOT_ROW, (int)DB_OK, (int)DB_FINISHED);
				}
				
				jj++;
			}
			db_free(&handle);
		}

		db_handle_t iter_handle_;
		db_result iter_result_;
		void iter_rewind() {
			iter_result_ = db_query(&iter_handle_, "SELECT s,p,o FROM rdf;");
			if(DB_ERROR(iter_result_)) {
				debug_->debug("erR:%s", db_get_result_message(iter_result_));
				db_free(&iter_handle_);
				return;
			}
			iter_result_ = db_process(&iter_handle_);
		}

		bool iter_end() { return (iter_result_ == DB_FINISHED); }
		void iter_inc() { iter_result_ = db_process(&iter_handle_); }

		void iter_get(block_data_t *s, block_data_t *p, block_data_t *o) {
			attribute_value_t value;
			db_result result;

			result = db_get_value(&value, &iter_handle_, 0);
			if(DB_ERROR(result)) { debug_->debug("DBerS:%s", db_get_result_message(result)); }
			strcpy((char*)s, (char*)VALUE_STRING(&value));

			result = db_get_value(&value, &iter_handle_, 1);
			if(DB_ERROR(result)) { debug_->debug("DBerP:%s", db_get_result_message(result)); }
			strcpy((char*)p, (char*)VALUE_STRING(&value));

			result = db_get_value(&value, &iter_handle_, 2);
			if(DB_ERROR(result)) { debug_->debug("DBerO:%s", db_get_result_message(result)); }
			strcpy((char*)o, (char*)VALUE_STRING(&value));
		}

		void iter_free() { db_free(&iter_handle_); }

};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	//Allocator allocator_;
	//Allocator& get_allocator() { return allocator_; }
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :
