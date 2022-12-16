#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"
#include <sys/types.h>
#include <sys/stat.h>

#define OPTIONS "b:i:n:d:s:vh"

void print_help() {
    printf(
        "SYNOPSIS\n  Generates an RSA public/private key pair.\n\nUSAGE\n  ./keygen [-hv] [-b "
        "bits] -n pbfile -d pvfile\n\nOPTIONS\n  -h              Display program help and usage.\n "
        " -v              Display verbose program output.\n  -b bits         Minimum bits needed "
        "for public key n.\n  -c confidence   Miller-Rabin iterations for testing primes (default: "
        "50).\n  -n pbfile       Public key file (default: rsa.pub).\n  -d pvfile       Private "
        "key file (default: rsa.priv).\n  -s seed         Random seed for testing.\n");
}

int main(int argc, char **argv) {
    int opt = 0;
    uint64_t seed = time(NULL);
    uint64_t bits = 256, iters = 50;
    FILE *pbfile, *pvfile;
    char pb_filename[SCHAR_MAX] = "rsa.pub";
    char pv_filename[SCHAR_MAX] = "rsa.priv";
    bool verbose = false;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': bits = strtoul(optarg, NULL, INT_MAX); break;
        case 'i': iters = strtoul(optarg, NULL, INT_MAX); break;
        case 'n': strncpy(pb_filename, optarg, SCHAR_MAX);
        case 'd': strncpy(pv_filename, optarg, SCHAR_MAX);
        case 's': seed = strtoul(optarg, NULL, INT_MAX); break;
        case 'v': verbose = true; break;
        case 'h':
        default: print_help(); return 0;
        }
    }
    pbfile = fopen(pb_filename, "w+");
    pvfile = fopen(pv_filename, "w+");
    if (pbfile == NULL) {
        printf("Fail to open file\n");
        return 1;
    }
    if (pvfile == NULL) {
        printf("Fail to open file\n");
        return 1;
    }

    //change file permission to 0600. read and write permissions only for the user
    fchmod(fileno(pbfile), 0600);
    fchmod(fileno(pvfile), 0600);

    randstate_init(seed);
    mpz_t p, q, n, e, d, usr, s;
    mpz_init(p);
    mpz_init(q);
    mpz_init(n);
    mpz_init(e);
    mpz_init(d);
    mpz_init(usr);
    mpz_init(s);
    rsa_make_pub(p, q, n, e, bits, iters); //make public key
    rsa_make_priv(d, e, p, q); //make private key

    char *username = getenv("USER");
    mpz_set_str(usr, username, 62); //change variable type from char* to mpz_t
    rsa_sign(s, usr, d, n);

    rsa_write_pub(n, e, s, username, pbfile); //write the public key to pbfile
    rsa_write_priv(n, d, pvfile);

    if (verbose) {
        //get the number of bits
        int s_size = mpz_sizeinbase(s, 2);
        int p_size = mpz_sizeinbase(p, 2);
        int q_size = mpz_sizeinbase(q, 2);
        int n_size = mpz_sizeinbase(n, 2);
        int e_size = mpz_sizeinbase(e, 2);
        int d_size = mpz_sizeinbase(d, 2);
        printf("username: %s\n", username);
        gmp_printf(
            "s (%d) = %Zd\np (%d) = %Zd\nq (%d) = %Zd\nn (%d) = %Zd\ne (%d) = %Zd\nd (%d) = %Zd\n",
            s_size, s, p_size, p, q_size, q, n_size, n, e_size, e, d_size, d);
    }

    fclose(pbfile);
    fclose(pvfile);
    randstate_clear();
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(usr);
    mpz_clear(s);
}
