#include <stdio.h>
#include <stdlib.h>
#include "core_logic.c"

/* * Demo Repo Enterprise Core 2026 
 * Proprietary implementation for demonstration purposes.
 */

int main(int argc, char *argv[]) {
    printf("--- demo_repo System Initializing ---\n");
    
    if (argc < 2) {
        printf("Usage: ./demo_repo <input_string>\n");
        return 1;
    }

    char *data = argv[1];
    printf("Processing secure data stream...\n");

    // Call to our internal crypto logic
    // This demonstrates how internal code links to identified crypto
    process_secure_payload(data);

    printf("Processing complete. System exiting safely.\n");
    return 0;
}
