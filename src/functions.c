#include "phonebook.h"

void db_log(const char *funcname, char *comment, DBnode_t *db)
{
    DBnode_t ptrDB;

    log_info("%s", comment);

    if (db) {
        log_info("BNnode passed from %s", funcname);
        memcpy(&ptrDB, db, sizeof(DBnode_t));
    } else {
        log_info("contacts->db");
        memcpy(&ptrDB, &contacts->db, sizeof(DBnode_t));
    }

    log_trace("id: (%d)", ptrDB.id);
    log_trace("fname: (%s)", ptrDB.fname);
    log_trace("lname: (%s)", ptrDB.lname);
    log_trace("organization: (%s)", ptrDB.organization);
    log_trace("job: (%s)", ptrDB.job);
    log_trace("hphone: (%s)", ptrDB.hphone);
    log_trace("wphone: (%s)", ptrDB.wphone);
    log_trace("pmobile: (%s)", ptrDB.pmobile);
    log_trace("bmobile: (%s)", ptrDB.bmobile);
    log_trace("pemail: (%s)", ptrDB.pemail);
    log_trace("bemail: (%s)", ptrDB.bemail);
    log_trace("address: (%s)", ptrDB.address);
    log_trace("zip: (%s)", ptrDB.zip);
    log_trace("city: (%s)", ptrDB.city);
    log_trace("state: (%s)", ptrDB.state);
    log_trace("country: (%s)", ptrDB.country);
    log_trace("birthday: (%u/%u/%u)", ptrDB.birthday.tm_mday, ptrDB.birthday.tm_mon, ptrDB.birthday.tm_year);

    return;
}
/*
check if fname exist and coincide with suorce_inode than open a new file
return the handle or -1 in case of error, -2 if source == dest, -3 user dont want overwriting
*/
static int opennew(const char *fname, const struct stat sbi)
{
    struct stat sbo;

    if (lstat(fname, &sbo) == 0) { // check if the target file already exists
        if (sbi.st_ino == sbo.st_ino)
            return FC_SAMEF;

        log_warn("Warning destination file %s exist", fname);
        if (unlink(fname))
            return FC_NOOWD;
    }

    return open(fname, O_CREAT | O_WRONLY | O_TRUNC, sbi.st_mode);
}

ssize_t filecopy(const char *source, const char *destination)
{
    int input, output;
    struct stat sb = {0};
    ssize_t chunk, bytesCopied, bytesTocopy = 0;
    char *errmsg, *data, *ptr, *end;
    _Bool tryAgain = FALSE;

    errno = 0;
    if (lstat(source, &sb)) { // read the attributes of the source file
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
        log_perror("File verification error %s\nerrno: [%d] %s", source, errno, errmsg);

        return FC_ERROR;
    }

    if ((input = open(source, O_RDONLY)) < 0) { // try to open source file
        log_perror("Error opening '%s'", source);

        return (ssize_t) input;
    }
    if ((output = opennew(destination, sb)) < 0) { // try to create the destination file
        close(input);
        log_perror("Error creating '%s'", destination);

        return (ssize_t) output;
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
        errmsg   = "Descriptor is not valid or locked, or an mmap(2)-like operation is not available for in_fd, or count is negative.";
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
        log_perror("Error sendfile() returned errno %d %s", errno, errmsg);

    if (tryAgain) { // read NOTES at https://man7.org/linux/man-pages/man2/sendfile.2.html
        close(output);
        unlink(destination);
        if ((output = opennew(destination, sb)) < 0) { // try to open the destinatiion file
            close(input);

            return (ssize_t) output;
        }
        log_warn("The application will try to copy again with a less performing strategy.");
        errno = 0;
        chunk = MIN(bytesTocopy, DEFAULT_CHUNK); // set better performance for copy
        data  = malloc((size_t) chunk);          // Allocate temporary data buffer.
        if (data) {
            bytesCopied = 0L;
            do { // read/write loop
                bytesTocopy = read(input, data, chunk);
                if (bytesTocopy <= 0)
                    break; // exit from do while loop

                ptr = data;
                end = (char *) (data + bytesTocopy);
                while (ptr < end) { // write data loop
                    bytesTocopy = write(output, ptr, (size_t) (end - ptr));
                    if (bytesTocopy <= 0) {
                        tryAgain = false; // exit from do while loop
                        break;            // exit from while loop
                    }
                    bytesCopied += bytesTocopy;
                    ptr += bytesTocopy;
                }
            } while (tryAgain);
            FREE(data);

            if (!errno)
                log_info("The new strategy worked, the file was copied!");
        }
    }
    close(input);  // close the handle
    close(output); // close the handle

    if (bytesCopied != bytesTocopy) { // verify that all bytes have been copied
        log_perror("Error not all data was copied!\nWrited %ld bytes on %ld bytes", bytesCopied, bytesTocopy);
        unlink(destination);
    }

    return bytesCopied; // return the number of bytes copied
}
