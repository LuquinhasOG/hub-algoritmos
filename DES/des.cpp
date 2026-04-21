#include <bitset>
#include <cstdint>
#include <vector>

#include "header/globals.h"
#include "header/bitselmaps.h"


dword SelectBits(dword msg, const BitSelection mask) {
    std::bitset<DWORD_SIZE> msgBits(msg);
    std::bitset<DWORD_SIZE> selected(0);
    for (int i = 0; i < DWORD_SIZE; i++)
        selected[DWORD_SIZE - i - 1] = msgBits[DWORD_SIZE - mask[i]];

    return selected.to_ullong();
}

word CircularLeftShift28(word w, int n) {
    return ((w << n) | (w >> (28 - n))) & 0x0FFFFFFF;
}

void KeySchedule(dword key, Keys keys) {
    dword permuted = SelectBits(key, PERMUTED_CHOICE1);
    int shifts[CIPHER_CYCLES] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };
    word C = (permuted & 0x00FFFFFFF0000000) >> 28;
    word D = permuted & 0x0FFFFFFF;

    for (int i = 0; i < CIPHER_CYCLES; i ++) {
        C = CircularLeftShift28(C, shifts[i]);
        D = CircularLeftShift28(D, shifts[i]);
        keys[i] = SelectBits(((dword) C << 28) | D, PERMUTED_CHOICE2);
    }
}

std::vector<byte> SliceIntoSBoxes(dword w) {
    std::vector<byte> boxes;
    for (int i = 0; i < QUANT_S_BOXES; i++) {
        dword mask = (dword)0x3F << (i * 6);
        byte box = (w & mask) >> (i * 6);
        boxes.insert(boxes.begin(), box);
    }

    return boxes;
}

word ApplySBoxes(std::vector<byte>& boxes) {
    word result = 0;
    for (int i = 0; i < QUANT_S_BOXES; i++) {
        byte block = boxes[i];
        int row = ((block & 0x20) >> 4) | (block & 0x01);
        int col = (block >> 1) & 0x0F;
        byte value = S_BOXES[i][row][col];
        result = (result << 4) | (value & 0x0F);
    }

    return result;
}

word CipherFunction(word R, dword key) { // TESTAR FUNCIONAMENTO
    dword extended = SelectBits(R, E_BIT_SELECTION);
    dword preSelection = extended ^ key;
    std::vector<byte> SBoxes = SliceIntoSBoxes(preSelection);
    word result = ApplySBoxes(SBoxes);

    return SelectBits(result, P_FUNC);
}

dword EncipheringComputation(dword msg, dword key) {
    dword initalPerm = SelectBits(msg, INITIAL_PERM);

    Keys keys;
    KeySchedule(key, keys);

    word L = (initalPerm & 0xFFFFFFFF00000000) >> 32;
    word R = initalPerm & 0xFFFFFFFF;
    for (int i = 0; i < CIPHER_CYCLES; i++) {
        word aux = R;
        R = L ^ CipherFunction(R, keys[i]);
        L = aux;
    }

    dword RL = ((dword) R << 32) | L;
    return SelectBits(RL, INVERSE_INITIAL_PERM);
}

dword DecipheringComputation(dword msg, dword key) {
    dword initalPerm = SelectBits(msg, INITIAL_PERM);

    Keys keys;
    KeySchedule(key, keys);

    word L = (initalPerm & 0xFFFFFFFF00000000) >> 32;
    word R = initalPerm & 0xFFFFFFFF;
    for (int i = 0; i < CIPHER_CYCLES; i++) {
        word aux = R;
        R = L ^ CipherFunction(R, keys[CIPHER_CYCLES - i - 1]);
        L = aux;
    }

    dword RL = ((dword) R << 32) | L;
    return SelectBits(RL, INVERSE_INITIAL_PERM);
}
