#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#define VIRTUAL_MEMORY_SIZE (1 << 24)  // 16 MB
#define PHYSICAL_MEMORY_SIZE (1 << 20) // 1 MB
#define PAGE_SIZE (1 << 12)            // 4 KB

#define PAGE_TABLE_SIZE (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)
#define FRAMES_NUMBER (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

#define FLAG_VALID (1 << 0)                      // c'è una corrispondenza tra pagina virtuale e frame fisico
#define FLAG_UNSWAPPABLE (1 << 1)                // la pagina non può essere swappata
#define FLAG_READ_BIT (1 << 2)                   // la pagina è stata letta 
#define FLAG_WRITE_BIT (1 << 3)                  // la pagina è stata modificata 
//aggiungo alcune flag utili
#define FLAG_SECOND_CHANCE_BIT (1 << 4)          // riconoscere la pagina non acceduta da più tempo 
#define FLAG_RESERVED (1 << 5)                   // pagina non modificabile (le prime due sono riservate alla taballea delle pagine)
#define FLAG_CREATED (1 << 6)                    // pagina creata



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

bool is_print_enabled = false;
#define my_printf(...)    \
    if (is_print_enabled) \
        printf(__VA_ARGS__);

FILE* open_swap_file(char *file_name){
        
    FILE *swap_file = fopen(file_name, "w+");
    fseek(swap_file, VIRTUAL_MEMORY_SIZE - 1, SEEK_SET);
    fputc('\0', swap_file);
    fseek(swap_file, 0, SEEK_SET);
}

void MMU_init(MMU *mmu, char *file_name)
{
    FILE *swap_file = open_swap_file(file_name);

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

    mmu->page_table[0].flags = FLAG_UNSWAPPABLE | FLAG_VALID | FLAG_RESERVED | FLAG_CREATED;
    mmu->page_table[0].frame_number = 0;

    mmu->page_table[1].flags = FLAG_UNSWAPPABLE | FLAG_VALID | FLAG_RESERVED | FLAG_CREATED;
    mmu->page_table[1].frame_number = 1;
}

void MMU_print_page_table_range(MMU *mmu, size_t min, size_t max)
{

    if (min > max)
    {
        fprintf(stderr, "min > max\n");
        exit(1);
    }

    if (max > PAGE_TABLE_SIZE)
    {
        fprintf(stderr, "MMU_print_page_table_range max should be less or equal to PAGE_TABLE_SIZE (%d)\n", PAGE_TABLE_SIZE);
        exit(1);
    }

    printf("Page table:\n");
    for (int i = min; i < max; i++)
    {
        printf("%.4d) %c%c%c%c%c%c%c >\t%d\n", i,
               mmu->page_table[i].flags & FLAG_VALID ? 'V' : '-',
               mmu->page_table[i].flags & FLAG_UNSWAPPABLE ? 'U' : '-',
               mmu->page_table[i].flags & FLAG_READ_BIT ? 'R' : '-',
               mmu->page_table[i].flags & FLAG_WRITE_BIT ? 'W' : '-',
               mmu->page_table[i].flags & FLAG_SECOND_CHANCE_BIT ? 'S' : '-',
               mmu->page_table[i].flags & FLAG_RESERVED ? 'X' : '-',
               mmu->page_table[i].flags & FLAG_CREATED ? 'C' : '-',
               mmu->page_table[i].frame_number);
               
    }
}

void MMU_print_page_table(MMU *mmu)
{
    MMU_print_page_table_range(mmu, 0, PAGE_TABLE_SIZE);
}

int find_page_to_swap_out(MMU *mmu){
    int page_to_swap_out = mmu->last_page_swapped_out;

    my_printf("Finding page to swap out, starting from %d\n", page_to_swap_out);

     
    while(1){

        if (mmu->page_table[page_to_swap_out].flags & FLAG_UNSWAPPABLE)
        {
            my_printf("\tSkipping unswappable page %d\n", page_to_swap_out);
            // pass
        }
        else if (mmu->page_table[page_to_swap_out].flags & FLAG_SECOND_CHANCE_BIT)
        {
            my_printf("\tSkipping page %d with second chance bit\n", page_to_swap_out);
            mmu->page_table[page_to_swap_out].flags &= ~FLAG_SECOND_CHANCE_BIT;
        }
        else if (mmu->page_table[page_to_swap_out].flags & FLAG_VALID)
        {
            my_printf("\tFound page %d to swap out\n", page_to_swap_out);
            // Ho trovato la pagina da swappare
            break;
        }

        page_to_swap_out = (page_to_swap_out + 1) % PAGE_TABLE_SIZE;
    }

    mmu->last_page_swapped_out = (page_to_swap_out + 1) % PAGE_TABLE_SIZE;
    return page_to_swap_out;
    
}

void MMU_exception(MMU *mmu, int pos){
    my_printf("Page fault at position %d\n", pos);

    int page_to_swap_in = pos / PAGE_SIZE;

    //controllo se lo swap è non necessario
    if(mmu->free_frame != 0){
        uint8_t busy_frames = FRAMES_NUMBER - mmu->free_frame;
        uint8_t frame_to_allocate = busy_frames;  
        mmu->page_table[page_to_swap_in].frame_number = frame_to_allocate;
        mmu->page_table[page_to_swap_in].flags = FLAG_VALID | FLAG_CREATED;
        mmu->free_frame--;
        my_printf("\tAllocated new frame %d to page %d\n", frame_to_allocate, page_to_swap_in);
    }
    else{
        
        int page_to_swap_out = find_page_to_swap_out(mmu);
        int frame_to_swap_out = mmu->page_table[page_to_swap_out].frame_number;

        // Scrivo la pagina su swap file se è stata modificata in scrittura
        if (mmu->page_table[page_to_swap_out].flags & FLAG_WRITE_BIT){
            my_printf("\tWriting page %d to swap file\n", page_to_swap_out);
            fseek(mmu->swap_file, page_to_swap_out * PAGE_SIZE, SEEK_SET);
            fwrite(&mmu->physical_memory[frame_to_swap_out * PAGE_SIZE], PAGE_SIZE, 1, mmu->swap_file);
            fflush(mmu->swap_file);
        }

        mmu->page_table[page_to_swap_out].flags &= ~(FLAG_VALID | FLAG_WRITE_BIT | FLAG_READ_BIT);


        if (mmu->page_table[page_to_swap_in].flags & FLAG_CREATED)
        {
            my_printf("\tReading page %d from swap file to frame %d\n", page_to_swap_in, frame_to_swap_out);
            fseek(mmu->swap_file, page_to_swap_in * PAGE_SIZE, SEEK_SET);
            fread(&mmu->physical_memory[frame_to_swap_out * PAGE_SIZE], PAGE_SIZE, 1, mmu->swap_file);
        }

        mmu->page_table[page_to_swap_in].frame_number = frame_to_swap_out;
        mmu->page_table[page_to_swap_in].flags |= FLAG_VALID | FLAG_CREATED;
        my_printf("\tSwapped out page %d from frame %d to allocate %d\n", page_to_swap_out, frame_to_swap_out, page_to_swap_in);

    }


}

void MMU_writeByte(MMU *mmu, int pos, char c)
{
    if (pos >= VIRTUAL_MEMORY_SIZE)
    {
        fprintf(stderr, "Cannot write to position %d, out of bounds\n", pos);
        exit(1);
    }

    int page_number = pos / PAGE_SIZE;
    my_printf("Writing %c at position %d on page %d\n", c, pos, page_number);

    if (mmu->page_table[page_number].flags & FLAG_RESERVED)
    {
        fprintf(stderr, "Cannot write to reserved page\n");
        exit(1);
    }

    if (!(mmu->page_table[page_number].flags & FLAG_VALID))
    {
       MMU_exception(mmu, pos);
    }

    int frame_number = mmu->page_table[page_number].frame_number;

    mmu->page_table[page_number].flags |= FLAG_WRITE_BIT;
    mmu->page_table[page_number].flags |= FLAG_SECOND_CHANCE_BIT;

    mmu->physical_memory[frame_number * PAGE_SIZE + pos % PAGE_SIZE] = c;

}

char *MMU_readByte(MMU *mmu, int pos)
{
    if (pos >= VIRTUAL_MEMORY_SIZE)
    {
        fprintf(stderr, "Cannot write to position %d, out of bounds\n", pos);
        exit(1);
    }

    int page_number = pos / PAGE_SIZE;

    my_printf("Reading at position %d on page %d\n", pos, page_number);

    if (mmu->page_table[page_number].flags & FLAG_RESERVED)
    {
        fprintf(stderr, "Cannot read to reserved page\n");
        exit(1);
    }

    if (!(mmu->page_table[page_number].flags & FLAG_CREATED))
    {
        fprintf(stderr, "Page fault\n");
        exit(1);
    }

    if (!(mmu->page_table[page_number].flags & FLAG_VALID))
    {
        MMU_exception(mmu, pos);
    }

    int frame_number = mmu->page_table[page_number].frame_number;

    mmu->page_table[page_number].flags |= FLAG_READ_BIT;
    mmu->page_table[page_number].flags |= FLAG_SECOND_CHANCE_BIT;

    return &mmu->physical_memory[frame_number * PAGE_SIZE + pos % PAGE_SIZE];

}

void MMU_close(MMU *mmu)
{
    fclose(mmu->swap_file);
    free(mmu->physical_memory);
}



void prova(){
    MMU mmu;
    MMU_init(&mmu, "prova.bin");

    is_print_enabled = false;

    for (size_t i = PAGE_SIZE * 2; i < VIRTUAL_MEMORY_SIZE; i++)
    {
        MMU_writeByte(&mmu, i, 'a');
    }

    int page = 2;
    while (true)
    {
        char b = getchar();
        if (b == 'q')

            break;
        if (b == 't')
            MMU_print_page_table(&mmu);
        else
        {
            MMU_readByte(&mmu, page * PAGE_SIZE);

            MMU_print_page_table_range(&mmu, page - 1, page + 2);
            page++;
        }
    }

}

int main()
{
    //test
    prova();
    return 0;
}