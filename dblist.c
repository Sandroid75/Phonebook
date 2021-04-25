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

int countList(PhoneBook_t *list) {
    PhoneBook_t *ptr = list;
    int count;

	REWIND(ptr); //rewind the list up to the first node
	for(count = 0; ptr; count++) { //walk thru the entire list to count the numbers of contacts
		NEXT(ptr);
	}

    return count; //return the numbers of contacts counted
}

void destroyNode(DBnode_t **node) {
    if((*node)) {
		//free all sds strings
		sdsfree((*node)->fname);
        sdsfree((*node)->lname);
        sdsfree((*node)->organization);
        sdsfree((*node)->job);
        sdsfree((*node)->hphone);
        sdsfree((*node)->wphone);
        sdsfree((*node)->pmobile);
        sdsfree((*node)->bmobile);
        sdsfree((*node)->pemail);
        sdsfree((*node)->bemail);
        sdsfree((*node)->address);
        sdsfree((*node)->zip);
        sdsfree((*node)->city);
        sdsfree((*node)->state);
        sdsfree((*node)->country);

        free((*node)); //destroy the node
        NULLSET((*node)); //set *node pointer to NULL
    }

	return;
}

void destroyList(PhoneBook_t **list) {
    PhoneBook_t *nextPtr;

    REWIND((*list)); //rewind the list up to the first node
    while((*list)) { //walk thru complete contacts list
        nextPtr = (*list)->next; //store point nextPtr to next node before destroy it

		//free all sds strings in each node
        sdsfree((*list)->db.fname);
        sdsfree((*list)->db.lname);
        sdsfree((*list)->db.organization);
        sdsfree((*list)->db.job);
        sdsfree((*list)->db.hphone);
        sdsfree((*list)->db.wphone);
        sdsfree((*list)->db.pmobile);
        sdsfree((*list)->db.bmobile);
        sdsfree((*list)->db.pemail);
        sdsfree((*list)->db.bemail);
        sdsfree((*list)->db.address);
        sdsfree((*list)->db.zip);
        sdsfree((*list)->db.city);
        sdsfree((*list)->db.state);
        sdsfree((*list)->db.country);

        free((*list)); //now to (*list) elment can be free
        (*list) = nextPtr; //step to next node
    }

	return;
}