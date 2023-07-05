#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

typedef struct
{
    uint8_t flags;                               
    uint8_t frame_number;                        // i frame sono numerati da 0 a 255
} page_table_entry;

typedef struct
{
    char *physical_memory;                       // assumiamo puntino entrambi alla stessa locazione di memoria
    page_table_entry *page_table;
    FILE *swap_file;                             // la memoria virtuale è un file
    int last_page_swapped_out;                   // frame da cui iniziare a cercare la pagina da swappare
    int free_frame;                              // n frame liberi
} MMU;


void MMU_writeByte(MMU *mmu, int pos, char c)
{

}

char *MMU_readByte(MMU *mmu, int pos)
{
    
}


int main()
{
    //test
    return 0;
}