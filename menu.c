#include "phonebook.h"

sds *buildMenuItems(char **items, int numbers) {
    sds *menuChoices;
    int i;

    if(!items) {
        return NULL;
    }

    menuChoices = (sds *) calloc(numbers, sizeof(sds)); //calculate one more also for empity sds

    numbers--; //decrease the index in order to add the empty item after for cycle
    for(i = 0; i < numbers; i++) {
        menuChoices[i] = sdscatprintf(sdsempty(), "[%d] %s", i+1, items[i]);
    }
    menuChoices[i] = sdsempty(); //last item must be empty

    return (sds *) menuChoices;
}

void MainMenu(WINDOW *win) { //main menu
    _Bool quit = false;
    int ch, n_choices; //number of menu items
    sds *choices;
    sds menuName = sdsnew(" Main Menu "), menuUpdate = sdsnew(" Show / Modify "), menuModify = sdsnew(" Modify ");
    char *items[] = { "Search Contact", "Add new Contact", "Show / Modify", "Import / Export", "Utility", "Exit" };

    REWIND(contacts);
    if(!contacts) { //the list is empty
        ch = read_db(); //read the sqlite file database and store it in the contacts list
        if(ch == -1) { //an error was found reading db
            logfile("%s: Error reading DB\n", __func__);

            sdsfree(menuModify); //free memory
            sdsfree(menuUpdate); //free memory
            sdsfree(menuName); //free memory

            return;
        }
        logfile("%s: DataBase '%s' is successfully opened, %d contacts readed...\n", __func__, DB, ch);
    }

    wrefresh(win);
    wclear(win);

    n_choices = ARRAY_SIZE(items) +1;  //calculate one more also for empity sds
    choices = buildMenuItems(items, n_choices); //build the menu items
    if(!choices) {
        sdsfree(menuModify); //free memory
        sdsfree(menuUpdate); //free memory
        sdsfree(menuName); //free memory

        return;
    }


    //start the operations
    do {
        REWIND(contacts); //at each interation we are sure that the pointer contacts stay at head of list
        switch(flexMenu(win, choices, n_choices, menuName)) {
            case 1:
                SearchMenu(win);
                break;
            case 2:
                AddMenu(win);
                break;
            case 3:
                UpdateMenu(win, contacts, menuUpdate, menuModify);
                break;
            case 4:
                break;
            case 5:
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

    for(int i = 0; i < n_choices; i++) { //walk through array index
        sdsfree(choices[i]); //free memory
    }
    sdsfree(menuModify); //free memory
    sdsfree(menuUpdate); //free memory
    sdsfree(menuName); //free memory

    return;
}

void SearchMenu(WINDOW *win) { //search menu
    _Bool quit = false; //check if option is valid
    int n_choices; //number of menu items
    sds *choices, menuName = sdsnew(" Search Menu ");
    char *items[] = { "Search with export to CSV", "Search only", "Back to Main Menu" };

    REWIND(contacts);
    if(!contacts) { //if no contacts in list
        messageBox(win, 10, " No contacts found in the DataBase! ", COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuName);

        return;
    }

    n_choices = ARRAY_SIZE(items) +1;  //calculate one more also for empity sds
    choices = buildMenuItems(items, n_choices); //build the menu items
    if(!choices) {
        sdsfree(menuName); //free memory

        return;
    }

    //start the operations
    while(!quit) {
        switch (flexMenu(win, choices, n_choices, menuName)) {
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

    for(int i = 0; i < n_choices; i++) { //walk through array index
        sdsfree(choices[i]); //free memory
    }
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
    do {
        choice = flexMenu(win, menuList, nb_fields, menuName); //pass the contacts list to felxMenu to elaborate it
        if(choice) { //if a choice is made
            if(choice < 0) { //check if the contact is deleted
                choice *= -1; //invert the sign to manage the right indexing
                delete = true; //the field must be deleted
            }
            REWIND(ptr); //in order to be sure that ptr pointer go to head of list
            for(i = 1; ptr && i < choice; i++) {  //walk to the choiced contact index
                NEXT(ptr);
            }
            if(!ptr) {
                logfile("%s: The menu item %d selected is out of list\n", __func__, choice);
                freeMenuList(&menuList, nb_fields); //release the memory

                return;
            }
            REWIND(resultList); //in order to be sure that resultList pointer go to head of list
            REWIND(contacts); //in order to be sure that contacts pointer go to head of list
            if(resultList != contacts) { //if the function is called from search menu this is true
                id = ptr->db.id; //store the value of id
                for(ptr = contacts; ptr && (ptr->db.id) != id; ptr = ptr->next); //walk contacts up to id
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
    
    freeMenuList(&menuList, nb_fields); //release the memory
    
    if(modified) { //if at least one contact was modified
        nb_fields = write_db(true); //if the modification is accepted write the new db with param true means UPDATE
        logfile("%s: Updated %d records in %s DataBase\n", __func__, nb_fields, DB);
    }

    return;
}

sds *buildMenuList(PhoneBook_t *fromList, int *nb_fileds) {
    PhoneBook_t *ptr = fromList;
    sds *menuList;
    int i;

    if(!ptr) { //check if the list is empty
        logfile("%s: No contacts to build a list|\n", __func__);
        (*nb_fileds) = 0; //no items in the menu list

        return (sds *) NULL;
    }

    i = countList(ptr) +1; //count the number of field found
    menuList = (sds *) calloc(i, sizeof(sds)); //allocate the memory for contact list

    for(i = 0; ptr; i++) { //walk thru the list
        menuList[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); //build the menu list
        NEXT(ptr);
    }
    menuList[i++] = sdsempty(); //assign NULL string to the last contact of the list
    (*nb_fileds) = i; //assign the total menu items

    return (sds *) menuList;
}

void freeMenuList(sds **menuList, int nb_fields) {
    for(int i = 0; i < nb_fields; i++) { //free the contacts list
        sdsfree((*menuList)[i]); //free the current contact of the list
    }
    if((*menuList)) { //check if the pointer is valid
        free((*menuList)); //release the memory
    }
    NULLSET((*menuList));

    return;
}
