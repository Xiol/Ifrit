#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ownetapi.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include "ifrit.h"
#include "statsd.h"

volatile sig_atomic_t g_running = 1;
pid_t pid;
pid_t sid;
int is_daemon = 0;

void sig_handler(int signum) {
    syslog(LOG_WARNING, "Signal received, quitting.\n");
    g_running = 0;
}

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

void daemonise(void) {
    /* http://www.danielhall.me/2010/01/writing-a-daemon-in-c/ */
    pid = fork();

    if (pid<0)
        exit(EXIT_FAILURE);
    
    if (pid>0)
        exit(EXIT_SUCCESS);

    umask(0);

    openlog("ifrit",LOG_NOWAIT|LOG_PID,LOG_USER);
    
    sid = setsid();

    if (sid<0) {
        syslog(LOG_ERR, "Could not create process group\n");
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Could not change working directory to /\n");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    is_daemon = 1;
}

void check_sensor(OWNET_HANDLE owh) {
    if (OWNET_present(owh, temp_path) != 0) {
        if (is_daemon) {
            syslog(LOG_ERR, "Sensor not detected. Quitting.\n");
            exit(EXIT_FAILURE);
        } else {
            printf("Sensor not detected. Quitting.\n");
            exit(EXIT_FAILURE);
        }
    }
}



int main(int argc, char **argv) {
    OWNET_HANDLE owh;
    statsdConnection *statsd = statsdConnect(statsd_host, statsd_port);
    uint8_t len;

    struct sigaction sigact; 
    sigact.sa_handler = sig_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);

    if (NULL == statsd) {
        printf("statsd connection failed\n");
        exit(EXIT_FAILURE);
    }

    if ((owh = OWNET_init(owserver_address)) < 0) {
        printf("OWNET_init(%s) failed.\n", owserver_address);
        exit(EXIT_FAILURE);
    }

    check_sensor(owh);

    daemonise();

    syslog(LOG_NOTICE, "Ifrit started.");

    while (g_running) {
        check_sensor(owh); 
        char *ret;

        OWNET_read(owh, temp_path, &ret);
        len = strip_whitespace(ret);
        char *buf = (char *)calloc(len+20, sizeof(char));

        if (buf != NULL) {
            sprintf(buf, "ifrit.temperature:%s|g", ret);
            statsdSend(statsd, buf, strlen(buf));
            if (TEMP_TO_SYSLOG) {
                syslog(LOG_INFO, "Read value: %s\n", ret);
            }
            free(buf);    
            free(ret);
        } else {
            syslog(LOG_ERR, "Unable to allocate memory!\n");
            exit(1);
        }

        sleep(SLEEPTIME);
    }
    statsdClose(statsd);
    closelog();

    return 0;
}
