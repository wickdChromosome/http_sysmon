#include <iostream>
#include <algorithm>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <sqlite3.h>
#include <nlohmann/json.hpp>


// SQLITE HELPER FUNCTIONS
//#############################################################################

class SqliteCallback {


	public:

	/*
	 * Callback function for sqlite select 
	 */
	static int select_callback(void *data, int argc, char **argv, char **azColName) {

		std::unordered_multimap<std::string,std::string>* results = 
			reinterpret_cast<std::unordered_multimap<std::string,std::string>*>(data);

		int i;
		fprintf(stderr, "%s: ", (const char*)data);

		for(i = 0; i<argc; i++){

			std::string coln = azColName[i];
			std::string argn = argv[i];	
			(*results).insert({ {coln,argn} });	

		}

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
};

/*
 * Generic helper for executing select sqlite commands
 *
 */
bool sqlite_select_exec(std::unordered_multimap<std::string,std::string>* results , std::string& command) {

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	bool status = false;

	rc = sqlite3_open("file::memory:", &db);

	if( rc ) {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	return false;
	} 

	SqliteCallback sqlite_callback;

	/* Execute SQL statement */
	rc = sqlite3_exec(db, command.c_str(), sqlite_callback.select_callback,results , &zErrMsg);

	if( rc != SQLITE_OK ){
	status = false;
	//fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);

	} else {
	status = true;
	// fprintf(stdout, "Records created successfully\n");
	}
	sqlite3_close(db);

	// To disable warning
	return status;

}



/*
 * Generic helper for executing sqlite commands
 *
 */
bool sqlite_exec(std::string& command) {

	bool status;
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("file::memory:", &db);

	if( rc ) {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	return false;
	} 

	SqliteCallback common_callback;

	/* Execute SQL statement */
	rc = sqlite3_exec(db, command.c_str(), common_callback.callback, 0, &zErrMsg);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		status = false;

	} else {
		fprintf(stdout, "Records created successfully\n");
		status = true;
	}
	sqlite3_close(db);

	return status;
}



/*
 * Gets a list of tables from the database
 *
 */
bool list_tables(std::vector<std::string>& tables) {

	std::string command = "SELECT name FROM sqlite_schema WHERE type='table' ORDER by name";
	std::unordered_multimap<std::string,std::string>* results = new std::unordered_multimap<std::string,std::string>;	
	if (sqlite_select_exec( results, command) == true) { 


		std::unordered_multimap<std::string,std::string> results_deref = *results;

		for (auto itr = results_deref.begin(); itr != results_deref.end(); itr++)   
			tables.push_back(itr->second);

		delete results;

	} else return false;

	// to disable warning
	return false;
}




/* 
 * Checks if that session ID is already registered in DB
 */
bool is_session_already_present(std::string session_token) {

	std::vector<std::string> tables = {};

	// This is just a string with a list of registered sessions 
	list_tables(tables);
	if ( std::find(tables.begin(), tables.end(), session_token) == tables.end() ) {

		// hostname not already present
		return false;

	} else return true;




}

/* 
* Add a new session table
*
*/

bool create_table(std::string& session_token) {

       std::string command = "CREATE TABLE '" + session_token + "'("  \
     "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
     "HOSTNAME TEXT NOT NULL," \
     "RAM_PERC FLOAT NOT NULL," \
     "UNIX_TIME INT NOT NULL," \
     "LOAD_AVG FLOAT NOT NULL);";

     if (sqlite_exec(command) == true) {

     	return true; 

     } else return false;


}


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

void ConvertMmapToJson(std::unordered_multimap<std::string,std::string>& in_map,
		nlohmann::json& jsonobj) {

	for(auto& element : in_map) {
		
		jsonobj[element.first].push_back(element.second);

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

void generate_session_token(std::string& token_placeholder) {

	// Set it to true to keep going until we can find
	// a string that doesn't exist yet
	bool token_already_exists = true;

	while(token_already_exists) {
	
		// Generate a token
		std::string access_token = random_string(32);

		// If this session ID doesn't exist yet, give it to the user
		if (is_session_already_present(access_token) == false) {
		
			token_placeholder = access_token;
			token_already_exists = false;

		}
			
	}


}

// MAIN WEBSITE FUNCTIONS
//#############################################################################



/*
 * Adds a new session table
 * 
 */
bool session_init(std::string& session_token) {

	if (is_session_already_present(session_token) == false) {
		
		if (create_table(session_token) == true) {
			return true;
		}
	} else return false;

	// To disable warning
	return false;
}


/*
 * Gets a list of the hostnames available for this session
 */
void list_hosts(std::vector<std::string>& hosts, std::string& session_token) {
	
	std::string command = "SELECT DISTINCT HOSTNAME FROM " + session_token;
	std::unordered_multimap<std::string,std::string>* results = new std::unordered_multimap<std::string,std::string>;	
	sqlite_select_exec( results, command);
	std::unordered_multimap<std::string,std::string> results_deref = *results;

	// Now fill the vector to be returned
	for (auto& result: results_deref) {

		hosts.push_back(result.second);

	}

	delete results;



}



/*
 * Fetches the latest data from the db for a requested machine
 *
 */

bool fetch_latest(std::unordered_multimap<std::string,std::string>* results, std::string& session_token, std::string& hostname) {

	std::string command = "SELECT HOSTNAME,LOAD_AVG,RAM_PERC,MAX(UNIX_TIME) FROM '" + session_token + "' WHERE HOSTNAME='" + hostname + "' LIMIT 1;";

	
	if (sqlite_select_exec( results, command) == true) {
	
		return true;



	} else return false;

}

/*
 * Insert new data for this session
 *
 */

bool insert_entry(std::string& session_token,
	       	std::string& hostname,
		std::string& unixtime,
		std::string& loadavg,
		std::string& ramperc) {

	std::string command = "INSERT INTO '" + session_token + "' ( UNIX_TIME , LOAD_AVG , RAM_PERC , HOSTNAME ) VALUES" +
	      			"( '" + unixtime + "' , '"
				    + loadavg + "' , '"
				    + ramperc + "' , '"  
				    + hostname + "' ); "; 

	if ( sqlite_exec( command ) == true ) {
	
		return true;			
	
	} else return false;


}



// MAIN
//#############################################################################
int main() {

	// HTTP
	httplib::Server svr;

	// This is where all the machines report to with a key 
	svr.Post("/report", [](const httplib::Request& req, httplib::Response &res) {

		std::string token = "";
		std::string hostname = "";
		std::string unixtime = "";
		std::string loadavg = "";
		std::string ramperc = "";

		// Check if token present in request
		if (req.has_param("access_token") && 
				req.has_param("host_name") &&
				req.has_param("unix_time") &&
				req.has_param("load_avg") &&
				req.has_param("ram_perc") ) {
		
			token = req.get_param_value("access_token");
			hostname = req.get_param_value("host_name");
			unixtime = req.get_param_value("unix_time");
			loadavg = req.get_param_value("load_avg");
			ramperc = req.get_param_value("ram_perc");

		// Throw error on client side
		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Token or hostname doesn't exist" << std::endl;}


		// Validate session ID(check if its in db)
		if (is_session_already_present(token) == true) {
	
			// Insert data for this machine for this session ID
			if ( insert_entry(token, 
					hostname,
					unixtime,
					loadavg,
					ramperc) == true) {
				
				res.set_content("{'type':{'Success'}}","text/json");
		
			}	

		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Unable to insert to db" << std::endl;}


			
	});


	// Remove a session from the list
	svr.Get("/list-sessions", [](const httplib::Request &, httplib::Response &res) {
	
		std:: cout << "Listing sessions..\n"; 	
		std::vector<std::string> tables = {};
		if (list_tables(tables) == false) res.set_content("{'type':{'Success'}}","text/json");
		else  res.set_content("{'type':{'Error'}}","text/json");

		// Placeholder for JSON reply
		nlohmann::json response;
		
		// Convert to JSON, then serialize in in reply
		ConvertToJson(tables, response);	
		res.set_content(response.dump(), "text/json");



	});

	// Remove a session from the list
	svr.Get("/delete", [](const httplib::Request &, httplib::Response &res) {
	 
		// Get session ID from request
		
		// Delete session table from db	

		// Respond w/ success message
	
	});

	// Serve data from the database for a hostname for this session
	svr.Post("/fetch", [](const httplib::Request& req, httplib::Response &res) {

		std::string token = "";
		std::string hostname = "";

		// Check if token present in request
		if (req.has_param("access_token") && req.has_param("host_name")) {
		
			token = req.get_param_value("access_token");
			hostname = req.get_param_value("host_name");

		// Throw error on client side
		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Token or hostname doesn't exist" << std::endl;}

		// Validate session ID(check if its in db)
		if (is_session_already_present(token) == true){

			// Fetch data for machines under for this session ID
			std::unordered_multimap<std::string,std::string>* results;
			results = new std::unordered_multimap<std::string,std::string>;	

			if (fetch_latest(results, token, hostname) == true) {
		
				std::unordered_multimap<std::string,std::string> results_deref = *results;
				// Placeholder for JSON reply
				nlohmann::json response;

				// Convert to JSON, then serialize in in reply
				ConvertMmapToJson(results_deref, response);	

				res.set_content(response.dump(), "text/json");
			} 
		
		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Token or hostname doesn't exist" << std::endl;}


	});

	// Register a new session on the website by the user(returns session key to user that they can
	// use to publish datasvr.Get("/reg", [](const httplib::Request &, httplib::Response &res) {
	svr.Get("/gentoken", [](const httplib::Request &, httplib::Response &res) {
		
		// Generate unique session key that does not yet exist among tables	
		std::string token="";
		generate_session_token(token);

		// Return session ID to user
		res.set_content(token, "text/raw");

	});

	svr.Post("/reg", [](const httplib::Request &req, httplib::Response &res) {
		
		std::string token = "";

		// Check if token present in request
		if (req.has_param("access_token")) {
		
			token = req.get_param_value("access_token");

		// Throw error on client side
		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Token Doesn't exist" << std::endl;}
	
		// Register new session and return token to client
		if (session_init(token) == true) {res.set_content("{'type':{'Success'}}", "text/json");std::cout << "added token" << std::endl;}
		else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Unable to add token" << std::endl;}


	});

	

	// List all the machine names available
	svr.Post("/list", [](const httplib::Request& req, httplib::Response &res) {

		std::string token = "";
		
		// Check if token present in request
		if (req.has_param("access_token")) {
		
			token = req.get_param_value("access_token");

		// Throw error on client side
		} else {res.set_content("{'type':{'Error'}}","text/json");std::cout << "Token Doesn't exist" << std::endl;}


		// Get machine names for this session
		std::vector<std::string> hosts = {};
		list_hosts(hosts, token);

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
