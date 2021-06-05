#include "phonebook.h"

PhoneBook_t *newNode(DBnode_t node) {
    PhoneBook_t *dbNode;
    
    dbNode = (PhoneBook_t *) malloc(sizeof(PhoneBook_t));
    if(!dbNode) {
        logfile("%s: Memory allocation error\n", __func__);

        return dbNode;
    }

    //copy all contents from DBnode passed from the call of addNode() function    
    dbNode->db.delete       = node.delete;
    dbNode->db.modified     = node.modified;
    dbNode->db.id           = node.id;

    dbNode->db.fname   		= sdsnew(node.fname);
    dbNode->db.lname	    = sdsnew(node.lname);
    dbNode->db.organization = sdsnew(node.organization);
    dbNode->db.job	    	= sdsnew(node.job);
    dbNode->db.hphone	    = sdsnew(node.hphone);
    dbNode->db.wphone	    = sdsnew(node.wphone);
    dbNode->db.pmobile	    = sdsnew(node.pmobile);
    dbNode->db.bmobile	    = sdsnew(node.bmobile);
    dbNode->db.pemail	    = sdsnew(node.pemail);
    dbNode->db.bemail	    = sdsnew(node.bemail);
    dbNode->db.address      = sdsnew(node.address);
    dbNode->db.zip          = sdsnew(node.zip);
    dbNode->db.city         = sdsnew(node.city);
    dbNode->db.state        = sdsnew(node.state);
    dbNode->db.country      = sdsnew(node.country);

    dbNode->db.birthday.tm_mday = node.birthday.tm_mday;
    dbNode->db.birthday.tm_mon  = node.birthday.tm_mon;
    dbNode->db.birthday.tm_year = node.birthday.tm_year;

    NULLSET(dbNode->prev); //set prev node to NULL pointer
    NULLSET(dbNode->next); //set next node to NULL pointer
    
    return (PhoneBook_t *) dbNode;
}

PhoneBook_t *addNode(PhoneBook_t **list, DBnode_t node) { //push the db data in global contacts list in sorted position
    PhoneBook_t *dbNode;
    
    dbNode = newNode(node);
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

DBnode_t *initNode(PhoneBook_t *list) {
    PhoneBook_t *ptr = list;
    DBnode_t *node;

    node = (DBnode_t *) malloc(sizeof(DBnode_t)); //reserve memory for db
    if(!node) {
        logfile("%s: Error allocating memory\n", __func__);

        return NULL;
    }

    node->delete = false; //always set delete flag to false
    node->modified = false; //always set modified flag to false
    FORWARD(ptr); //go to the tail of contacts list
    node->id = ptr ? (ptr->db.id +1) : 1; //if ptr is NULL means that it's a new list

    node->fname   		= sdsempty();
    node->lname	        = sdsempty();
    node->organization  = sdsempty();
    node->job	    	= sdsempty();
    node->hphone	    = sdsempty();
    node->wphone	    = sdsempty();
    node->pmobile	    = sdsempty();
    node->bmobile	    = sdsempty();
    node->pemail	    = sdsempty();
    node->bemail	    = sdsempty();
    node->address       = sdsempty();
    node->zip           = sdsempty();
    node->city          = sdsempty();
    node->state         = sdsempty();
    node->country       = sdsempty();

    node->birthday.tm_mday = 1;
    node->birthday.tm_mon  = 1;
    node->birthday.tm_year = 1900;

    return (DBnode_t *) node;
}

void deleteNode(PhoneBook_t **list, PhoneBook_t *del) {
    if (!(*list) || !del) { //nothing to delete
        return;
    }
 
    if ((*list) == del) { // If node to be deleted is head node
        (*list) = del->next;
    }
 
    if (del->next != NULL) { // Change next only if node to be deleted is NOT the last node
        del->next->prev = del->prev;
    }

    if (del->prev != NULL) { /// Change prev only if node to be deleted is NOT the first node
        del->prev->next = del->next;
    }
    //free all sds strings
    sdsfree(del->db.fname);
    sdsfree(del->db.lname);
    sdsfree(del->db.organization);
    sdsfree(del->db.job);
    sdsfree(del->db.hphone);
    sdsfree(del->db.wphone);
    sdsfree(del->db.pmobile);
    sdsfree(del->db.bmobile);
    sdsfree(del->db.pemail);
    sdsfree(del->db.bemail);
    sdsfree(del->db.address);
    sdsfree(del->db.zip);
    sdsfree(del->db.city);
    sdsfree(del->db.state);
    sdsfree(del->db.country);

    free(del); // Finally, free the memory occupied by del
    NULLSET(del);

    return;
}

int countList(PhoneBook_t *list) {
    PhoneBook_t *ptr = list;
    int count;

	REWIND(ptr); //rewind the list up to the first node
	for(count = 0; ptr; count++) { //walk thru the entire list to count the numbers of contacts
		NEXT(ptr);
	}

    return count; //return the numbers of contacts counted
}

void destroyNode(DBnode_t *node) {
    if(node) {
		//free all sds strings
		sdsfree(node->fname);
        sdsfree(node->lname);
        sdsfree(node->organization);
        sdsfree(node->job);
        sdsfree(node->hphone);
        sdsfree(node->wphone);
        sdsfree(node->pmobile);
        sdsfree(node->bmobile);
        sdsfree(node->pemail);
        sdsfree(node->bemail);
        sdsfree(node->address);
        sdsfree(node->zip);
        sdsfree(node->city);
        sdsfree(node->state);
        sdsfree(node->country);

        free(node); //destroy the node
        NULLSET(node); //set *node pointer to NULL
    }

	return;
}

void destroyList(PhoneBook_t *list) {
    PhoneBook_t *nextPtr;

    REWIND(list); //rewind the list up to the first node
    while(list) { //walk thru complete contacts list
        nextPtr = list->next; //store point nextPtr to next node before destroy it
        deleteNode(&list, list); //delete node
        list = nextPtr; //step to next node
    }

	return;
}

void nodeDBswap(PhoneBook_t *left, PhoneBook_t *right) {
    DBnode_t tmp;

    tmp = left->db;
    left->db = right->db;
    right->db = tmp;

    left->db.modified = right->db.modified = true;

    return;
}

PhoneBook_t *partition(PhoneBook_t *head, PhoneBook_t *tail, _Bool compare(PhoneBook_t *first, PhoneBook_t *second)) {
	PhoneBook_t *ptr, *prevHead = head->prev;

	for(ptr = head; ptr != tail; NEXT(ptr)) {
		if(compare(ptr, tail)) {
			prevHead = prevHead ? prevHead->next : head;

			nodeDBswap(prevHead, ptr);
		}
	}
	prevHead = prevHead ? prevHead->next : head;
	nodeDBswap(prevHead, tail);

	return prevHead;
}

void QuickSort(PhoneBook_t *head, PhoneBook_t *tail, _Bool compare(PhoneBook_t *first, PhoneBook_t *second)) {
	if(tail && head != tail && head != tail->next) {
        PhoneBook_t *ptr = partition(head, tail, compare);
		QuickSort(head, ptr->prev, compare);
		QuickSort(ptr->next, tail, compare);
	}

    return;
}

_Bool FirstNameAZ(PhoneBook_t *first, PhoneBook_t *second) {
    return (strcasecmp(first->db.fname, second->db.fname) < 0) ? true : false;
}

_Bool FirstNameZA(PhoneBook_t *first, PhoneBook_t *second) {
    return (strcasecmp(first->db.fname, second->db.fname) > 0) ? true : false;
}

_Bool LastNameAZ(PhoneBook_t *first, PhoneBook_t *second) {
    return (strcasecmp(first->db.lname, second->db.lname) < 0) ? true : false;
}

_Bool LastNameZA(PhoneBook_t *first, PhoneBook_t *second) {
    return (strcasecmp(first->db.lname, second->db.lname) > 0) ? true : false;
}

void RenumberListID(PhoneBook_t *list) {
    PhoneBook_t *ptr = list;

    REWIND(ptr);
    for(int index = 1; ptr; index++, NEXT(ptr)) {
        ptr->db.id = index;
    }

    return;
}

void SortList(WINDOW *win, PhoneBook_t *list, _Bool compare(PhoneBook_t *first, PhoneBook_t *second)) {
    PhoneBook_t *head, *tail;
    int nb_records;

    head = tail = list; //point head and tail to list
    REWIND(head); //move head to the begin of the list
    FORWARD(tail); //move tail to the end of the list
    wattron(win, A_BLINK);
    print_in_middle(win, 11, " Sorting contacts list, please waiting... ", COLOR_PAIR(PAIR_EDIT));
    wattroff(win, A_BLINK);
    wrectangle(win, 10, 16, 12, 63);
    QuickSort(head, tail, compare); //compute sorting with compare() criteria
    RenumberListID(list); //renumber the ID of the enteire list
    wclear(win);
    wrefresh(win);
    wattron(win, A_BLINK);
    print_in_middle(win, 11, " Updating DataBase file, please waiting... ", COLOR_PAIR(PAIR_EDIT));
    wattroff(win, A_BLINK);
    wrectangle(win, 10, 15, 12, 63);
    nb_records = write_db(true); //Updating database
    logfile("%s: Updated %d records in %s DataBase after sorting\n", __func__, nb_records, DB);
    wclear(win);
    wrefresh(win);

    return;
}