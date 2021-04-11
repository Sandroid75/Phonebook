#include "phonebook.h"

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    DBnode_t *db;
    int i;

    NotUsed = 0;

    //read the entire row of sqlite file and put the value in each db variable
    i = 0;

    db = (DBnode_t *) malloc(sizeof(DBnode_t)); //reserve memory for db
    if(!db) {
        logfile("%s: Error allocating memory\n", __func__);

        return -1;
    }

    db->delete = false; //always set delete flag to false
    db->modified = false; //always set modified flag to false

    db->id = argv ? atoi(argv[i++]) : 0; //if argv is NULL means that it's a new file and filed need to be edit

    db->fname   		= argv ? sdsnew(argv[i++]) : sdsnewlen("", STEXT);
    db->lname	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", STEXT);
    db->organization    = argv ? sdsnew(argv[i++]) : sdsnewlen("", MTEXT);
    db->job	    	    = argv ? sdsnew(argv[i++]) : sdsnewlen("", STEXT);
    db->hphone	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", PHONE);
    db->wphone	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", PHONE);
    db->pmobile	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", PHONE);
    db->bmobile	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", PHONE);
    db->pemail	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", MTEXT);
    db->bemail	        = argv ? sdsnew(argv[i++]) : sdsnewlen("", MTEXT);
    db->address         = argv ? sdsnew(argv[i++]) : sdsnewlen("", LTEXT);
    db->zip             = argv ? sdsnew(argv[i++]) : sdsnewlen("", ZIP);
    db->city            = argv ? sdsnew(argv[i++]) : sdsnewlen("", MTEXT);
    db->state           = argv ? sdsnew(argv[i++]) : sdsnewlen("", STATE);
    db->country         = argv ? sdsnew(argv[i++]) : sdsnewlen("", STEXT);

    db->birthday.tm_mday = argv ? atoi(argv[i++]) : 1;
    db->birthday.tm_mon  = argv ? atoi(argv[i++]) : 1;
    db->birthday.tm_year = argv ? atoi(argv[i++]) : 1900;

    addNode(&contacts, db); //for each readed row push the assigned db in the global contacts list

    destroyNode(db); //destroy the node and all sds strings

    return 0;
}

int read_db(void) { //read existing SQL database or creat a new if doesn't exist, return -1 on error, 0 if is new db, or numbers of fields readed
    sqlite3 *sqlDB; //sqlite variables
    char *sql_err_msg;
    int rc;
    //_Bool isnew;

    if(access(DB, R_OK)) {
        callback(NULL, 0, NULL, NULL); //callback without parameters initialize contacts list and set id to 0 (menas new database)

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
        logfile("%s: File database is successfully opened, %d contacts readed...\n", __func__, rc);
    }


    sqlite3_close(sqlDB); //close SQL database
    
    return rc;
}

PhoneBook_t *newNode(DBnode_t *node) {
    PhoneBook_t *dbNode = (PhoneBook_t *) malloc(sizeof(PhoneBook_t));

    if(!dbNode) {
        logfile("%s: Memory allocation error\n", __func__);

        return dbNode;
    }

    //copy all contents from DBnode passed from the call of addNode() function    
    dbNode->db.delete       = node->delete;
    dbNode->db.modified     = node->modified;
    dbNode->db.id           = node->id;

    dbNode->db.fname   		= sdsnew(node->fname);
    dbNode->db.lname	    = sdsnew(node->lname);
    dbNode->db.organization = sdsnew(node->organization);
    dbNode->db.job	    	= sdsnew(node->job);
    dbNode->db.hphone	    = sdsnew(node->hphone);
    dbNode->db.wphone	    = sdsnew(node->wphone);
    dbNode->db.pmobile	    = sdsnew(node->pmobile);
    dbNode->db.bmobile	    = sdsnew(node->bmobile);
    dbNode->db.pemail	    = sdsnew(node->pemail);
    dbNode->db.bemail	    = sdsnew(node->bemail);
    dbNode->db.address      = sdsnew(node->address);
    dbNode->db.zip          = sdsnew(node->zip);
    dbNode->db.city         = sdsnew(node->city);
    dbNode->db.state        = sdsnew(node->state);
    dbNode->db.country      = sdsnew(node->country);

    dbNode->db.birthday.tm_mday = node->birthday.tm_mday;
    dbNode->db.birthday.tm_mon  = node->birthday.tm_mon;
    dbNode->db.birthday.tm_year = node->birthday.tm_year;

    NULLSET(dbNode->prev); //set prev node to NULL pointer
    NULLSET(dbNode->next); //set next node to NULL pointer

    return (PhoneBook_t *) dbNode;
}

PhoneBook_t *addNode(PhoneBook_t **list, DBnode_t *node) { //push the db data in global contacts list in sorted position
    PhoneBook_t *dbNode = newNode(node);

    if(!dbNode) {
        logfile("%s: Memory allocation error\n", __func__);

        return dbNode;
    }

    if(!(*list)) { //if no contacts lists exist
        (*list) = dbNode; //the node will be the only node in the list, no more thing to do than return
    } else {
        //contacts list exist now we have to found the right place to push the new node
        FORWARD((*list)); //forward the list up to the last node
        (*list)->next = dbNode;
        dbNode->prev = (*list);
    }

    return (PhoneBook_t *) dbNode; //return the pointer to the new node
}

void deleteNodeID(int id) {
    PhoneBook_t *ptr = contacts;

    REWIND(ptr);
    while(ptr && ptr->db.id != id) { //walk up to id to del or end of list
        NEXT(ptr); //step next
    }
    if(ptr) {
        if(ptr->next) {
            ptr->next->prev = ptr->prev; //assign the next pointer of prev node to next node
        } 
        if(ptr->prev) {
            ptr->prev->next = ptr->next; //assign the prev pointer of next node to prev node
        }
        NULLSET(ptr->prev); //in order to isolate the note to delete
        NULLSET(ptr->next); //in order to isolate the note to delete
        destroyList(ptr); //destroy the node from contacts list
        NULLSET(ptr);
    }

    return;
}

DBnode_t *initNode(PhoneBook_t *ptrList) {
    DBnode_t *node;

    node = (DBnode_t *) malloc(sizeof(DBnode_t)); //reserve memory for db
    if(!node) {
        logfile("%s: Error allocating memory\n", __func__);

        return node;
    }

    node->delete = false; //always set delete flag to false
    node->modified = false; //always set modified flag to false
    FORWARD(ptrList);
    node->id = ptrList ? (ptrList->db.id +1) : 1; //if ptrList is NULL means that it's a new list

    node->fname   		= sdsnewlen("", STEXT);
    node->lname	        = sdsnewlen("", STEXT);
    node->organization  = sdsnewlen("", MTEXT);
    node->job	    	= sdsnewlen("", STEXT);
    node->hphone	    = sdsnewlen("", PHONE);
    node->wphone	    = sdsnewlen("", PHONE);
    node->pmobile	    = sdsnewlen("", PHONE);
    node->bmobile	    = sdsnewlen("", PHONE);
    node->pemail	    = sdsnewlen("", MTEXT);
    node->bemail	    = sdsnewlen("", MTEXT);
    node->address       = sdsnewlen("", LTEXT);
    node->zip           = sdsnewlen("", ZIP);
    node->city          = sdsnewlen("", MTEXT);
    node->state         = sdsnewlen("", STATE);
    node->country       = sdsnewlen("", STEXT);

    node->birthday.tm_mday = 1;
    node->birthday.tm_mon  = 1;
    node->birthday.tm_year = 1900;

    return (DBnode_t *) node;
}

int countList(PhoneBook_t *ptr) {
    int count = 0;

	REWIND(ptr); //rewind the list up to the first node
	while(ptr) {
		NEXT(ptr);
		count++;
	}

    return count;
}

void destroyNode(DBnode_t *ptr) {
    if(ptr) {
		//free all sds strings
		sdsfree(ptr->fname);
        sdsfree(ptr->lname);
        sdsfree(ptr->organization);
        sdsfree(ptr->job);
        sdsfree(ptr->hphone);
        sdsfree(ptr->wphone);
        sdsfree(ptr->pmobile);
        sdsfree(ptr->bmobile);
        sdsfree(ptr->pemail);
        sdsfree(ptr->bemail);
        sdsfree(ptr->address);
        sdsfree(ptr->zip);
        sdsfree(ptr->city);
        sdsfree(ptr->state);
        sdsfree(ptr->country);

        free(ptr); //destroy the node
    }

	return;
}

void destroyList(PhoneBook_t *list) {
	PhoneBook_t *nextPtr, *ptr = list;

    REWIND(ptr); //rewind the list up to the first node
    while(ptr) { //walk thru complete contacts list
        nextPtr = ptr->next; //store point nextPtr to next node before destroy it

		//free all sds strings in each node
		sdsfree(ptr->db.fname);
        sdsfree(ptr->db.lname);
        sdsfree(ptr->db.organization);
        sdsfree(ptr->db.job);
        sdsfree(ptr->db.hphone);
        sdsfree(ptr->db.wphone);
        sdsfree(ptr->db.pmobile);
        sdsfree(ptr->db.bmobile);
        sdsfree(ptr->db.pemail);
        sdsfree(ptr->db.bemail);
        sdsfree(ptr->db.address);
        sdsfree(ptr->db.zip);
        sdsfree(ptr->db.city);
        sdsfree(ptr->db.state);
        sdsfree(ptr->db.country);

        free(ptr); //now to ptr elment can be free
        ptr = nextPtr; //step to next node
    }

	return;
}

int write_db(WINDOW *win) {
    sqlite3 *sqlDB; //sqlite variables
    sds sql;
    PhoneBook_t *ptr = contacts; //point to actual position of contacts
    char *sql_err_msg;
    int rc, records;
    _Bool isnew = true;

    if(!ptr) {
        logfile("%s: No records in contact to save!\n", __func__);

        return 0;
    }

    if(access(DB, R_OK) == 0) { //if the database exist
        remove(DBAK); //delete the sqlitefile backup file
        rc = filecopy(DB, DBAK); //copy the current db file in backup file before write it
        if(rc < 0) {
            logfile("%s: Error copying '%s' to '%s'\n", __func__, DB, DBAK);

            return -1;
        }
        logfile("%s: %d byte copyed from '%s' to '%s'\n", __func__, rc, DB, DBAK);
        isnew = false; //the database file exists
    }

    REWIND(ptr); //rewind the list up to the first node
    if(isnew) {
        sql = sdsnew(DEFAULT_SQL_TABLE); //create table in db using default table
    } else {
        sql = sdsempty();
    }
    for(records = 0; ptr; ptr = ptr->next) { //walk thru complete contacts list nodes
        //build the SQLite3 statement to insert the contacts list in SQL database
        if(ptr->db.modified) { //only modified records will be writed
            sql = sdscatprintf(sql, "%s (%u, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d);", 
                                INSERT_SQL_VALUE,
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
            ptr->db.modified = false; //reset the flag to false because os storing
            records++;
        }
    }

    if(!records) {
        logfile("%s: No records to save!\n", __func__);
        sdsfree(sql);

        return 0;
    }

    rc = sqlite3_open(DB, &sqlDB); //Open database and create a new one with tables
    if (rc != SQLITE_OK) {
        logfile("%s: Can't open database: %s\n", __func__, sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);

        return -1;
    }
    
    rc = sqlite3_exec(sqlDB, sql, NULL, 0, &sql_err_msg); //Execute SQL statement and call function to write the date to file

    if(rc != SQLITE_OK) { //databese was correctly writed
        logfile("%s: SQL error: %s\n", __func__, sql_err_msg);
        sqlite3_free(sql_err_msg);
    } else {
        logfile("%s: File database is successfully writed, %d contacts...\n", __func__, records);
    }

    sqlite3_close(sqlDB); //close SQL database
    sdsfree(sql); //free memory
    
    return records;
}

int update_db(WINDOW *win) {
    sqlite3 *sqlDB; //sqlite variables
    PhoneBook_t *ptr, *tobeupdate = NULL;
    char *sql_err_msg;
    int rc;
    sds sql = sdsempty(); //used for sql statement

    if(access(DB, R_OK)) { //if the database doesn't exist
        rc = write_db(win); //create a new ones

        return rc; //than return
    }
    remove(DBAK); //delete the sqlitefile backup file
    rc = filecopy(DB, DBAK); //copy the current db file in backup file before update it
    if(rc < 0) {
        logfile("%s: Error copying '%s' to '%s'\n", __func__, DB, DBAK);

        return -1;
    }
    logfile("%s: %d byte copyed from '%s' to '%s'\n", __func__, rc, DB, DBAK);

    ptr = contacts; //point to actual position of contacts
    REWIND(ptr); //rewind the list up to the first node of contacts list
    while(ptr) { //check if ptr is valid pointers
        if(ptr->db.modified) { //check if the contacts was modified
            ptr->db.modified = false; //set the flag to false before push the contact into tobeupdate list
            if(!addNode(&tobeupdate, &(ptr->db))) { //build tobeupdate list and check the result
                destroyList(tobeupdate); //destroing the list before return
                logfile("%s: Error storing id=%d, the contact will not be updated...\n", __func__, ptr->db.id);

                return -1;
            }
        } else if(ptr->db.delete) { //check if the contacts need to be deleted
            if(!addNode(&tobeupdate, &(ptr->db))) { //build tobeupdate list and check the result
                destroyList(tobeupdate); //destroing the list before return
                logfile("%s: Error storing id=%d, the contact will not be deleted...\n", __func__, ptr->db.id);

                return -1;
            }
        }
        NEXT(ptr); //step next
    }
    if(!tobeupdate) { //check if there are contacts to be update
        logfile("%s: Nothing to update or delete\n", __func__);

        return 0; //no contacts to update
    }
    
    ptr = tobeupdate; //point to actual position of tobeupdate
    REWIND(ptr); //rewind the list up to the first node of tobeupdate list
    while(ptr) { //walk thru complete contacts list nodes
        //build the SQLite3 statement to insert the contacts list in SQL database
        if(ptr->db.delete) { //the record is set to delete
            sql = sdscatprintf(sql, DELETE_SQL_RECORD_ID, ptr->db.id); //the id is used for statement WHERE SQLite
            deleteNodeID(ptr->db.id); //delete node by id from the contacts list
            logfile("%s: Record id=%d deleted...\n", __func__, ptr->db.id);
        } else { //the record is set to update
            sql = sdscatprintf(sql, UPDATE_SQL_ROW,
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
                                    ptr->db.birthday.tm_year,
                                    ptr->db.id);                //the id is used for statement WHERE SQLite
            logfile("%s: Record id=%d updated...\n", __func__, ptr->db.id);
        }
        NEXT(ptr); //step to next node
    }

    rc = sqlite3_open(DB, &sqlDB); //Open existing database
    if(rc != SQLITE_OK) {
        logfile("%s: Can't open database: %s\n", __func__, sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);
        destroyList(tobeupdate);
        sdsfree(sql);

        return rc;
    }

    rc = sqlite3_exec(sqlDB, sql, NULL, 0, &sql_err_msg); //Execute SQL statement and call function to update the date to file
    sdsfree(sql); //destroy the sds after use

    if(rc != SQLITE_OK) { //check that everything went well
        logfile("%s: SQL error: %s id=%d unsuccessful\n", __func__, sql_err_msg, ptr->db.id);
        sqlite3_free(sql_err_msg);

        //not need return error code because the function is end anyway
    }
    sqlite3_close(sqlDB); //close SQL database
    destroyList(tobeupdate); //destroy the list

    return rc;
}

int filecopy(const char* source, const char* destination) {
    int input, output, result = -1;
    struct stat fileinfo = { 0 };
    off_t bytesCopied = 0;

    if((input = open(source, O_RDONLY)) == -1) { //try to open source file
        logfile("%s: Error opening '%s'\n", __func__, source);

        return result;
    }
    if((output = open(destination, O_CREAT|O_WRONLY|O_TRUNC, 0600)) == -1) { //try to create the destination file
        close(input);
        logfile("%s: Error creating '%s'\n", __func__, destination);

        return result;
    }

    fstat(input, &fileinfo); //get the file attributes
    result = sendfile(output, input, &bytesCopied, fileinfo.st_size); //sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+

    close(input); //close the handle
    close(output); //close the handle

    return result; //return the number of bytes copied
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
