#include "phonebook.h"

sds *buildMenuItems(char **items, int numbers) {
    sds *menuChoices;
    int i = numbers;

    if(!items) {
        return NULL;
    }

    menuChoices = (sds *) calloc(i, sizeof(sds)); //calculate one more also for empity sds

    for(i = 0; i < (numbers -1); i++) {
        menuChoices[i] = sdscatprintf(sdsempty(), "[%d] %s", i+1, items[i]);
    }
    menuChoices[i] = sdsempty();

    return (sds *) menuChoices;
}

void MainMenu(WINDOW *win) { //main menu
    bool quit = FALSE;
    int ch, n_choices; //number of menu items
    sds *choices;
    sds menuName = sdsnew(" Main Menu "), menuUpdate = sdsnew(" Show / Modify "), menuModify = sdsnew(" Modify ");
    char *items[] = { "Search Contact", "Add new Contact", "Show / Modify", "Import / Export", "Utility", "Exit" };

    wrefresh(win);
    wclear(win);

    n_choices = ARRAY_SIZE(items) +1;  //calculate one more also for empity sds
    choices = buildMenuItems(items, n_choices);
    if(!choices) {
        sdsfree(menuModify); //free memory
        sdsfree(menuUpdate); //free memory
        sdsfree(menuName); //free memory

        return;
    }

    if(!contacts) { //new database
        AddMenu(win);
        wrefresh(win);
        wclear(win);
    }

    //start the operations
    do {
        switch(flexMenu(win, choices, n_choices, menuName)) {
            case 1:
                SearchMenu(win);
                break;
            case 2:
                AddMenu(win);
                break;
            case 3:
                UpdateMenu(win, contacts, menuUpdate, menuModify, FALSE);
                break;
            case 4:
                wprintw(win, "4");
                break;
            case 5:
                break;
            case 6: //Menu Exit was selected
            case 0: //ESCape key was pressed
                ch = messageBox(win, 10, " press Y to quit... ", COLOR_PAIR(PAIR_EDIT));
                if(toupper(ch) == 'Y') {
                    quit = TRUE;
                }
                break;
            default:
                break;
        }
        wrefresh(win);
        wclear(win);
    } while(quit != TRUE);

    while(--n_choices >= 0) { //walk through array index
        sdsfree(choices[n_choices]); //free memory
    }
    sdsfree(menuModify); //free memory
    sdsfree(menuUpdate); //free memory
    sdsfree(menuName); //free memory

    return;
}

void SearchMenu(WINDOW *win) { //search menu
    bool quit = FALSE; //check if option is valid
    int n_choices; //number of menu items
    sds *choices, menuName = sdsnew(" Search Menu ");
    char *items[] = { "Search with export to CSV", "Search only", "Back to Main Menu" };

    if(!contacts) { //if no contacts in list
        messageBox(win, 10, " No contacts was found in DataBase! ", COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuName);

        return;
    }

    n_choices = ARRAY_SIZE(items) +1;  //calculate one more also for empity sds
    choices = buildMenuItems(items, n_choices);
    if(!choices) {
        sdsfree(menuName); //free memory

        return;
    }

    //start the operations
    while (quit != TRUE) {
        switch (flexMenu(win, choices, n_choices, menuName)) {
            case 1:
                do_search(win, TRUE);
                break;
            case 2:
                do_search(win, FALSE);
                break;
            case 3: //back menu was selected
            case 0: //ESCape was pressed
                quit = TRUE;
                break; //back to Main Menu
            default:
                break;
        }
        wrefresh(win);
        wclear(win);
    }

    while(--n_choices >= 0) { //walk through array index
        sdsfree(choices[n_choices]); //free memory
    }
    sdsfree(menuName); //free memory

    //back to Main Menu

    return;
}

//the main search function @exp = allow exports?
int do_search(WINDOW *win, bool csv_export) {
    DBnode_t *dbFlex;
    PhoneBook_t *ptr, *resultList = NULL;
    int nb_records, rows;
    _Bool found, modified = false;
    sds menuCriteria = sdsnew(" Search Filters "), menuName = sdsnew(" Show / Modify ");

    dbFlex = malloc(sizeof(DBnode_t)); //reserve memory for node
    memset(dbFlex, 0, sizeof(DBnode_t)); //set to 0 everything in the node
    if(flexForm(win, &dbFlex, menuCriteria) != 1) { //fill the criteria to search (1 mean confirm, 0 mean abort, -1 mean error)
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(dbFlex);

        return 0;
    }

    for(nb_records = 0, ptr = contacts; ptr; ptr = ptr->next) { //search if with input filters match with some contacts
        found = FALSE; //reset to false for every cycle that means every record
        if(sdscasesds(ptr->db.fname,        dbFlex->fname) >=0)         found = TRUE; //for better readability I have aligned the code
        if(sdscasesds(ptr->db.lname,        dbFlex->lname) >=0)         found = TRUE; //if only one of the filed match with criteria
        if(sdscasesds(ptr->db.organization, dbFlex->organization) >=0)  found = TRUE; //the boolean variable is set to found
        if(sdscasesds(ptr->db.job,          dbFlex->job) >=0)           found = TRUE;
        if(sdscasesds(ptr->db.hphone,       dbFlex->hphone) >=0)        found = TRUE;
        if(sdscasesds(ptr->db.wphone,       dbFlex->wphone) >=0)        found = TRUE;
        if(sdscasesds(ptr->db.pmobile,      dbFlex->pmobile) >=0)       found = TRUE;
        if(sdscasesds(ptr->db.bmobile,      dbFlex->bmobile) >=0)       found = TRUE;
        if(sdscasesds(ptr->db.pemail,       dbFlex->pemail) >=0)        found = TRUE;
        if(sdscasesds(ptr->db.bemail,       dbFlex->bemail) >=0)        found = TRUE;
        if(sdscasesds(ptr->db.address,      dbFlex->address) >=0)       found = TRUE;
        if(sdscasesds(ptr->db.zip,          dbFlex->zip) >=0)           found = TRUE;
        if(sdscasesds(ptr->db.city,         dbFlex->city) >=0)          found = TRUE;
        if(sdscasesds(ptr->db.state,        dbFlex->state) >=0)         found = TRUE;
        if(sdscasesds(ptr->db.country,      dbFlex->country) >=0)       found = TRUE;

        if(found) { //check if at least one of criteria was found in current contact field
            addNode(&resultList, &ptr->db); //if found some store the contacts in temp list
            modified = true; //set at least one time if something was found walking the list
            nb_records++; //count the numbers of records
        }
    }

    if(!modified) { //if nothing was found
        messageBox(win, 10, " No contacts was found with input filters ", COLOR_PAIR(PAIR_EDIT));
        sdsfree(menuCriteria);
        sdsfree(menuName);
        destroyNode(dbFlex);

        return 0;
    }
    logfile("%s: Found %d records\n", __func__, nb_records);

    if(csv_export) { //if the user choose with export
        rows = write_csv(SEARCH_CSV, resultList); //export the results to csv file
        if(rows == -1) {
            sdsfree(menuCriteria); //destroy the criteeria string
            sdsfree(menuName); //destory the menu name
            destroyNode(dbFlex);
            destroyList(resultList); //destroy the result list

            return -1;
        }
        logfile("%s: Exported %d rows to %s\n", __func__, rows, SEARCH_CSV);
    }

    REWIND(resultList);
    UpdateMenu(win, resultList, menuName, menuCriteria, TRUE); //this is the main cycle to manage the result of search
 
    sdsfree(menuCriteria); //destroy the criteeria string
    sdsfree(menuName); //destory the menu name
    destroyNode(dbFlex);
    destroyList(resultList); //destroy the result list

    return nb_records;
}

void AddMenu(WINDOW *win) { //add menu
    int rc;
    _Bool store = false;

    do {
        DBnode_t *dbFlex = initNode(contacts);
        if(!dbFlex) {
            logfile("%s: Error allocating memory\n", __func__);

            return;
        }
        rc = flexForm(win, &dbFlex, " Add New Contact "); //create a new entry node only if the user confirm the input than 1 is returned
        //flexForm can return 1 mean confirm, 0 mean abort, -1 mean error
        if(rc == -1) { //error in func flexForm()
                logfile("%s: Error adding new contact\n", __func__);
        } else if(rc == 1) { //add new contact to the list
            store = addNode(&contacts, dbFlex) ? true : store; //if at least one time store is set to true, it remain true, otherwise false
        }
        destroyNode(dbFlex);
        REWIND(contacts);
    } while(rc == 1); //cycle while the user inputs new contact

    if(store) {
        write_db(win); //write the contacts list to sqlite file database only if user confirm at least one time
    }


    return;
}

void UpdateMenu(WINDOW *win, PhoneBook_t *resultList, sds menuName, sds menuModify, _Bool search) {
    PhoneBook_t *ptr = resultList;
    DBnode_t *dbModify;
    sds *contacts_list;
    int nb_fields, i, choice;
    unsigned int id = 0;
    _Bool modified = false, delete = false;

    if(!ptr) { //if no contacts in list
        messageBox(win, 10, " No contacts was found in DataBase! ", COLOR_PAIR(PAIR_EDIT));

        return;
    }

    contacts_list = buildcontacts(ptr, &nb_fields); //build the list from resultList

    do {
        choice = flexMenu(win, contacts_list, nb_fields, menuName); //pass the contacts list to felxMenu to elaborate it
        if(choice) { //if a choice is made
            if(choice < 0) {
                choice *= -1; //invert the sign
                delete = true; //the field must be deleted
            }
            for(ptr = resultList, i = 1; ptr && i < choice; ptr = ptr->next, i++); //walk to the choiced contact index
            if(search) {
                if(ptr) {
                    id = ptr->db.id;
                }
                for(ptr = contacts; ptr && (ptr->db.id) != id; ptr = ptr->next); //walk contacts up to id
            }
            dbModify = &(ptr->db); //assign the choiced contact to modify ptr
            i = choice -1; //assign the correct index number
            if(delete) {
                if(ptr->db.delete) {
                    sdsfree(contacts_list[i]); //wipe the old field
                    contacts_list[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); //rebuild the modified contact
                    ptr->db.delete = false; //set delete to false
                } else {
                    choice = messageBox(win, 10, " Press 'Y' to confirm deletion of any key to discard ", COLOR_PAIR(PAIR_EDIT));
                    if(toupper(choice) != 'Y') { //if confirm mark as deleted
                        continue;
                    }
                    sdsfree(contacts_list[i]); //wipe the old field
                    contacts_list[i] = (sds) sdscatprintf(sdsempty(), "*%s %s* # CANC to restore #", ptr->db.fname, ptr->db.lname); //rebuild the modified contact
                    ptr->db.delete = true; //mark as deleted
                }

                modified = true; //set because of something is modified
                delete = false; //restore delete to false
            } else if(flexForm(win, &dbModify, menuModify) == 1) { //pass the modify ptr to form for modification of contact (1 mean confirm, 0 mean abort, -1 mean error)
                sdsfree(contacts_list[i]); //wipe the old field
                contacts_list[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname); //rebuild the modified contact

                ptr->db.delete = false; //if was deleted now is restored
                modified = true; //if contact is modified
            }
        }
    } while(choice); //if no selection exit flexMenu return 0 if nothing was selected

    if(modified) { //if at least one contact was modified
        update_db(win); //if the modification is accepted write the new db
    }
    destroycontacts(contacts_list, nb_fields);
    if(contacts_list) {
        free(contacts_list); //release the memory of the list
    }

    return;
}

sds *buildcontacts(PhoneBook_t *fromList, int *nb_fileds) {
    PhoneBook_t *ptr = fromList;
    sds *contacts_list;
    int i;

    if(!ptr) {
        logfile("%s: No contacts to build a list|\n", __func__);
        (*nb_fileds) = 0;

        return (sds *) NULL;
    }

    i = countList(ptr) +1; //count the number of field found
    contacts_list = (sds *) calloc(i, sizeof(sds)); //allocate the memory for contact list

    for(i = 0; ptr; i++) {
        contacts_list[i] = (sds) sdscatprintf(sdsempty(), "%s %s", ptr->db.fname, ptr->db.lname);
        NEXT(ptr);
    }
    contacts_list[i++] = sdsempty(); //assign NULL string to the last contact of the list
    (*nb_fileds) = i;

    return (sds *) contacts_list;
}

void destroycontacts(sds *contacts_list, int nb_fields) {
    int i;

    for(i = 0; i < nb_fields; i++) { //free the contacts list
        sdsfree(contacts_list[i]); //free the current contact of the list
    }

    return;
}
