#define N 9  // input/enclave service threads
#define ENCLAVENAME "enclave_anonymbe"
#include <enclave_anonymbe_u.h>
#include <anonymbe_args.h>
#include <argp.h>
#include <string>

const char *argp_program_version = "A-SKY Key Manager";
const char *argp_program_bug_address = "<rafael.pires@unine.ch>";

/* Program documentation. */
static char doc[] = "A-SKY: Anonymous sharing key manager";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "port",   'p', "port", 0, "Listening port"},
    { "mongo",  'm', "string", 0, "Connection string for MongoDB" },
    { 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    Arguments *args = (Arguments*)state->input;
    switch(key) {
    case 'p':
        args->port = std::stoi(arg);
        break;
    case 'm':
        strncpy(args->mongo, arg, sizeof(args->mongo));
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    };
    return 0;
}

void init_args(Arguments *args) {
    args->port = 4444;
    strncpy(args->mongo, "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true", sizeof(args->mongo));
}

#include "sgxserver_bootstrap.cpp"

