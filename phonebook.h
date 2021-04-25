#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>
#include <ncurses.h>
#include <malloc.h>
#include <menu.h>
#include <form.h>
#include <time.h>
#include <fcntl.h>
//#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <locale.h>
#include <assert.h>
//#define __USE_GNU //strcasestr();
//#define __STDC_WANT_LIB_EXT1__ 1 //memset_s()
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "../sds/sds.h"
#include "../sds/sds_extra.h"


//macro deinitions
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define PREV(list)       { list = list->prev; }
#define NEXT(list)       { list = list->next; }
#define REWIND(list)    while(list && list->prev) { PREV(list); }
#define FORWARD(list)   while(list && list->next) { NEXT(list); }
#define NULLSET(pointer) if(pointer) { pointer = NULL; }

#ifndef KEY_SPACE
#define KEY_SPACE 32
#endif
#ifndef KEY_ESC
#define KEY_ESC 27
#endif
#ifndef KEY_TAB
#define KEY_TAB 9
#endif
#ifndef KEY_RETURN
#define KEY_RETURN 10
#endif
#ifndef KEY_CTRL_DEL
#define KEY_CTRL_DEL 519
#endif
#ifndef KEY_CTRL_BS
#define KEY_CTRL_BS 8
#endif
#ifndef KEY_CTRL_LEFT
#define KEY_CTRL_LEFT 545
#endif
#ifndef KEY_CTRL_RIGHT
#define KEY_CTRL_RIGHT 560
#endif

#define LOGFILE "PHONEBOOK.log"
#define PAIR_STD      1
#define PAIR_TITLE    2
#define PAIR_EDIT     3
#define PAIR_MODIFIED 4

#define DB "phbook.sqlite3"	//database file name
#define DBAK "phbook.bak"	//database file name
#define SEARCH_CSV "search_dump.csv" //search csv dump file name
#define MAX_BUFFER 256 //max text array dimension
#define LTEXT 72 //for long field
#define MTEXT 35 //for medium field
#define STEXT 24 //for short field
#define PHONE 16 //for phone numbers
#define ZIP 5 //for zip
#define STATE 2 //for state or province
#define DEFAULT_SQL_TABLE   "DROP TABLE IF EXISTS Contacts;" \
                            "CREATE TABLE Contacts (id INTEGER PRIMARY KEY," \
                            "FirstName TEXT," \
                            "LastName TEXT," \
                            "Organization TEXT," \
                            "Job TEXT," \
                            "HPhone TEXT," \
                            "WPhone TEXT," \
                            "PMobile TEXT," \
                            "BMobile TEXT," \
                            "PEmail TEXT," \
                            "BEmail TEXT," \
                            "Address TEXT," \
                            "Zip TEXT," \
                            "City TEXT," \
                            "State TEXT," \
                            "Country TEXT," \
                            "mDay INTEGER," \
                            "Mon INTEGER," \
                            "Year INTEGER);"

#define UPDATE_SQL_ROW      "UPDATE Contacts SET " \
                            "FirstName = '%s', " \
                            "LastName = '%s', " \
                            "Organization = '%s', " \
                            "Job = '%s', " \
                            "HPhone = '%s', " \
                            "WPhone = '%s', " \
                            "PMobile = '%s', " \
                            "BMobile = '%s', " \
                            "PEmail = '%s', " \
                            "BEmail = '%s', " \
                            "Address = '%s', " \
                            "Zip = '%s', " \
                            "City = '%s', " \
                            "State = '%s', " \
                            "Country = '%s', " \
                            "mDay = %d, " \
                            "Mon = %d, " \
                            "Year = %d " \
                            "WHERE ID = %d;"

#define INSERT_SQL_VALUE "INSERT INTO Contacts VALUES (%u, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d);"
#define SELECT_FROM_CONTACTS "SELECT * FROM Contacts"
#define DELETE_SQL_RECORD_ID "DELETE FROM Contacts WHERE ID = %d;"

#define CSV_HEADER "id,FirstName,LastName,Organization,Job,HPhone,WPhone,PMobile,BMobile,PEmail,BEmail,Address,Zip,City,State,Country,mDay,Mon,Year\r\n"
#define CSV_SCHEMA "%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d\r\n"

#define CSV_GOOGLE_HEADER "Name,Given Name,Additional Name,Family Name,Yomi Name,Given Name Yomi,Additional Name Yomi,Family Name Yomi,Name Prefix,Name Suffix,Initials,Nickname,Short Name,Maiden Name,Birthday,Gender,Location,Billing Information,Directory Server,Mileage,Occupation,Hobby,Sensitivity,Priority,Subject,Notes,Language,Photo,Group Membership,E-mail 1 - Type,E-mail 1 - Value,E-mail 2 - Type,E-mail 2 - Value,E-mail 3 - Type,E-mail 3 - Value,Phone 1 - Type,Phone 1 - Value,Phone 2 - Type,Phone 2 - Value,Phone 3 - Type,Phone 3 - Value,Phone 4 - Type,Phone 4 - Value,Phone 5 - Type,Phone 5 - Value,Address 1 - Type,Address 1 - Formatted,Address 1 - Street,Address 1 - City,Address 1 - PO Box,Address 1 - Region,Address 1 - Postal Code,Address 1 - Country,Address 1 - Extended Address,Organization 1 - Type,Organization 1 - Name,Organization 1 - Yomi Name,Organization 1 - Title,Organization 1 - Department,Organization 1 - Symbol,Organization 1 - Location,Organization 1 - Job Description,Website 1 - Type,Website 1 - Value,Website 2 - Type,Website 2 - Value,Custom Field 1 - Type,Custom Field 1 - Value"

//char *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };

typedef struct DBnode DBnode_t;
typedef struct PhoneBook PhoneBook_t;

struct DBnode {
    _Bool delete;
    _Bool modified;
    unsigned id;
    sds fname;
    sds lname;
    sds organization;
    sds job;
    sds hphone;
    sds wphone;
    sds pmobile;
    sds bmobile;
    sds pemail;
    sds bemail;
    sds address;
    sds zip;
    sds city;
    sds state;
    sds country;
    struct tm birthday;
};

struct PhoneBook {
    PhoneBook_t *prev;
    DBnode_t db;
    PhoneBook_t *next;
};

extern PhoneBook_t *contacts; //Global contatcts phonebook

/*
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
*/

//functions
sds *buildMenuItems(char **items, int numbers);
void MainMenu(WINDOW *win);	//main menu
void SearchMenu(WINDOW *win);	//search menu
void AddMenu(WINDOW *win);	//add menu
void UpdateMenu(WINDOW *win, PhoneBook_t *resultList, sds menuName, sds menuModify);
int do_search(WINDOW *win, _Bool csv_export);	//main search function
sds *buildMenuList(PhoneBook_t *fromList, int *nb_fileds);
void freeMenuList(sds **menuList, int nb_fields);
int flexMenu(WINDOW *win, sds *choices, int n_choices, char *menuName);
int flexForm(WINDOW *win, DBnode_t *db, const char *menuName);
int initField(FIELD **field, DBnode_t *db); //initialize all field with db
int set_field_digit(FIELD *field, int buf, int digit); //like set_field_buffer but int values instead of char * return like set_field_buffer()
int field_digit(FIELD *field, int buf); //return the int value of filed_buffer
void print_in_middle(WINDOW *win, int y, const char *string, chtype color);
int messageBox(WINDOW *win, int y, const char *string, chtype color);
void printLabels(WINDOW *win, chtype color);
int write_db(_Bool update);
sds SDSinsertSQL(sds sql, DBnode_t node);
sds SDSupdateSQL(sds sql, DBnode_t node);
//int update_db(void);
int doSQLstatement(sds sql);
int read_db(void);
int callback(void *NotUsed, int argc, char **argv, char **azColName);
PhoneBook_t *newNode(DBnode_t node);
PhoneBook_t *addNode(PhoneBook_t **list, DBnode_t node); //push the element of db in contacts list, return the pointer of the new node
DBnode_t *initNode(PhoneBook_t *list);
int filecopy(const char* source, const char* destination);
int write_csv(const char *csv_file, PhoneBook_t *contact_csv);
void destroyNode(DBnode_t **node);
void destroyList(PhoneBook_t **list);
int countList(PhoneBook_t *list);
void db_log(const char *funcname, char *comment, DBnode_t *db);
void logfile(const char *fmt, ...);

//#include "functions.c"
//#include "programenu.c"
//#include "sqlite_funcs.c"
