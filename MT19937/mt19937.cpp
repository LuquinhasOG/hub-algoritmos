#include <cstdint>
#include <iostream>
#include <string>


using word = uint32_t;

const size_t STATE_VECTOR_SIZE = 624;
const size_t M = 397;
const size_t WORD_SIZE = 32;

const word UPPER_MASK = 0x80000000;
const word LOWER_MASK = 0x7FFFFFFF;

const word A = 0x9908B0DF;
const word B = 0x9D2C5680;
const word C = 0xEFC60000;
const size_t U = 11;
const size_t S = 7;
const size_t T = 15;
const size_t L = 18;

using StateVector = word[STATE_VECTOR_SIZE];

size_t index = 0;
StateVector state;

void StateGen(StateVector s, word seed);
void ApplyRecurrence(StateVector state);
double GenRandNumber();

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Especifique argumentos, nesta ordem: seed quant_de_numeros comeco fim";
        return 1;
    }

    word seed = std::stoi(argv[1]);
    size_t n = std::stoi(argv[2]);
    size_t c = std::stoi(argv[3]);
    size_t f = std::stoi(argv[4]);

    StateGen(state, seed);
    for (int i = 0; i < n; i++)
        std::cout << (int) (GenRandNumber()*(f-c) + c) << ' ';

    return 0;
}

// linear recurrence initialization
void StateGen(StateVector s, word seed) {
    s[0] = seed;
    for (int i = 1; i < STATE_VECTOR_SIZE; i++)
        s[i] = (1812433253*(s[i-1] ^ (s[i-1] >> 30)) + i) & 0xFFFFFFFF;
}

// original initialization
/*
void StateGen(StateVector s, word seed) {
    s[0] = seed & 0xFFFFFFFF;
    for (int i = 1; i < STATE_VECTOR_SIZE; i++)
        s[i] = (69069*s[i-1]) & 0xFFFFFFFF;
}
*/

void ApplyRecurrence(StateVector s) {
    word matA[2] = {0, A};

    for (int i = 0; i < STATE_VECTOR_SIZE; i++) {
        word y = (s[i] & UPPER_MASK) | (s[(i + 1) % STATE_VECTOR_SIZE] & LOWER_MASK);
        s[i] = s[(i + M) % STATE_VECTOR_SIZE] ^ (y >> 1) ^ matA[y & 0x01];
    }
}

double GenRandNumber() {
    if (index == 0)
        ApplyRecurrence(state);

    word y = state[index];
    y ^= y >> U;
    y ^= (y << S) & B;
    y ^= (y << T) & C;
    y ^= y >> L;

    index = (index + 1) % STATE_VECTOR_SIZE;

    return ((double) y / 0xFFFFFFFF);
}
