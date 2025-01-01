# Virtual Memory Simulation of Multi-level Page Table Lookups and Allocation

### **How to customize config.h?<br>**
This program is simulating translation and allocation in a multi-level page table.<br>
Use config.h to define/change the number of LEVELS and PAGE OFFSET BITS.<br>

### **Known bugs or limitations<br>**
N/A

### **Code Samples<br>**
Call functions by passing in a virtual address* (can be done in hex format as well), such as below:<code>
translate(0x1000);
page_allocate(0x456789abcdef);
page_deallocate();
</code>
*Note that page_deallocate takes no arguments

### **Big-O analysis (time, space, or both).<br>**
_Time complexity_: O(n) where n = LEVELS<br>
    - Including deallocate function, the time complexity is: O(n * pagesize) where n = LEVELS<br>
Space complexity: O(n*pagesize) where n = LEVELS

### **Memory Deallocation**<br>
I propose an interface for page_deallocate() that follows a similar structure to both translate and page_allocate(). I will instantiate an array with the values of the vpn segments for the current virtual address, iterate through, find their respective page table entries, and free the memory associated with each level. In order to avoid hanging page tables, which would result in heap use after free errors, or unpredictable memory leaks, I plan to implement page_deallocate such that it deallocates any and all memory that was allocated upon previous calls to page_allocate. 

I believe this implementation is possible given my structure for page_allocate() and translate(), so I have added it to both pagetable_sim.c and mlpt.h

### **Suggestions for future expansion<br>**
A more comprehensive page_deallocate() that is memory-efficient (as opposed to freeing the final page of a virtual address) and able to deallocate given specific addresses. This would have to be be more complicated than my current proposed interface in order to avoid edge cases of freeing entire pages that might be in use by other addresses.
