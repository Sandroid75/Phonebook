#ifndef _PHONEBOOK_H_
#define _PHONEBOOK_H_

#define _GNU_SOURCE
#define LOG_USE_COLOR

#include "../../log/src/log.h"
#include "../../sds/sds.h"
#include "../../sds/sds_extra.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <malloc.h>
#include <ncursesw/form.h>
#include <ncursesw/menu.h>
#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// macro deinitions
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define PREV(list) list = list->prev
#define NEXT(list) list = list->next
#define REWIND(list)             \
    while (list && list->prev) { \
        PREV(list);              \
    }
#define FORWARD(list)            \
    while (list && list->next) { \
        NEXT(list);              \
    }
#define NULLSET(pointer) \
    if (pointer) {       \
        pointer = NULL;  \
    }

#ifndef MIN
#define MIN(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#endif
#ifndef MAX
#define MAX(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#endif

#define _(x) gettext(x)

/* Safe version of free() don't need assign NULL after free */
#ifndef FREE
#define FREE(p)        \
    {                  \
        if ((p))       \
            free((p)); \
        (p) = NULL;    \
    }
#endif

/* https://stackoverflow.com/questions/42156041/copying-a-huge-file-using-read-write-and-open-apis-in-linux */
#define DEFAULT_CHUNK ((ssize_t) 262144L)

/* KEY not defined by curses.h */
#ifndef KEY_SPACE
#define KEY_SPACE 0x20
#endif
#ifndef KEY_ESC
#define KEY_ESC 0x1B
#endif
#ifndef KEY_TAB
#define KEY_TAB 0x09
#endif
#ifndef KEY_RETURN
#define KEY_RETURN 0x0A
#endif
#ifndef KEY_CTRL_DEL
#define KEY_CTRL_DEL 0x207
#endif
#ifndef KEY_CTRL_BS
#define KEY_CTRL_BS 0x08
#endif
#ifndef KEY_CTRL_LEFT
#define KEY_CTRL_LEFT 0x221
#endif
#ifndef KEY_CTRL_RIGHT
#define KEY_CTRL_RIGHT 0x230
#endif
#ifndef KEY_CTRL_X
#define KEY_CTRL_X 0x18
#endif
#ifndef KEY_CTRL_C
#define KEY_CTRL_C 0x03
#endif
#ifndef KEY_CTRL_V
#define KEY_CTRL_V 0x16
#endif
#ifndef KEY_CTRL_A
#define KEY_CTRL_A 0x01
#endif
#ifndef KEY_CTRL_S
#define KEY_CTRL_S 0x13
#endif
#ifndef KEY_CTRL_D
#define KEY_CTRL_D 0x04
#endif

#define MATCH_NO_MATCH 0x0

#define MATCH_HPHONE_HPONE 0x1
#define MATCH_HPHONE_WPHONE 0x2
#define MATCH_HPHONE_PMOBILE 0x4
#define MATCH_HPHONE_BMOBILE 0x8

#define MATCH_WPHONE_HPHONE 0x10
#define MATCH_WPHONE_WPHONE 0x20
#define MATCH_WPHONE_PMOBILE 0x40
#define MATCH_WPHONE_BMOBILE 0x80

#define MATCH_PMOBILE_HPHONE 0x100
#define MATCH_PMOBILE_WPHONE 0x200
#define MATCH_PMOBILE_PMOBILE 0x400
#define MATCH_PMOBILE_BMOBILE 0x800

#define MATCH_BMOBILE_HPHONE 0x1000
#define MATCH_BMOBILE_WPHONE 0x2000
#define MATCH_BMOBILE_PMOBILE 0x4000
#define MATCH_BMOBILE_BMOBILE 0x8000

#define MATCH_FIRST_HPHONE 0xF
#define MATCH_FIRST_WPHONE 0xF0
#define MATCH_FIRST_PMOBILE 0xF00
#define MATCH_FIRST_BMOBILE 0xF000

#define MATCH_SECOND_HPHONE 0x1111
#define MATCH_SECOND_WPHONE 0x2222
#define MATCH_SECOND_PMOBILE 0x4444
#define MATCH_SECOND_BMOBILE 0x8888

#define PAIR_STD 1
#define PAIR_TITLE 2
#define PAIR_EDIT 3
#define PAIR_MODIFIED 4
#define PAIR_LOGO 5
#define PAIR_ERROR 6

#define LOGFILE "phonebook.log"      // LOG file name
#define DB "phonebook.sqlite3"       // database file name
#define DBAK "phonebook.bak"         // database BAK file name
#define DB_CSV "phonebook.csv"       // CSV file name
#define GOOGLE_CSV "contacts.csv"    // Google CSV file name
#define SEARCH_CSV "search_dump.csv" // search CSV dump file name

#define REGEXP_PHONE "^\\+?[0-9]+ "
#define REGEXP_EMAIL "^([a-zA-Z0-9_\\-\\.]+)@([a-zA-Z0-9_\\-\\.]+)\\.([a-zA-Z]{2,5})"
#define REGEXP_ZIP "^[0-9]*$"

#ifndef MAX_BUFFER
#define MAX_BUFFER SDS_MAX_PREALLOC // max text array dimension
#endif
#define PHONE_FIELDS 20 // total numbers of fields
#define MATCH_FIELDS 29 // total numbers of fields in match duplicates
#define LTEXT 72        // for long field
#define MTEXT 35        // for medium field
#define STEXT 24        // for short field
#define PHONE 16        // for phone numbers
#define ZIP 5           // for zip
#define STATE 2         // for state or province
#define DEFAULT_SQL_TABLE "DROP TABLE IF EXISTS Contacts;"                 \
                          "CREATE TABLE Contacts (id INTEGER PRIMARY KEY," \
                          "FirstName TEXT,"                                \
                          "LastName TEXT,"                                 \
                          "Organization TEXT,"                             \
                          "Job TEXT,"                                      \
                          "HPhone TEXT,"                                   \
                          "WPhone TEXT,"                                   \
                          "PMobile TEXT,"                                  \
                          "BMobile TEXT,"                                  \
                          "PEmail TEXT,"                                   \
                          "BEmail TEXT,"                                   \
                          "Address TEXT,"                                  \
                          "Zip TEXT,"                                      \
                          "City TEXT,"                                     \
                          "State TEXT,"                                    \
                          "Country TEXT,"                                  \
                          "mDay INTEGER,"                                  \
                          "Mon INTEGER,"                                   \
                          "Year INTEGER);"

#define UPDATE_SQL_ROW "UPDATE Contacts SET "    \
                       "FirstName = \"%s\", "    \
                       "LastName = \"%s\", "     \
                       "Organization = \"%s\", " \
                       "Job = \"%s\", "          \
                       "HPhone = \"%s\", "       \
                       "WPhone = \"%s\", "       \
                       "PMobile = \"%s\", "      \
                       "BMobile = \"%s\", "      \
                       "PEmail = \"%s\", "       \
                       "BEmail = \"%s\", "       \
                       "Address = \"%s\", "      \
                       "Zip = \"%s\", "          \
                       "City = \"%s\", "         \
                       "State = \"%s\", "        \
                       "Country = \"%s\", "      \
                       "mDay = %d, "             \
                       "Mon = %d, "              \
                       "Year = %d "              \
                       "WHERE ID = %d;"

#define INSERT_SQL_VALUE "INSERT INTO Contacts VALUES (%u, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d);"
#define SELECT_FROM_CONTACTS "SELECT * FROM Contacts"
#define DELETE_SQL_RECORD_ID "DELETE FROM Contacts WHERE ID = %d;"

#define CSV_HEADER "id,FirstName,LastName,Organization,Job,HPhone,WPhone,PMobile,BMobile,PEmail,BEmail,Address,Zip,City,State,Country,mDay,Mon,Year\r\n"
#define CSV_SCHEMA "%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d\r\n"
#define CSV_GOOGLE_HEADER "Name,Given Name,Additional Name,Family Name,Yomi Name,Given Name Yomi,Additional Name Yomi,Family Name Yomi,Name Prefix,Name Suffix,Initials,Nickname,Short Name,Maiden Name,Birthday,Gender,Location,Billing Information,Directory Server,Mileage,Occupation,Hobby,Sensitivity,Priority,Subject,Notes,Language,Photo,Group Membership,E-mail 1 - Type,E-mail 1 - Value,E-mail 2 - Type,E-mail 2 - Value,E-mail 3 - Type,E-mail 3 - Value,Phone 1 - Type,Phone 1 - Value,Phone 2 - Type,Phone 2 - Value,Phone 3 - Type,Phone 3 - Value,Phone 4 - Type,Phone 4 - Value,Phone 5 - Type,Phone 5 - Value,Address 1 - Type,Address 1 - Formatted,Address 1 - Street,Address 1 - City,Address 1 - PO Box,Address 1 - Region,Address 1 - Postal Code,Address 1 - Country,Address 1 - Extended Address,Organization 1 - Type,Organization 1 - Name,Organization 1 - Yomi Name,Organization 1 - Title,Organization 1 - Department,Organization 1 - Symbol,Organization 1 - Location,Organization 1 - Job Description,Website 1 - Type,Website 1 - Value,Website 2 - Type,Website 2 - Value,Custom Field 1 - Type,Custom Field 1 - Value\r\n"
#define CSV_GOOGLE_SCHEMA "%s %s,%s,,%s,,,,,,,,,,,%s,,,,,,,,,,,,,,* myContacts,,%s,,,,,Mobile,%s,,%s,,%s,,,,,,,%s,%s,,%s,%s,%s,,,%s,,%s,,,,,,,,,,\r\n"

#ifndef NCURSES_WIDECHAR
#define SPECIAL_CHARS "ÀÁÂÃÄÅĀĂĄÆàáâãäåāăąæÇĆČçćčĎĐđÈÉÊËĒĖĘĚĔèéêëēėęěĕĢĞģğÎÍÌĮĪÏîíìıįīïĶķŁĻĹłļĺŇŅŃÑňņńñÕÔÓÒŐØÖŒõôóòőøöœŔŘŕřŚŠŞśšşÞŤȚŢþțţÜÛÚÙŲŰŮŪüûúùųűůūµÝŸýÿŹŻŽźżž"
#endif

/* filecopy() returns error */
#define FC_ERROR -1L // unable to copy
#define FC_SAMEF -2L // suorce and destination are the same file
#define FC_NOOWD -3L // unable to overwrite the destination
#define FC_USKIP -4L // user skiping overwrite

// char *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };

typedef struct DBnode DBnode_t;
typedef struct PhoneBook PhoneBook_t;
typedef struct csv_parser CSV_Parser_t;
typedef struct Counts Counts_t;

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

struct Counts {
    unsigned int fields;
    unsigned int rows;
    _Bool isGoogle;
    DBnode_t *db;
};

extern PhoneBook_t *contacts; // Global contatcts phonebook

/* function declarations */
// db_func.c
int callback(void *NotUsed, int argc, char **argv, char **azColName);
int read_db(void);
int write_db(_Bool update);
sds SDSinsertSQL(sds sql, DBnode_t node);
sds SDSupdateSQL(sds sql, DBnode_t node);
int doSQLstatement(sds sql);
int write_csv(const char *csvFile, PhoneBook_t *contact_csv);
sds SDSinsertCSV(sds csvRow, DBnode_t node);
sds SDSgoogleCSV(sds csvRow, DBnode_t node);
int importCSV(sds csvFile);

// dblist.c
PhoneBook_t *newNode(DBnode_t node);
PhoneBook_t *addNode(PhoneBook_t **list, DBnode_t node); // push the element of db in contacts list, return the pointer of the new node
DBnode_t *initNode(PhoneBook_t *list);
void deleteNode(PhoneBook_t **list, PhoneBook_t *del);
int countList(PhoneBook_t *list);
void destroyNode(DBnode_t *node);
void destroyList(PhoneBook_t *list);
void nodeDBswap(PhoneBook_t *left, PhoneBook_t *right);
PhoneBook_t *partition(PhoneBook_t *head, PhoneBook_t *tail, _Bool compare(PhoneBook_t *first, PhoneBook_t *second));
void QuickSort(PhoneBook_t *head, PhoneBook_t *tail, _Bool compare(PhoneBook_t *first, PhoneBook_t *second));
_Bool FirstNameAZ(PhoneBook_t *first, PhoneBook_t *second);
_Bool FirstNameZA(PhoneBook_t *first, PhoneBook_t *second);
_Bool LastNameAZ(PhoneBook_t *first, PhoneBook_t *second);
_Bool LastNameZA(PhoneBook_t *first, PhoneBook_t *second);
void RenumberListID(PhoneBook_t *list);

// functions.c
void db_log(const char *funcname, char *comment, DBnode_t *db);
ssize_t filecopy(const char *source, const char *destination);

// menu.c
void MainMenu(WINDOW *win);                   // main menu
void SearchMenu(WINDOW *win);                 // search menu
int do_search(WINDOW *win, _Bool csv_export); // main search function
void AddMenu(WINDOW *win);                    // add menu
void UpdateMenu(WINDOW *win, PhoneBook_t *resultList, sds menuName, sds menuModify);
void ImpExpMenu(WINDOW *win);
void UtilityMenu(WINDOW *win);
int FindDuplicates(WINDOW *win);
unsigned int checkMatch(DBnode_t first, DBnode_t second);
void mergeDuplicate(WINDOW *win, DBnode_t first, DBnode_t second, unsigned int check);
sds *buildMenuItems(char **items);
sds *buildMenuList(PhoneBook_t *fromList, int *nb_fileds);
void SortList(WINDOW *win, PhoneBook_t *list, _Bool compare(PhoneBook_t *first, PhoneBook_t *second));

// ui_ncurses.c
int flexMenu(WINDOW *win, sds *choices, char *menuName);
int flexForm(WINDOW *win, DBnode_t *db, const char *menuName);
FIELD **initField(DBnode_t *db);                       // initialize all field with db
int set_field_digit(FIELD *field, int buf, int digit); // like set_field_buffer but int values instead of char * return like set_field_buffer()
int field_digit(FIELD *field, int buf);                // return the int value of filed_buffer
int showMatch(WINDOW *win, DBnode_t first, DBnode_t second, unsigned int check);
FIELD **initMatchField(DBnode_t first, DBnode_t second, unsigned int check);
void print_in_middle(WINDOW *win, int y, const char *string, chtype color);
int messageBox(WINDOW *win, int y, const char *string, chtype color);
void printLabels(WINDOW *win, chtype color);
void logo(WINDOW *win, int y);
void wrectangle(WINDOW *win, int y1, int x1, int y2, int x2);

#endif
