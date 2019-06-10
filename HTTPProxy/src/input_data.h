#ifndef TPE_PROTOS_INPUT_DATA_C_H
#define TPE_PROTOS_INPUT_DATA_C_H

#include <stdint.h>
#include <netinet/in.h>
#include <stdbool.h>

typedef struct input_data {
    char * listen_address;
    uint16_t port;
    char * replacement_msg;
    char * origin_server;
    uint16_t origin_port;
    char * filter_command;
    char * version;
    struct addrinfo * listenadddrinfo;
    char * management_address;
    uint16_t management_port;
    bool transf_activated;
    struct addrinfo * managementaddrinfo;
    char *transformation_command;
    int transformation_command_index;
    char *transformation_types;
    int transformation_types_index;

    char * user;
    char * pass;
    char * error_file;
};


typedef struct input_data * input_data;

void print_version();

void print_help();

void resolve_args(int argc, char **argv);

void resolve_address(char *address, uint16_t port, struct addrinfo **addrinfo);

long resolve_port(char * port_name, char *optarg);

extern input_data params;

#endif
