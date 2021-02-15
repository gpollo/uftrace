#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>

#include "uftrace.h"
#include "libmcount/mcount.h"
#include "utils/utils.h"

/* Socket file descriptor, to communicate with the daemon */
int sfd;

void send_option(enum uftrace_dopt opt) {
    if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1)
            pr_err("error sending option type");
}

int command_client(int argc, char *argv[], struct opts *opts) {
    struct sockaddr_un addr;
    char *channel = NULL;
    char command[MCOUNT_DOPT_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        pr_err("error opening socket");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    xasprintf(&channel, "%s/%s", opts->dirname, ".socket");
    strncpy(addr.sun_path, channel,
            sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1)
        pr_err("error connecting to socket");

    if (opts->patt_type) {
        send_option(UFTRACE_DOPT_PATT_TYPE);

        if (write(sfd, &opts->patt_type,
                  sizeof(enum uftrace_pattern_type)) == -1)
            pr_err("error sending options");
    }

    if (opts->patch) {
        send_option(UFTRACE_DOPT_PATCH);
        pr_dbg3("changing patch options\n");

        strcpy(command, opts->patch);
        if (write(sfd, &command, MCOUNT_DOPT_SIZE) == -1)
            pr_err("error sending options");
    }

    if (opts->filter) {
        char *filter_str = uftrace_clear_kernel(opts->filter);

        pr_dbg3("changing filter options\n");
        if (filter_str) {
            send_option(UFTRACE_DOPT_FILTER);

            if (write(sfd, filter_str, MCOUNT_DOPT_SIZE) == -1)
                pr_err("error sending options");

            free(filter_str);
        }
    }

    if (opts->caller) {
        pr_dbg3("changing caller filter options\n");
        send_option(UFTRACE_DOPT_CALLER_FILTER);

        strcpy(command, opts->caller);
        if (write(sfd, &command, MCOUNT_DOPT_SIZE) == -1)
            pr_err("error sending options");
    }

    if (opts->args) {
        pr_dbg3("changing argument options\n");
        send_option(UFTRACE_DOPT_ARGUMENT);

        strcpy(command, opts->args);
        if (write(sfd, &command, MCOUNT_DOPT_SIZE) == -1)
            pr_err("error sending options");
    }

    if (opts->retval) {
        pr_dbg3("changing retval options\n");
        send_option(UFTRACE_DOPT_RETVAL);

        strcpy(command, opts->retval);
        if (write(sfd, &command, MCOUNT_DOPT_SIZE) == -1)
            pr_err("error sending options");
    }

    /* send_option(UFTRACE_DOPT_KILL);    /\* HACK artificial killing of the daemon *\/ */
    send_option(UFTRACE_DOPT_CLOSE);
    close(sfd);

    return 0;
}

