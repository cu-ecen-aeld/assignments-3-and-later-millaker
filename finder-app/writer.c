#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Syslog
    openlog("writer", LOG_CONS, LOG_USER);
    // Check arguments
    if (argc != 3) {
        syslog(LOG_ERR, "Usage: writer FILENAME STRING, Got %d arguments instead\n", argc - 1);
        exit(1);
    }
    char *filename = argv[1];
    char *str = argv[2];
    // Open file for write
    FILE *fd = fopen(filename, "w");
    if (!fd) {
        syslog(LOG_ERR, "Cannot open file %s for write\n", filename);
        exit(1);
    }
    // Write content
    fprintf(fd, "%s", str);
    syslog(LOG_DEBUG, "Writing %s to %s\n", str, filename);
    fclose(fd);
    closelog();
    return 0;
}
