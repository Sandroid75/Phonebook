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

PhoneBook_t *contacts = NULL; // Global contatcts phonebook

int main(void)
{ // Phonebook main function
	WINDOW *win;
	int xmax, ymax;

	logfile("%s: *** NEW SESSION ***\n", __func__);

	setlocale(LC_ALL, "");
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
	if (ymax < 24 || xmax < 80) {
		logfile("%s: Terminal height not enough you have %d rows, you need at least 24 rows\n", __func__, ymax);
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
