#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "input_data.h"
#include "selector.h"
#include "http.h"
// #include "management.h"
// #include "metrics.h"


int create_passive_socket(struct addrinfo *address, int proto) {
    int passive_socket;
    int sock_opt = true;

    if ((passive_socket = socket(address->ai_family, SOCK_STREAM, proto)) == 0) {
        perror("unable to create socket");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections
    // this is just a good habit, it will work without this
    if (setsockopt(passive_socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&sock_opt, sizeof(sock_opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(passive_socket, address->ai_addr, address->ai_addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return passive_socket;
}

// metrics metricas;

int main (int argc, char ** argv) {

    // resolve_args(argc,argv);

    // metricas = calloc(1, sizeof(*metricas));

    int passive_tcp_socket = create_passive_socket(params->listenadddrinfo, IPPROTO_TCP);

    if (listen(passive_tcp_socket, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on TCP %s:%d \n", params->listen_address, params->port);

    int passive_sctp_socket = create_passive_socket(params->managementaddrinfo, IPPROTO_SCTP);

    if (listen(passive_sctp_socket, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on SCTP %s:%d \n", params->management_address, params->management_port);

    //accept the incoming connection
    puts("Waiting for connections ...");

    // there is nothing to read from stdin
    close(0);
    signal(SIGPIPE, SIG_IGN);

    const char       *err_msg;
    selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

    const struct selector_init conf = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec  = 10,
                    .tv_nsec = 0,
            },
    };
    if(0 != selector_init(&conf)) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(1024);
    if(selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }

    const struct fd_handler HTTP_handler = {
            .handle_read       = &http_passive_accept,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
    };

    // proxy server management protocol
    const struct fd_handler psmp_handler = {
            // .handle_read       = &psmp_accept_connection,
            .handle_read       = NULL,
            .handle_write      = NULL,
            .handle_close      = NULL,
    };

    selector_status ss_http = selector_register(selector, passive_tcp_socket,
                                                &http_handler, OP_READ, NULL);

    selector_status ss_psmp = selector_register(selector, passive_sctp_socket,
                                                &psmp_handler, OP_READ, NULL);

    if(ss_http != SELECTOR_SUCCESS || ss_psmp != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }

    for(;;) {
        err_msg  = NULL;
        ss  = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            break;
        }
    }

    if(err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;

finally:
    if(ss!= SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                ss_http == SELECTOR_IO
                ? strerror(errno)
                : selector_error(ss_pop3));
        ret = 2;
    } else if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(selector!= NULL) {
        selector_destroy(selector);
    }
    selector_close();
    http_pool_destroy();

    if(passive_tcp_socket >= 0) {
        close(passive_tcp_socket);
    }
    return ret;

}