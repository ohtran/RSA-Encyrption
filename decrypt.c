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

#define OPTIONS   "i:o:n:vh"
#define NAME_SIZE 50
void print_help() {
    printf("SYNOPSIS\n  Decrypts data using RSA decryption.\n  Encrypted data is encrypted by the "
           "encrypt program.\n\nUSAGE\n  ./decrypt [-hv] [-i infile] [-o outfile] -n pubkey -d "
           "privkey\n\nOPTIONS\n  -h              Display program help and usage.\n  -v            "
           "  Display verbose program output.\n  -i infile       Input file of data to decrypt "
           "(default: stdin).\n  -o outfile      Output file for decrypted data (default: "
           "stdout).\n  -d pvfile       Private key file (default: rsa.priv).\n");
}

int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = stdin, *outfile = stdout, *pvfile;
    bool verbose = false;
    char pv_filename[NAME_SIZE] = "rsa.priv";
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w+"); break;
        case 'n': strncpy(pv_filename, optarg, NAME_SIZE); break;
        case 'v': verbose = true; break;
        case 'h':
        default: print_help(); return 0;
        }
    }
    pvfile = fopen(pv_filename, "r");
    if (pvfile == NULL || infile == NULL || outfile == NULL) {
        printf("Fail to open file\n");
        return 1;
    }

    mpz_t n, d;
    mpz_init(n);
    mpz_init(d);

    rsa_read_priv(n, d, pvfile);

    if (verbose) {
        int n_size = mpz_sizeinbase(n, 2);
        int d_size = mpz_sizeinbase(d, 2);
        gmp_printf("n (%d) = %Zd\ne (%d) = %Zd\n", n_size, n, d_size, d);
    }

    rsa_decrypt_file(infile, outfile, n, d);

    fclose(pvfile);
    fclose(infile);
    fclose(outfile);
    mpz_clear(n);
    mpz_clear(d);
}
