#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include <math.h>
#include "randstate.h"

gmp_randstate_t state;

void gcd(mpz_t g, mpz_t a, mpz_t b) {
    mpz_t a_copy, b_copy; //make a copy of a and b
    mpz_init(a_copy);
    mpz_init(b_copy);
    mpz_set(a_copy, a);
    mpz_set(b_copy, b);
    while (mpz_cmp_ui(b_copy, 0) != 0) {
        //initialize t and set it to b;
        mpz_set(g, b_copy);
        mpz_mod(b_copy, a_copy, b_copy); //b = a % b;
        mpz_set(a_copy, g);
    }
    mpz_set(g, a_copy);

    mpz_clear(a_copy);
    mpz_clear(b_copy);
}

void mod_inverse(mpz_t o, mpz_t a, mpz_t n) {
    mpz_t r, r1, t, t1, q, temp_r, temp_r1, temp_t, temp_t1;
    mpz_init_set(r, n); //r =n
    mpz_init_set(r1, a); //r1=a
    mpz_init_set_ui(t, 0); //t =0
    mpz_init_set_ui(t1, 1); //t1=1
    mpz_init(q);
    mpz_init(temp_r);
    mpz_init(temp_r1);
    mpz_init(temp_t);
    mpz_init(temp_t1);

    while (mpz_cmp_ui(r1, 0) != 0) {
        mpz_div(q, r, r1);

        mpz_set(temp_r, r);
        mpz_set(temp_r1, r1);
        mpz_set(temp_t, t);
        mpz_set(temp_t1, t1);

        //(r,r′) ← (r′,r − q × r′)
        mpz_set(r, temp_r1);
        mpz_mul(temp_r1, q, temp_r1);
        mpz_sub(r1, temp_r, temp_r1);

        //(t,t′) ← (t′,t − q × t′)
        mpz_set(t, temp_t1);
        mpz_mul(temp_t1, q, temp_t1); // temp_t1 = q*temp_t1
        mpz_sub(t1, temp_t, temp_t1); // t1 = temp_t - temp_t1;
    }

    if (mpz_cmp_ui(r, 1) > 0) { // if r>1
        mpz_set_ui(o, 0);

        mpz_clear(r);
        mpz_clear(r1);
        mpz_clear(t);
        mpz_clear(t1);
        mpz_clear(q);
        mpz_clear(temp_r);
        mpz_clear(temp_r1);
        mpz_clear(temp_t);
        mpz_clear(temp_t1);

        return;
    }
    if (mpz_cmp_ui(t, 0) < 0) // if t<0
        mpz_add(t, t, n);
    mpz_set(o, t);

    mpz_clear(r);
    mpz_clear(r1);
    mpz_clear(t);
    mpz_clear(t1);
    mpz_clear(q);
    mpz_clear(temp_r);
    mpz_clear(temp_r1);
    mpz_clear(temp_t);
    mpz_clear(temp_t1);
}

// o = a^d % n
void pow_mod(mpz_t o, mpz_t a, mpz_t d, mpz_t n) {
    mpz_t p, d_copy, dMod2;
    mpz_init_set(p, a);
    mpz_init_set(d_copy, d);
    mpz_init(dMod2);

    mpz_set_ui(o, 1);

    while (mpz_cmp_ui(d_copy, 0) > 0) { //d>0
        mpz_mod_ui(dMod2, d_copy, 2);
        // if d is odd
        if (mpz_cmp_ui(dMod2, 0) != 0) {
            // o = (o*p) % n;
            mpz_mul(o, o, p);
            mpz_mod(o, o, n);
        }
        // p = (p*p) %n;
        mpz_mul(p, p, p);
        mpz_mod(p, p, n);
        //d = d/2;
        mpz_div_ui(d_copy, d_copy, 2);
    }
    mpz_clear(dMod2);
    mpz_clear(p);
    mpz_clear(d_copy);
}

//CITE: from Prof. Darrell Long's Lecture slide
bool witness(mpz_t a, mpz_t n) {
    mpz_t u, even;
    mpz_init(u);
    mpz_init(even);
    mpz_sub_ui(u, n, 1); // u=n-1
    uint64_t t = 0;

    mpz_mod_ui(even, u, 2);
    while (mpz_cmp_ui(even, 0) == 0) { // check if u is even
        mpz_fdiv_q_ui(u, u, 2);
        t++;
        mpz_mod_ui(even, u, 2);
    }

    mpz_t result, two, y, nMinusOne;
    mpz_init(result);
    mpz_init(two);
    mpz_init(y);
    mpz_init(nMinusOne);
    mpz_set_ui(two, 2);
    pow_mod(result, a, u, n); //result = a^u mod n

    for (uint64_t i = 0; i < t; i++) {
        pow_mod(y, result, two, n); // y = result^2 mod n
        mpz_sub_ui(nMinusOne, n, 1);
        // y == 1 and x!= 1 and x!= n-1
        if (mpz_cmp_ui(y, 1) == 0 && mpz_cmp_ui(result, 1) != 0 && mpz_cmp(result, nMinusOne) != 0)
            return true;
        mpz_set(result, y);
    }
    mpz_clear(u);
    mpz_clear(even);
    mpz_clear(two);
    mpz_clear(y);
    mpz_clear(nMinusOne);
    if (mpz_cmp_ui(result, 1) != 0) {
        mpz_clear(result);
        return true;
    }
    mpz_clear(result);
    return false;
}

//CITE: from Prof. Darrell Long's Lecture slide
bool is_prime(mpz_t n, uint64_t iters) {
    mpz_t nMod2;
    mpz_init(nMod2);
    mpz_mod_ui(nMod2, n, 2);

    // n<2 or (n!=2 and n%2==0)  --> not prime
    if (mpz_cmp_ui(n, 2) < 0 || (mpz_cmp_ui(n, 2) != 0 && mpz_cmp_ui(nMod2, 2) == 0))
        return false;
    // n < 4 --> is prime
    if (mpz_cmp_ui(n, 4) < 0)
        return true;
    mpz_t random;
    mpz_init(random);
    mpz_t range;
    mpz_init(range);
    mpz_sub_ui(range, n, 3);

    for (uint64_t i = 0; i < iters; i++) {
        mpz_urandomm(random, state, range); //generate random number from 0 to n-4
        mpz_add_ui(random, random, 2); //set range of random 2 < random < n-1
        if (witness(random, n)) {
            mpz_clear(random);
            mpz_clear(range);
            mpz_clear(nMod2);

            return false;
        }
    }
    mpz_clear(random);
    mpz_clear(range);
    mpz_clear(nMod2);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    do {
        mpz_urandomb(p, state, bits);
    } while (is_prime(p, iters) == false);
}
