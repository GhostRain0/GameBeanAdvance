
/* 
    so, the general idea behind the GBA is this:
    at least now, while im testing the THUMB. we're gonna skip all the ARM instructions
    and just set the PC straight to the beginnings of THUMB. then, we're gonna see what
    we can do from there by editing test-jumptable.cpp
*/

#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <cstring>

#include "gba.h"
#include "memory.h"
#include "util.h"
#include "jumptable/jumptable.h"

extern Memory memory;

void run(std::string rom_name) {
    setup_memory();
    
    get_rom_as_bytes(rom_name, memory.rom_1, SIZE_ROM_1);

    // extract the game name
    char game_name[GAME_TITLE_SIZE]; 
    for (int i = 0; i < GAME_TITLE_SIZE; i++) {
        game_name[i] = memory.rom_1[GAME_TITLE_OFFSET + i];
    }
    std::cout << game_name << std::endl;
}

void get_rom_as_bytes(std::string rom_name, uint8_t* out, int out_length) {
    // open file
    std::ifstream infile;
    infile.open(rom_name, std::ios::binary);

    // check if file exists
    if (!infile.good()) {
        error("ROM not found, are you sure you gave the right file name?");
    }

    // get length of file
    infile.seekg(0, std::ios::end);
    size_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);

    // read file
    char* buffer = new char[length];
    infile.read(buffer, length);

    length = infile.gcount();
    if (out_length < length) {
        warning("ROM file too large, truncating.");
        length = out_length;
    }

    for (int i = 0; i < length; i++) {
        out[i] = buffer[i];
    }
}

// note that prefetches might not even be needed, if i just subtract the proper amount
// when running the opcode.
int fetch() {
    // TODO: this fetch should operate differnetly based on bit T, which dictates ARM or Thumb mode.
    // memory.pc must be rounded to the nearest even number before being used for fetching here, hence the & 0xFFFFFFFE.
    uint16_t opcode = *((uint16_t*)(memory.main + (*memory.pc & 0xFFFFFFFE)));
    *memory.pc += 2;
    return opcode;
}

void execute(int opcode) {
    // TODO: this execute should operate differnetly based on bit T, which dictates ARM or Thumb mode.
    jumptable[opcode >> 8](opcode);
}