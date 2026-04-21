#ifndef H_GLOBALS_H
#define H_GLOBALS_H

const size_t DWORD_SIZE = 64;
const size_t CIPHER_CYCLES = 16;
const size_t QUANT_S_BOXES = 8;

using byte = uint8_t;
using word = uint32_t;
using dword = uint64_t;

using BitSelection = byte[DWORD_SIZE]; // selection map

using SBoxes = byte[8][4][16];
using Keys = dword[16]; // K1, ..., K16 array

const size_t READ_BUFFER_SIZE = 128; // bites to read; 1kb.
const int BLOCK_SIZE = 8;

#endif // H_GLOBALS_H
