#define _XOPEN_SOURCE 700
#include "config.h"
#include "mlpt.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>


size_t ptbr = 0; // base reg; default to 0 when nothing's been allocated
size_t arr_vpn_segs[LEVELS]; // make an array to hold each of the vpn segments, which = num levels


/**
Given a virtual address, populate a global array with each vpn segment, in order
 */
void setUpVpnSegArray(size_t va) {

    int pagesize = pow(2,POBITS); 
    int num_pte = pagesize/8; // each PTE is given to be 8 bytes
    size_t vpn = va >> POBITS; 
    size_t num_bits_per_vpn_seg = (size_t) log2(num_pte);

    // fills array in reverse order, since it's more convinient to start populating with the lowest order x bits (last vpn segment)
    // doing it this way to dodge potential unused bits at the higher order
    for (size_t i = 0; i < LEVELS; i += 1) {
        size_t startBit = i*num_bits_per_vpn_seg;
        unsigned mask = ((1 << num_bits_per_vpn_seg) - 1) << startBit; 
        size_t isolatedXbits = (vpn & mask) >> startBit;
        arr_vpn_segs[LEVELS-i-1] = isolatedXbits; 
    }
}


/**
 * Given a virtual address, return the physical address.
 * Return a value consisting of all 1 bits if this 
 * virtual address does not have a physical address.
 * *** NOTE TO SELF ~ need to compiled with -lm flag for the exponent to work ***
 */
size_t translate(size_t va) { 

    size_t physical_address = (size_t)~0; // instantiate this as all 1s for later
    if (ptbr == 0) {  // important initial check: if base reg is 0, table has not been allocated yet
        return physical_address;
    }
    size_t curr_base_reg = ptbr; // this value will change if we have multiple levels, but initial base reg is ptbr

    // -------------- TRANSLATION --------------
    setUpVpnSegArray(va);
    int pagesize = pow(2,POBITS); 
    int offset = va % pagesize;

    for (int i = 0; i < LEVELS; i += 1) { 

        size_t vpn = arr_vpn_segs[i]; 
        size_t pte_calculation = curr_base_reg + 8*vpn; // find PTE address by using the vpn & base reg to index into the table
        size_t* pte_address = (size_t*)pte_calculation; // cast it to a pointer since we wanna find the value @ this address
        size_t pte_value = *pte_address; // find the value @ the pte address -> the PTE itself!

        if (pte_value % 2 == 1) { 
            // if valid, proceed
            size_t ppn = pte_value >> POBITS; 
            curr_base_reg = ppn*pagesize; // new base register for next iteration is the realigned ppn
            physical_address = ppn*pagesize;

            if (LEVELS == 1 | i == LEVELS - 1) { 
                // if only 1 level OR last level: return the final physical address (accounting for offset)
                return physical_address + offset;
            }
        }
        else {
            // if invalid, return a value consisting of all 1 bits
            size_t physical_address = (size_t)~0; 
            return physical_address; 
        } 
    }
    return physical_address;
}



/**
 * Use posix_memalign to create page tables and other pages sufficient
 * to have a mapping between the given virtual address and some physical address.
 * If there already is such a page, does nothing.
 */
void page_allocate(size_t va) {

    size_t pagesize = pow(2,POBITS);
    if (ptbr == 0) { // first, allocate main page table
        void* page_table_pointer;
        size_t alignment = pagesize; 
        size_t size = pagesize;
        if (posix_memalign(&page_table_pointer, alignment, size) != 0 ) {
            printf("posix_memalign() failed with errno = %d\n",errno);
        };

        memset(page_table_pointer, 0, size);  // initialize slots in memory to 0; no PTE in the table valid yet
        ptbr = (size_t) page_table_pointer; // set ptbr :)
    }
    size_t curr_base_reg = ptbr;

    // -------------- PAGE ALLOCATION --------------
    setUpVpnSegArray(va);

    for (int i = 0; i < LEVELS; i += 1) {

        size_t curr_vpn_seg = arr_vpn_segs[i]; 
        size_t pte_calculation = curr_base_reg + 8*curr_vpn_seg; 
        size_t* pte_address = (size_t*)pte_calculation; 
        size_t pte_value = *pte_address; // find the value @ the pte address -> the PTE itself

        if (pte_value % 2 != 1) { // if PTE *not* valid, not been allocated yet, allocate one page for data

            void* physical_page_pointer; // points to start of page
            size_t alignment = pagesize;
            size_t size = pagesize;

            if (posix_memalign(&physical_page_pointer, alignment, size) != 0) {
                printf("posix_memalign() failed with errno = %d\n",errno);
            };
            
            memset(physical_page_pointer, 0, size);  

            size_t physical_page_pointer_as_int = (size_t) physical_page_pointer; // cast to an int so we can work with it
            physical_page_pointer_as_int = ((physical_page_pointer_as_int >> POBITS) << POBITS); // shift right & left to clear PO bits
            size_t updated_pte_value = physical_page_pointer_as_int | 1; // set last bit to 1, since page is now allocated & valid
            *pte_address = updated_pte_value; 

            curr_base_reg = (updated_pte_value >> POBITS) << POBITS;

        }
        else {
            curr_base_reg = ((pte_value >> POBITS) << POBITS); // if PTE valid, update curr_base_reg for next iteration

        }
    }
}

/*
Deallocates all memory allocated for all page table
 */
void page_deallocate(void) {

    if (ptbr == 0) { // if trying to deallocate when ptbr is 0 (no page tables)
        return;
    }
    
    size_t pagesize = pow(2,POBITS);
    size_t curr_base_reg = ptbr;

    // -------------- PAGE DE-ALLOCATION --------------
    for (int j = 0; j < pagesize; j += 1) { // for each va in the main page table...
        size_t va = ptbr + j;
        setUpVpnSegArray(va);
        for (int i = 0; i < LEVELS; i += 1) {

            size_t curr_vpn_seg = arr_vpn_segs[i]; 
            size_t pte_calculation = curr_base_reg + 8*curr_vpn_seg; 
            size_t* pte_address = (size_t*)pte_calculation; 
            size_t pte_value = *pte_address; 

            if (pte_value % 2 == 1 && pte_value > 0) { // if PTE valid, has been allocated, need to deallocate

                size_t* pointer_to_page_table = (size_t*) (curr_base_reg); // set last bit back to 0 (since it got set to 1 when allocated)
                free(pointer_to_page_table);

                size_t ppn = pte_value >> POBITS; 
                curr_base_reg = ppn*pagesize; // update curr_base_reg
            }
            else { // if PTE invalid, was not allocated, nothing below it
                return;
            }
        }
    }
}
