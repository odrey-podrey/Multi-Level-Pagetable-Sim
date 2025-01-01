#include "config.h"
#include "mlpt.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// currently setting this manually in main for testing
// contains the value of the page table base register
size_t ptbr = 0; 

/**
 * Given a virtual address, return the physical address.
 * Return a value consisting of all 1 bits if this 
 * virtual address does not have a physical address.
 */
size_t translate(size_t va) {

    // *** NOTE TO SELF: need to compiled with -lm flag for the exponent to work ***

    size_t physical_address = (size_t)~0; // instantiate this var as all 1s for later
    int PAGESIZE = pow(2,POBITS); // will be 4096 for a PO of 12 bits

    if (ptbr == 0) {  // important initial check: if base reg is 0, table has not been allocated yet
        return physical_address;
    }

    printf("virtual address: %d\n", (int)va);

    size_t vpn = va/PAGESIZE; // same as doing vpn >> 12 (shaving off the last 12 PO bits)
    printf("VPN: %d\n", (int)vpn);

    int offset = va % PAGESIZE; // find the offset
    // printf("offset = %d\n", offset); 
 
    size_t pte_calculation = ptbr + 8*vpn; // find the address of the PTE by using the vpn & base reg to index into the table
    size_t* pte_address = (size_t*)pte_calculation; // cast it to a pointer since we wanna find the value @ this address

    // printf("pte address: %d\n", (int*)pte_address);

    size_t pte_value = *pte_address; // find the value @ the pte address -> the PTE itself!
    printf("value at pte address: %d\n", (int)pte_value);

    // check if this PTE is valid 
    if (pte_value % 2 == 1) { 
        // if valid, proceed
        printf("VALID\n");
        size_t ppn = pte_value/PAGESIZE; // knock the last PO bits off
        physical_address = ppn*PAGESIZE + offset; // re-align the ppn & add the offset
        printf("**physical address is: %d\n", (int)physical_address);
        return physical_address;
    }
    else {
        // if invalid, return a value consisting of all 1 bits
        printf("INVALID\n");
        printf("should return a value of all 1s: %zu\n",physical_address);
        return physical_address; 
    }
}

// int main() {

//     alignas(4096)
//     static size_t testing_page_table[512];

//     alignas(4096)
//     static char data_for_page_3[4096];
    
//     size_t address_of_data_for_page_3_as_integer = (size_t) &data_for_page_3[0]; 
//     size_t physical_page_number_of_data_for_page_3 = address_of_data_for_page_3_as_integer >> 12;
//     size_t page_table_entry_for_page_3 = (
//         (physical_page_number_of_data_for_page_3 << 12) | 1 );  

//     testing_page_table[3] = page_table_entry_for_page_3;
//     printf("physical page number of data for page 3 = %zu\n",address_of_data_for_page_3_as_integer);
//     ptbr = (size_t) &testing_page_table[0];
//     printf("**address for data for page 3 =  %d\n", &data_for_page_3[0x45]);
//     // printf("*TEST: %d\n", translate(0x3045));
    
//     if (translate(0x3045) == (size_t) &data_for_page_3[0x45]) {
//         printf("Test passed!\n");
//     }

//     return 0;    

// }