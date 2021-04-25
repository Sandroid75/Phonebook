#include "phonebook.h"

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    DBnode_t *db;
    int i = 0;

    NotUsed = 0;
 
    if(!argv) { //if argv is false means that DataBase doesn't exist
        return 0; //do nothing
    }

    db = (DBnode_t *) malloc(sizeof(DBnode_t)); //reserve memory for db
    if(!db) {
        logfile("%s: Error allocating memory\n", __func__);

        return -1;
    }
    //read the entire row of sqlite file and put the value in each db variable
    db->delete = false; //always set delete flag to false
    db->modified = false; //always set modified flag to false
    db->id = atoi(argv[i++]); 

    db->fname   		= sdsnew(argv[i++]);
    db->lname	        = sdsnew(argv[i++]);
    db->organization    = sdsnew(argv[i++]);
    db->job	    	    = sdsnew(argv[i++]);
    db->hphone	        = sdsnew(argv[i++]);
    db->wphone	        = sdsnew(argv[i++]);
    db->pmobile	        = sdsnew(argv[i++]);
    db->bmobile	        = sdsnew(argv[i++]);
    db->pemail	        = sdsnew(argv[i++]);
    db->bemail	        = sdsnew(argv[i++]);
    db->address         = sdsnew(argv[i++]);
    db->zip             = sdsnew(argv[i++]);
    db->city            = sdsnew(argv[i++]);
    db->state           = sdsnew(argv[i++]);
    db->country         = sdsnew(argv[i++]);

    db->birthday.tm_mday = atoi(argv[i++]);
    db->birthday.tm_mon  = atoi(argv[i++]);
    db->birthday.tm_year = atoi(argv[i++]);

    addNode(&contacts, (*db)); //for each readed row push the assigned db in the global contacts list
    destroyNode(&db); //destroy the node and all sds strings

    return 0;
}

int read_db(void) { //read existing SQL database or creat a new if doesn't exist, return -1 on error, 0 if is new db, or numbers of fields readed
    sqlite3 *sqlDB; //sqlite variables
    char *sql_err_msg;
    int rc;

    if(access(DB, R_OK)) {
        logfile("%s: The DataBase %s doesn't exist\n", __func__, DB);

        return 0;
    }

    rc = sqlite3_open(DB, &sqlDB); //Open database
    if(rc != SQLITE_OK) {
        logfile("%s: Can't open database: %s\n", __func__, sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);

        return -1; //error return code
    }

    rc = sqlite3_exec(sqlDB, SELECT_FROM_CONTACTS, callback, 0, &sql_err_msg); //Execute SQL statement and call function callback to read the database rows by  SQL
    if(rc != SQLITE_OK) {
        logfile("%s: SQL error: %s\n", __func__, sql_err_msg);
        sqlite3_free(sql_err_msg);

        return -1; //error return code
    } else {
        rc = countList(contacts);
    }
    sqlite3_close(sqlDB); //close SQL database
    REWIND(contacts); //to be sure that contacts pointer go to head of the list
    
    return rc;
}

int write_db(_Bool update) {
    PhoneBook_t *ptr;
    int bytes, records = 0;
    sds sql;

    if(!contacts) {
        logfile("%s: No records in contact to save!\n", __func__);

        return records;
    }

    if(access(DB, R_OK) == 0) { //if the file DataBase exist
        remove(DBAK); //delete the sqlitefile backup file
        bytes = filecopy(DB, DBAK); //copy the current db file in backup file before write it
        if(bytes < 0) {
            logfile("%s: Error copying '%s' to '%s'\n", __func__, DB, DBAK);

            return -1;
        }
        logfile("%s: %d byte copyed from '%s' to '%s'\n", __func__, bytes, DB, DBAK);
        sql = sdsempty(); //init the sql statement for existing DataBase
    } else { //the file DataBase exist
        sql = sdsnew(DEFAULT_SQL_TABLE); //no DataBase exist, than create new table using default schema
    }

    REWIND(contacts); //to be sure that contacts pointer go to head of the list
    for(ptr = contacts; ptr; ptr = ptr->next) { //walk thru the complete contacts list
        //build the SQLite3 statement to manage the contacts list in SQL DataBase
        if(ptr->db.delete) { //check if the contacts need to be deleted
            sql = sdscatprintf(sql, DELETE_SQL_RECORD_ID, ptr->db.id); //the id is used for statement WHERE SQLite
            logfile("%s: Record id=%d deleted...\n", __func__, ptr->db.id);
            records++; //counting the numbers of records to be write or update
        } else if(ptr->db.modified) { //check if the contacts was modified
            sql = update ? SDSupdateSQL(sql, ptr->db) : SDSinsertSQL(sql, ptr->db); //check if the use the UPDATE statement or INSERT statement
            ptr->db.modified = false; //set the flag to false before push the contact into tobeupdate list
            records++; //counting the numbers of records to be write or update
        } 
    }

    if(records) {
        if(doSQLstatement(sql) != SQLITE_OK) { //Execute the SQL statement and check that everything went well
            logfile("%s: Error %s %d records in the DataBase\n", __func__, update ? "updating" : "writing", records);
        } else { //re-align the stored DB with memory DB
            destroyList(&contacts); //in order to be alligned the memory list where destroy
            bytes = read_db(); //in order to be alligned the stored list where re-readed
            logfile("%s: Re-readed %d contacts from %s DataBase\n", __func__, bytes, DB);
        }
    }
    sdsfree(sql); //free memory
    REWIND(contacts); //to be sure that contacts pointer go to head of the list
    
    return records;
}

sds SDSinsertSQL(sds sql, DBnode_t node) { //build INSERT statement from node
    sql = sdscatprintf(sql, INSERT_SQL_VALUE, 
            node.id, node.fname, node.lname, node.organization, node.job,
            node.hphone, node.wphone, node.pmobile, node.bmobile, node.pemail, node.bemail,
            node.address, node.zip, node.city,node.state, node.country,
            node.birthday.tm_mday, node.birthday.tm_mon,node.birthday.tm_year);

    return (sds) sql;
}

sds SDSupdateSQL(sds sql, DBnode_t node) { //build UPDATE statement from node
    sql = sdscatprintf(sql, UPDATE_SQL_ROW,
            node.fname, node.lname, node.organization, node.job,
            node.hphone, node.wphone, node.pmobile, node.bmobile, node.pemail, node.bemail,
            node.address, node.zip, node.city, node.state, node.country,
            node.birthday.tm_mday, node.birthday.tm_mon, node.birthday.tm_year,
            node.id); //the id is used for statement WHERE SQLite 

    return (sds) sql;
}

int doSQLstatement(sds sql) {
    sqlite3 *sqlDB; //sqlite variables
    char *sql_err_msg;
    int rc;

    rc = sqlite3_open(DB, &sqlDB); //Open existing database
    if(rc != SQLITE_OK) {
        logfile("%s: Can't open database: %s\n", __func__, sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);
        sqlite3_free(sql_err_msg);

        return rc;
    }

    rc = sqlite3_exec(sqlDB, sql, NULL, 0, &sql_err_msg); //Execute SQL statement and call function to update the date to file
    if(rc != SQLITE_OK) { //check that everything went well
        logfile("%s: SQL error: %s\n", __func__, sql_err_msg);
        sqlite3_free(sql_err_msg);
    }

    sqlite3_close(sqlDB); //close SQL database

    return rc;
}

int write_csv(const char *csv_file, PhoneBook_t *contact_csv) {
    PhoneBook_t *ptr = contact_csv;
    FILE *fp;
    sds csv_table = sdsempty();
    int rows;
    size_t bytes;

    if(access(csv_file, R_OK)) { //if the csv_file doesn't exist
        csv_table = sdsnew(CSV_HEADER); //build the CSV header
    }
    REWIND(ptr);
    for(rows = 0; ptr; ptr = ptr->next, rows++) { //build the CSV table with rows
        csv_table = sdscatprintf(csv_table, CSV_SCHEMA, 
                                            ptr->db.id,
                                            ptr->db.fname,
                                            ptr->db.lname,
                                            ptr->db.organization,
                                            ptr->db.job,
                                            ptr->db.hphone,
                                            ptr->db.wphone,
                                            ptr->db.pmobile,
                                            ptr->db.bmobile,
                                            ptr->db.pemail,
                                            ptr->db.bemail,
                                            ptr->db.address,
                                            ptr->db.zip,
                                            ptr->db.city,
                                            ptr->db.state,
                                            ptr->db.country,
                                            ptr->db.birthday.tm_mday,
                                            ptr->db.birthday.tm_mon,
                                            ptr->db.birthday.tm_year);
    }

    fp = fopen(csv_file, "a"); //open or create for appending
    if(!fp) {
        logfile("%s: error creating %s\n", __func__, csv_file);

        return -1;
    }    
    bytes = sdslen(csv_table); //calculate how many bytes to writes
    while(bytes) { //while there is bytes to write
        bytes = fwrite(csv_table, sizeof(char), bytes, fp); //writes the table bytes
        sdsrange(csv_table, bytes, -1); //remove the writed byte from the string
        bytes = sdslen(csv_table); //calculate if remains bytes to writes
    }

    fclose(fp); //close the file pointer
    sdsfree(csv_table); //free memory

    return rows; //returns the number of rows writed
}