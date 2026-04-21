#ifndef H_DES_H
#define H_DES_H

dword SelectBits(dword msg, const BitSelection mask);
word CircularLeftShift28(word w, int n);
void KeySchedule(dword key, Keys keys);
std::vector<byte> SliceIntoSBoxes(dword w);
word ApplySBoxes(std::vector<byte>& boxes);
word CipherFunction(word R, dword key);
dword EncipheringComputation(dword msg, dword key);
dword DecipheringComputation(dword msg, dword key);

#endif // H_DES_H
