# SO_Project
Esame Sistemi Operativi, sessione estiva 2022/2023

## To do:
Virtual Memory System Simulator
   Using an MMU class, with paging allocate a large virtual memory space (16 MB), and use
   a "swap file" having that size.
   Use a 1 MB buffer as phyisical memory.
   
   The page table sits at the beginning of the memory (simplification), and
   should support the following flags:
   - valid
   - unswappable
   - read_bit (set by the mmu each time that page is read)
   - write_bit (set by the mmu each time a page is written)

   define the functions:
   MMU_writeByte(MMU* mmu, int pos, char c), 
   char* MMU_readByte(MMU* mmu, int pos );

   that wraps the memory access, doing the right side effect on the mmu/page table

   If an attempt to access at an invalid page, the function  MMU_exception(MMU* mmu, int pos)
   is called and it has to handle the page fault doing the appropriate operation on disk.
   As a page replacement policy implement the second chance algorithm.

   Stress the memory with different access patterns, and assume all the virtual memory is allocated