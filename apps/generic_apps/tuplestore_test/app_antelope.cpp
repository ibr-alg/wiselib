
#define DB_FEATURE_COFFEE 0
#define DB_MAX_ELEMENT_SIZE 120
#define DB_MAX_CHAR_SIZE_PER_ROW 360
#define AQL_MAX_QUERY_LENGTH 360


#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;

#define APP_DATABASE_DEBUG 0

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
			db_query(NULL, "REMOVE RELATION rdf;");
			db_query(NULL, "CREATE RELATION rdf;");
			db_query(NULL, "CREATE ATTRIBUTE s DOMAIN VARCHAR(120) IN rdf;");
			db_query(NULL, "CREATE ATTRIBUTE p DOMAIN VARCHAR(120) IN rdf;");
			db_query(NULL, "CREATE ATTRIBUTE o DOMAIN VARCHAR(120) IN rdf;");
		}

		void insert_tuple(char *s, char *p, char *o) {
			debug_->debug("ins (%s,%s,%s)", s, p, o);

			db_result_t result = db_query(NULL, "INSERT (\'%s\', \'%s\', \'%s\') INTO rdf;", s, p, o);

			if(DB_ERROR(result)) {
				debug_->debug("ERROR");
				debug_->debug(db_get_result_message(result));
			}

		}
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
