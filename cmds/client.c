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

int command_client(int argc, char *argv[], struct opts *opts) {
    int sfd;
    struct sockaddr_un addr;
    char *channel = NULL;
    enum uftrace_dopt opt;
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

    /* if (opts->patt_type) { */
    /*     opt = UFTRACE_DOPT_PATT_TYPE; */
    /*     if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1) */
    /*         pr_err("error sending option type"); */

    /*     if (write(sfd, &opts->patt_type, */
    /*               sizeof(enum uftrace_pattern_type)) == -1) */
    /*         pr_err("error sending options"); */
    /* } */

    if (opts->filter) {
        char *filter_str = uftrace_clear_kernel(opts->filter);

        if (filter_str) {
            opt = UFTRACE_DOPT_FILTER;
            if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1)
                pr_err("error sending option type");

            if (write(sfd, filter_str, MCOUNT_DOPT_SIZE) == -1)
                pr_err("error sending options");

            free(filter_str);
        }
    }

    /* if (opts->patch) { */
    /*     enum uftrace_dopt opt = UFTRACE_DOPT_PATCH; */
    /*     if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1) */
    /*         pr_err("error sending options"); */

    /*     strcpy(command, opts->patch); */
    /*     if (write(sfd, &command, MCOUNT_DOPT_SIZE) == -1) */
    /*         pr_err("error sending options"); */
    /* } */

    /* opt = UFTRACE_DOPT_CLOSE; */
    /* if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1) */
    /*     pr_err("error sending options"); */

    /* HACK artificial killing of the daemon */
    opt = UFTRACE_DOPT_KILL;
    if (write(sfd, &opt, sizeof(enum uftrace_dopt)) == -1)
        pr_err("error sending options");

    close(sfd);

    return 0;
}

