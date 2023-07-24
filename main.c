#include <stdio.h>
#include <SDL.h>
#include <SDL_audio.h>
#include <math.h>
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

// Verbosity
bool verbose = false;

// SDL elements
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

// SDL audio elements
const int AMPLITUDE = 28000;
const int SAMPLE_RATE = 44100;
SDL_AudioSpec want;
int sample_nr = 0;

// Functions
void init();
bool init_SDL();
bool init_SDL_audio();
bool cycle();
void draw_graphics();
void play_sound();
void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes);

int
main(int argc, char **argv)
{
        // Checking input arguments
	if (argc < 2) {
		printf("Missing file name");
		return -1;
	}

        if (argc >= 3) {
                verbose = argv[2];
        }

        // Initialize memory
        init();

        // Initialize sdl
        init_SDL();

        // Initialize sdl audio
        if (!init_SDL_audio())  printf("Audio device failed to initialize.\n");

        // Loading game file
        char *filename = argv[1];

        if (verbose) {
                printf("Opening file: %s", filename);
        }

        FILE *rom = fopen(filename, "rb");

        if (rom == NULL) {
                printf("Error");
                return(-1);
        }

        // Copying into memory
        fseek(rom, 0, SEEK_END);
        long fsize = ftell(rom);
        fseek(rom, 0, SEEK_SET); 

        fread(memory + 0x200, fsize, 1, rom);
        fclose(rom);

        SDL_Event event;
        bool active = true;
        bool draw = false;
        //TODO: add draw_update;

        // Emulator loop
        while (active == true) {
                // Syncing framerate
                SDL_Delay(5);
                // Reading keyboard events
                while(SDL_PollEvent( &event ) ){
                        switch( event.type ){
                                case SDL_KEYDOWN:
                                        switch( event.key.keysym.sym ){
                                                case SDLK_ESCAPE:
                                                active = false;
                                                break; // Exiting emulator
                                                case SDLK_1:
                                                keypad[0] = 1;
                                                break;
                                                case SDLK_2:
                                                keypad[1] = 1;
                                                break;
                                                case SDLK_3:
                                                keypad[2] = 1;
                                                break;
                                                case SDLK_4:
                                                keypad[3] = 1;
                                                break;
                                                case SDLK_q:
                                                keypad[4] = 1;
                                                break;
                                                case SDLK_w:
                                                keypad[5] = 1;
                                                break;
                                                case SDLK_e:
                                                keypad[6] = 1;
                                                break;
                                                case SDLK_r:
                                                keypad[7] = 1;
                                                break;
                                                case SDLK_a:
                                                keypad[8] = 1;
                                                break;
                                                case SDLK_s:
                                                keypad[9] = 1;
                                                break;
                                                case SDLK_d:
                                                keypad[10] = 1;
                                                break;
                                                case SDLK_f:
                                                keypad[11] = 1;
                                                break;
                                                case SDLK_z:
                                                keypad[12] = 1;
                                                break;
                                                case SDLK_x:
                                                keypad[13] = 1;
                                                break;
                                                case SDLK_c:
                                                keypad[14] = 1;
                                                break;
                                                case SDLK_v:
                                                keypad[15] = 1;
                                                break;
                                        }
                                break;

                                case SDL_KEYUP:
                                        switch( event.key.keysym.sym ){
                                                case SDLK_1:
                                                keypad[0] = 0;
                                                break;
                                                case SDLK_2:
                                                keypad[1] = 0;
                                                break;
                                                case SDLK_3:
                                                keypad[2] = 0;
                                                break;
                                                case SDLK_4:
                                                keypad[3] = 0;
                                                break;
                                                case SDLK_q:
                                                keypad[4] = 0;
                                                break;
                                                case SDLK_w:
                                                keypad[5] = 0;
                                                break;
                                                case SDLK_e:
                                                keypad[6] = 0;
                                                break;
                                                case SDLK_r:
                                                keypad[7] = 0;
                                                break;
                                                case SDLK_a:
                                                keypad[8] = 0;
                                                break;
                                                case SDLK_s:
                                                keypad[9] = 0;
                                                break;
                                                case SDLK_d:
                                                keypad[10] = 0;
                                                break;
                                                case SDLK_f:
                                                keypad[11] = 0;
                                                break;
                                                case SDLK_z:
                                                keypad[12] = 0;
                                                break;
                                                case SDLK_x:
                                                keypad[13] = 0;
                                                break;
                                                case SDLK_c:
                                                keypad[14] = 0;
                                                break;
                                                case SDLK_v:
                                                keypad[15] = 0;
                                                break;
                                        }
                                break;
                        }
                }

                // Updating timers
                if (d_timer > 0) d_timer--;
                if (s_timer > 0) {
                        if (s_timer == 1) {
                                play_sound();
                        }
                        s_timer--;
                }
                // Running emulator cycle
                draw = cycle();

                // Displaying on screen
                if (draw) {
                        draw_graphics();
                }
        }

        // Clean up SDL elements
        SDL_DestroyWindow(window); // Destroy window
        SDL_DestroyRenderer(renderer); // Destroy renderer
        SDL_DestroyTexture(texture); // Destroy texture
        SDL_Quit();             // Close SDL
        return (1);
}
/*
 * Initializes and zeros chip-8 cpu
 */
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
/*
 * Initializes the visual SDL elements, returns false if there is an error.
 */
bool
init_SDL()
{
        // Initialize all SDL systems
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                // Send message if fails
                printf("error initializing SDL: %s\n", SDL_GetError());
                return false;
        }
        // Create parts of window (64 by 32 pixels)
        window = SDL_CreateWindow("Chip-8", 
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        1280, 640, 0);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_RenderSetLogicalSize(renderer, 64, 32);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, 
                                        SDL_TEXTUREACCESS_STREAMING, 64, 32);

        return true; // Return true when there is no problem
}

/*
 * Emulates the reading of a single opcode
 */
bool
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

        // Opcode debug message
        if (verbose) {
                printf("Running opcode %x at %x\n", opcode, pc);
        }

        // Running different opcodes
        switch (opcode & 0xF000) {     // Switch on first value

                case 0x0000:
                switch (opcode) {
                        case 0x00E0:
                        memset(graphics, 0, 2048 * sizeof(uint8_t)); //Clear screen
                        return true;
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
                        reg[15] = 0;
                        if ((int) reg[x] + (int) reg[y] > 255) {
                                reg[15] = 1;
                        }
                        reg[x] += reg[y]; // adds register y to register x, updates reg F
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
                reg[15] = 0;
                uint8_t row;
                uint8_t pixel;
                for (int i = 0; i < n; i++){
                        row = memory[ind + i];
                        for (int j = 0; j < 8; j++) {
                                pixel = (row >> (7 - j)) & 0x1;
                                if (pixel)
                                {
                                        if (graphics[reg[x] + j + (reg[y] + i) * 64]) {
                                                reg[15] = 1;
                                        }
                                        graphics[reg[x] + j + (reg[y] + i) * 64] ^= pixel;
                                }
                        }
                } // ors an n by 8 rectangle from memory onto the location reg x, reg y
                return true;
                break;
                case 0xE000:
                switch (opcode & 0x00FF) {
                        case 0x009E:
                        if (keypad[reg[x]]){
                                pc += 2;
                        } // skips if reg X key is pressed
                        break;
                        case 0x00A1:
                        if (!keypad[reg[x]]) {
                                pc += 2;
                        }; // skips if reg X key is not pressed
                        break;
                }
                break;
                case 0xF000:
                switch (opcode & 0x00FF) {
                        case 0x0007:
                        reg[x] = d_timer; // sets reg X to delay timer
                        break;
                        case 0x000A:
                        bool key = false;
                        for (int i = 0; i < 16; i ++) {
                                if (keypad[i]) {
                                        key = true;
                                        reg[x] = i;
                                        break;
                                }
                        }
                        if (!key) {
                                pc -= 2;
                        } // blocking wait to store key in reg X
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
                        ind = 5 * reg[x]; // sets ind to sprite location of reg x
                        break;
                        case 0x0033:
                        memory[ind] = (reg[x] - reg[x] % 100) / 100;
                        memory[ind + 1] = (reg[x] % 100 - reg[x] % 10) / 10;
                        memory[ind + 2] = reg[x] % 10; // stores binary digits of reg X to ind
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
        return false;
}

/*
 * Draws the pixels represented by graphics onto the SDL screen
 */
void draw_graphics() 
{

        // Creating and zeroing array of pixels
        uint8_t pixels[8196];
        memset(pixels, 0, 8196);
        
        // Filling pixels corresponding to graphics array
        for (int i = 0; i < 2048; i++) {
                if (graphics[i]) {
                        // Casting to uint32_t to set RGBA simultaneously
                        uint32_t *int32s = (uint32_t *) pixels;
                        int32s[i] = 4294967295;
                }
        }

        // Applying texture to screen
        int texture_pitch = 0;
        void* texture_pixels = NULL;
        if (SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch) != 0) {
                SDL_Log("Unable to lock texture: %s", SDL_GetError());
        }
        else {
                memcpy(texture_pixels, pixels, texture_pitch * 32);
        }
        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
}


/*
 * Initializes the audio SDL elements, returns false if there is an error.
 */
bool
init_SDL_audio()
{

        SDL_AudioSpec want;
        want.freq = SAMPLE_RATE; // number of samples per second
        want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
        want.channels = 1; // only one channel
        want.samples = 2048; // buffer-size
        want.callback = audio_callback; // function SDL calls periodically to refill the buffer
        want.userdata = &sample_nr; // counter, keeping track of current sample number

        // Open audio device
        SDL_AudioSpec have;
        if(SDL_OpenAudio(&want, &have) != 0) {
                printf("Failed to open audio: %s\n", SDL_GetError());
                return false;
        }
        return true;
}
/*
 * audio_callback functino used with SDL_AudioSpec
 */
void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
{
        uint16_t *buffer = (uint16_t*)raw_buffer;
        int length = bytes / 2; 
        int sample_nr = *(int*)user_data;

        // Sine wave 
        for(int i = 0; i < length; i++, sample_nr++)
                {
                double time = (double)sample_nr / (double)SAMPLE_RATE;
                buffer[i] = (uint16_t)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time)); // render 441 HZ sine wave
        }

        // Audio cutoff
        if (sample_nr > 100) {
                sample_nr = 0;
                SDL_PauseAudio(1);
        }
}

/*
 * Plays a sound
 */
void
play_sound() {
        // Unpause audio device
        SDL_PauseAudio(0);
        if (verbose) {
                printf("Playing sound\n");
        }
}