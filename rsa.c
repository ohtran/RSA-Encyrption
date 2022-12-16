#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gmp.h>
#include <math.h>

#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"

//CITE: adapted from the function biginteger_log in the file bigintegerR.cc in gmp R package
uint64_t logbase2(mpz_t op) {
    long int exp;
    double ans = mpz_get_d_2exp(&exp, op);
    //log(op)
    double log_op = log(ans) + log(2) * (double) exp;
    //log2(op) = log(op) / log(2);
    return (uint64_t)(log_op / log(2));
}

void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    srand(time(NULL));
    uint64_t lower_bound = nbits / 4;
    uint64_t upper_bound = (3 * nbits) / 4;
    uint64_t rand_num_p
        = (rand() % (upper_bound - lower_bound)) + lower_bound; //number of bits for p
    uint64_t rand_num_q = nbits - rand_num_p; //number of bits for q

    //loop run until log2(n) >= nbits
    do {
        make_prime(p, rand_num_p, iters); //Make prime q
        make_prime(q, rand_num_q, iters); //Make prime q
        mpz_mul(n, q, p); // n = p*q
        rand_num_p++;
        rand_num_q++;
    } while (logbase2(n) < nbits);

    mpz_t totient; //φ(n)
    mpz_init(totient);
    //n-p-q+1
    mpz_sub(totient, n, q);
    mpz_sub(totient, totient, p);
    mpz_add_ui(totient, totient, 1);

    //gmp_printf("totient: %Zd\n", euler_totient);

    //We choose an encryption key e such that is it relatively prime to φ(n), that is, gcd(e,φ(n)) = 1.
    mpz_t g;
    mpz_init(g);
    do {
        mpz_urandomb(e, state, nbits); //generate rand # in range 2^nbits -1
        gcd(g, e, totient);
    } while (mpz_cmp_ui(g, 1) != 0);

    mpz_clear(totient);
    mpz_clear(g);
}

void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    // public key n,e,s then username
    //n,e,s should be hexstring
    gmp_fprintf(pbfile, "%Zx\n%Zx\n%Zx\n%s\n", n, e, s, username);
}

void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n%Zx\n%Zx\n%s\n", n, e, s, username);
}

void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    // d = e % φ(n) = (p − 1)(q − 1)

    // caculate for φ(n)
    mpz_t totient, n;
    mpz_init(totient);
    mpz_init(n);
    mpz_mul(n, q, p); // n = p*q
    mpz_sub(totient, n, q);
    mpz_sub(totient, totient, p);
    mpz_add_ui(totient, totient, 1);

    // calculate d
    mod_inverse(d, e, totient);

    mpz_clear(totient);
    mpz_clear(n);
}

void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n%Zx\n", n, d);
}

void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n%Zx\n", n, d);
}

void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    //E(m) = c = m^e (mod n)
    pow_mod(c, m, e, n);
}

void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    //block of data must be less than n
    //value of a block can't be 0 or 1
    //value of prepended byte will be 0xFF
    //block size k = (log2(n)-1)/8
    //allocate an rray that can hold k bytes --> uint8_t
    //set zeroth byte to 0xFF
    //start from index 1
    mpz_t m, c;
    mpz_init(m);
    mpz_init(c);
    fseek(infile, 0, SEEK_END);
    uint64_t k = (logbase2(n) - 1) / 8, j = 0, byte_read = 0, fileSize = ftell(infile);
    rewind(infile);

    //Read the file and encrypt block of data size k until reach EOF
    do {
        uint8_t *buf = (uint8_t *) malloc(k * sizeof(uint8_t));
        buf[0] = 0xFF;
        j = fread(buf + 1, sizeof(uint8_t), k - 1, infile);
        byte_read += j;
        //change uint8_t *buf into mpz_t m
        mpz_import(m, j + 1, 1, sizeof(buf[0]), 1, 0, buf);
        //encrypt the message
        rsa_encrypt(c, m, e, n);
        gmp_fprintf(outfile, "%Zx\n", c);
        free(buf);
        buf = NULL;
    } while (byte_read < fileSize); //read k bytes into buf

    mpz_clear(m);
    mpz_clear(c);
}

void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    //D(c) = m = c^d (mod n)
    pow_mod(m, c, d, n);
}

void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    uint64_t k = (logbase2(n) - 1) / 8;
    size_t j = 0;
    mpz_t c, m;
    mpz_init(c);
    mpz_init(m);
    uint8_t *buf = (uint8_t *) malloc(k * sizeof(uint8_t));
    //scan in a line and decrypt it
    while (gmp_fscanf(infile, "%Zx\n", c) != EOF) {
        rsa_decrypt(m, c, d, n);
        //change the decrypted m into uint8_t *buf
        mpz_export(buf, &j, 1, sizeof(buf[0]), 1, 0, m);
        for (size_t i = 1; i < j; i++) {
            fprintf(outfile, "%c", (char) buf[i]); //print decrypted letter
        }
    }
    mpz_clear(c);
    mpz_clear(m);
    free(buf);
}

void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    //S(m) = s = m^d (mod n)
    pow_mod(s, m, d, n);
}

bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    //V(s) = t = s^e (mod n)
    mpz_t t;
    mpz_init(t);
    pow_mod(t, s, e, n);
    //compare the signature with the calculated signature
    if (mpz_cmp(t, m) == 0) {
        mpz_clear(t);
        return true;
    }
    mpz_clear(t);
    return false;
}
