#include "phonebook.h"

PhoneBook_t *contacts = NULL; // Global contatcts phonebook

int main(void)
{ // Phonebook main function
	WINDOW *win;
	int xmax, ymax;

	logfile("%s: *** NEW SESSION ***\n", __func__);

	setlocale(LC_ALL, "");
	bindtextdomain("Phonebook", "./po/");
	textdomain("Phonebook");

	initscr(); // init ncurses
	start_color(); // initialize colour
	cbreak(); // NOSTOP input
	noecho(); // off echo
	keypad(stdscr, true); // make keys work

	// initializing color pairs
	init_pair(PAIR_STD, COLOR_WHITE, COLOR_BLACK); // initializing 1 as a color pair for standard
	init_pair(PAIR_TITLE, COLOR_RED, COLOR_CYAN); // initializing 2 as a color pair for titles
	init_pair(PAIR_EDIT, COLOR_BLUE, COLOR_WHITE); // initializing 3 as a color pair for editing
	init_pair(PAIR_MODIFIED, COLOR_YELLOW, COLOR_BLUE); // initializing 4 as a color pair for modified
	init_pair(PAIR_LOGO, COLOR_RED, COLOR_BLACK); // initializing 4 as a color pair for modified
	init_pair(PAIR_ERROR, COLOR_YELLOW, COLOR_RED); // initializing 4 as a color pair for modified

	curs_set(false); // hide cursor
	timeout(100); // dalay for system
	getmaxyx(stdscr, ymax, xmax); // get current window console surface dimension
	if (ymax < 24 || xmax < 80) { // check if the current console window have enough rows and columns
		if(ymax < 24) // check the rows
			logfile("%s: Terminal height not enough you have %d rows, you need at least 24 rows\n", __func__, ymax);
		if(xmax < 80) // check the columns
			logfile("%s: Terminal width not enough you have %d columns, you need at least 80 columns\n", __func__, xmax);

		fprintf(stderr, "%s: Terminal error, see %s file please.", __func__, LOGFILE);
		getchar();
	} else {
		win = newwin(ymax, xmax, 0, 0); // create the main window
		wrefresh(win); // refresh the main window
		MainMenu(win); // run main program menu
	}

	curs_set(true); // show cursor
	endwin(); // close ncurses
	destroyList(contacts); // free memory routine for PhoneBook_t contacts

	logfile("%s: *** END SESSION ***\n", __func__);

	return EXIT_SUCCESS;
}
