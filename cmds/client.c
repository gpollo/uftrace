#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "uftrace.h"

int command_client(int argc, char *argv[], struct opts *opts) {
    int sfd;
    struct sockaddr_un addr;
    char *channel = NULL;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        pr_err("error opening socket\n");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    xasprintf(&channel, "%s/%s", opts->dirname, ".socket");
    strncpy(addr.sun_path, channel,
            sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1)
        pr_err("error connecting to socket\n");

    write(sfd, 0, sizeof(int));

    return 0;
}

