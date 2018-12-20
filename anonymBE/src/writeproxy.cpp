#define N 1  // input/enclave service threads
#define ENCLAVENAME "enclave_writeproxy"
#include <argp.h>
#ifndef NATIVE
#include <enclave_writeproxy_u.h>
#endif
#include <writeproxy_args.h>
#include <string>
#include <cstring>

const char *argp_program_version = "A-SKY Writer proxy";
const char *argp_program_bug_address = "<rafael.pires@unine.ch>";

/* Program documentation. */
static char doc[] = "A-SKY: Anonymous sharing writer proxy";
static char args_doc[] = "";
static struct argp_option options[] = {
    {"port", 'p', "port", 0, "Server listening port (Default: 4445)"},
    {"endpoint", 'm', "host:port", 0, "Minio server endpoint"},
    {"access", 'a', "ACCESSKEY", 0, "Minio access key"},
    {"secret", 'k', "SECRETKEY", 0, "Minio secret key"},
    {"tokenuser", 'u', "USERNAME", 0, "Username of identity server"},
    {"tokenpass", 'w', "PASSWORD", 0, "Password of identity server"},
    {"tokenendpoint", 't', "host:port", 0, "Identity server endpoint"},
    {"amendpoint", 'e', "host:port", 0, "Access Monitor (anonymbe) endpoint. "
                                        "When user rights should be checked"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    Arguments *args = (Arguments *)state->input;
    switch (key) {
        case 'p':
            args->port = std::stoi(arg);
            break;
        case 'm':
            strncpy(args->minioendpoint, arg, sizeof(args->minioendpoint) - 1);
            break;
        case 'a':
            strncpy(args->minioaccesskey, arg,
                    sizeof(args->minioaccesskey) - 1);
            break;
        case 'k':
            strncpy(args->miniosecret, arg, sizeof(args->miniosecret) - 1);
            break;
        case 'u':
            strncpy(args->tokenuser, arg, sizeof(args->tokenuser) - 1);
            break;
        case 'w':
            strncpy(args->tokenpass, arg, sizeof(args->tokenpass) - 1);
            break;
        case 't':
            strncpy(args->tokenendpoint, arg, sizeof(args->tokenendpoint) - 1);
            break;
        case 'e':
            strncpy(args->amendpoint, arg, sizeof(args->amendpoint) - 1);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    };
    return 0;
}

//------------------------------------------------------------------------------
void init_args(Arguments *args) { 
    args->port = 4445;
    memset(args->minioendpoint, 0, sizeof(args->minioendpoint));
    memset(args->minioaccesskey, 0, sizeof(args->minioaccesskey));
    memset(args->miniosecret, 0, sizeof(args->miniosecret));
    memset(args->tokenuser, 0, sizeof(args->tokenuser));
    memset(args->tokenpass, 0, sizeof(args->tokenpass));
    memset(args->tokenendpoint, 0, sizeof(args->tokenendpoint));
    memset(args->amendpoint, 0, sizeof(args->amendpoint));
}

#include <sgxserver_bootstrap.cpp>
