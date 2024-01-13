#include "../../libcsv/csv.h"
#include "phonebook.h"
#include <sqlite3.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    DBnode_t *db;
    int i = 0;

    NotUsed = 0;

    if (!argv)    // if argv is false means that DataBase doesn't exist
        return 0; // do nothing

    db = (DBnode_t *) malloc(sizeof(DBnode_t)); // reserve memory for db
    if (!db) {
        log_perror("Error allocating memory");

        return -1;
    }
    // read the entire row of sqlite file and put the value in each db variable
    db->delete   = false; // always set delete flag to false
    db->modified = false; // always set modified flag to false
    db->id       = atoi(argv[i++]);

    db->fname        = sdsnew(argv[i++]);
    db->lname        = sdsnew(argv[i++]);
    db->organization = sdsnew(argv[i++]);
    db->job          = sdsnew(argv[i++]);
    db->hphone       = sdsnew(argv[i++]);
    db->wphone       = sdsnew(argv[i++]);
    db->pmobile      = sdsnew(argv[i++]);
    db->bmobile      = sdsnew(argv[i++]);
    db->pemail       = sdsnew(argv[i++]);
    db->bemail       = sdsnew(argv[i++]);
    db->address      = sdsnew(argv[i++]);
    db->zip          = sdsnew(argv[i++]);
    db->city         = sdsnew(argv[i++]);
    db->state        = sdsnew(argv[i++]);
    db->country      = sdsnew(argv[i++]);

    db->birthday.tm_mday = atoi(argv[i++]);
    db->birthday.tm_mon  = atoi(argv[i++]);
    db->birthday.tm_year = atoi(argv[i++]);

    addNode(&contacts, (*db)); // for each readed row push the assigned db in the global contacts list
    destroyNode(db);           // destroy the node and all sds strings

    return 0;
}

int read_db(void)
{                   // read existing SQL database or creat a new if doesn't exist, return -1 on error, 0 if is new db, or numbers of fields readed
    sqlite3 *sqlDB; // sqlite variables
    char *sql_err_msg;
    int rc;

    if (access(DB, R_OK)) {
        log_perror("The DataBase %s doesn't exist", DB);

        return 0;
    }

    rc = sqlite3_open(DB, &sqlDB); // Open database
    if (rc != SQLITE_OK) {
        log_perror("Can't open database: %s", sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);

        return -1; // error return code
    }

    rc = sqlite3_exec(sqlDB, SELECT_FROM_CONTACTS, callback, 0, &sql_err_msg); // Execute SQL statement and call function callback to read the database rows by  SQL
    if (rc != SQLITE_OK) {
        log_perror("SQL error: %s", sql_err_msg);
        sqlite3_free(sql_err_msg);

        return -1; // error return code
    } else
        rc = countList(contacts);

    sqlite3_close(sqlDB); // close SQL database
    REWIND(contacts);     // to be sure that contacts pointer go to head of the list

    return rc;
}

int write_db(_Bool update)
{
    PhoneBook_t *ptr, *pnext;
    int records = 0;
    ssize_t bytes;
    sds sql;

    if (!contacts) {
        log_warn("No records in contact to save!");

        return records;
    }

    if (access(DB, R_OK) == 0) {    // if the file DataBase exist
        unlink(DBAK);               // delete the sqlitefile backup file
        bytes = filecopy(DB, DBAK); // copy the current db file in backup file before write it
        if (bytes < 0) {
            log_perror("Error copying '%s' to '%s'", DB, DBAK);

            return -1;
        }
        log_info("%ld bytes copyed from '%s' to '%s'", (long) bytes, DB, DBAK);
        sql = sdsempty();                // init the sql statement for existing DataBase
    } else                               // the file DataBase exist
        sql = sdsnew(DEFAULT_SQL_TABLE); // no DataBase exist, than create new table using default schema

    REWIND(contacts);                        // to be sure that contacts pointer go to head of the list
    for (ptr = contacts; ptr; ptr = pnext) { // walk thru the complete contacts list
        pnext = ptr->next;                   // safe pointing to next node, in case of deletion of the node
        // build the SQLite3 statement to manage the contacts list in SQL DataBase
        if (ptr->db.delete) {                                          // check if the contacts need to be deleted
            sql = sdscatprintf(sql, DELETE_SQL_RECORD_ID, ptr->db.id); // the id is used for statement WHERE SQLite
            log_info("Record id=%d deleted...", ptr->db.id);
            deleteNode(&contacts, ptr);
            records++;                                                                           // counting the numbers of records to be write or update
        } else if (ptr->db.modified) {                                                           // check if the contacts was modified
            sql              = update ? SDSupdateSQL(sql, ptr->db) : SDSinsertSQL(sql, ptr->db); // check if the use the UPDATE statement or INSERT statement
            ptr->db.modified = false;                                                            // set the flag to false before push the contact into tobeupdate list
            records++;                                                                           // counting the numbers of records to be write or update
        }
    }

    if (records) {
        if (doSQLstatement(sql) != SQLITE_OK) // Execute the SQL statement and check that everything went well
            log_perror("Error %s %d records in the DataBase", update ? "updating" : "writing", records);
        else
            log_info("%d records %s in the DataBase", records, update ? "updated" : "writed");
    }
    sdsfree(sql);     // free memory
    REWIND(contacts); // to be sure that contacts pointer go to head of the list

    return records;
}

sds SDSinsertSQL(sds sql, DBnode_t node)
{ // build INSERT statement from node
    sql = sdscatprintf(sql, INSERT_SQL_VALUE,
                       node.id, node.fname, node.lname, node.organization, node.job,
                       node.hphone, node.wphone, node.pmobile, node.bmobile, node.pemail, node.bemail,
                       node.address, node.zip, node.city, node.state, node.country,
                       node.birthday.tm_mday, node.birthday.tm_mon, node.birthday.tm_year);

    return (sds) sql;
}

sds SDSupdateSQL(sds sql, DBnode_t node)
{ // build UPDATE statement from node
    sql = sdscatprintf(sql, UPDATE_SQL_ROW,
                       node.fname, node.lname, node.organization, node.job,
                       node.hphone, node.wphone, node.pmobile, node.bmobile, node.pemail, node.bemail,
                       node.address, node.zip, node.city, node.state, node.country,
                       node.birthday.tm_mday, node.birthday.tm_mon, node.birthday.tm_year,
                       node.id); // the id is used for statement WHERE SQLite

    return (sds) sql;
}

int doSQLstatement(sds sql)
{
    sqlite3 *sqlDB; // sqlite variables
    char *sql_err_msg;
    int rc;

    rc = sqlite3_open(DB, &sqlDB); // Open existing database
    if (rc != SQLITE_OK) {
        log_perror("Can't open database: %s", sqlite3_errmsg(sqlDB));
        sqlite3_close(sqlDB);
        sqlite3_free(sql_err_msg);

        return rc;
    }

    rc = sqlite3_exec(sqlDB, sql, NULL, 0, &sql_err_msg); // Execute SQL statement and call function to update the date to file
    if (rc != SQLITE_OK) {                                // check that everything went well
        log_perror("SQL error: %s", sql_err_msg);
        sqlite3_free(sql_err_msg);
    }

    sqlite3_close(sqlDB); // close SQL database

    return rc;
}

int write_csv(const char *csvFile, PhoneBook_t *contact_csv)
{
    PhoneBook_t *ptr = contact_csv;
    FILE *fp;
    sds csv_table;
    int rows;
    size_t bytes;
    _Bool isGoogle;

    REWIND(ptr);
    if (!ptr) {
        log_warn("There is no contacts to export in '%s'", csvFile);

        return -1;
    }

    isGoogle = strcmp(csvFile, GOOGLE_CSV) ? false : true; // check if is Google csv file

    if (access(csvFile, R_OK))                                         // if the csvFile doesn't exist
        csv_table = sdsnew(isGoogle ? CSV_GOOGLE_HEADER : CSV_HEADER); // build the CSV header
    else                                                               // the csvFile exist
        csv_table = sdsempty();                                        // init an empty table

    for (rows = 0; ptr; rows++) { // build the CSV table with rows
        if (isGoogle)
            csv_table = SDSgoogleCSV(csv_table, ptr->db); // insert each row from db node
        else
            csv_table = SDSinsertCSV(csv_table, ptr->db); // insert each row from db node

        NEXT(ptr); // step to next
    }

    fp = fopen(csvFile, "a"); // open or create for appending
    if (!fp) {
        log_perror("error creating %s", csvFile);

        return -1;
    }
    bytes = (size_t) sdslen(csv_table);                     // calculate how many bytes to writes
    while (bytes) {                                         // while there is bytes to write
        bytes = fwrite(csv_table, sizeof(char), bytes, fp); // writes the table bytes
        sdsrange(csv_table, bytes, -1);                     // remove the writed byte from the string
        bytes = (size_t) sdslen(csv_table);                 // calculate if remains bytes to writes
    }
    sdsfree(csv_table); // free memory

    if (ferror(fp)) {
        log_perror("error while writing file %s", csvFile);
        fclose(fp); // close the file pointer

        return -1;
    }

    fclose(fp); // close the file pointer

    return rows; // returns the number of rows writed
}

sds SDSinsertCSV(sds csvRow, DBnode_t node)
{ // build INSERT statement from node
    csvRow = sdscatprintf(csvRow, CSV_SCHEMA,
                          node.id, node.fname, node.lname, node.organization, node.job,
                          node.hphone, node.wphone, node.pmobile, node.bmobile, node.pemail, node.bemail,
                          node.address, node.zip, node.city, node.state, node.country,
                          node.birthday.tm_mday, node.birthday.tm_mon, node.birthday.tm_year);

    return (sds) csvRow;
}

sds SDSgoogleCSV(sds csvRow, DBnode_t node)
{ // build INSERT statement from node
    sds birthday = sdsempty();

    if (node.birthday.tm_year != 1900 && node.birthday.tm_mon != 1 && node.birthday.tm_mday != 1) // check if is birthday was not default
        birthday = sdscatprintf(birthday, "%04d-%02d-%02d", node.birthday.tm_year, node.birthday.tm_mon, node.birthday.tm_mday);

    csvRow = sdscatprintf(csvRow, CSV_GOOGLE_SCHEMA,
                          node.fname, node.lname, node.fname, node.lname,
                          birthday, // populate only if is a valid date
                          node.pemail, node.pmobile, node.hphone, node.wphone,
                          node.address, node.city, node.state, node.zip, node.country,
                          node.organization, node.job);
    // node.pmobile, node.bmobile, node.pemail, node.bemail,

    sdsfree(birthday);

    return (sds) csvRow;
}

static int is_space(unsigned char c)
{
    if (c == CSV_SPACE || c == CSV_TAB)
        return 1;

    return 0;
}

static int is_term(unsigned char c)
{
    if (c == CSV_CR || c == CSV_LF)
        return 1;

    return 0;
}

void csv_cb_field(void *s, size_t len, void *data)
{ // callback fuction called every field
    int count;
    sds *tokens, field;

    if (!((Counts_t *) data)->rows) // if is the header row
        return;                     // do nothing

    if (!((Counts_t *) data)->fields)                 // every first field
        ((Counts_t *) data)->db = initNode(contacts); // initializing the db node

    ((Counts_t *) data)->fields++; // increment the field counter

    if (!len)   // if the field is empty
        return; // do nothing

    field = sdsnewlen(s, len);        // cduplicate the field content
    sdstrim(field, " \n");            // remove spaces and newlines from the left and the right of the sds string
    field = sdschremove(field, "\""); // remove spaces and newlines from the left and the right of the sds string

    if (((Counts_t *) data)->isGoogle)         // if parsing Google CSV file
        switch (((Counts_t *) data)->fields) { // check the field number
        case 2:                                // FirstName
            ((Counts_t *) data)->db->fname = sdsnewlen(field, STEXT);
            break;
        case 4: // LastName
            ((Counts_t *) data)->db->lname = sdsnewlen(field, STEXT);
            break;
        case 15: // Birthday '1975-03-28'
            tokens                                    = sdssplitlen(field, len, "-", 1, &count);
            ((Counts_t *) data)->db->birthday.tm_year = atoi(tokens[0]);
            ((Counts_t *) data)->db->birthday.tm_mon  = atoi(tokens[1]);
            ((Counts_t *) data)->db->birthday.tm_mday = atoi(tokens[2]);
            sdsfreesplitres(tokens, count);
            break;
        case 31: // Pemail
            ((Counts_t *) data)->db->pemail = sdsnewlen(field, MTEXT);
            break;
        case 37:                                                        // Pmobile
            field                            = sdschremove(field, " "); // remove spaces form field sds string
            ((Counts_t *) data)->db->pmobile = sdsnewlen(field, PHONE);
            break;
        case 39:                                                       // Wphone
            field                           = sdschremove(field, " "); // remove spaces form field sds string
            ((Counts_t *) data)->db->wphone = sdsnewlen(field, PHONE);
            break;
        case 41:                                                       // Hphone
            field                           = sdschremove(field, " "); // remove spaces form field sds string
            ((Counts_t *) data)->db->hphone = sdsnewlen(field, PHONE);
            break;
        case 48: // Address
            ((Counts_t *) data)->db->address = sdsnewlen(field, LTEXT);
            break;
        case 49: // City
            ((Counts_t *) data)->db->city = sdsnewlen(field, MTEXT);
            break;
        case 51: // State
            ((Counts_t *) data)->db->state = sdsnewlen(field, STATE);
            break;
        case 52: // zip
            ((Counts_t *) data)->db->zip = sdsnewlen(field, ZIP);
            break;
        case 53: // Country
            ((Counts_t *) data)->db->country = sdsnewlen(field, STEXT);
            break;
        case 56: // Organizzation
            ((Counts_t *) data)->db->organization = sdsnewlen(field, MTEXT);
            break;
        case 58: // Job
            ((Counts_t *) data)->db->job = sdsnewlen(field, STEXT);
            break;

        default: // do nothing
            break;
        }
    else                                       // parsing PhoneBook CSV file
        switch (((Counts_t *) data)->fields) { // check the field number
        case 1:                                // id
            break;
        case 2: // FirstName
            ((Counts_t *) data)->db->fname = sdsnew(field);
            break;
        case 3: // LastName
            ((Counts_t *) data)->db->lname = sdsnew(field);
            break;
        case 4: // Organization
            ((Counts_t *) data)->db->organization = sdsnew(field);
            break;
        case 5: // Job
            ((Counts_t *) data)->db->job = sdsnew(field);
            break;
        case 6: // HPhone
            ((Counts_t *) data)->db->hphone = sdsnew(field);
            break;
        case 7: // WPhone
            ((Counts_t *) data)->db->wphone = sdsnew(field);
            break;
        case 8: // PMobile
            ((Counts_t *) data)->db->pmobile = sdsnew(field);
            break;
        case 9: // BMobile
            ((Counts_t *) data)->db->bmobile = sdsnew(field);
            break;
        case 10: // PEmail
            ((Counts_t *) data)->db->pemail = sdsnew(field);
            break;
        case 11: // BEmail
            ((Counts_t *) data)->db->bemail = sdsnew(field);
            break;
        case 12: // Address
            ((Counts_t *) data)->db->address = sdsnew(field);
            break;
        case 13: // Zip
            ((Counts_t *) data)->db->zip = sdsnew(field);
            break;
        case 14: // City
            ((Counts_t *) data)->db->city = sdsnew(field);
            break;
        case 15: // State
            ((Counts_t *) data)->db->state = sdsnew(field);
            break;
        case 16: // Country
            ((Counts_t *) data)->db->country = sdsnew(field);
            break;
        case 17: // mDay
            ((Counts_t *) data)->db->birthday.tm_mday = atoi(field);
            break;
        case 18: // Mon
            ((Counts_t *) data)->db->birthday.tm_mon = atoi(field);
            break;
        case 19: // Year
            ((Counts_t *) data)->db->birthday.tm_year = atoi(field);
            break;

        default: // do nothing
            break;
        }

    sdsfree(field); // release the memory

    return;
}

void csv_cb_row(int c, void *data)
{                                    // callback fuction called every row
    ((Counts_t *) data)->fields = 0; // set the filed to 0 means a new row

    if (((Counts_t *) data)->rows) { // exclude the header of CSV
        ((Counts_t *) data)->db->modified = true;
        addNode(&contacts, *((Counts_t *) data)->db); // add the row to contacts list
        destroyNode(((Counts_t *) data)->db);         // free memory of each sds strings
    }

    ((Counts_t *) data)->rows++; // increment the rows counter

    return;
}

int importCSV(sds csvFile)
{
    FILE *fp;
    CSV_Parser_t parse;
    size_t bytes_read, pos = 0;
    char buffer[MAX_BUFFER];
    int retVal;
    unsigned char options = 0;
    Counts_t data;

    data.fields   = 0;
    data.rows     = 0;
    data.isGoogle = strcmp(csvFile, GOOGLE_CSV) ? false : true; // check if is Google csv file

    if (csv_init(&parse, options)) {
        log_perror("Failed to initialize csv parser");

        return -1;
    }

    csv_set_space_func(&parse, is_space);
    csv_set_term_func(&parse, is_term);

    fp = fopen(csvFile, "rb");
    if (!fp) {
        log_perror("Failed to open %s: %s", csvFile, strerror(errno));
        csv_fini(&parse, csv_cb_field, csv_cb_row, &data);
        csv_free(&parse);

        return -1;
    }

    while ((bytes_read = fread(buffer, 1, MAX_BUFFER, fp)) > 0) {
        retVal = csv_parse(&parse, buffer, bytes_read, csv_cb_field, csv_cb_row, &data);
        pos += bytes_read;
        if (retVal == bytes_read)
            log_info("reading '%s' up to %lu bytes", csvFile, (unsigned long) pos);
        else if (csv_error(&parse) == CSV_EPARSE) {
            log_info("malformed at byte %lu at row %u at field %u", (unsigned long) pos, data.rows, data.fields);
            csv_fini(&parse, csv_cb_field, csv_cb_row, &data);
            csv_free(&parse);

            return -1;
        } else {
            log_perror("error while parsing file '%s': %s", csvFile, csv_strerror(csv_error(&parse)));
            csv_fini(&parse, csv_cb_field, csv_cb_row, &data);
            csv_free(&parse);

            return -1;
        }
    }

    csv_fini(&parse, csv_cb_field, csv_cb_row, &data);
    csv_free(&parse);

    if (ferror(fp)) {
        log_perror("error while reading file %s", csvFile);
        csv_free(&parse);
        fclose(fp);

        return -1;
    }

    fclose(fp);

    return data.rows - 1; //-1 in order to esclude the header of CSV
}