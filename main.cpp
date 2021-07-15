#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <sqlite3.h>
#include <unordered_set>
#include <nlohmann/json.hpp>

// RANDOM HELPERS
//#############################################################################


/*
 * Simple helper to convert a vector of strings to json
 *
 */

void ConvertToJson(std::vector<std::string>& in_set,
		nlohmann::json& jsonobj) {

	for(auto& element : in_set) {
		
		jsonobj.push_back(element);

	}


}

/*
 * For generating a random access token, taken from
 * https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c/24586587
 *
 */
std::string random_string( size_t length ) {

	auto randchar = []() -> char {
		const char charset[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[ rand() % max_index ];
	};

	std::string str(length,0);
	std::generate_n( str.begin(), length, randchar );
	return str;
}


/*
 * Generates a random string to act as the token to access both the
 * web interface and to publish data
 */

void generate_session_id(std::string& token_placeholder) {

	// Set it to true to keep going until we can find
	// a string that doesn't exist yet
	bool token_already_exists = true;

	while(token_already_exists) {
	
		// Generate a token
		std::string access_token = random_string(256);

		// If this session ID doesn't exist yet, give it to the user
		if (is_token_in_db(access_token) == false) {
		
			token_placeholder = access_token;
			token_already_exists = false;

		}
			
	}


}

// SQLITE HELPER FUNCTIONS
//#############################################################################

/*
 * Callback function for sqlite select 
 */
static int sqlite_select_callback(void *data, int argc, char **argv, char **azColName) {

	int i;
	fprintf(stderr, "%s: ", (const char*)data);

	for(i = 0; i<argc; i++){

				
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}

/*
 * Callback function for sqlite select 
 */
static int callback(void *data, int argc, char **argv, char **azColName){
	int i;
	fprintf(stderr, "%s: ", (const char*)data);

	for(i = 0; i<argc; i++){
	       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}


/*
 * Generic helper for executing select sqlite commands
 *
 */
void sqlite_select_exec( std::vector<std::string> results, std::string& command) {

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("test.db", &db);

	if( rc ) {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	return;
	} 

	/* Execute SQL statement */
	rc = sqlite3_exec(db, command.c_str(), sqlite_select_callback,0 , &zErrMsg);

	if( rc != SQLITE_OK ){
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
	} else {
	fprintf(stdout, "Records created successfully\n");
	}
	sqlite3_close(db);


}



/*
 * Generic helper for executing sqlite commands
 *
 */
void sqlite_exec(std::string& command) {

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("test.db", &db);

	if( rc ) {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	return;
	} 



	/* Execute SQL statement */
	rc = sqlite3_exec(db, command.c_str(), callback, 0, &zErrMsg);

	if( rc != SQLITE_OK ){
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
	} else {
	fprintf(stdout, "Records created successfully\n");
	}
	sqlite3_close(db);


}



/*
 * Gets a list of tables from the database
 *
 */
void list_tables(std::vector<std::string>& tables, std::string& session_token) {


	std::string command = "SELECT name FROM sqlite_schema WHERE type='table' ORDER by name";
	sqlite_select_exec( tables, command);


}




/* 
 * Checks if that session ID is already registered in DB
 */
bool is_session_already_present(std::string session_token) {

	std::vector<std::string> tables = {};

	// This is just an unordered map with a list of registered hosts 
	list_tables(tables, session_token);
	std::vector<std::string>::const_iterator got = find(tables.begin(), tables.end(), session_token);

	if ( got == tables.end() ) {
		// hostname already present
		return true;

	} else return false;




}

/* 
* Add a new session table
*
*/

void create_table(std::string& session_token) {

       std::cout << "Creating table\n";

       std::string command = "CREATE TABLE " + session_token + "("  \
     "ID INT PRIMARY KEY AUTOINCREMENT NOT NULL," \
     "LABEL TEXT NOT NULL," \
     "RAM_PERC INT NOT NULL," \
     "LOAD_AVG FLOAT NOT NULL);";

     sqlite_exec(command);


}




// MAIN WEBSITE FUNCTIONS
//#############################################################################



/*
 * Adds a new session table
 * 
 */

void session_init(std::string& session_token) {

	std::cout << "Checking if session already exists\n";

	if (is_session_already_present(session_token) == false) {
		create_table(session_token);
	} else return;

}


/*
 * Gets a list of the hostnames available for this session
 */
void list_hosts(std::vector<std::string>& hosts, std::string& session_token) {
	
	hosts = {"host1","host2"};
		
		
		
}



// MAIN
//#############################################################################
int main() {

	// HTTP
	httplib::Server svr;

	// This is where all the machines report to with a key 
	svr.Get("/report", [](const httplib::Request &, httplib::Response &res) {

		// Get session ID from request	
		res.set_content("Received report","text/plain");	

		// Validate session ID(check if its in db)
			// Insert data for this machine for this session ID
		

		// Return success message to user
			
	});

	// Remove a session from the list
	svr.Get("/delete", [](const httplib::Request &, httplib::Response &res) {
	 
		// Get session ID from request
		
		// Delete session table from db	

		// Respond w/ success message
	
	});

	// Serve data from the database for a hostname for this session
	svr.Post("/fetch", [](const httplib::Request &, httplib::Response &res) {
	  
		
		std::string command = "SELECT name FROM sqlite_schema WHERE type='table' ORDER by name";
	
		// Get session ID from request	
		res.set_content("Received report","text/plain");	

		// Validate session ID(check if its in db)
	
		// Get requested time period

		// Fetch data for machines under for this session ID
		// in requested time-frame

		// Return data to user
	


	});

	// Register a new session on the website by the user(returns session key to user that they can
	// use to publish data
	svr.Get("/reg", [](const httplib::Request &, httplib::Response &res) {
 
		// Generate unique session key that does not yet exist among tables	
		std::string token="";
		generate_session_token(token);

		// Return session ID to user
		res.set_content(token, "text/raw");

	});



	// List all the machine names available
	svr.Post("/list", [](const httplib::Request &, httplib::Response &res) {

		std::string sess_name = "test_session";
		// Get session ID from request


		// Get machine names for this session
		std::vector<std::string> hosts = {};
		std::string session_token = "test_session";
		list_hosts(hosts, session_token);

		// Placeholder for JSON reply
		nlohmann::json response;

		// Convert to JSON, then serialize in in reply
		ConvertToJson(hosts, response);	
		res.set_content(response.dump(), "text/json");

		
	});


	// Mount / to ./www directory
	auto ret = svr.set_mount_point("/", "../www");
	if (!ret) {
	  // The specified base directory doesn't exist...
	  std::cout << "Somethings fucked, dir doesnt exist" << std::endl;
	}

	svr.listen("127.0.0.1", 8080);

	return 0;
}
