# RSA Encyrption 

This program implement the RSA algorithm  

## Build

run `make` to build the program

## Running

`./keygen [-hv] [-b bits] -n pbfile -d pvfile`

`./encrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey`

`./decrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey`

## Keygen Output

SYNOPSIS
   Generates an RSA public/private key pair.

USAGE
  ./keygen [-hv] [-b bits] -n pbfile -d pvfile

OPTIONS
  -h              Display program help and usage.
  -v              Display verbose program output.
  -b bits         Minimum bits needed for public key n.
  -c confidence   Miller-Rabin iterations for testing primes (default: 50).
  -n pbfile       Public key file (default: rsa.pub).
  -d pvfile       Private key file (default: rsa.priv).
  -s seed         Random seed for testing.

## Encrypt Output

SYNOPSIS
   Encrypts data using RSA encryption.
   Encrypted data is decrypted by the decrypt program.

USAGE
   ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey

OPTIONS
  -h              Display program help and usage.
  -v              Display verbose program output.
  -i infile       Input file of data to encrypt (default: stdin).
  -o outfile      Output file for encrypted data (default: stdout).
  -n pbfile       Public key file (default: rsa.pub).

## Decrypt Output 

SYNOPSIS
   Decrypts data using RSA decryption.
   Encrypted data is encrypted by the encrypt program.

USAGE
  ./decrypt [-hv] [-i infile] [-o outfile] -n pubkey -d privkey

OPTIONS
  -h              Display program help and usage.
  -v              Display verbose program output.
  -i infile       Input file of data to decrypt (default: stdin).
  -o outfile      Output file for decrypted data (default: stdout).
  -d pvfile       Private key file (default: rsa.priv).
