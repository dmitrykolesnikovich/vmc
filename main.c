/* We're going to be creating a primitive virtual machine / emulator to perform basic arithmetic and print the result to the console
 * we're going to have 10 registers R00-R09 or 0x00-0x09.  Lets begin shall we?
 */

/* need some includes here */
#include <stdio.h>    /* printf      */
#include <string.h>   /* memset      */
#include <stdlib.h>   /* malloc/free */
#include <inttypes.h> /* uintptr_t   */

/* Lets define our arithmetic instructions here */
#define IADD 0x00
#define ISUB 0x01
#define IMUL 0x02
#define IDIV 0x03

/* our print instruction */
#define PRNT 0x04

/* implement a basic memory manager */
typedef struct meminfo {
    unsigned int   info_line;
    const    char *info_file;
    unsigned int   info_byte; /* Amount of data allocated */
} meminfo_t;

void *_mem_a(size_t byte, unsigned int line, const char *file) {
    meminfo_t *data = malloc(sizeof(struct meminfo) + byte);
    if (!data) return NULL;
    data->info_line = line;
    data->info_file = file;
    data->info_byte = byte;
    printf("[MEM] A %08d (bytes) at %s:%d\n", byte, file, line);
    return (void*)((uintptr_t)data + sizeof(struct meminfo));
}

void _mem_d(void *ptrn, unsigned int line, const char *file) {
    if (!ptrn) return;
    void      *data = (void*)((uintptr_t)ptrn - sizeof(struct meminfo));
    meminfo_t *info = (meminfo_t*)data;
    printf("[MEM] D %08d (bytes) at %s:%u\n", info->info_byte, file, line);
    free(data);
}

#define mem_a(B) _mem_a((B), __LINE__, __FILE__)
#define mem_d(P) _mem_d((P), __LINE__, __FILE__)

/* Now lets implement the virtual machine */
typedef struct cpu {
    unsigned int   registers[0x0A]; /* 0-11                */
    unsigned int   esp;             /* Instruction Pointer */
    unsigned int   size;            /* code size        */
    unsigned char *code;            /* Pointer to code     */
} cpu_t;

cpu_t *cpu_new(unsigned char *code, unsigned int size){
    if (!code || !size) return NULL;
    cpu_t *cpun = mem_a(sizeof(struct cpu));
    if (!cpun) return NULL;
    cpun->esp  = 0;
    cpun->size = size; /* oops :3 */
    cpun->code = code; /* oops :3 */
    memset(cpun->registers, 0, sizeof(cpun->registers)/sizeof(*cpun->registers)); /* clear registers */
    return cpun;
}

void cpu_del(cpu_t *cpup) {
    if (!cpup) return;
    printf("[CPU] Registers:\n");
    printf("      R00: 0x%08X | REG01: 0x%08X | REG02: 0x%08X | REG03: 0x%08X | REG04: 0x%08X | REG05: 0x%08X\n", cpup->registers[0], cpup->registers[1], cpup->registers[2], cpup->registers[3], cpup->registers[4] , cpup->registers[5]);
    printf("      R06: 0x%08X | REG07: 0x%08X | REG08: 0x%08X | REG09: 0x%08X | REG10: 0x%08X | ESP:   0x%08X\n", cpup->registers[6], cpup->registers[7], cpup->registers[8], cpup->registers[9], cpup->registers[10], cpup->esp);
    mem_d(cpup);
}

/* Main virtual machine */
void cpu_run(cpu_t *cpup) {
    static unsigned int iterates = 0;
    if (!cpup) return;
    cpup->esp = 0; /* always start from 0 */
    while (cpup->esp < cpup->size) {
        switch(cpup->code[cpup->esp]) {
        #define MATHOP(INST, OP) \
            case INST: {         \
                cpup->esp ++;    \
                cpup->registers[cpup->code[cpup->esp]] = cpup->code[cpup->esp + 1] OP cpup->code[cpup->esp + 2]; \
                cpup->esp ++;    \
                cpup->esp ++;    \
                break;           \
            }
            MATHOP(IADD, +)
            MATHOP(ISUB, -)
            MATHOP(IMUL, *)
            MATHOP(IDIV, /)
            #undef MATHOP

            /* now our print instruction */
            case PRNT: {
                cpup->esp ++;
                printf("[stdout] register R%02d = %d\n", cpup->code[cpup->esp], cpup->registers[cpup->code[cpup->esp]]);
                break;
            }
            default: printf("executed: 0x%08X\n", cpup->code[cpup->esp]); break;
        }
        cpup->esp ++; /* Increment instruction pointer */
    }
    iterates ++; /* Increment iterated */
    printf("[CPU] Iterations %u\n", iterates);
}

/* lets put this code in binary file */

int main(int argc, char **argv) {
    if (!*argv[1]) return 0;
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) printf("[SVM] Failed to load program %s\n", argv[1]);
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);
    rewind(fp);

    unsigned char *code = mem_a(size);
    if (!code) { fclose(fp); return 0; }
    fread(code, 1, size, fp);
    fclose(fp);

    cpu_t *cpu = cpu_new(code, size);
    if (!cpu) return 0;
    cpu_run(cpu);
    cpu_del(cpu);
    mem_d(code); /* free code */

    return 1; /* return success */
}
