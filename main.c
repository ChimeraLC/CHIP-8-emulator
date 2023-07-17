//https://stackoverflow.com/questions/64396979/how-do-i-use-sdl2-in-my-programs-correctly

#include <stdio.h>
#include <SDL.h>

/*
 * CPU and memory values
 */
uint16_t opcode;
uint8_t memory[4096];
uint8_t registers[16];
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


int
main(int argc, char **argv)
{
        (void) argc;
        (void) argv;

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
        memset(registers, 0, 16 * sizeof(uint8_t));
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

        // Variable opcode values
        uint16_t addr;
        uint8_t kk, n, x, y;

        addr = opcode & 0x0FFF;         // lowest 12 bits
        kk = opcode & 0x00FF;           // lowest 8 bits
        n = opcode & 0x000F;            // lowest 4 bits
        y = (opcode & 0x00F0) >> 4;     // upper 4 bits of lower byte
        x = (opcode & 0x0F00) >> 8;     // lower 4 bits of upper byte


        // Running different opcodes
        switch (opcode & 0xF000) {     // Switch on first value

                case 0x0000:
                switch (opcode) {
                        case 0x00E0:
                        (void) 0; //Clear screen
                        case 0x00EE:
                        (void) 0; //Return from subroutine
                }

                case 0x1000:
                (void) 0; // Goto addr
                case 0x2000:
                (void) 0; // subroutine addr
                case 0x3000:
                (void) 0; // skip if register x = kk
                case 0x4000:
                (void) 0; // skip if register x != kk
                case 0x5000:
                (void) 0; // skip if register x = register y
                case 0x6000:
                (void) 0; // sets register x to kk
                case 0x7000:
                (void) 0; // increments register x by kk
                case 0x8000:
                switch (opcode & 0x000F) {
                        case 0x0000:
                        (void) 0; // sets register x to register y
                        case 0x0001:
                        (void) 0; // ors register x with register y
                        case 0x0002:
                        (void) 0; // ands register x with register y
                        case 0x0003:
                        (void) 0; // xors register x with register y
                        case 0x0004:
                        (void) 0; // adds register y to register x, updates reg F
                        case 0x0005:
                        (void) 0; // subtracts reg y from reg x, updates reg F
                        case 0x0006:
                        (void) 0; // sets reg x >>= 1, stores lost bit in reg F
                        case 0x0007:
                        (void) 0; // sets reg x to reg y - reg x, updates reg F
                        case 0x000E:
                        (void) 0; // sets reg x <<= 1, stores lost bit in reg F
                }
                case 0x9000:
                (void) 0; // skips if reg x != reg y
                case 0xA000:
                (void) 0; // sets ind to addr
                case 0xB000:
                (void) 0; // jumps to addr + reg 0
                case 0xC000:
                (void) 0; // sets reg X to a random value || KK
                case 0xD000:
                (void) 0; // displays a rectangle
                case 0xE000:
                switch (opcode & 0x00FF) {
                        case 0x009E:
                        (void) 0; // skips if reg X key is pressed
                        case 0x00A1:
                        (void) 0; // skips if reg X key is not pressed
                }
                case 0xF000:
                switch (opcode & 0x00FF) {
                        case 0x0007:
                        (void) 0; // sets reg X to delay timer
                        case 0x000A:
                        (void) 0; // blocking wait to store key in reg X
                        case 0x0015:
                        (void) 0; // sets delay timer to reg X
                        case 0x0018:
                        (void) 0; // sets sound timer to reg X
                        case 0x001E:
                        (void) 0; // adds reg X to ind
                        case 0x0029:
                        (void) 0; // sprite stuff?
                        case 0x0033:
                        (void) 0; // stores binary digits of reg X to ind
                        case 0x0055:
                        (void) 0; // stores regs in memory starting at ind
                        case 0x0065:
                        (void) 0; // fills regs from memory starting from ind
                }
        }
}