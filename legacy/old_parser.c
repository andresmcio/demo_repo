/* * OpenSSL Heartbleed Vulnerable Snippet (CVE-2014-0160)
 * For Educational Audit & Compliance Demonstration Purposes Only.
 */

#include <string.h>
#include <stdlib.h>

int process_heartbeat_request(unsigned char *p, unsigned int payload_length) {
    /*
     * THE VULNERABILITY:
     * This code trusts the 'payload_length' sent by the user without 
     * checking if it actually matches the size of the record 'p'.
     */
    
    unsigned char *pl;
    unsigned char *buffer;
    unsigned int padding = 16; /* Standard padding */

    /* 'p' is the pointer to the incoming heartbeat request */
    /* The requester says the payload is 'payload_length' bytes long */
    pl = p; 

    /* We allocate a buffer based on what the USER says, not the reality */
    buffer = (unsigned char *)malloc(1 + 2 + payload_length + padding);
    
    if (buffer == NULL) return -1;

    unsigned char *bp = buffer;

    /* Enter response type and length */
    *bp++ = 2; // TLS1_HB_RESPONSE
    *bp++ = (payload_length >> 8) & 0xff;
    *bp++ = payload_length & 0xff;

    /* * CRITICAL FAILURE: 
     * memcpy copies 'payload_length' bytes from 'pl' into 'bp'.
     * If the user sent a 1-byte payload but CLAIMED it was 65535 bytes,
     * this will copy 65534 bytes of the server's private memory.
     */
    memcpy(bp, pl, payload_length); 
    
    bp += payload_length;
    /* Add random padding */
    memset(bp, 0, padding);

    // Send buffer... (omitted for demo)
    free(buffer);
    return 0;
}
