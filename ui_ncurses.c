#include "phonebook.h"

int flexMenu(WINDOW *win, sds *choices, int n_choices, char *menuName) {
	ITEM **my_items;
	MENU *my_menu;
    WINDOW *my_menu_win, *derWindow;
    int mrows, mcols, i, ch, index;
    _Bool quit = false;

	/* Create items */
    my_items = (ITEM **) calloc(n_choices, sizeof(ITEM *)); //calculate the size of memory to allocate for menu items
	if(my_items == NULL) {
		logfile("%s: error allocationg memory for items\n", __func__);
		return -1;
	}

    for(i = 0; i < n_choices; ++i) {
            my_items[i] = new_item(choices[i], (char *) NULL); //insert each choice in each menu item
    }

	/* Crate menu */
	my_menu = new_menu((ITEM **)my_items);
	if(my_menu == NULL) {
		logfile("%s: error menu initialization\n", __func__);
		return -1;
	}
    
    scale_menu(my_menu, &mrows, &mcols); //calculates the right dimension of window menu to fit all elements
    i = sdslen(menuName); //calclate the lenght of menu name
    mcols = mcols < i ? i : mcols; //check if the calculated scale of cols is less than menu name
    mcols = mcols %2 ? mcols +1 : mcols; //check the numbers of cols if is even otherwise add 1
    mrows += 4; //to fit the title and draw horizontal lines
    mcols += 3; //to draw the vertical lines

    my_menu_win = newwin(mrows, mcols, 1, 2); // Create the window to be associated with the menu
    keypad(my_menu_win, true); //initialize the keypad for menu window
	
    set_menu_win(my_menu, my_menu_win); // Set main window and sub window
	mrows = mrows -3; //set the numbers of items (or rows) per page
    derWindow = derwin(my_menu_win, mrows, mcols -2, 3, 1); //create a sub window from menu window
    set_menu_sub(my_menu, derWindow); //set menu sub window by derivated window
	
    set_menu_mark(my_menu, " "); //Set menu mark to the string can be "*"

	/* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0); //print the frame of menu window
	print_in_middle(my_menu_win, 1, menuName, COLOR_PAIR(PAIR_TITLE)); //print the title of the menu in the middle using color pair 1
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE); //print the intersection at left
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, mcols -2); //print the division line between the title and menu
	mvwaddch(my_menu_win, 2, mcols -1, ACS_RTEE); //print the intersection at right
    mvwprintw(win, LINES -2, 0, "Press ENTER or SPACE to Select, ESC to Exit");
	wrefresh(win);
    refresh();
        
	post_menu(my_menu); // Post the menu
	wrefresh(my_menu_win);

    while(quit != true) {
		index = item_index(current_item(my_menu)) +1; //set the current item index
        ch = wgetch(my_menu_win); //wait for user input
        switch(ch) {
            case KEY_BTAB: //BACK TAB
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM); //go to prev menu item
				break;
            case KEY_TAB: //TAB KEY
            case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM); //go to next menu item
				break;
			case KEY_HOME: //HOME KEY
				menu_driver(my_menu, REQ_FIRST_ITEM); //go up to the first item
				break;
			case KEY_END: //END KEY
				menu_driver(my_menu, REQ_LAST_ITEM); //go up to the last item
				break;
			case KEY_PPAGE: //PREVIOUS PAGE
				menu_driver(my_menu, REQ_SCR_UPAGE); //go to prev menu page
				break;
			case KEY_NPAGE: //NEXT PAGE
				menu_driver(my_menu, REQ_SCR_DPAGE); //go to next menu page
				break;
            case KEY_SPACE: //SPACE
            case KEY_ENTER: //ENTER numeric pad
            case KEY_RETURN: //ENTER
                quit = true; //quitting
                break;
            case KEY_ESC: //ESC
                index = 0; //nothing selected
                quit = true; //quitting
                break;
			case KEY_DC: //CANC
				index *= -1; //invert the sign
				quit = true; //quitting
				break;
		}
        if(is_wintouched(my_menu_win)) {
            touchwin(my_menu_win);
        }
        wrefresh(my_menu_win);
	}	

    unpost_menu(my_menu); // Unpost and free all the memory taken up
    free_menu(my_menu);
    for(i = 0; i < n_choices; ++i) {
            free_item(my_items[i]);
	}
	if(my_items) {
		free(my_items);
	}

    wclear(my_menu_win);
    wclear(win);
    wrefresh(my_menu_win);
    wrefresh(win);
    delwin(derWindow);
    delwin(my_menu_win);

	return index;
}

int flexForm(WINDOW *win, DBnode_t *db, const char *formName) {
	FIELD **field;
	FORM  *my_form;
	WINDOW *my_form_win, *derWindow;
	DBnode_t *ptrDB = db;
	int i, ch, field_validation, frows, fcols, insertMode = REQ_INS_MODE;
    _Bool quit = false, store = false;
	
    curs_set(true); //show cursor

	field = (FIELD **) calloc(20, sizeof(FIELD *)); //allocate memory for fields there are 19 fields+1 NULL
	if(field == NULL) {
		logfile("%s: error allocationg memory for fields\n", __func__);

		return -1;
	}

	initField(field, ptrDB); // Initialize the fields and respective labels

	if(field == NULL) { //check if field are well initialized
		logfile("%s: error initializing fields\n", __func__);
		return -1;
	}
		
	my_form = new_form(field); // Create the form and post it
	if(my_form == NULL) {
		logfile("%s: error form initialization\n", __func__);
		
		return -1;
	}
	
	scale_form(my_form, &frows, &fcols); // Calculate the area required for the form
	frows += 4; //to fit the title and draw horizontal lines
    fcols += 3; //to draw the vertical lines

	my_form_win = newwin(frows, fcols, 1, 2); // Create the window to be associated with the form
    keypad(my_form_win, true);

	set_form_win(my_form, my_form_win); // Set main window and sub window
	derWindow = derwin(my_form_win, frows -4, fcols -2, 3, 1); //create a sub window from form window
	set_form_sub(my_form, derWindow); //set menu sub window by derivated window

	/* Print a border around the main window and print a title */
    box(my_form_win, 0, 0); //print the frame of menu window
	print_in_middle(my_form_win, 1, formName, COLOR_PAIR(PAIR_TITLE)); //print the title of the menu in the middle using color pair 1
	mvwaddch(my_form_win, 2, 0, ACS_LTEE); //print the intersection at left
	mvwhline(my_form_win, 2, 1, ACS_HLINE, fcols -2); //print the division line between the title and menu
	mvwaddch(my_form_win, 2, fcols -1, ACS_RTEE); //print the intersection at right
	mvwprintw(my_form_win, 1, getmaxx(my_form_win) -6, "INS"); //print the insert mode in win_body
    mvwprintw(win, LINES -2, 0, "Use arrow keys or TAB to move between fields");
    mvwprintw(win, LINES -1, 0, "Press F1 to confirm and Exit or press ESC to cancel and Exit");
    wrefresh(win);
    refresh();

	post_form(my_form); // Post the form with all fields
	printLabels(my_form_win, COLOR_PAIR(PAIR_STD)); //print the field's label
	wrefresh(my_form_win);
	
	do { // Loop through to get user requests to pass at form_driver function
		field_validation = form_driver(my_form, REQ_VALIDATION);
		ch = wgetch(my_form_win);
		form_driver(my_form, insertMode); //set current insert mode status
		switch (ch) {
            case KEY_TAB: //TAB
            case KEY_ENTER: //ENTER numeric pad
            case KEY_RETURN: //ENTER numeric pad
            case KEY_DOWN:
				if(field_validation == E_OK) {
					set_field_back(current_field(my_form), A_UNDERLINE | A_NORMAL);
					form_driver(my_form, REQ_NEXT_FIELD);
                    if(field_status(current_field(my_form)) == true) //is the field modified?
                        form_driver(my_form, REQ_END_LINE); //if true go to end of field line
					set_field_back(current_field(my_form), A_UNDERLINE | A_REVERSE);
				}
				break;

			case KEY_BTAB: //back tab
			case KEY_UP:
				if(field_validation == E_OK) {
					set_field_back(current_field(my_form), A_UNDERLINE | A_NORMAL);
					form_driver(my_form, REQ_PREV_FIELD);
                    if(field_status(current_field(my_form)) == true) //is the field modified?
                        form_driver(my_form, REQ_END_LINE); //if true go to end of field line
					set_field_back(current_field(my_form), A_UNDERLINE | A_REVERSE);
				}
				break;

			case KEY_LEFT: //move cursor to left
				form_driver(my_form, REQ_PREV_CHAR);
				break;

			case KEY_RIGHT: //move cursor to right
				form_driver(my_form, REQ_NEXT_CHAR);
				break;

			case KEY_HOME: //move cursor to begin of field
                form_driver(my_form, REQ_BEG_FIELD);
				break;

			case KEY_END: //move cursor to end of field
                form_driver(my_form, REQ_END_FIELD);
				break;

            case KEY_CTRL_LEFT: //move cursot to prev word
                form_driver(my_form, REQ_PREV_WORD);
                break;

            case KEY_CTRL_RIGHT: //move cursot to next word
                form_driver(my_form, REQ_NEXT_WORD);
                break;

            case KEY_CTRL_BS: //delete prev word
                form_driver(my_form, REQ_PREV_WORD);
                form_driver(my_form, REQ_NEXT_WORD);
                form_driver(my_form, REQ_DEL_WORD);
                break;

            case KEY_CTRL_DEL: //delete next word
                form_driver(my_form, REQ_DEL_WORD);
                break;

            case KEY_BACKSPACE: // Delete the char before cursor
				form_driver(my_form, REQ_DEL_PREV);
				break;

			case KEY_DC: // Delete the char under the cursor
				form_driver(my_form, REQ_DEL_CHAR);
				break;

			case KEY_IC: //toggle insert char mode
				if(insertMode == REQ_INS_MODE) {
					insertMode = REQ_OVL_MODE;
					mvwprintw(my_form_win, 1, getmaxx(my_form_win) -6, "OVL"); //print the insert mode in win_body
				} else {
					insertMode = REQ_INS_MODE;
					mvwprintw(my_form_win, 1, getmaxx(my_form_win) -6, "INS"); //print the insert mode in win_body
				}
				break;

            case KEY_F(1): //F1 key was pressed
                for(i = 0, ch = 0; field[i]; i++) { //exlude the last field as NULL
                    ch += field_status(field[i]); //check if field as been modified
                }
                if(ch) { //one or more fields it been modified
                    store = true;
                }
                quit = true;
                break;

            case KEY_ESC: //ESCape
                for(i = 0, ch = 0; field[i]; i++) { //exlude the last field as NULL
                    ch += field_status(field[i]); //check if field as been modified
                }
                if(ch) { //one or more fields it been modified
                    ch = messageBox(win, 18, "press any key to save, N to discard changes or ESC to continuing editing...", COLOR_PAIR(PAIR_EDIT));
                    ch = toupper(ch);
                    if(ch == 'N') { //discard changes
                        quit = true;
                    } else if(ch != KEY_ESC) { //confirm changes end exit
                        store = true;
                        quit = true;
                    } else { //ESCape was pressed back to editing form
                        post_form(my_form);
                        wrefresh(my_form_win);
                    }
                } else { //no fields was modified than exit without ask
                    quit = true;
                }
                break;

			default:
                    //logfile("%s: keypressed: %d\n", __func__, ch);
                    form_driver(my_form, ch);
				break;
		}
    } while(quit != true);

	if(store) { //if input data was confirmed by user
		i = 0;
        ptrDB->modified = true; //set the field as modified in order to update db
		ptrDB->id = field_digit(field[i++], 0);
		
		//in next lines assign the value returned by form fields than trim removing white space from the tail of th string
		ptrDB->fname 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->fname, " "); */
		ptrDB->lname 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->lname, " "); */
		ptrDB->organization	= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->organization, " "); */
		ptrDB->job 			= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->job, " "); */
		ptrDB->hphone 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->hphone, " "); */
		ptrDB->wphone 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->wphone, " "); */
		ptrDB->pmobile 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->pmobile, " "); */
		ptrDB->bmobile 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->bmobile, " "); */
		ptrDB->pemail 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->pemail, " "); */
		ptrDB->bemail 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->bemail, " "); */
		ptrDB->address 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->address, " "); */
		ptrDB->zip 			= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->zip, " "); */
		ptrDB->city 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->city, " "); */
		ptrDB->state 		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->state, " "); */
		ptrDB->country		= sdsnew(field_buffer(field[i++], 0));	/* sdstrim(ptrDB->country, " "); */

		ptrDB->birthday.tm_mday = field_digit(field[i++], 0);
		ptrDB->birthday.tm_mon 	= field_digit(field[i++], 0);
		ptrDB->birthday.tm_year = field_digit(field[i++], 0);
	}
	/* Un post form and free the memory */
	unpost_form(my_form);
	for(i = 0; field[i]; i++) {
        free(field_buffer(field[i], 0));
        free_field(field[i]);
	}
    if(field) {
        free(field);
		NULLSET(field);
    }

    free_form(my_form);
    wclear(my_form_win);
    wclear(win);
    wrefresh(my_form_win);
    wrefresh(win);
    delwin(derWindow);
    delwin(my_form_win);

    curs_set(false); //hide cursor

	return (store ? true : false); //return true if there is something to store otherwise false
}

int initField(FIELD **field, DBnode_t *db) {
	DBnode_t *ptrDB = db;
	int i, rows, start_phone, start_email, i_zip, start_bday;	

	/* Initialize the fields */
	i = 0; //for array index
	rows = 0; //for row counting where the field are on same row there is 3 more cols as spaced
	field[i++] = new_field(1, 3, 		rows,	1, 					0, 0); //id
	rows += 2;
	field[i++] = new_field(1, STEXT, 	rows, 	1,					0, 0); //fname
	field[i++] = new_field(1, STEXT, 	rows,	STEXT +3, 			0, 0); //lname
	rows += 2;
	field[i++] = new_field(1, MTEXT, 	rows,	1, 					0, 0); //organization
	field[i++] = new_field(1, STEXT, 	rows,	MTEXT +3,			0, 0); //job
	rows += 2;
	start_phone = i; //store the first phone field
	field[i++] = new_field(1, PHONE, 	rows,	1,					0, 0); //hphone
	field[i++] = new_field(1, PHONE, 	rows,	PHONE +3,			0, 0); //wphone
	field[i++] = new_field(1, PHONE, 	rows,	PHONE*2 +7,			0, 0); //pmobile
	field[i++] = new_field(1, PHONE, 	rows,	PHONE*3 +9,			0, 0); //bmobile
	rows += 2;
	start_email = i; //store the last phone field
	field[i++] = new_field(1, MTEXT, 	rows,	1,					0, 0); //pemail
	field[i++] = new_field(1, MTEXT, 	rows,	MTEXT +3,			0, 0); //bemail
	rows += 2;
	field[i++] = new_field(1, LTEXT, 	rows,	1,					0, 0); //address
	rows += 2;
	i_zip = i; //store the zip field
	field[i++] = new_field(1, ZIP,	 	rows,	1,					0, 0); //zip
	field[i++] = new_field(1, MTEXT, 	rows,	ZIP +3,				0, 0); //city
	field[i++] = new_field(1, STATE, 	rows,	ZIP+MTEXT +5,		0, 0); //state/province
	field[i++] = new_field(1, STEXT, 	rows,	ZIP+MTEXT+STATE +7,	0, 0); //country
	rows = 2; //birthday is on the same line of name
	start_bday = i; //store the first birthday field array index ***NB: the birthday is on the same line of name***
    field[i++] = new_field(1, 2,	 	rows,	STEXT*2 +13,			0, 0); //day
    field[i++] = new_field(1, 2,	 	rows,	STEXT*2 +17,			0, 0); //mon
	field[i++] = new_field(1, 4,	 	rows,	STEXT*2 +21,			0, 0); //year
	NULLSET(field[i]);
	rows = i; //store the numbers of counted fields

	//now for not alphanumeric field a new type will assigned
	for(i = start_phone; i < start_email; i++) { //all phone fields are set with own regular expression
		set_field_type(field[i], TYPE_REGEXP, "^\\+?[0-9]+ "); //accept only valid phone numbers with or without the international code prefix +
	}

	i = start_email;
    set_field_type(field[i++], TYPE_REGEXP, "^([a-zA-Z0-9_\\-\\.]+)@([a-zA-Z0-9_\\-\\.]+)\\.([a-zA-Z]{2,5})");
    set_field_type(field[i++], TYPE_REGEXP, "^([a-zA-Z0-9_\\-\\.]+)@([a-zA-Z0-9_\\-\\.]+)\\.([a-zA-Z]{2,5})");

	set_field_type(field[i_zip], TYPE_REGEXP, "^[0-9]*$"); //set valid numbers and precision of zip codes accept only field with 5 digit
	i = start_bday; //set i as the first bithday field array index
	set_field_type(field[i++], TYPE_INTEGER, 0, 1, 31); //set valid numbers and precision of day from 1 to 31
	set_field_type(field[i++], TYPE_INTEGER, 0, 1, 12); //set valid numbers and precision of month from 1 to 12
	set_field_type(field[i++], TYPE_INTEGER, 0, 1900, 2100); //set valid numbers and precision of year from 1900 to 2100

	for(i = 0; i < rows; i++) { //exlude the last field as NULL
		if(i) { //all fields except id field [0]
            field_opts_on(field[i], O_VISIBLE);
			field_opts_on(field[i], O_PUBLIC);
			field_opts_on(field[i], O_EDIT);
			field_opts_on(field[i], O_ACTIVE);
			set_field_fore(field[i], A_BOLD);
			set_field_back(field[i], A_UNDERLINE);
		} else { //only id field [0]
			set_field_type(field[i], TYPE_INTEGER); //as the filed[0] is id index
			set_field_opts(field[i], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
			set_field_back(field[i], A_REVERSE); //set reverse id field
			set_field_just(field[i], JUSTIFY_RIGHT);
		}
	}
    i = start_bday; //set i as the first bithday field array index
    field_opts_on(field[i++], O_AUTOSKIP); //set day AUTOSKIP when the field is filled 2 chars
    field_opts_on(field[i++], O_AUTOSKIP); //set month AUTOSKIP when the field is filled 2 chars
    field_opts_on(field[i++], O_AUTOSKIP); //set year AUTOSKIP when the field is filled 4 chars

	set_field_back(field[1], A_UNDERLINE | A_REVERSE); //the first filde (name) in order to will be reversed at first call of form

	i = 0;
	set_field_digit(field[i++], 0, ptrDB->id); 
	
	set_field_buffer(field[i++], 0, ptrDB->fname);
	set_field_buffer(field[i++], 0, ptrDB->lname);
	set_field_buffer(field[i++], 0, ptrDB->organization);
	set_field_buffer(field[i++], 0, ptrDB->job);
	set_field_buffer(field[i++], 0, ptrDB->hphone);
	set_field_buffer(field[i++], 0, ptrDB->wphone);
	set_field_buffer(field[i++], 0, ptrDB->pmobile);
	set_field_buffer(field[i++], 0, ptrDB->bmobile);
	set_field_buffer(field[i++], 0, ptrDB->pemail);
	set_field_buffer(field[i++], 0, ptrDB->bemail);
	set_field_buffer(field[i++], 0, ptrDB->address);
	set_field_buffer(field[i++], 0, ptrDB->zip);
	set_field_buffer(field[i++], 0, ptrDB->city);
	set_field_buffer(field[i++], 0, ptrDB->state);
	set_field_buffer(field[i++], 0, ptrDB->country);

	set_field_digit(field[i++], 0, ptrDB->birthday.tm_mday);
	set_field_digit(field[i++], 0, ptrDB->birthday.tm_mon);
	set_field_digit(field[i++], 0, ptrDB->birthday.tm_year);
	
	for(i = 0; i < rows; i++) { //exlude the last field as NULL
        set_field_status(field[i], false); //set the field as no modified
	}

	return rows; //return the numbers of fields
}

int set_field_digit(FIELD *field, int buf, int digit) { //the equivalent of set_field_buffer for INTEGER field
	sds string;
    int retVal;

	if(field_type(field) != TYPE_INTEGER) { //check if field is a integer type
		return E_BAD_ARGUMENT;
	}

	string = sdsfromlonglong((long long) digit); //convert integer to sds string
	retVal = set_field_buffer(field, buf, string); //set the buffer with the string value
	sdsfree(string); //free memory

	return retVal; //return the value of set_field_buffer
}

int field_digit(FIELD *field, int buf) { //the equivalent of field_buffer for INTEGER field
	return atoi(field_buffer(field, buf)); //return the integer value converting filed_buffer string
}

void print_in_middle(WINDOW *win, int y, const char *string, chtype color) {
    int x, length;

	if(win == NULL)
		win = stdscr;

	length = (int) strlen(string);
	x = (int) ((getmaxx(win) - length) /2);

	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();

	return;
}

int messageBox(WINDOW *win, int y, const char *string, chtype color) {
	WINDOW *msgWin;
	int prevCurs, x, length, key;

    prevCurs = curs_set(false);
	
	if(win == NULL)
		win = stdscr;		
	
	length = (int) strlen(string) +6;
	x = (int) ((getmaxx(win) - length) /2);

	msgWin = newwin(3, length, y, x); //create a message window
	box(msgWin, 0, 0); //create a box around the message window
	wattron(msgWin, color);
	mvwprintw(msgWin, 1, 2, " %s ", string);
	wattroff(msgWin, color);
	key = wgetch(msgWin);
	wclear(msgWin); //clear message window contents
	wrefresh(msgWin); //refresh message window contents
	delwin(msgWin); //destroyu message window pointer
	
	curs_set(prevCurs);

	return key;
}

void printLabels(WINDOW *win, chtype color) {
	int row;

	wattron(win, color);
	row = 4;
	mvwprintw(win, row, 2, "name"); mvwprintw(win, row, STEXT +4, "last name");
	row += 2;
	mvwprintw(win, row, 2, "organizzation"); mvwprintw(win, row, MTEXT +4, "job");
	row += 2;
	mvwprintw(win, row, 2, "home phone"); mvwprintw(win, row, PHONE +4, "work phone");
	mvwprintw(win, row, PHONE*2 +8, "personal mobile"); mvwprintw(win, row, PHONE*3 +10, "business mobile");
	row += 2;
	mvwprintw(win, row, 2, "personal email"); mvwprintw(win, row, MTEXT +4, "business email");
	row += 2;
	mvwprintw(win, row, 2, "address");
	row += 2;
	mvwprintw(win, row, 2, "zip"); mvwprintw(win, row, ZIP +4, "city");
	mvwprintw(win, row +1, ZIP+MTEXT +5, "("); mvwprintw(win, row +1, ZIP+MTEXT +8, ")");
	mvwprintw(win, row, ZIP+MTEXT+STATE +8, "country");
	row = 4; //birthday is on same line of name
	mvwprintw(win, row, STEXT*2 +16, "birthday");
	row = 6; //format under the date field
	mvwprintw(win, row, STEXT*2 +14, "dd"); mvwprintw(win, row, STEXT*2 +18, "mm"); mvwprintw(win, row, STEXT*2 +22, "yyyy");
	wattroff(win, color);

	return;
}