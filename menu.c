#include "phonebook.h"

void MainMenu(WINDOW *win) { //main menu
    _Bool quit = false;
    int ch;
    sds *choices;
    sds menuName = sdsnew(" Main Menu "), menuUpdate = sdsnew(" Show / Modify "), menuModify = sdsnew(" Modify ");
    char *items[] = { "Search Contact", "Add new Contact", "Show / Modify", "Import / Export", "Utility", "Exit", NULL };

    REWIND(contacts);
    if(!contacts) { //the list is empty
        ch = read_db(); //read the sqlite file database and store it in the contacts list
        if(ch == -1) { //an error was found reading db
            logfile("%s: Error reading DB\n", __func__);

            sdsfree(menuModify); //free memory
            sdsfree(menuUpdate); //free memory
            sdsfree(menuName); //free memory

            return;
        } else if(ch > 0) {
            logfile("%s: DataBase '%s' is successfully opened, %d contacts readed...\n", __func__, DB, ch);
        } else {
            logfile("%s: No database found, new one will be created...\n", __func__);
        }
    }

    wrefresh(win);
    wclear(win);

    choices = buildMenuItems(items); //build the menu items
    if(!choices) {
        sdsfree(menuModify); //free memory
        sdsfree(menuUpdate); //free memory
        sdsfree(menuName); //free memory

        return;
    }

    //start the operations
    do {
        REWIND(contacts); //at each interation we are sure that the pointer contacts stay at head of list
        switch(flexMenu(win, choices, menuName)) {
            case 1: //Search Contact
                SearchMenu(win);
                break;
            case 2: //Add new Contact
                AddMenu(win);
                break;
            case 3: //Show / Modify
                UpdateMenu(win, contacts, menuUpdate, menuModify);
                break;
            case 4: //Import / Export
                ImpExpMenu(win);
                break;
            case 5: //Utility
                break;
            case 6: //Menu Exit is selected
            case 0: //ESCape key is pressed
                ch = messageBox(win, 10, " press Y to quit... ", COLOR_PAIR(PAIR_EDIT));
                if(toupper(ch) == 'Y') {
                    quit = true;
                }
                break;
            default:
                break;
        }
        wrefresh(win);
        wclear(win);
    } while(quit != true);

    sdsfreesplitres(choices, ARRAY_SIZE(items)); //free memory
    sdsfree(menuModify); //free memory
    sdsfree(menuUpdate); //free memory
    sdsfree(menuName); //free memory

    return;
}

void SearchMenu(WINDOW *win) { //search menu
    _Bool quit = false; //check if option is valid
    sds *choices, menuName = sdsnew(" Search Menu ");
    char *items[] = { "Search with export to CSV", "Search only", "Back to Main Menu", NULL };

    REWIND(contacts);
    if(!contacts) { //if no contacts in list
        messageBox(win, 10, " No contacts found in the DataBase! ", COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuName);

        return;
    }

    choices = buildMenuItems(items); //build the menu items
    if(!choices) {
        sdsfree(menuName); //free memory

        return;
    }

    //start the operations
    while(!quit) {
        switch (flexMenu(win, choices, menuName)) {
            case 1:
                do_search(win, true);
                break;
            case 2:
                do_search(win, false);
                break;
            case 3: //back menu was selected
            case 0: //ESCape was pressed
                quit = true;
                break; //back to Main Menu
            default:
                break;
        }
        wrefresh(win);
        wclear(win);
    }

    sdsfreesplitres(choices, ARRAY_SIZE(items)); //free memory
    sdsfree(menuName); //free memory

    return; //back to Main Menu
}

int do_search(WINDOW *win, _Bool csv_export) {
    DBnode_t *dbFlex;
    PhoneBook_t *ptr, *resultList = NULL;
    int nb_records, rows;
    _Bool found, modified = false;
    sds menuCriteria = sdsnew(" Search Filters "), menuName = sdsnew(" Show / Modify ");

    dbFlex = malloc(sizeof(DBnode_t)); //reserve memory for node
    memset(dbFlex, 0, sizeof(DBnode_t)); //set to 0 everything in the node
    dbFlex->id = countList(contacts); //count the number of list contacts
    mvwprintw(win, 18, 2, "* The number highlighted in the upper left corner shows the contacts count!");

    if(flexForm(win, dbFlex, menuCriteria) != 1) { //fill the criteria to search (1 mean confirm, 0 mean abort, -1 mean error)
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(&dbFlex);

        return 0;
    }

    for(nb_records = 0, ptr = contacts; ptr; ptr = ptr->next) { //search if with input filters match with some contacts
        found = false; //reset to false for every cycle that means every record
        if(sdscasesds(ptr->db.fname,        dbFlex->fname) >= 0)        found = true; //for better readability I have aligned the code
        if(sdscasesds(ptr->db.lname,        dbFlex->lname) >= 0)        found = true; //if only one of the filed match with criteria
        if(sdscasesds(ptr->db.organization, dbFlex->organization) >= 0) found = true; //the boolean variable is set to found
        if(sdscasesds(ptr->db.job,          dbFlex->job) >= 0)          found = true;
        if(sdscasesds(ptr->db.hphone,       dbFlex->hphone) >= 0)       found = true;
        if(sdscasesds(ptr->db.wphone,       dbFlex->wphone) >= 0)       found = true;
        if(sdscasesds(ptr->db.pmobile,      dbFlex->pmobile) >= 0)      found = true;
        if(sdscasesds(ptr->db.bmobile,      dbFlex->bmobile) >= 0)      found = true;
        if(sdscasesds(ptr->db.pemail,       dbFlex->pemail) >= 0)       found = true;
        if(sdscasesds(ptr->db.bemail,       dbFlex->bemail) >= 0)       found = true;
        if(sdscasesds(ptr->db.address,      dbFlex->address) >= 0)      found = true;
        if(sdscasesds(ptr->db.zip,          dbFlex->zip) >= 0)          found = true;
        if(sdscasesds(ptr->db.city,         dbFlex->city) >= 0)         found = true;
        if(sdscasesds(ptr->db.state,        dbFlex->state) >= 0)        found = true;
        if(sdscasesds(ptr->db.country,      dbFlex->country) >= 0)      found = true;

        if(found) { //check if at least one of criteria was found in current contact field
            addNode(&resultList, ptr->db); //if found some store the contacts in temp list
            modified = true; //set at least one time if something was found walking the list
            nb_records++; //count the numbers of records
        }
    }

    if(!modified) { //if nothing was found
        messageBox(win, 10, " No contacts was found with input filters ", COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(&dbFlex);
        destroyList(&resultList); //destroy the result list

        return 0;
    }
    logfile("%s: %d records found!\n", __func__, nb_records);

    if(csv_export) { //if the user choose with export
        rows = write_csv(SEARCH_CSV, resultList); //export the results to csv file
        if(rows == -1) {
            sdsfree(menuCriteria); //destroy the criteeria string
            sdsfree(menuName); //destory the menu name
            destroyNode(&dbFlex);
            destroyList(&resultList); //destroy the result list

            return -1;
        }
        logfile("%s: Exported %d rows to %s\n", __func__, rows, SEARCH_CSV);
    }

    REWIND(resultList);
    UpdateMenu(win, resultList, menuName, menuCriteria); //this is the main cycle to manage the result of search
 
    sdsfree(menuCriteria); //destroy the criteeria string
    sdsfree(menuName); //destory the menu name
    destroyNode(&dbFlex);
    destroyList(&resultList); //destroy the result list

    return nb_records;
}

void AddMenu(WINDOW *win) { //add menu
    int rc;
    _Bool store = false;

    do {
        DBnode_t *dbFlex = initNode(contacts); //create and initialize a node
        if(!dbFlex) {
            logfile("%s: Error allocating memory\n", __func__);

            return;
        }
        rc = flexForm(win, dbFlex, " Add New Contact "); //create a new entry node only if the user confirm the input than 1 is returned
        //flexForm can return 1 mean confirm, 0 mean abort, -1 mean error
        if(rc == -1) { //error in func flexForm()
                logfile("%s: Error adding new contact\n", __func__);
        } else if(rc == 1) { //the user input a new contact
            store = addNode(&contacts, (*dbFlex)) ? true : store; //if at least one time store is set to true, it remain true, otherwise false
            REWIND(contacts);
        }
        destroyNode(&dbFlex);
    } while(rc == 1); //cycle while the user inputs new contact

    if(store) {
        rc = write_db(false); //write the contacts list to sqlite file database only if user confirm at least one time with param fale means INSERT
        logfile("%s: Inserted %d records in %s DataBase\n", __func__, rc, DB);
    }

    return;
}

void UpdateMenu(WINDOW *win, PhoneBook_t *resultList, sds menuName, sds menuModify) {
    PhoneBook_t *ptr = resultList;
    DBnode_t *dbModify;
    sds *menuList;
    int nb_fields, i, choice;
    unsigned int id = 0;
    _Bool modified = false, delete = false;

    REWIND(ptr); //in order to be sure that ptr pointer go to head of list
    if(!ptr) { //if no contacts in list
        messageBox(win, 10, " No contacts found in the DataBase! ", COLOR_PAIR(PAIR_EDIT));

        return;
    }

    menuList = buildMenuList(ptr, &nb_fields); //build the list from resultList
    if(!menuList) {
        return;
    }

    do {
        choice = flexMenu(win, menuList, menuName); //pass the contacts list to felxMenu to elaborate it
        if(choice) { //if a choice is made
            if(choice < 0) { //check if the contact is deleted
                choice *= -1; //invert the sign to manage the right indexing
                delete = true; //the field must be deleted
            }
            REWIND(resultList); //in order to be sure that resultList pointer go to head of list
            ptr = resultList;        
            for(i = 1; ptr && i < choice; i++) {  //walk to the choiced contact index
                NEXT(ptr);
            }
            if(!ptr) {
                logfile("%s: The menu item %d selected is out of list\n", __func__, choice);
                sdsfreesplitres(menuList, nb_fields); //release the memory

                return;
            }
            REWIND(contacts); //in order to be sure that contacts pointer go to head of list
            if(resultList != contacts) { //if the function is called from search menu this is true
                id = ptr->db.id; //store the value of id
                for(ptr = contacts; ptr && (ptr->db.id) != id; ptr = ptr->next); //walk contacts up to id
            }
            if(!ptr) {
                logfile("%s: The menu item %d selected is out of list\n", __func__, choice);
                sdsfreesplitres(menuList, nb_fields); //release the memory

                return;
            }
            dbModify = &(ptr->db); //assign the choiced contact to modify ptr
            i = choice -1; //assign the correct index number
            if(delete) {
                if(ptr->db.delete) { //check if the contact is already set to delete
                    sdsfree(menuList[i]); //wipe the old field
                    menuList[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); //rebuild the modified contact
                    ptr->db.delete = false; //set delete to false
                } else {
                    choice = messageBox(win, 10, " Press 'Y' to confirm deletion of any key to discard ", COLOR_PAIR(PAIR_EDIT));
                    if(toupper(choice) != 'Y') { //if confirm mark as deleted
                        continue; //back to the begin of do-while
                    }
                    sdsfree(menuList[i]); //wipe the old field
                    menuList[i] = (sds) sdscatprintf(sdsempty(), "*%s %s* # CANC to Restore #", ptr->db.fname, ptr->db.lname); //rebuild the modified contact
                    ptr->db.delete = true; //mark as deleted
                }
                modified = true; //set because of something is modified
                delete = false; //restore delete to false
            } else if(flexForm(win, dbModify, menuModify) == 1) { //pass the modify ptr to form for modification of contact (1 mean confirm, 0 mean abort, -1 mean error)
                sdsfree(menuList[i]); //wipe the old field
                menuList[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); //rebuild the modified contact
                ptr->db.delete = false; //if was deleted now is restored
                modified = true; //if contact is modified
            }
        }
    } while(choice); //if no selection exit flexMenu return 0 if nothing was selected

    sdsfreesplitres(menuList, nb_fields); //release the memory
    
    if(modified) { //if at least one contact was modified
        nb_fields = write_db(true); //if the modification is accepted write the new db with param true means UPDATE
        logfile("%s: Updated %d records in %s DataBase\n", __func__, nb_fields, DB);
    }

    return;
}

void ImpExpMenu(WINDOW *win) { //search menu
    _Bool quit = false; //check if option is valid
    int rows;
    sds  *choices, menuName = sdsnew(" Import & Export Menu ");
    char *items[] = { "Import from CSV", "Import from Google CSV", "Export to CSV", "Export to Google CSV", "Back to Main Menu", NULL };

    choices = buildMenuItems(items); //build the menu items
    if(!choices) {
        sdsfree(menuName); //free memory

        return;
    }

    //start the operations
    while(!quit) {
        sds csvFile = sdsempty(); //create an empty sds string
        char *message = NULL; //create a NULL string
        switch(flexMenu(win, choices, menuName)) {
            case 1: //Import from CSV
                csvFile = sdscpy(csvFile, DB_CSV); //assign the standard csv file name
            case 2: //Import from Google CSV
                if(sdslen(csvFile) == 0) { //check if the file name is already set
                    csvFile = sdscpy(csvFile, GOOGLE_CSV); //assign the standard Google csv file name
                }
                rows = importCSV(csvFile); //import the contacts from csv
                if(rows > 0) { //check if records are exported
                    asprintf(&message, " Imported %d records from '%s' successfully. ", rows, csvFile); //build message string
                } else {
                    asprintf(&message, " No records imported from '%s' ", csvFile); //build message string
                }
                messageBox(win, 15, message, COLOR_PAIR(PAIR_MODIFIED));
                if(rows > 0) {
                    rows = write_db(false); //write the imported contacts list to sqlite file database
                    logfile("%s: Imported %d rows in '%s' DataBase\n", __func__, rows, DB);
                } else {
                    logfile("%s: %s\n", __func__, message); //write result to log file
                }
                break;
            case 3: //Export to CSV
                rows = write_csv(DB_CSV, contacts); //write or append records to DB_CSV
                if(rows > 0) { //check if records are exported
                    asprintf(&message, " Exported %d records to '%s' successfully ", rows, DB_CSV); //build message string
                } else {
                    asprintf(&message, " No records exported to '%s' ", DB_CSV); //build message string
                }
                messageBox(win, 15, message, COLOR_PAIR(PAIR_MODIFIED));
                logfile("%s: %s\n", __func__, message); //write result to log file
                break;
            case 4: //Export to Google CSV
                rows = write_csv(GOOGLE_CSV, contacts); //write or append records to GOOGLE_CSV
                if(rows > 0) { //check if records are exported
                    asprintf(&message, " Exported %d records to '%s' successfully ", rows, GOOGLE_CSV); //build message string
                } else {
                    asprintf(&message, " No records exported to '%s' ", GOOGLE_CSV); //build message string
                }
                messageBox(win, 15, message, COLOR_PAIR(PAIR_MODIFIED));
                logfile("%s: %s\n", __func__, message); //write result to log file
                break;
            case 5: //Back to Main Menu
            case 0: //ESCape was pressed
                quit = true;
                break; //back to Main Menu
            default:
                break;
        }
        if(message) {
            free(message); //realese the memory
        }
        sdsfree(csvFile); //realese the memory
        wrefresh(win);
        wclear(win);
    }

    rows = ARRAY_SIZE(items);
    sdsfreesplitres(choices, rows); //free memory
    sdsfree(menuName); //free memory

    return; //back to Main Menu
}

sds *buildMenuItems(char **items) {
    sds *menuChoices;
    int i, n;

    if(!items) {
        return NULL;
    }

    for(n = 0; items[n]; n++); //count the number of items (NULL terminated)
    menuChoices = (sds *) calloc(n+1, sizeof(sds)); //calculate one more also for empity sds

    for(i = 0; i < n; i++) {
        menuChoices[i] = sdscatprintf(sdsempty(), "[%d] %s", i+1, items[i]);
    }
    menuChoices[i] = sdsempty(); //last item must be empty

    return (sds *) menuChoices;
}

sds *buildMenuList(PhoneBook_t *fromList, int *nb_fileds) {
    PhoneBook_t *ptr = fromList;
    sds *menuList, fname, lname;
    int i;

    if(!ptr) { //check if the list is empty
        logfile("%s: No contacts to build a list|\n", __func__);
        (*nb_fileds) = 0; //no items in the menu list

        return (sds *) NULL;
    }

    i = countList(ptr) +1; //count the number of field found
    menuList = (sds *) calloc(i, sizeof(sds)); //allocate the memory for contact list

    for(i = 0; ptr; i++) { //walk thru the list
        fname = sdsnew(ptr->db.fname);
        fname = sdschremove(fname, SPECIAL_CHARS); //remove special chars from sds
        lname = sdsnew(ptr->db.lname);
        lname = sdschremove(lname, SPECIAL_CHARS); //remove special chars from sds

        menuList[i] = (sds) sdscatprintf(sdsempty(), "%s %s", fname, lname); //build the menu list
        sdsfree(fname);
        sdsfree(lname);
        NEXT(ptr);
    }
    menuList[i++] = sdsempty(); //assign NULL string to the last contact of the list
    (*nb_fileds) = i; //assign the total menu items

    return (sds *) menuList;
}
