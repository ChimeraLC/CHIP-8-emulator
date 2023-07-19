//https://stackoverflow.com/questions/64396979/how-do-i-use-sdl2-in-my-programs-correctly

#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <unistd.h>
/*
 * CPU and memory values
 */
uint16_t opcode;
uint8_t memory[4096];
uint8_t reg[16];
uint16_t ind;
uint16_t pc;

uint8_t graphics[2048];

uint8_t d_timer;
uint8_t s_timer;

uint16_t stack[16];
uint16_t sp;

uint8_t keypad[16];

uint8_t fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


// Functions
void init();
void cycle();

int
main(int argc, char **argv)
{
        // Checking input arguments
	if (argc < 2) {
		printf("Missing file name");
		return -1;
	}

        // Initialize memory
        init();

        // Loading game file
        char *filename = argv[1];

        FILE *rom = fopen(filename, "r");

        if (rom == NULL) {
                printf("Error");
                return(-1);
        }

        // Copying into memory
        fseek(rom, 0, SEEK_END);
        long fsize = ftell(rom);
        printf("size: %ld\n", fsize);
        fseek(rom, 0, SEEK_SET); 

        fread(memory + 0x200, fsize, sizeof(uint16_t), rom);
        fclose(rom);

        // Random seed (should this be included?)
        srand(0);
        // Emulator loop
        while (true) {
                // Syncing framerate
                sleep(1);
                cycle();
        }

        return (1);
}

void 
init()
{
        // Set initial values
        pc = 0x200;
        opcode = 0;
        ind = 0;
        sp = 0; 
        d_timer = 0;
        s_timer = 0;

        // Zero memory
        memset(memory, 0, 4096 * sizeof(uint8_t));
        memset(reg, 0, 16 * sizeof(uint8_t));
        memset(stack, 0, 16 * sizeof(uint16_t));
        memset(graphics, 0, 2048 * sizeof(uint8_t));

        // Copy over fontset
        memcpy(memory, fontset, 80 * sizeof(uint8_t));
}
void
cycle()
{

        // Get opcode at current memory position
        opcode = memory[pc] << 8 | memory[pc + 1];
        pc += 2;

        // Variable opcode values
        uint16_t addr;
        uint8_t kk, n, x, y;

        addr = opcode & 0x0FFF;         // lowest 12 bits
        kk = opcode & 0x00FF;           // lowest 8 bits
        n = opcode & 0x000F;            // lowest 4 bits
        y = (opcode & 0x00F0) >> 4;     // upper 4 bits of lower byte
        x = (opcode & 0x0F00) >> 8;     // lower 4 bits of upper byte

        printf("%x : %x\n", opcode, pc);
        (void)n;

        // Running different opcodes
        switch (opcode & 0xF000) {     // Switch on first value

                case 0x0000:
                switch (opcode) {
                        case 0x00E0:
                        memset(graphics, 0, 2048 * sizeof(uint8_t)); //Clear screen
                        break;
                        case 0x00EE:
                        sp--; 
                        pc = stack[sp]; //Return from subroutine
                        break;
                }
                        break;

                case 0x1000:
                pc = addr; // Goto addr
                break;
                case 0x2000:
                stack[sp] = pc;
                sp++;
                pc = addr; // subroutine addr
                break;
                case 0x3000:
                if (reg[x] == kk) {
                        pc +=2;
                } // skip if register x = kk
                break;
                case 0x4000:
                if (reg[x] != kk) {
                        pc += 2;
                } // skip if register x != kk
                break;
                case 0x5000:
                if (reg[x] == reg[y]) {
                        pc += 2;
                } // skip if register x = register y
                break;
                case 0x6000:
                reg[x] = kk; // sets register x to kk
                break;
                case 0x7000:
                reg[x] += kk; // increments register x by kk
                break;
                case 0x8000:
                switch (opcode & 0x000F) {
                        case 0x0000:
                        reg[x] = reg[y]; // sets register x to register y
                        break;
                        case 0x0001:
                        reg[x] |= reg[y]; // ors register x with register y
                        break;
                        case 0x0002:
                        reg[x] &= reg[y]; // ands register x with register y
                        break;
                        case 0x0003:
                        reg[x] ^= reg[y]; // xors register x with register y
                        break;
                        case 0x0004:
                        (void) 0; // adds register y to register x, updates reg F
                        break;
                        case 0x0005:
                        reg[15] = 0;
                        if (reg[x] > reg[y]) {
                                reg[15] = 1;
                        }
                        reg[x] -= reg[y]; // subtracts reg y from reg x, updates reg F
                        break;
                        case 0x0006:
                        reg[15] = reg[x] & 0x1;
                        reg[x] >>= 1; // sets reg x >>= 1, stores lost bit in reg F
                        break;
                        case 0x0007:
                        reg[15] = 0;
                        if (reg[x] < reg[y]) {
                                reg[15] = 1;
                        }
                        reg[x] = reg[y] - reg[x]; // sets reg x to reg y - reg x, updates reg F
                        break;
                        case 0x000E:
                        reg[15] = reg[x] >> 7;
                        reg[x] <<= 1; // sets reg x <<= 1, stores lost bit in reg F
                        break;
                }
                break;
                case 0x9000:
                if (reg[x] != reg[y]) {
                        pc += 2;
                } // skips if reg x != reg y
                break;
                case 0xA000:
                ind = addr; // sets ind to addr
                break;
                case 0xB000:
                pc = addr + reg[0]; // jumps to addr + reg 0
                break;
                case 0xC000:
                reg[x] = (rand() % 256) & kk; // sets reg X to a random value || KK
                break;
                case 0xD000:
                (void) 0; // displays a rectangle
                break;
                case 0xE000:
                switch (opcode & 0x00FF) {
                        case 0x009E:
                        (void) 0; // skips if reg X key is pressed
                        break;
                        case 0x00A1:
                        (void) 0; // skips if reg X key is not pressed
                        break;
                }
                break;
                case 0xF000:
                switch (opcode & 0x00FF) {
                        case 0x0007:
                        reg[x] = d_timer; // sets reg X to delay timer
                        break;
                        case 0x000A:
                        (void) 0; // blocking wait to store key in reg X
                        break;
                        case 0x0015:
                        d_timer = reg[x]; // sets delay timer to reg X
                        break;
                        case 0x0018:
                        s_timer = reg[x]; // sets sound timer to reg X
                        break;
                        case 0x001E:
                        ind += reg[x]; // adds reg X to ind
                        break;
                        case 0x0029:
                        (void) 0; // sprite stuff?
                        break;
                        case 0x0033:
                        (void) 0; // stores binary digits of reg X to ind
                        break;
                        case 0x0055:
                        for (int i = 0; i <= x; i++) {
                                memory[ind + i] = reg[i];
                        } // stores regs in memory starting at ind
                        break;
                        case 0x0065:
                        for (int i = 0; i <= x; i++) {
                                reg[i] = memory[ind + i];
                        }; // fills regs from memory starting from ind
                        break;
                }
                break;
        }
}