#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ownetapi.h>
#include <string.h>
#include <ctype.h>
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
    char *ret = (char *)calloc(30, sizeof(char));
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

    OWNET_read(owh, temp_path, &ret);
    
    len = strip_whitespace(ret);

    char *buf = (char *)calloc(len+20, sizeof(char));
    sprintf(buf, "ifrit.temperature:%s|g", ret);
    statsdSend(statsd, buf, strlen(buf));

    printf("Read value: %s\n", ret);

    statsdClose(statsd);
    free(ret);
    free(buf);

    return 0;
}
