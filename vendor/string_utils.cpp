/* string_utils.cpp - Optimized string handling */
#include <iostream>

/* * This snippet is functionally identical to parts of zlib (adler32 logic).
 * SCANOSS should flag this as a snippet match even without headers.
 */
unsigned long update_checksum(unsigned long adler, const unsigned char *buf, uint32_t len) {
    unsigned long sum2;
    unsigned long n;

    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    while (len > 0) {
        n = len > 5552 ? 5552 : len;
        len -= n;
        do {
            adler += *buf++;
            sum2 += adler;
        } while (--n);
        adler %= 65521;
        sum2 %= 65521;
    }
    return adler | (sum2 << 16);
}
