#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#define VIRTUAL_MEMORY_SIZE (1 << 24)  // 16 MB
#define PHYSICAL_MEMORY_SIZE (1 << 20) // 1 MB
#define PAGE_SIZE (1 << 12)            // 4 KB

#define PAGE_TABLE_SIZE (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)
#define FRAMES_NUMBER (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

#define FLAG_VALID (1 << 0)                      // c'è una corrispondenza tra pagina virtuale e frame fisico
#define FLAG_UNSWAPPABLE (1 << 1)                // la pagina non può essere swappata
#define FLAG_READ_BIT (1 << 2)                   // la pagina è stata letta 
#define FLAG_WRITE_BIT (1 << 3)                  // la pagina è stata modificata 


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

void MMU_init(MMU *mmu, char *file_name)
{
    FILE *swap_file = fopen(file_name, "w+");
    fseek(swap_file, VIRTUAL_MEMORY_SIZE - 1, SEEK_SET);
    fputc('\0', swap_file);
    fseek(swap_file, 0, SEEK_SET);

    char *physical_memory = calloc(PHYSICAL_MEMORY_SIZE, 1);

    mmu->physical_memory = physical_memory;
    mmu->swap_file = swap_file;
    mmu->page_table = (page_table_entry *)physical_memory;
    mmu->last_page_swapped_out = 0;

    // 2 frame sono riservati per la tabella delle pagine
    mmu->free_frame = 254;

    // Inizializzo la tabella delle pagine
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
    {
        mmu->page_table[i].flags = 0;
        mmu->page_table[i].frame_number = 0;
    }

    mmu->page_table[0].flags = FLAG_UNSWAPPABLE | FLAG_VALID ;
    mmu->page_table[0].frame_number = 0;

    mmu->page_table[1].flags = FLAG_UNSWAPPABLE | FLAG_VALID ;
    mmu->page_table[1].frame_number = 1;
}

void MMU_writeByte(MMU *mmu, int pos, char c)
{

}

char *MMU_readByte(MMU *mmu, int pos)
{
    
}


void prova(){
    MMU mmu;
    MMU_init(&mmu, "prova.bin");
    
    printf("%d\n", mmu.free_frame);
    //fin qui tutto ok

}

int main()
{
    //test
    prova();
    return 0;
}