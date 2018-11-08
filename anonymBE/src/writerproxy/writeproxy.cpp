#define N 9  // input/enclave service threads
#define ENCLAVENAME "enclave_writeproxy"
#include <enclave_writeproxy_u.h>
#include <writeproxy_args.h>
#include <argp.h>
#include <string>

const char *argp_program_version = "A-SKY Writer proxy";
const char *argp_program_bug_address = "<rafael.pires@unine.ch>";

/* Program documentation. */
static char doc[] = "A-SKY: Anonymous sharing writer proxy";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "port",   'p', "port", 0, "Listening port"},
    { 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    Arguments *args = (Arguments*)state->input;
    switch(key) {
    case 'p':
        args->port = std::stoi(arg);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    };
    return 0;
}

void init_args(Arguments *args) {
    args->port = 4445;
}

#include <sgxserver_bootstrap.cpp>


