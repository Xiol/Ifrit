/*
 * Copyright (c) 2012, Krzysztof Jagiello <balonyo at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "statsd.h"

statsdConnection *statsdConnect(const char *ip, unsigned int port) {
    statsdConnection *c;

    // alloc memory for connection structure
    c = malloc(sizeof(statsdConnection));

    // return NULL if we are out of memory
    if (c == NULL) {
        return NULL;
    }

    // memory from malloc may not be clean, we clean it
    memset(c, 0, sizeof(statsdConnection));

    // create socket and check for errors
    if ((c->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Could not create a socket.");
        return NULL;
    }

    // inet_aton
    memset((void *)&c->si_dest, 0, sizeof(c->si_dest));
    c->si_dest.sin_family = AF_INET;
    c->si_dest.sin_port = htons(port);

    if (inet_aton(ip, &c->si_dest.sin_addr) == 0) {
        perror("inet_aton failed.");
        return NULL;
    }

    return c;
}

void statsdSend(statsdConnection *c, const char *buf, int buflen) {
    if (sendto(c->socket, buf, buflen, 0, (const struct sockaddr *)&c->si_dest, sizeof(c->si_dest)) == -1) {
        perror("sendto failed.");
    }
}

void statsdClose(statsdConnection *c) {
    if (c->socket != -1) {
        close(c->socket);
    }

    free(c);
}