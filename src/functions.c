#include "phonebook.h"

void logfile(const char *format, ...)
{
	va_list args;
	FILE *fd;
	time_t current_time;
	struct tm *local;
	static unsigned int errcount = 0; // each session start from 0

	fd = fopen(LOGFILE, "a+"); // a+ (create + append) option will allow appending which is useful in a log file
	if (!fd) {
		fprintf(stderr, "ERROR APPEND/WRITE LOG FILE\n");
		sleep(3);

		return;
	}

	time(&current_time); // get time
	local = localtime(&current_time); // convert to local
	fprintf(fd, "%d/%02d/%02d - ", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday); // write current date yyyy/mm/dd
	fprintf(fd, "%02d:%02d:%02d ", local->tm_hour, local->tm_min, local->tm_sec); // write local time hh:mm:ss
	fprintf(fd, "[%03u] ", errcount++); // write the index log of current session

	va_start(args, format);
	vfprintf(fd, format, args); // write the info recieved from function arguments
	va_end(args);

	fclose(fd);

	return;
}

void db_log(const char *funcname, char *comment, DBnode_t *db)
{
	DBnode_t ptrDB;

	logfile("%s: %s\n", funcname, comment);

	if (db) {
		logfile("%s: BNnode passed from %s\n", __func__, funcname);
		memcpy(&ptrDB, db, sizeof(DBnode_t));
	} else {
		logfile("%s: contacts->db\n", __func__);
		memcpy(&ptrDB, &contacts->db, sizeof(DBnode_t));
	}

	logfile("id: (%d)\n", ptrDB.id);
	logfile("fname: (%s)\n", ptrDB.fname);
	logfile("lname: (%s)\n", ptrDB.lname);
	logfile("organization: (%s)\n", ptrDB.organization);
	logfile("job: (%s)\n", ptrDB.job);
	logfile("hphone: (%s)\n", ptrDB.hphone);
	logfile("wphone: (%s)\n", ptrDB.wphone);
	logfile("pmobile: (%s)\n", ptrDB.pmobile);
	logfile("bmobile: (%s)\n", ptrDB.bmobile);
	logfile("pemail: (%s)\n", ptrDB.pemail);
	logfile("bemail: (%s)\n", ptrDB.bemail);
	logfile("address: (%s)\n", ptrDB.address);
	logfile("zip: (%s)\n", ptrDB.zip);
	logfile("city: (%s)\n", ptrDB.city);
	logfile("state: (%s)\n", ptrDB.state);
	logfile("country: (%s)\n", ptrDB.country);
	logfile("birthday: (%u/%u/%u)\n", ptrDB.birthday.tm_mday, ptrDB.birthday.tm_mon, ptrDB.birthday.tm_year);

	return;
}
/*
check if fname exist and coincide with suorce_inode than open a new file 
return the handle or -1 in case of error, -2 if source == dest, -3 user dont want overwriting
*/
static int opennew(const char *fname, const struct stat sbi)
{
	struct stat sbo;

	if (lstat(fname, &sbo) == 0) { //check if the target file already exists
		if (sbi.st_ino == sbo.st_ino)
			return FC_SAMEF;

		logfile("%s: Warning destination file %s exist\n", __func__, fname);
		if (unlink(fname))
			return FC_NOOWD;
	}

	return open(fname, O_CREAT | O_WRONLY | O_TRUNC, sbi.st_mode);
}

ssize_t filecopy(const char *source, const char *destination)
{
	int input, output;
	struct stat sb = { 0 };
	ssize_t chunk, bytesCopied, bytesTocopy = 0;
	char *errmsg, *data, *ptr, *end;
	_Bool tryAgain = FALSE;

	errno = 0;
	if (lstat(source, &sb)) { //read the attributes of the source file
		switch (errno) {
		case EACCES:
			errmsg = "Search permission is denied for one of the directories in the path prefix of source file.";
			break;
		case EBADF:
			errmsg = "fd is not a valid open file descriptor.";
			break;
		case EFAULT:
			errmsg = "Bad address.";
			break;
		case ELOOP:
			errmsg = "Too many symbolic links encountered while traversing the path.";
			break;
		case ENAMETOOLONG:
			errmsg = "Source pathname is too long.";
			break;
		case ENOENT:
			errmsg = "A component of source pathname does not exist or is a dangling symbolic link.";
			break;
		case ENOMEM:
			errmsg = "Out of memory (i.e., kernel memory).";
			break;
		case ENOTDIR:
			errmsg = "A component of the path prefix of pathname is not a directory.";
			break;
		case EOVERFLOW:
			errmsg = "Source pathname refers to a file whose size, inode number, or number of blocks cannot be represented in, respectively, the types off_t, ino_t, or blkcnt_t.\nThis error can occur when, for example, an application compiled on a 32-bit platform without -D_FILE_OFFSET_BITS=64 calls stat() on a file whose size exceeds (1<<31)-1 bytes.";
			break;
		case 0:
		default:
			errmsg = NULL;
			break;
		}
		logfile("%s: File verification error %s\nerrno: [%d] %s\n", __func__, source, errno, errmsg);

		return FC_ERROR;
	}

	if ((input = open(source, O_RDONLY)) < 0) { // try to open source file
		logfile("%s: Error opening '%s'\n", __func__, source);

		return (ssize_t)input;
	}
	if ((output = opennew(destination, sb)) < 0) { // try to create the destination file
		close(input);
		logfile("%s: Error creating '%s'\n", __func__, destination);

		return (ssize_t)output;
	}

	errno       = 0;
	bytesTocopy = sb.st_size;
	bytesCopied = sendfile(output, input, NULL, bytesTocopy); // sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+
	switch (errno) {
	case EAGAIN:
		errmsg = "Nonblocking I/O has been selected using O_NONBLOCK and the write would block.";
		break;
	case EBADF:
		errmsg = "The input file was not opened for reading or the output file was not opened for writing.";
		break;
	case EFAULT:
		errmsg = "Bad address.";
		break;
	case EINVAL:
	case ENOSYS:
		errmsg    = "Descriptor is not valid or locked, or an mmap(2)-like operation is not available for in_fd, or count is negative.";
		tryAgain = TRUE; // read NOTES at https://man7.org/linux/man-pages/man2/sendfile.2.html
		break;
	case EIO:
		errmsg = "Unspecified error while reading from in_fd.";
		break;
	case ENOMEM:
		errmsg = "Insufficient memory to read from in_fd.";
		break;
	case EOVERFLOW:
		errmsg = "count is too large, the operation would result in exceeding the maximum size of either the input file or the output file.";
		break;
	case ESPIPE:
		errmsg = "offset is not NULL but the input file is not seekable.";
		break;
	case 0:
	default:
		errmsg = NULL;
		break;
	}
	if (errno)
		logfile("%s: Error sendfile() returned errno %d %s\n", __func__, errno, errmsg);

	if (tryAgain) { // read NOTES at https://man7.org/linux/man-pages/man2/sendfile.2.html
		close(output);
		unlink(destination);
		if ((output = opennew(destination, sb)) < 0) { //try to open the destinatiion file
			close(input);

			return (ssize_t)output;
		}
		logfile("%s: The application will try to copy again with a less performing strategy.\n", __func__);
		errno = 0;
		chunk = MIN(bytesTocopy, DEFAULT_CHUNK); //set better performance for copy
		data  = malloc((size_t)chunk); // Allocate temporary data buffer.
		if (data) {
			bytesCopied = 0L;
			do { // read/write loop
				bytesTocopy = read(input, data, chunk);
				if (bytesTocopy <= 0)
					break; // exit from do while loop

				ptr = data;
				end = (char *)(data + bytesTocopy);
				while (ptr < end) { //write data loop
					bytesTocopy = write(output, ptr, (size_t)(end - ptr));
					if (bytesTocopy <= 0) {
						tryAgain = false; // exit from do while loop
						break; // exit from while loop
					}
					bytesCopied += bytesTocopy;
					ptr += bytesTocopy;
				}
			} while (tryAgain);
			FREE(data);

			if (!errno)
				logfile("%s: The new strategy worked, the file was copied!\n", __func__);
		}
	}
	close(input); //close the handle
	close(output); //close the handle

	if (bytesCopied != bytesTocopy) { //verify that all bytes have been copied
		logfile("\n%s: Error not all data was copied!\nWrited %ld bytes on %ld bytes\n", __func__, bytesCopied, bytesTocopy);
		unlink(destination);
	}

	return bytesCopied; //return the number of bytes copied
}
