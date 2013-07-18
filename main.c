#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ownetapi.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "ifrit.h"
#include "statsd.h"

int strip_whitespace(char *s) {
    char *p = s;
    int l = strlen(p);

    while (isspace(p[l - 1]))
        p[--l] = 0;
    while (*p && isspace(*p)) 
        ++p, --l;

    memmove(s, p, l + 1);
    return (l + 1);
}

int main(int argc, char **argv) {
    OWNET_HANDLE owh;
    statsdConnection *statsd = statsdConnect(statsd_host, statsd_port);
    unsigned short len;

    if (NULL == statsd) {
        printf("statsd connection failed\n");
        exit(1);
    }

    if ((owh = OWNET_init(owserver_address)) < 0) {
        printf("OWNET_init(%s) failed.\n", owserver_address);
        exit(1);
    }
    
    if (OWNET_present(owh, temp_path) != 0) {
        printf("Sensor not detected.\n");
        exit(1);
    }

    for (;;) {
        char *ret;

        OWNET_read(owh, temp_path, &ret);
        len = strip_whitespace(ret);
        char *buf = (char *)calloc(len+20, sizeof(char));

        if (buf != NULL) {
            sprintf(buf, "ifrit.temperature:%s|g", ret);
            statsdSend(statsd, buf, strlen(buf));
            if (TEMP_TO_STDOUT) {
                printf("Read value: %s\n", ret);
            }
            free(buf);    
            free(ret);
        } else {
            printf("Unable to allocate memory!\n");
            exit(1);
        }

        sleep(SLEEPTIME);
    }

    // We'll never get here :(
    // Shouldn't matter too much as statsd uses UDP
    statsdClose(statsd);

    return 0;
}
