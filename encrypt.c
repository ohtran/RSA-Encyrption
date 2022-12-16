#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmp.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"
#include <sys/types.h>
#include <sys/stat.h>

#define OPTIONS "i:o:n:vh"

void print_help() {
    printf("SYNOPSIS\n  Encrypts data using RSA encryption.\n  Encrypted data is decrypted by the "
           "decrypt program.\n\nUSAGE\n  ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey -d "
           "privkey\n\nOPTIONS\n  -h              Display program help and usage.\n  -v            "
           "  Display verbose program output.\n  -i infile       Input file of data to encrypt "
           "(default: stdin).\n  -o outfile      Output file for encrypted data (default: "
           "stdout).\n  -n pbfile       Public key file (default: rsa.pub).\n");
}

int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = stdin, *outfile = stdout, *pbfile;
    bool verbose = false;
    char pb_filename[SCHAR_MAX] = "rsa.pub";

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w+"); break;
        case 'n': strncpy(pb_filename, optarg, SCHAR_MAX); break;
        case 'v': verbose = true; break;
        case 'h':
        default: print_help(); return 0;
        }
    }

    pbfile = fopen(pb_filename, "r");
    if (pbfile == NULL) {
        printf("Fail to open public key file\n");
        return 1;
    }
    if (infile == NULL) {
        printf("Fail to open infile\n");
        return 1;
    }

    if (outfile == NULL) {
        printf("Fail to open outfile\n");
        return 1;
    }

    char username[SCHAR_MAX];
    mpz_t n, e, s, usr;
    mpz_init(n);
    mpz_init(e);
    mpz_init(s);
    mpz_init(usr);

    rsa_read_pub(n, e, s, username, pbfile);
    mpz_set_str(usr, username, 62); //change variable type from char* to mpz_t

    if (verbose) {
        int s_size = mpz_sizeinbase(s, 2);
        int n_size = mpz_sizeinbase(n, 2);
        int e_size = mpz_sizeinbase(e, 2);
        printf("username: %s\n", username);
        gmp_printf("s (%d)= %Zd\nn (%d) = %Zd\ne (%d) = %Zd\n", s_size, s, n_size, n, e_size, e);
    }

    //verify signature
    if (rsa_verify(usr, s, e, n) == false) {
        printf("Incorrect signature\n");
        return 1;
    }

    rsa_encrypt_file(infile, outfile, n, e);

    fclose(pbfile);
    fclose(infile);
    fclose(outfile);
    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(s);
    mpz_clear(usr);
}
