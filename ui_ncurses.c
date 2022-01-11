/*********************************************************************************
**                                                                              **
**      Copyleft 1991-2021 by Sandroid75 with GNU General Public License.       **
**                                                                              **
**                                                                              **
**  Redistribution and use in source and binary forms, with or without          **
**  modification, are permitted provided that the following conditions are met: **
**                                                                              **
**   * Redistributions of source code must retain the above copyleft notice,    **
**     this list of conditions and the following disclaimer.                    **
**   * Redistributions in binary form must reproduce the above copyleft         **
**     notice, this list of conditions and the following disclaimer in the      **
**     documentation and/or other materials provided with the distribution.     **
**   * Neither the name of Sandroid75 nor the names of its contributors may     **
**     be used to endorse or promote products derived from this software        **
**     without specific prior written permission.                               **
**                                                                              **
**  THIS SOFTWARE IS PROVIDED BY THE COPYLEFT HOLDERS AND CONTRIBUTORS "AS IS"  **
**  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   **
**  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  **
**  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYLEFT OWNER OR CONTRIBUTORS BE     **
**  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         **
**  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        **
**  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    **
**  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     **
**  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     **
**  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  **
**  POSSIBILITY OF SUCH DAMAGE.                                                 **
**                                                                              **
**  This code was made by Sandroid75 to illustrate various C language powers.   **
**                                                                              **
**  My recommendations for developing robust C code are:                        **
**  - first of all read The C Programming Language: ANSI C Version by K&R (C)   **
**  - after read K&R keep always in mind rules and methods                      **
**  - one of the most characteristics and powers of the C is the pointers       **
**  - pointers are very helpfull and flexibile to write efficient code          **
**  - pointers can be dangerous for stable code if you forget the rules         **
**  - if you use pointers for list of datas don't forget to reserve memory      **
**  - if you use pointers for list of datas don't forget to release memory      **
**  - write well-formatted code                                                 **
**  - only good formatted code makes the code readable                          **
**  - good formatting code reduces the risk of errors                           **
**  - good formatting code facilitates the debugging                            **
**  - good formatting code facilitates the maintenences                         **
**  - good formatting code facilitates future implementantions                  **
**  - commenting the code is another good and necessary practice                **
**  - commenting the code means understanding what you are doing                **
**  - commenting the code means understanding what you have done                **
**  - commenting the code is not a waste of time                                **
**  - at the last but not least, remember rules and methods                     **
**                                                                              **
**                  Have fun with C programming by Sandroid75                   **
**                                                                              **
**********************************************************************************/

#include "phonebook.h"

int flexMenu(WINDOW *win, sds *choices, char *menuName) {
    ITEM **my_items;
    MENU *my_menu;
    WINDOW *my_menu_win, *derWindow;
    int n_choices, mrows, mcols, i, ch, index, xpos;
    _Bool quit = false;

    for(i = 0; sdslen(choices[i]); i++); //count the numbers of choices
    n_choices = i +1; //set the numbres of choics array

    /* Create items */
    my_items = (ITEM **) calloc(n_choices, sizeof(ITEM *)); //calculate the size of memory to allocate for menu items
    if(!my_items) {
        logfile("%s: error allocationg memory for items\n", __func__);
        return -1;
    }

    for(i = 0; i < n_choices; ++i) {
        my_items[i] = new_item(choices[i], (char *) NULL); //insert each choice in each menu item
    }

    /* Crate menu */
    my_menu = new_menu((ITEM **) my_items);
    if(!my_menu) {
        logfile("%s: error menu initialization\n", __func__);
        return -1;
    }

    scale_menu(my_menu, &mrows, &mcols); //calculates the right dimension of window menu to fit all elements
    i = sdslen(menuName); //calclate the lenght of menu name
    mcols = mcols < i ? i : mcols; //check if the calculated scale of cols is less than menu name
    mcols = mcols % 2 ? mcols + 1 : mcols; //check the numbers of cols if is even otherwise add 1
    mrows += 4; //to fit the title and draw horizontal lines
    mcols += 3; //to draw the vertical lines

    xpos = getmaxx(win); //get current window max x
    xpos =  (xpos / 2) - (mcols / 2); //determinate the start x position in order to center the menu
    my_menu_win = newwin(mrows, mcols, 1, xpos); // Create the window to be associated with the menu
    keypad(my_menu_win, true); //initialize the keypad for menu window

    set_menu_win(my_menu, my_menu_win); // Set main window and sub window
    mrows = mrows - 3; //set the numbers of items (or rows) per page
    derWindow = derwin(my_menu_win, mrows, mcols - 2, 3, 1); //create a sub window from menu window
    set_menu_sub(my_menu, derWindow); //set menu sub window by derivated window

    set_menu_mark(my_menu, " "); //Set menu mark to the string can be "*"

    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0); //print the frame of menu window
    print_in_middle(my_menu_win, 1, menuName, COLOR_PAIR(PAIR_TITLE)); //print the title of the menu in the middle using color pair 1
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE); //print the intersection at left
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, mcols - 2); //print the division line between the title and menu
    mvwaddch(my_menu_win, 2, mcols - 1, ACS_RTEE); //print the intersection at right
    mvwprintw(win, LINES - 2, 0, "Press ENTER or SPACE to Select, ESC to Exit");
    wrefresh(win);
    refresh();

    logo(win, 12);
    post_menu(my_menu); // Post the menu
    wrefresh(my_menu_win);

    while(quit != true) {
        index = item_index(current_item(my_menu)) + 1; //set the current item index
        ch = wgetch(my_menu_win); //wait for user input
        switch (ch) {
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
    FORM *my_form;
    WINDOW *my_form_win, *derWindow;
    DBnode_t *ptrDB = db;
    int i, ch, field_validation, frows, fcols, insertMode = REQ_INS_MODE;
    _Bool quit = false, store = false, status;

    curs_set(true); //show cursor

    field = initField(ptrDB); // Initialize the fields and respective labels
    if(!field) { //check if field are well initialized
        logfile("%s: error initializing fields\n", __func__);
        return -1;
    }

    my_form = new_form(field); // Create the form and post it
    if(!my_form) {
        logfile("%s: error form initialization\n", __func__);

        return -1;
    }

    scale_form(my_form, &frows, &fcols); // Calculate the area required for the form
    frows += 4; //to fit the title and draw horizontal lines
    fcols += 3; //to draw the vertical lines

    my_form_win = newwin(frows, fcols, 1, 2); // Create the window to be associated with the form
    keypad(my_form_win, true);

    set_form_win(my_form, my_form_win); // Set main window and sub window
    derWindow = derwin(my_form_win, frows - 4, fcols - 2, 3, 1); //create a sub window from form window
    set_form_sub(my_form, derWindow); //set menu sub window by derivated window

    /* Print a border around the main window and print a title */
    box(my_form_win, 0, 0); //print the frame of menu window
    print_in_middle(my_form_win, 1, formName, COLOR_PAIR(PAIR_TITLE)); //print the title of the menu in the middle using color pair 1
    mvwaddch(my_form_win, 2, 0, ACS_LTEE); //print the intersection at left
    mvwhline(my_form_win, 2, 1, ACS_HLINE, fcols - 2); //print the division line between the title and menu
    mvwaddch(my_form_win, 2, fcols - 1, ACS_RTEE); //print the intersection at right
    mvwprintw(my_form_win, 1, getmaxx(my_form_win) - 6, "INS"); //print the insert mode in win_body
    mvwprintw(win, LINES - 2, 0, "Use arrow keys or TAB to move between fields");
    mvwprintw(win, LINES - 1, 0, "Press F1 to confirm and Exit or press ESC to cancel and Exit");
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
                insertMode = insertMode == REQ_INS_MODE ? REQ_OVL_MODE : REQ_INS_MODE;
                char *modemsg = insertMode == REQ_INS_MODE ? "OVL" : "INS";
                mvwprintw(my_form_win, 1, getmaxx(my_form_win) - 6, modemsg); //print the insert mode in win_body
                break;

            case KEY_F(1): //F1 key was pressed
                for(i = 0, status  = false; field[i]; i++) { //exlude the last field as NULL
                    status |= field_status(field[i]); //check if field as been modified
                }
                if(status) { //one or more fields it been modified
                    store = true;
                }
                quit = true;
                break;

            case KEY_ESC: //ESCape
                for(i = 0, status = false; field[i]; i++) { //exlude the last field as NULL
                    status |= field_status(field[i]); //check if field as been modified
                }
                if(status) { //one or more fields it been modified
                    ch = messageBox(win, 18, "any key to save, 'N' to discard changes or ESC to continuing editing...", COLOR_PAIR(PAIR_EDIT));
                    if(toupper(ch) == 'N') { //discard changes
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
                form_driver(my_form, ch);
                break;
        }
    } while(!quit);

    if(store) { //if input data was confirmed by user
        i = 0;
        ptrDB->modified = true; //set the field as modified in order to update db
        ptrDB->id = field_digit(field[i++], 0);

        //in next lines assign the value returned by form fields than trim removing white space from the tail of th string
        ptrDB->fname = sdsnew(field_buffer(field[i++], 0));         sdstrim(ptrDB->fname, " ");
        ptrDB->lname = sdsnew(field_buffer(field[i++], 0));         sdstrim(ptrDB->lname, " ");
        ptrDB->organization = sdsnew(field_buffer(field[i++], 0));  sdstrim(ptrDB->organization, " ");
        ptrDB->job = sdsnew(field_buffer(field[i++], 0));           sdstrim(ptrDB->job, " ");
        ptrDB->hphone = sdsnew(field_buffer(field[i++], 0));        sdstrim(ptrDB->hphone, " ");
        ptrDB->wphone = sdsnew(field_buffer(field[i++], 0));        sdstrim(ptrDB->wphone, " ");
        ptrDB->pmobile = sdsnew(field_buffer(field[i++], 0));       sdstrim(ptrDB->pmobile, " ");
        ptrDB->bmobile = sdsnew(field_buffer(field[i++], 0));       sdstrim(ptrDB->bmobile, " ");
        ptrDB->pemail = sdsnew(field_buffer(field[i++], 0));        sdstrim(ptrDB->pemail, " ");
        ptrDB->bemail = sdsnew(field_buffer(field[i++], 0));        sdstrim(ptrDB->bemail, " ");
        ptrDB->address = sdsnew(field_buffer(field[i++], 0));       sdstrim(ptrDB->address, " ");
        ptrDB->zip = sdsnew(field_buffer(field[i++], 0));           sdstrim(ptrDB->zip, " ");
        ptrDB->city = sdsnew(field_buffer(field[i++], 0));          sdstrim(ptrDB->city, " ");
        ptrDB->state = sdsnew(field_buffer(field[i++], 0));         sdstrim(ptrDB->state, " ");
        ptrDB->country = sdsnew(field_buffer(field[i++], 0));       sdstrim(ptrDB->country, " ");

        ptrDB->birthday.tm_mday = field_digit(field[i++], 0);
        ptrDB->birthday.tm_mon = field_digit(field[i++], 0);
        ptrDB->birthday.tm_year = field_digit(field[i++], 0);
    }
    /* Unpost form and free the memory */
    unpost_form(my_form);
    free_form(my_form);

    for(i = 0; i <= PHONE_FIELDS; i++) {
        set_field_fore(field[i], A_NORMAL);
        set_field_back(field[i], A_NORMAL);
        free_field(field[i]);
        NULLSET(field[i]);
    }
    if(field) {
        free(field);
        NULLSET(field);
    }

    wclear(my_form_win);
    wclear(win);
    wrefresh(my_form_win);
    wrefresh(win);
    delwin(derWindow);
    delwin(my_form_win);

    curs_set(false); //hide cursor

    return (int) store; //return true if there is something to store otherwise false
}

FIELD **initField(DBnode_t *db) {
    FIELD **field;
    DBnode_t *ptrDB = db;
    int i, rows, start_bday;

    field = (FIELD **) calloc(PHONE_FIELDS, sizeof(FIELD *)); //allocate memory for fields there are 19 fields+1 NULL
    if(!field) {
        logfile("%s: error allocationg memory for fields\n", __func__);

        return NULL;
    }

    /* Initialize the fields */
    i = 0; //for array index
    rows = 0; //for row counting where the field are on same row there is 3 more cols as spaced
    field[i++] = new_field(1, 4, rows, 1, 0, 0); //id
    rows += 2;
    field[i++] = new_field(1, STEXT, rows, 1, 0, 0); //fname
    field[i++] = new_field(1, STEXT, rows, STEXT + 3, 0, 0); //lname
    rows += 2;
    field[i++] = new_field(1, MTEXT, rows, 1, 0, 0); //organization
    field[i++] = new_field(1, STEXT, rows, MTEXT + 3, 0, 0); //job
    rows += 2;
    field[i] = new_field(1, PHONE, rows, 1, 0, 0); //hphone
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_PHONE); //accept only valid phone numbers with or without the international code prefix +
    field[i] = new_field(1, PHONE, rows, PHONE + 3, 0, 0); //wphone
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_PHONE); //accept only valid phone numbers with or without the international code prefix +
    field[i] = new_field(1, PHONE, rows, PHONE * 2 + 7, 0, 0); //pmobile
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_PHONE); //accept only valid phone numbers with or without the international code prefix +
    field[i] = new_field(1, PHONE, rows, PHONE * 3 + 9, 0, 0); //bmobile
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_PHONE); //accept only valid phone numbers with or without the international code prefix +
    rows += 2;
    field[i] = new_field(1, MTEXT, rows, 1, 0, 0); //pemail
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_EMAIL);
    field[i] = new_field(1, MTEXT, rows, MTEXT + 3, 0, 0); //bemail
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_EMAIL);
    rows += 2;
    field[i++] = new_field(1, LTEXT, rows, 1, 0, 0); //address
    rows += 2;
    field[i] = new_field(1, ZIP, rows, 1, 0, 0); //zip
    set_field_type(field[i++], TYPE_REGEXP, REGEXP_ZIP); //set valid numbers and precision of zip codes accept only field with 5 digit
    field[i++] = new_field(1, MTEXT, rows, ZIP + 3, 0, 0); //city
    field[i++] = new_field(1, STATE, rows, ZIP + MTEXT + 5, 0, 0); //state/province
    field[i++] = new_field(1, STEXT, rows, ZIP + MTEXT + STATE + 7, 0, 0); //country
    rows = 2; //birthday is on the same line of name
    start_bday = i; //store the first birthday field array index ***NB: the birthday is on the same line of name***
    field[i] = new_field(1, 2, rows, STEXT * 2 + 13, 0, 0); //day
    set_field_type(field[i++], TYPE_INTEGER, 0, 1, 31); //set valid numbers and precision of day from 1 to 31
    field[i] = new_field(1, 2, rows, STEXT * 2 + 17, 0, 0); //mon
    set_field_type(field[i++], TYPE_INTEGER, 0, 1, 12); //set valid numbers and precision of month from 1 to 12
    field[i] = new_field(1, 4, rows, STEXT * 2 + 21, 0, 0); //year
    set_field_type(field[i++], TYPE_INTEGER, 0, 1900, 2100); //set valid numbers and precision of year from 1900 to 2100
    
    NULLSET(field[i]);

    for(i = 0; i < PHONE_FIELDS; i++) { //exlude the last field as NULL
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

    for(i = 0; i < PHONE_FIELDS; i++) { //exlude the last field as NULL
        set_field_status(field[i], false); //set the field as no modified
    }

    return field; //return pointer to field
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

int showMatch(WINDOW *win, DBnode_t first, DBnode_t second, unsigned int check) {
    FIELD **field;
    FORM *my_form;
    WINDOW *my_form_win, *derWindow;
    int i, ch, frows, fcols;
    _Bool quit = false;

    field = initMatchField(first, second, check);
    if(!field) { //check if field are well initialized
        logfile("%s: error initializing fields\n", __func__);
        return -1;
    }

    my_form = new_form(field); // Create the form and post it
    if(!my_form) {
        logfile("%s: error form initialization\n", __func__);

        return -1;
    }

    scale_form(my_form, &frows, &fcols); // Calculate the area required for the form
    frows += 4; //to fit the title and draw horizontal lines
    fcols += 3; //to draw the vertical lines

    my_form_win = newwin(frows, fcols, 1, 2); // Create the window to be associated with the form
    keypad(my_form_win, true);

    set_form_win(my_form, my_form_win); // Set main window and sub window
    derWindow = derwin(my_form_win, frows - 4, fcols - 2, 3, 1); //create a sub window from form window
    set_form_sub(my_form, derWindow); //set menu sub window by derivated window

    /* Print a border around the main window and print a title */
    box(my_form_win, 0, 0); //print the frame of menu window
    print_in_middle(my_form_win, 1, " Contacts with Matches ", COLOR_PAIR(PAIR_TITLE)); //print the title of the menu in the middle using color pair 1
    mvwaddch(my_form_win, 2, 0, ACS_LTEE); //print the intersection at left
    mvwhline(my_form_win, 2, 1, ACS_HLINE, fcols - 2); //print the division line between the title and menu
    mvwaddch(my_form_win, 2, fcols - 1, ACS_RTEE); //print the intersection at right
    mvwprintw(win, LINES - 2, 0, "Press F1 to edit matches, F4 to auto merge, ESC ignore duplicates");
    mvwprintw(win, LINES - 1, 0, "Note: the duplication match is highlighted");
    wrefresh(win);
    refresh();

    post_form(my_form); // Post the form with all fields
    mvwhline(my_form_win, 8, 1, ACS_HLINE, fcols - 2); //print the division line between the two matches
    wrefresh(my_form_win);

    do {
        form_driver(my_form, REQ_VALIDATION);
        ch = wgetch(my_form_win);
        switch (ch) {
            case KEY_F(1):
            case KEY_F(4):
            case KEY_ESC:
                quit = true;
                break;        
            default:
                quit = false;
                break;
        }
    } while(!quit);
    /* Unpost form and free the memory */
    unpost_form(my_form);
    free_form(my_form);

    for(i = 0; i < MATCH_FIELDS; i++) {
        set_field_fore(field[i], A_NORMAL);
        set_field_back(field[i], A_NORMAL);
        free_field(field[i]);
        NULLSET(field[i]);
    }
    if(field) {
        free(field);
        NULLSET(field);
    }

    wclear(my_form_win);
    wclear(win);
    wrefresh(my_form_win);
    wrefresh(win);
    delwin(derWindow);
    delwin(my_form_win);

    return ch; //return user choice
}

FIELD **initMatchField(DBnode_t first, DBnode_t second, unsigned int check) {
    FIELD **field;
    int index, rows;

    field = (FIELD **) calloc(MATCH_FIELDS, sizeof(FIELD *)); //allocate memory for fields there are 28 fields+1 NULL
    if(!field) {
        logfile("%s: error allocationg memory for fields\n", __func__);

        return NULL;
    }

    /* Initialize the fields */
    index = 0; //for array index
    rows = 0; //for row counting where the field are on same row there is 3 more cols as spaced
    
    field[index] = new_field(1, 3, rows, 1, 0, 0); //id label
    set_field_buffer(field[index++], 0, "id:");
    field[index] = new_field(1, 4, rows++, 5, 0, 0); //id
    set_field_type(field[index], TYPE_INTEGER); //id index
    set_field_digit(field[index++], 0, first.id);

    field[index] = new_field(1, STEXT, rows++, 1, 0, 0); //fname label
    set_field_buffer(field[index++], 0, "name");
    field[index] = new_field(1, STEXT, rows--, 1, 0, 0); //fname
    set_field_buffer(field[index++], 0, first.fname);

    field[index] = new_field(1, STEXT, rows++, STEXT + 3, 0, 0); //lname label
    set_field_buffer(field[index++], 0, "last name");
    field[index] = new_field(1, STEXT, rows++, STEXT + 3, 0, 0); //lname
    set_field_buffer(field[index++], 0, first.lname);

    field[index] = new_field(1, PHONE, rows++, 1, 0, 0); //hphone label
    set_field_buffer(field[index++], 0, "home phone");
    field[index] = new_field(1, PHONE, rows--, 1, 0, 0); //hphone
    set_field_buffer(field[index++], 0, first.hphone);
    if(check & MATCH_FIRST_HPHONE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE + 3, 0, 0); //wphone
    set_field_buffer(field[index++], 0, "work phone");
    field[index] = new_field(1, PHONE, rows--, PHONE + 3, 0, 0); //wphone
    set_field_buffer(field[index++], 0, first.wphone);
    if(check & MATCH_FIRST_WPHONE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE * 2 + 7, 0, 0); //pmobile label
    set_field_buffer(field[index++], 0, "personal mobile");
    field[index] = new_field(1, PHONE, rows--, PHONE * 2 + 7, 0, 0); //pmobile
    set_field_buffer(field[index++], 0, first.pmobile);
    if(check & MATCH_FIRST_PMOBILE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE * 3 + 9, 0, 0); //bmobile label
    set_field_buffer(field[index++], 0, "business mobile");
    field[index] = new_field(1, PHONE, rows++, PHONE * 3 + 9, 0, 0); //bmobile
    set_field_buffer(field[index++], 0, first.bmobile);
    if(check & MATCH_FIRST_BMOBILE) { set_field_back(field[index -1], A_REVERSE); }
    
    rows++;

    field[index] = new_field(1, 3, rows, 1, 0, 0); //id label
    set_field_buffer(field[index++], 0, "id:");
    field[index] = new_field(1, 4, rows++, 5, 0, 0); //id
    set_field_type(field[index], TYPE_INTEGER); //id index
    set_field_digit(field[index++], 0, second.id);

    field[index] = new_field(1, STEXT, rows++, 1, 0, 0); //fname label
    set_field_buffer(field[index++], 0, "name");
    field[index] = new_field(1, STEXT, rows--, 1, 0, 0); //fname
    set_field_buffer(field[index++], 0, second.fname);

    field[index] = new_field(1, STEXT, rows++, STEXT + 3, 0, 0); //lname label
    set_field_buffer(field[index++], 0, "last name");
    field[index] = new_field(1, STEXT, rows++, STEXT + 3, 0, 0); //lname
    set_field_buffer(field[index++], 0, second.lname);

    field[index] = new_field(1, PHONE, rows++, 1, 0, 0); //hphone label
    set_field_buffer(field[index++], 0, "home phone");
    field[index] = new_field(1, PHONE, rows--, 1, 0, 0); //hphone
    set_field_buffer(field[index++], 0, second.hphone);
    if(check & MATCH_SECOND_HPHONE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE + 3, 0, 0); //wphone
    set_field_buffer(field[index++], 0, "work phone");
    field[index] = new_field(1, PHONE, rows--, PHONE + 3, 0, 0); //wphone
    set_field_buffer(field[index++], 0, second.wphone);
    if(check & MATCH_SECOND_WPHONE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE * 2 + 7, 0, 0); //pmobile label
    set_field_buffer(field[index++], 0, "personal mobile");
    field[index] = new_field(1, PHONE, rows--, PHONE * 2 + 7, 0, 0); //pmobile
    set_field_buffer(field[index++], 0, second.pmobile);
    if(check & MATCH_SECOND_PMOBILE) { set_field_back(field[index -1], A_REVERSE); }

    field[index] = new_field(1, PHONE, rows++, PHONE * 3 + 9, 0, 0); //bmobile label
    set_field_buffer(field[index++], 0, "business mobile");
    field[index] = new_field(1, PHONE, rows++, PHONE * 3 + 9, 0, 0); //bmobile
    set_field_buffer(field[index++], 0, second.bmobile);
    if(check & MATCH_SECOND_BMOBILE) { set_field_back(field[index -1], A_REVERSE); }

    NULLSET(field[index]);

    for(index = 0; index < MATCH_FIELDS; index++) { //exlude the last field as NULL
        if(index % 2) { //all fields
            field_opts_on(field[index], O_VISIBLE);
            field_opts_on(field[index], O_PUBLIC);
            field_opts_on(field[index], O_ACTIVE);
            set_field_fore(field[index], A_BOLD);
        } else { //all label
            set_field_fore(field[index], A_NORMAL);
            set_field_back(field[index], A_NORMAL);
            //set_field_type(field[index], TYPE_ALPHA);        
            //set_field_opts(field[index], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
            //set_field_just(field[index], JUSTIFY_RIGHT);
        }
    }
    
    for(index = 0; index < MATCH_FIELDS; index++) { //exlude the last field as NULL
        set_field_status(field[index], false); //set the field as no modified
    }
    
    return field;
}

void print_in_middle(WINDOW *win, int y, const char *string, chtype color) {
    int x, length;

    if(!win)
        win = stdscr;

    length = (int) strlen(string);
    x = (int)((getmaxx(win) - length) / 2);

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

    if(!win)
        win = stdscr;

    length = (int) strlen(string) + 6;
    x = (int)((getmaxx(win) - length) / 2);

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
    mvwprintw(win, row, 2, "name");
    mvwprintw(win, row, STEXT + 4, "last name");
    row += 2;
    mvwprintw(win, row, 2, "organizzation");
    mvwprintw(win, row, MTEXT + 4, "job");
    row += 2;
    mvwprintw(win, row, 2, "home phone");
    mvwprintw(win, row, PHONE + 4, "work phone");
    mvwprintw(win, row, PHONE * 2 + 8, "personal mobile");
    mvwprintw(win, row, PHONE * 3 + 10, "business mobile");
    row += 2;
    mvwprintw(win, row, 2, "personal email");
    mvwprintw(win, row, MTEXT + 4, "business email");
    row += 2;
    mvwprintw(win, row, 2, "address");
    row += 2;
    mvwprintw(win, row, 2, "zip");
    mvwprintw(win, row, ZIP + 4, "city");
    mvwprintw(win, row + 1, ZIP + MTEXT + 5, "(");
    mvwprintw(win, row + 1, ZIP + MTEXT + 8, ")");
    mvwprintw(win, row, ZIP + MTEXT + STATE + 8, "country");
    row = 4; //birthday is on same line of name
    mvwprintw(win, row, STEXT * 2 + 16, "birthday");
    row = 6; //format under the date field
    mvwprintw(win, row, STEXT * 2 + 14, "dd");
    mvwprintw(win, row, STEXT * 2 + 18, "mm");
    mvwprintw(win, row, STEXT * 2 + 22, "yyyy");
    wattroff(win, color);

    return;
}

void logo(WINDOW *win, int y) {
    print_in_middle(win, y++, "      _____  _                      ____              _         ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "     |  __ \\| |                    |  _ \\            | |        ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "     | |__| | |__   ___  _ __   ___| |_| | ___   ___ | | __     ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "     |  ___/| '_ \\ / _ \\| '_ \\ / _ \\  _ < / _ \\ / _ \\| |/ /     ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "     | |    | | | | |_| | | | |  __/ |_| | |_| | |_| |   <      ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "     |_|    |_| |_|\\___/|_| |_|\\___|____/ \\___/ \\___/|_|\\_\\     ", COLOR_PAIR(PAIR_LOGO));
    print_in_middle(win, y++, "                                                                ", COLOR_PAIR(PAIR_LOGO));
    wrefresh(win);

    return;
}

void wrectangle(WINDOW *win, int y1, int x1, int y2, int x2) {
    int width, height;

    width = x2 - x1;
    height = y2 - y1;
    mvwhline(win, y1, x1, 0, width);
    mvwhline(win, y2, x1, 0, width);
    mvwvline(win, y1, x1, 0, height);
    mvwvline(win, y1, x2, 0, height);
    mvwaddch(win, y1, x1, ACS_ULCORNER);
    mvwaddch(win, y2, x1, ACS_LLCORNER);
    mvwaddch(win, y1, x2, ACS_URCORNER);
    mvwaddch(win, y2, x2, ACS_LRCORNER);
    wrefresh(win);

    return;
}
