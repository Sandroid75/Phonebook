#include "phonebook.h"

void MainMenu(WINDOW *win)
{ // main menu
    _Bool quit = false;
    int ch;
    sds *choices;
    sds menuName = sdsnew(_(" Main Menu ")), menuUpdate = sdsnew(_(" Show / Modify ")), menuModify = sdsnew(_(" Modify "));
    char *items[] = {_("Search Contact"), _("Add new Contact"), _("Show / Modify"), _("Import / Export"), _("Utility"), _("Exit"), NULL};

    REWIND(contacts);
    if (!contacts) {    // the list is empty
        ch = read_db(); // read the sqlite file database and store it in the contacts list
        if (ch == -1) { // an error was found reading db
            logfile("%s: Error reading DB\n", __func__);

            sdsfree(menuModify); // free memory
            sdsfree(menuUpdate); // free memory
            sdsfree(menuName);   // free memory

            return;
        } else if (ch > 0)
            logfile("%s: DataBase '%s' is successfully opened, %d contacts readed...\n", __func__, DB, ch);
        else
            logfile("%s: No database found, new one will be created...\n", __func__);
    }

    wrefresh(win);
    wclear(win);

    choices = buildMenuItems(items); // build the menu items
    if (!choices) {
        sdsfree(menuModify); // free memory
        sdsfree(menuUpdate); // free memory
        sdsfree(menuName);   // free memory

        return;
    }

    // start the operations
    do {
        REWIND(contacts); // at each interation we are sure that the pointer contacts stay at head of list
        switch (flexMenu(win, choices, menuName)) {
        case 1: // Search Contact
            SearchMenu(win);
            break;
        case 2: // Add new Contact
            AddMenu(win);
            break;
        case 3: // Show / Modify
            UpdateMenu(win, contacts, menuUpdate, menuModify);
            break;
        case 4: // Import / Export
            ImpExpMenu(win);
            break;
        case 5: // Utility
            UtilityMenu(win);
            break;
        case 6: // Menu Exit is selected
        case 0: // ESCape key is pressed
            logo(win, 5);
            ch = messageBox(win, 15, _(" press Y to quit... "), COLOR_PAIR(PAIR_EDIT));
            if (toupper(ch) == 'Y') {
                quit = true;
            }
            break;
        default:
            break;
        }
        wrefresh(win);
        wclear(win);
    } while (quit != true);

    sdsfreesplitres(choices, ARRAY_SIZE(items)); // free memory
    sdsfree(menuModify);                         // free memory
    sdsfree(menuUpdate);                         // free memory
    sdsfree(menuName);                           // free memory

    return;
}

void SearchMenu(WINDOW *win)
{                       // search menu
    _Bool quit = false; // check if option is valid
    sds *choices, menuName = sdsnew(_(" Search Menu "));
    char *items[] = {_("Search with export to CSV"), _("Search only"), _("Back to Main Menu"), NULL};

    REWIND(contacts);
    if (!contacts) { // if no contacts in list
        messageBox(win, 10, _(" No contacts found in the DataBase! "), COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuName);

        return;
    }

    choices = buildMenuItems(items); // build the menu items
    if (!choices) {
        sdsfree(menuName); // free memory

        return;
    }

    // start the operations
    while (!quit) {
        switch (flexMenu(win, choices, menuName)) {
        case 1:
            do_search(win, true);
            break;
        case 2:
            do_search(win, false);
            break;
        case 3: // back menu was selected
        case 0: // ESCape was pressed
            quit = true;
            break; // back to Main Menu
        default:
            break;
        }
        wrefresh(win);
        wclear(win);
    }

    sdsfreesplitres(choices, ARRAY_SIZE(items)); // free memory
    sdsfree(menuName);                           // free memory

    return; // back to Main Menu
}

int do_search(WINDOW *win, _Bool csv_export)
{
    DBnode_t *dbFlex;
    PhoneBook_t *ptr, *resultList = NULL;
    int nb_records, rows;
    _Bool found, modified = false;
    sds menuCriteria = sdsnew(_(" Search Filters ")), menuName = sdsnew(_(" Show / Modify "));

    dbFlex = malloc(sizeof(DBnode_t));   // reserve memory for node
    memset(dbFlex, 0, sizeof(DBnode_t)); // set to 0 everything in the node
    dbFlex->id = countList(contacts);    // count the number of list contacts
    mvwprintw(win, 18, 2, _("* The number highlighted in the upper left corner shows the contacts count!"));

    if (flexForm(win, dbFlex, menuCriteria) != 1) { // fill the criteria to search (1 mean confirm, 0 mean abort, -1 mean error)
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(dbFlex);

        return 0;
    }

    REWIND(contacts);
    for (nb_records = 0, ptr = contacts; ptr; NEXT(ptr)) { // search if with input filters match with some contacts
        found = false;                                     // reset to false for every cycle that means every record
        if (sdscasesds(ptr->db.fname, dbFlex->fname) >= 0)
            found = true; // for better readability I have aligned the code
        if (sdscasesds(ptr->db.lname, dbFlex->lname) >= 0)
            found = true; // if only one of the filed match with criteria
        if (sdscasesds(ptr->db.organization, dbFlex->organization) >= 0)
            found = true; // the boolean variable is set to found
        if (sdscasesds(ptr->db.job, dbFlex->job) >= 0)
            found = true;
        if (sdscasesds(ptr->db.hphone, dbFlex->hphone) >= 0)
            found = true;
        if (sdscasesds(ptr->db.wphone, dbFlex->wphone) >= 0)
            found = true;
        if (sdscasesds(ptr->db.pmobile, dbFlex->pmobile) >= 0)
            found = true;
        if (sdscasesds(ptr->db.bmobile, dbFlex->bmobile) >= 0)
            found = true;
        if (sdscasesds(ptr->db.pemail, dbFlex->pemail) >= 0)
            found = true;
        if (sdscasesds(ptr->db.bemail, dbFlex->bemail) >= 0)
            found = true;
        if (sdscasesds(ptr->db.address, dbFlex->address) >= 0)
            found = true;
        if (sdscasesds(ptr->db.zip, dbFlex->zip) >= 0)
            found = true;
        if (sdscasesds(ptr->db.city, dbFlex->city) >= 0)
            found = true;
        if (sdscasesds(ptr->db.state, dbFlex->state) >= 0)
            found = true;
        if (sdscasesds(ptr->db.country, dbFlex->country) >= 0)
            found = true;

        if (found) {                       // check if at least one of criteria was found in current contact field
            addNode(&resultList, ptr->db); // if found some store the contacts in temp list
            modified = true;               // set at least one time if something was found walking the list
            nb_records++;                  // count the numbers of records
        }
    }

    if (!modified) { // if nothing was found
        messageBox(win, 10, _(" No contacts was found with input filters "), COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(dbFlex);
        destroyList(resultList); // destroy the result list

        return 0;
    }
    logfile("%s: %d records found!\n", __func__, nb_records);

    if (csv_export) {                             // if the user choose with export
        rows = write_csv(SEARCH_CSV, resultList); // export the results to csv file
        if (rows == -1) {
            sdsfree(menuCriteria); // destroy the criteeria string
            sdsfree(menuName);     // destory the menu name
            destroyNode(dbFlex);
            destroyList(resultList); // destroy the result list

            return -1;
        }
        logfile("%s: Exported %d rows to %s\n", __func__, rows, SEARCH_CSV);
    }

    REWIND(resultList);
    UpdateMenu(win, resultList, menuName, menuCriteria); // this is the main cycle to manage the result of search

    sdsfree(menuCriteria); // destroy the criteeria string
    sdsfree(menuName);     // destory the menu name
    destroyNode(dbFlex);
    destroyList(resultList); // destroy the result list

    return nb_records;
}

void AddMenu(WINDOW *win)
{ // add menu
    int rc;
    _Bool store = false;

    do {
        DBnode_t *dbFlex = initNode(contacts); // create and initialize a node
        if (!dbFlex) {
            logfile("%s: Error allocating memory\n", __func__);

            return;
        }
        rc = flexForm(win, dbFlex, _(" Add New Contact ")); // create a new entry node only if the user confirm the input than 1 is returned
        // flexForm can return 1 mean confirm, 0 mean abort, -1 mean error
        if (rc == -1) { // error in func flexForm()
            logfile("%s: Error adding new contact\n", __func__);
        } else if (rc == 1) {                                     // the user input a new contact
            store = addNode(&contacts, (*dbFlex)) ? true : store; // if at least one time store is set to true, it remain true, otherwise false
            REWIND(contacts);
        }
        destroyNode(dbFlex);
    } while (rc == 1); // cycle while the user inputs new contact

    if (store) {
        rc = write_db(false); // write the contacts list to sqlite file database only if user confirm at least one time with param fale means INSERT
        logfile("%s: Inserted %d records in %s DataBase\n", __func__, rc, DB);
    }

    return;
}

void UpdateMenu(WINDOW *win, PhoneBook_t *resultList, sds menuName, sds menuModify)
{
    PhoneBook_t *ptr = resultList;
    DBnode_t *dbModify;
    sds *menuList;
    int nb_fields, i, choice;
    unsigned int id = 0;
    _Bool modified = false, delete = false;

    REWIND(ptr); // in order to be sure that ptr pointer go to head of list
    if (!ptr) {  // if no contacts in list
        messageBox(win, 10, _(" No contacts found in the DataBase! "), COLOR_PAIR(PAIR_EDIT));

        return;
    }

    menuList = buildMenuList(ptr, &nb_fields); // build the list from resultList
    if (!menuList)
        return;

    do {
        choice = flexMenu(win, menuList, menuName); // pass the contacts list to felxMenu to elaborate it
        if (choice) {                               // if a choice is made
            if (choice < 0) {                       // check if the contact is deleted
                choice *= -1;                       // invert the sign to manage the right indexing
                delete = true;                      // the field must be deleted
            }
            REWIND(resultList); // in order to be sure that resultList pointer go to head of list
            ptr = resultList;
            for (i = 1; ptr && i < choice; i++) { // walk to the choiced contact index
                NEXT(ptr);
            }
            if (!ptr) {
                logfile("%s: The menu item %d selected is out of list\n", __func__, choice);
                sdsfreesplitres(menuList, nb_fields); // release the memory

                return;
            }
            REWIND(contacts);             // in order to be sure that contacts pointer go to head of list
            if (resultList != contacts) { // if the function is called from MainMenu() this is false
                id = ptr->db.id;          // store the value of id
                for (ptr = contacts; ptr && (ptr->db.id) != id; NEXT(ptr))
                    ; // walk contacts up to id
            }
            if (!ptr) {
                logfile("%s: The menu item %d selected is out of list\n", __func__, choice);
                sdsfreesplitres(menuList, nb_fields); // release the memory

                return;
            }
            dbModify = &(ptr->db); // assign the choiced contact to modify ptr
            i        = choice - 1; // assign the correct index number
            if (delete) {
                if (ptr->db.delete) {                                                                       // check if the contact is already set to delete
                    sdsfree(menuList[i]);                                                                   // wipe the old field
                    menuList[i]    = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); // rebuild the modified contact
                    ptr->db.delete = false;                                                                 // set delete to false
                } else {
                    choice = messageBox(win, 10, _(" Press 'Y' to confirm deletion of any key to discard "), COLOR_PAIR(PAIR_EDIT));
                    if (toupper(choice) != 'Y') { // if confirm mark as deleted
                        continue;                 // back to the begin of do-while
                    }
                    sdsfree(menuList[i]);                                                                                            // wipe the old field
                    menuList[i]    = (sds) sdscatprintf(sdsempty(), _("*%s %s* # CANC to Restore #"), ptr->db.fname, ptr->db.lname); // rebuild the modified contact
                    ptr->db.delete = true;                                                                                           // mark as deleted
                }
                modified = true;                                                                        // set because of something is modified
                delete   = false;                                                                       // restore delete to false
            } else if (flexForm(win, dbModify, menuModify) == 1) {                                      // pass the modify ptr to form for modification of contact (1 mean confirm, 0 mean abort, -1 mean error)
                sdsfree(menuList[i]);                                                                   // wipe the old field
                menuList[i]    = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); // rebuild the modified contact
                ptr->db.delete = false;                                                                 // if was deleted now is restored
                modified       = true;                                                                  // if contact is modified
            }
        }
    } while (choice); // if no selection exit flexMenu return 0 if nothing was selected

    sdsfreesplitres(menuList, nb_fields); // release the memory

    if (modified) {                 // if at least one contact was modified
        nb_fields = write_db(true); // if the modification is accepted write the new db with param true means UPDATE
        logfile("%s: Updated %d records in %s DataBase\n", __func__, nb_fields, DB);
    }

    return;
}

void ImpExpMenu(WINDOW *win)
{                       // search menu
    _Bool quit = false; // check if option is valid
    int rows, midx;
    sds *choices, menuName = sdsnew(_(" Import & Export Menu "));
    char *message, *csvFile, *items[] = {_("Import from CSV"), _("Import from Google CSV"), _("Export to CSV"), _("Export to Google CSV"), _("Back to Main Menu"), NULL};

    choices = buildMenuItems(items); // build the menu items
    if (!choices) {
        sdsfree(menuName); // free memory

        return;
    }

    // start the operations
    while (!quit) {
        midx    = getmaxx(win) / 2;
        csvFile = message = NULL; // set NULL
        switch (flexMenu(win, choices, menuName)) {
        case 1:                       // Import from CSV
            csvFile = DB_CSV;         // assign the standard csv file name
        case 2:                       // Import from Google CSV
            if (!csvFile)             // check if the file name is already set
                csvFile = GOOGLE_CSV; // assign the standard Google csv file name

            rows = importCSV(csvFile);                                                                 // import the contacts from csv
            if (rows > 0)                                                                              // check if records are exported
                asprintf(&message, _(" Imported %d records from '%s' successfully. "), rows, csvFile); // build message string
            else
                asprintf(&message, _(" No records imported from '%s' "), csvFile); // build message string

            wattron(win, A_BLINK);
            print_in_middle(win, 11, " Updating DataBase file, please wait... ", COLOR_PAIR(PAIR_EDIT));
            wattroff(win, A_BLINK);
            wrectangle(win, 10, midx - 22, 12, midx + 21);
            if (rows > 0) {
                rows = write_db(false); // write the imported contacts list to sqlite file database
                logfile("%s: Imported %d rows in '%s' DataBase\n", __func__, rows, DB);
            } else
                logfile("%s: %s\n", __func__, message); // write result to log file

            wclear(win);
            wrefresh(win);
            messageBox(win, 15, message, COLOR_PAIR(PAIR_MODIFIED));
            break;
        case 3:                       // Export to CSV
            csvFile = DB_CSV;         // assign the standard csv file name
        case 4:                       // Export to Google CSV
            if (!csvFile)             // check if the file name is already set
                csvFile = GOOGLE_CSV; // assign the standard Google csv file name

            wattron(win, A_BLINK);
            print_in_middle(win, 11, _(" Writing CSV file, please waiting... "), COLOR_PAIR(PAIR_EDIT));
            wattroff(win, A_BLINK);
            wrectangle(win, 10, midx - 22, 12, midx + 21);
            rows = write_csv(csvFile, contacts);                                                    // write or append records to DB_CSV
            if (rows > 0)                                                                           // check if records are exported
                asprintf(&message, _(" Exported %d records to '%s' successfully "), rows, csvFile); // build message string
            else
                asprintf(&message, _(" No records exported to '%s' "), csvFile); // build message string

            wclear(win);
            wrefresh(win);
            messageBox(win, 15, message, COLOR_PAIR(PAIR_MODIFIED));
            logfile("%s: %s\n", __func__, message); // write result to log file
            break;
        case 5: // Back to Main Menu
        case 0: // ESCape was pressed
            quit = true;
            break; // back to Main Menu
        default:
            break;
        }
        if (message)
            FREE(message); // realese the memory

        wrefresh(win);
        wclear(win);
    }

    rows = ARRAY_SIZE(items);
    sdsfreesplitres(choices, rows); // free memory
    sdsfree(menuName);              // free memory

    return; // back to Main Menu
}

void UtilityMenu(WINDOW *win)
{                       // search menu
    _Bool quit = false; // check if option is valid
    sds *choices, menuName = sdsnew(_(" Utility Menu "));
    char *items[] = {_("Sort contacts by First Name A->Z"), _("Sort contacts by First Name Z->A"), _("Sort contacts by Last Name A->Z"), _("Sort contacts by Last Name Z->A"), _("Find Duplicates"), _("Back to Main Menu"), NULL};

    REWIND(contacts);
    if (!contacts) { // if no contacts in list
        messageBox(win, 10, _(" No contacts found in the DataBase! "), COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuName);

        return;
    }

    choices = buildMenuItems(items); // build the menu items
    if (!choices) {
        sdsfree(menuName); // free memory

        return;
    }

    // start the operations
    while (!quit) {
        switch (flexMenu(win, choices, menuName)) {
        case 1: // Sort by First Name A->Z
            SortList(win, contacts, FirstNameAZ);
            messageBox(win, 10, _(" Sorting done by First Name A->Z... "), COLOR_PAIR(PAIR_MODIFIED));
            break;
        case 2: // Sort by First Name Z->A
            SortList(win, contacts, FirstNameZA);
            messageBox(win, 10, _(" Sorting done by First Name Z->A... "), COLOR_PAIR(PAIR_MODIFIED));
            break;
        case 3: // Sort by Last Name A->Z
            SortList(win, contacts, LastNameAZ);
            messageBox(win, 10, _(" Sorting done by Last Name A->Z... "), COLOR_PAIR(PAIR_MODIFIED));
            break;
        case 4: // Sort by Last Name A->A
            SortList(win, contacts, LastNameZA);
            messageBox(win, 10, _(" Sorting done by Last Name Z->A... "), COLOR_PAIR(PAIR_MODIFIED));
            break;
        case 5: // Find Duplicate
            FindDuplicates(win);
            break;
        case 0: // ESCape was pressed
        case 6:
            quit = true;
            break; // back to Main Menu
        default:
            break;
        }
        wrefresh(win);
        wclear(win);
    }
    sdsfreesplitres(choices, ARRAY_SIZE(items)); // free memory
    sdsfree(menuName);                           // free memory

    return; // back to Main Menu
}

int FindDuplicates(WINDOW *win)
{
    PhoneBook_t *match, *ptr, *mnext, *pnext;
    unsigned int check = MATCH_NO_MATCH;
    int nb_records     = 0;
    _Bool found        = false;

    REWIND(contacts);
    for (match = contacts; match; match = mnext) {
        mnext = match->next;                                    // safe pointing to next node, in case of deletion of the node
        for (ptr = match->next; ptr; ptr = pnext) {             // search matches in phone numbers
            pnext = ptr->next;                                  // safe pointing to next node, in case of deletion of the node
            check = checkMatch(match->db, ptr->db);             // check matches and set the metch bits
            if (check) {                                        // check if a match was found in current contact field
                mergeDuplicate(win, match->db, ptr->db, check); // let choice to the user if the match will be merged
                found = true;
                nb_records++; // count the numbers of records
            }
        }
    }

    if (!found) { // if nothing was found
        messageBox(win, 10, _(" No duplicated contacts was found "), COLOR_PAIR(PAIR_EDIT));

        return 0;
    }
    logfile("%s: Found %d pairs of records with duplicates!\n", __func__, nb_records);

    return nb_records;
}

unsigned int checkMatch(DBnode_t first, DBnode_t second)
{
    unsigned int check = MATCH_NO_MATCH;

    if (sdscasesds(first.hphone, second.hphone) >= 0)
        check |= MATCH_HPHONE_HPONE;
    if (sdscasesds(first.hphone, second.wphone) >= 0)
        check |= MATCH_HPHONE_WPHONE;
    if (sdscasesds(first.hphone, second.pmobile) >= 0)
        check |= MATCH_HPHONE_PMOBILE;
    if (sdscasesds(first.hphone, second.bmobile) >= 0)
        check |= MATCH_HPHONE_BMOBILE;

    if (sdscasesds(first.wphone, second.hphone) >= 0)
        check |= MATCH_WPHONE_HPHONE;
    if (sdscasesds(first.wphone, second.wphone) >= 0)
        check |= MATCH_WPHONE_WPHONE;
    if (sdscasesds(first.wphone, second.pmobile) >= 0)
        check |= MATCH_WPHONE_PMOBILE;
    if (sdscasesds(first.wphone, second.bmobile) >= 0)
        check |= MATCH_WPHONE_BMOBILE;

    if (sdscasesds(first.pmobile, second.hphone) >= 0)
        check |= MATCH_PMOBILE_HPHONE;
    if (sdscasesds(first.pmobile, second.wphone) >= 0)
        check |= MATCH_PMOBILE_WPHONE;
    if (sdscasesds(first.pmobile, second.pmobile) >= 0)
        check |= MATCH_PMOBILE_PMOBILE;
    if (sdscasesds(first.pmobile, second.bmobile) >= 0)
        check |= MATCH_PMOBILE_BMOBILE;

    if (sdscasesds(first.bmobile, second.hphone) >= 0)
        check |= MATCH_BMOBILE_HPHONE;
    if (sdscasesds(first.bmobile, second.wphone) >= 0)
        check |= MATCH_BMOBILE_WPHONE;
    if (sdscasesds(first.bmobile, second.pmobile) >= 0)
        check |= MATCH_BMOBILE_PMOBILE;
    if (sdscasesds(first.bmobile, second.bmobile) >= 0)
        check |= MATCH_BMOBILE_BMOBILE;

    return check;
}

void mergeDuplicate(WINDOW *win, DBnode_t first, DBnode_t second, unsigned int check)
{
    PhoneBook_t *ptr, *resultList = NULL;
    DBnode_t *dbMerge = NULL;
    int ch;
    sds menuMatch = sdsnew(_(" Edit Duplicate ")), menuName = sdsnew(_(" Show / Modify "));

    ch = showMatch(win, first, second, check);
    switch (ch) {
    case KEY_F(1):
        REWIND(contacts);
        for (ptr = contacts; ptr; NEXT(ptr)) {                     // walk to the enteire list
            if (ptr->db.id == first.id || ptr->db.id == second.id) // find the duplicated contacts
                addNode(&resultList, ptr->db);                     // add the every single duplicate to temp list
        }
        REWIND(resultList);
        UpdateMenu(win, resultList, menuName, menuMatch); // to modify the duplicated contacts
        destroyList(resultList);                          // destroy the result list
        break;
    case KEY_F(4):
        dbMerge = initNode(contacts);

        dbMerge->fname        = strlen(first.fname) > strlen(second.fname) ? sdsnew(first.fname) : sdsnew(second.fname);
        dbMerge->lname        = strlen(first.lname) > strlen(second.lname) ? sdsnew(first.lname) : sdsnew(second.lname);
        dbMerge->organization = strlen(first.organization) > strlen(second.organization) ? sdsnew(first.organization) : sdsnew(second.organization);
        dbMerge->job          = strlen(first.job) > strlen(second.job) ? sdsnew(first.job) : sdsnew(second.job);
        dbMerge->hphone       = strlen(first.hphone) > strlen(second.hphone) ? sdsnew(first.hphone) : sdsnew(second.hphone);
        dbMerge->wphone       = strlen(first.wphone) > strlen(second.wphone) ? sdsnew(first.wphone) : sdsnew(second.wphone);
        dbMerge->pmobile      = strlen(first.pmobile) > strlen(second.pmobile) ? sdsnew(first.pmobile) : sdsnew(second.pmobile);
        dbMerge->bmobile      = strlen(first.bmobile) > strlen(second.bmobile) ? sdsnew(first.bmobile) : sdsnew(second.bmobile);
        dbMerge->pemail       = strlen(first.pemail) > strlen(second.pemail) ? sdsnew(first.pemail) : sdsnew(second.pemail);
        dbMerge->bemail       = strlen(first.bemail) > strlen(second.bemail) ? sdsnew(first.bemail) : sdsnew(second.bemail);
        dbMerge->address      = strlen(first.address) > strlen(second.address) ? sdsnew(first.address) : sdsnew(second.address);
        dbMerge->zip          = strlen(first.zip) > strlen(second.zip) ? sdsnew(first.zip) : sdsnew(second.zip);
        dbMerge->city         = strlen(first.city) > strlen(second.city) ? sdsnew(first.city) : sdsnew(second.city);
        dbMerge->state        = strlen(first.state) > strlen(second.state) ? sdsnew(first.state) : sdsnew(second.state);
        dbMerge->country      = strlen(first.country) > strlen(second.country) ? sdsnew(first.country) : sdsnew(second.country);

        dbMerge->birthday.tm_year = first.birthday.tm_year > second.birthday.tm_year ? first.birthday.tm_year : second.birthday.tm_year;
        dbMerge->birthday.tm_mon  = first.birthday.tm_mon > second.birthday.tm_mon ? first.birthday.tm_mon : second.birthday.tm_mon;
        dbMerge->birthday.tm_mday = first.birthday.tm_mday > second.birthday.tm_mday ? first.birthday.tm_mday : second.birthday.tm_mday;

        dbMerge->modified = true;       // set the merged contact to modified
        addNode(&contacts, (*dbMerge)); // add the new merged contact to contacts list
        destroyNode(dbMerge);           // destroy the node

        REWIND(contacts);
        for (ptr = contacts; ptr; NEXT(ptr)) {                     // walk to the enteire list
            if (ptr->db.id == first.id || ptr->db.id == second.id) // find the duplicated contacts
                ptr->db.delete = true;                             // set the original contacto to delete
        }

        write_db(true); // if the modification is accepted write the new db with param true means UPDATE
        logfile("%s: Updated records in %s DataBase\n", __func__, DB);
        break;
    case KEY_ESC:
    default:
        break;
    }

    sdsfree(menuMatch); // destroy the criteeria string
    sdsfree(menuName);  // destory the menu name

    return;
}

sds *buildMenuItems(char **items)
{
    sds *menuChoices;
    int i, n;

    if (!items) {
        return NULL;
    }

    for (n = 0; items[n]; n++)
        ;                                             // count the number of items (NULL terminated)
    menuChoices = (sds *) calloc(n + 1, sizeof(sds)); // calculate one more also for empity sds

    for (i = 0; i < n; i++)
        menuChoices[i] = sdscatprintf(sdsempty(), "[%d] %s", i + 1, items[i]);

    menuChoices[i] = sdsempty(); // last item must be empty

    return (sds *) menuChoices;
}

sds *buildMenuList(PhoneBook_t *fromList, int *nb_fileds)
{
    PhoneBook_t *ptr = fromList;
    sds *menuList, fname, lname;
    int i;

    REWIND(ptr);
    if (!ptr) { // check if the list is empty
        logfile("%s: No contacts to build a list|\n", __func__);
        (*nb_fileds) = 0; // no items in the menu list

        return (sds *) NULL;
    }

    i        = countList(ptr) + 1;             // count the number of field found
    menuList = (sds *) calloc(i, sizeof(sds)); // allocate the memory for contact list

    for (i = 0; ptr; i++) { // walk thru the list
        fname = sdsnew(ptr->db.fname);
        lname = sdsnew(ptr->db.lname);
#ifndef NCURSES_WIDECHAR
        fname = sdschremove(fname, SPECIAL_CHARS); // remove special chars from sds
        lname = sdschremove(lname, SPECIAL_CHARS); // remove special chars from sds
#endif

        menuList[i] = (sds) sdscatprintf(sdsempty(), "%s %s", fname, lname); // build the menu list
        sdsfree(fname);
        sdsfree(lname);
        NEXT(ptr);
    }
    menuList[i++] = sdsempty(); // assign NULL string to the last contact of the list
    (*nb_fileds)  = i;          // assign the total menu items

    return (sds *) menuList;
}

void SortList(WINDOW *win, PhoneBook_t *list, _Bool compare(PhoneBook_t *first, PhoneBook_t *second))
{
    PhoneBook_t *head, *tail;
    int nb_records, midx;

    head = tail = list;             // point head and tail to list
    REWIND(head);                   // move head to the begin of the list
    FORWARD(tail);                  // move tail to the end of the list
    QuickSort(head, tail, compare); // compute sorting with compare() criteria
    RenumberListID(list);           // renumber the ID of the enteire list

    wattron(win, A_BLINK); // set text blinking ON
    print_in_middle(win, 11, _(" Updating DataBase file, please wait... "), COLOR_PAIR(PAIR_EDIT));
    wattroff(win, A_BLINK); // set text blinking OFF
    midx = getmaxx(win) / 2;
    wrectangle(win, 10, midx - 22, 12, midx + 21);

    nb_records = write_db(true); // Updating database
    logfile("%s: Updated %d records in %s DataBase after sorting\n", __func__, nb_records, DB);

    wclear(win);
    wrefresh(win);

    return;
}
