#include "phonebook.h"

void logfile(const char *format, ...) {
	va_list args;
	FILE *fd;
	time_t current_time;
	struct tm *local;
	static unsigned int errcount = 0; //each session start from 0

	fd = fopen(LOGFILE, "a+"); // a+ (create + append) option will allow appending which is useful in a log file
	if(fd == NULL) {
		fprintf(stderr, "ERROR APPEND/WRITE LOG FILE\n");
		sleep(3);

		return;
	}

	time(&current_time); //get time
	local = localtime(&current_time); //convert to local
    fprintf(fd, "%d/%02d/%02d - ", local->tm_year+1900, local->tm_mon+1, local->tm_mday); // write current date yyyy/mm/dd
	fprintf(fd, "%02d:%02d:%02d ", local->tm_hour, local->tm_min, local->tm_sec); // write local time hh:mm:ss
    fprintf(fd, "[%03u] ", errcount++); //write the index log of current session

    va_start (args, format);
    vfprintf(fd, format, args); //write the info recieved from function arguments
    va_end (args);

	fclose(fd);

	return;
}

void db_log(const char *funcname, char *comment, DBnode_t *db) {
	DBnode_t ptrDB;

	logfile("%s: %s\n", funcname, comment);

	if(db) {
		logfile("%s: BNnode passed from %s\n", __func__, funcname);
		memcpy(&ptrDB, db, sizeof(DBnode_t));
	} else {
		logfile("%s: contacts->db\n", __func__);
		memcpy(&ptrDB, &contacts->db, sizeof(DBnode_t));
	}

	logfile("id: (%d)\n",	 			ptrDB.id);
	logfile("fname: (%s)\n",	 		ptrDB.fname);
	logfile("lname: (%s)\n",	 		ptrDB.lname);
	logfile("organization: (%s)\n",		ptrDB.organization);
	logfile("job: (%s)\n",	 			ptrDB.job);
	logfile("hphone: (%s)\n",	 		ptrDB.hphone);
	logfile("wphone: (%s)\n",	 		ptrDB.wphone);
	logfile("pmobile: (%s)\n",	 		ptrDB.pmobile);
	logfile("bmobile: (%s)\n",	 		ptrDB.bmobile);
	logfile("pemail: (%s)\n",	 		ptrDB.pemail);
	logfile("bemail: (%s)\n",	 		ptrDB.bemail);
	logfile("address: (%s)\n",	 		ptrDB.address);
	logfile("zip: (%s)\n",	 			ptrDB.zip);
	logfile("city: (%s)\n",	 			ptrDB.city);
	logfile("state: (%s)\n",	 		ptrDB.state);
	logfile("country: (%s)\n",	 		ptrDB.country);
	logfile("birthday: (%u/%u/%u)\n",	ptrDB.birthday.tm_mday, ptrDB.birthday.tm_mon, ptrDB.birthday.tm_year);

	return;
}

int filecopy(const char* source, const char* destination) {
    int input, output, result = -1;
    struct stat fileinfo = { 0 };
    off_t bytesCopied = 0;

    if((input = open(source, O_RDONLY)) == -1) { //try to open source file
        logfile("%s: Error opening '%s'\n", __func__, source);

        return result;
    }
    if((output = open(destination, O_CREAT|O_WRONLY|O_TRUNC, 0600)) == -1) { //try to create the destination file
        close(input);
        logfile("%s: Error creating '%s'\n", __func__, destination);

        return result;
    }

    fstat(input, &fileinfo); //get the file attributes
    result = sendfile(output, input, &bytesCopied, fileinfo.st_size); //sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+

    close(input); //close the handle
    close(output); //close the handle

    return result; //return the number of bytes copied
}