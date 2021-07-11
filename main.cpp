#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <sqlite3.h>
#include <unordered_set>

/*
 * Callback function for sqlite 
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
void sqlite_list_tables(std::unordered_set<std::string>& tables, std::string& session_token) {


	tables = {"session1","session2"};


}

/* 
 * Checks if that session ID is already registered in DB
 */

bool is_table_present(std::string session_token) {

	std::unordered_set<std::string> tables = {};

	// This is just an unordered map with a list of registered hosts 
	sqlite_list_tables(tables, session_token);
	std::unordered_set<std::string,double>::const_iterator got = tables.find(session_token);

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

	std::string command = "CREATE TABLE " + session_token + "("  \
      "ID INT PRIMARY KEY AUTOINCREMENT NOT NULL," \
      "LABEL TEXT NOT NULL," \
      "RAM_PERC INT NOT NULL," \
      "LOAD_AVG FLOAT NOT NULL);";

      sqlite_exec(command);


}

/*
 * Adds a new session table
 * 
 */

void sqlite_init(std::string& session_token) {

	if (is_table_present(session_token) == false) {
		create_table(session_token);
	} else return;

}

int main() {

	// HTTP
	httplib::Server svr;

	// This is where all the machines report to with a key 
	svr.Get("/report", [](const httplib::Request &, httplib::Response &res) {
	
		res.set_content("Received report","text/plain");	

			
	});

	// Remove a session from the list
	svr.Get("/delete", [](const httplib::Request &, httplib::Response &res) {
	  res.set_content("Hello World!", "text/plain");
	});

	// Serve data from the database for a hostname for this session
	svr.Get("/fetch", [](const httplib::Request &, httplib::Response &res) {
	  res.set_content("Hello World!", "text/plain");
	});

	// List all the machine names available
	svr.Get("/list", [](const httplib::Request &, httplib::Response &res) {
	  res.set_content("Hello World!", "text/plain");
	});








	// Mount / to ./www directory
	auto ret = svr.set_mount_point("/", "./www");
	if (!ret) {
	  // The specified base directory doesn't exist...
	  std::cout << "Somethings fucked, dir doesnt exist" << std::endl;
	}

	// Remove mount /
	//ret = svr.remove_mount_point("/");

	// Remove mount /public
	//ret = svr.remove_mount_point("/public");

	svr.listen("127.0.0.1", 8080);

	return 0;
}
