#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "header/globals.h"
#include "header/bitselmaps.h"
#include "header/des.h"


char rdbuff[READ_BUFFER_SIZE];

bool ReadNextChars(char* rdbuff, std::ifstream& file);
dword ReadKey();
byte CalculateParity(byte b);
void GenKey();
void Encrypt(std::string inFile, std::string outFile, bool hasKey=true);
void Decrypt(std::string inFile, std::string outFile);

int main(int argc, char* argv[]) {
    std::string input, output;
    char op; // 'e' to encrypt; 'd' to decrypt
    bool hasKey = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            input = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            output = argv[++i];
        } else if (strcmp(argv[i], "-gK") == 0) {
            hasKey = false;
        } else if (strcmp(argv[i], "e") == 0) {
            op = 'e';
        } else if (strcmp(argv[i], "d") == 0) {
            op = 'd';
        }
    }

    if (op == 'e') {
        Encrypt(input, output, hasKey);
    } else if (op == 'd') {
        Decrypt(input, output);
    } else {
        std::cerr << "Escolha uma operação!";
        return -1;
    }

    return 0;
}

bool ReadNextChars(char* rdbuff, std::ifstream& file, size_t buffSize) {
    if (file.read(rdbuff, buffSize) || file.gcount() > 0) {
        rdbuff[file.gcount()] = '\0';
        return true;
    }

    return false;
}

dword ReadKey() {
    std::ifstream key(".key", std::ios::binary);
    if (!key) {
        std::cerr << "A chave de criptografia não se encontra no mesmo diretório da mensagem!\n";
        std::exit(EXIT_FAILURE);
    }

    char keybuff[8];
    ReadNextChars(keybuff, key, BLOCK_SIZE);

    dword keyWords = 0;
    std::memcpy(&keyWords, keybuff, BLOCK_SIZE);

    return keyWords;
}

byte CalculateParity(byte b) {
    int c = 0;

    while (b) {
        c += b & 1;
        b >>= 1;
    }

    return c % 2;
}

void GenKey() {
    dword seed = std::chrono::system_clock::now().time_since_epoch().count();;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(0, 127);
    char keyBytes[8];

    for (int i = 0; i < 8; i++) {
        byte value = (byte) distrib(gen);
        keyBytes[i] = (value << 1) | CalculateParity(value);
    }

    std::ofstream key(".key", std::ios::binary);
    key.write(keyBytes, BLOCK_SIZE);
    key.close();
}

void Encrypt(std::string inFile, std::string outFile, bool hasKey) {
    std::ifstream input(inFile, std::ios::binary);
    std::ofstream output(outFile, std::ios::binary);

    if (!input) {
        std::cerr << "O arquivo de entrada especificado não foi encontrado!\n";
        std::exit(EXIT_FAILURE);
    }

    if (!hasKey)
        GenKey();

    dword key = ReadKey();
    while(ReadNextChars(rdbuff, input, READ_BUFFER_SIZE)) {
        int bytesReaded = input.gcount();
        for (int i = 0; i < bytesReaded; i += BLOCK_SIZE)  {
            int curr_block_size = std::min(BLOCK_SIZE, bytesReaded - i);

            dword msg = 0;
            std::memcpy(&msg, rdbuff + i, curr_block_size);

            dword ciphered = EncipheringComputation(msg, key);

            char bytes[curr_block_size + 1] = {0};
            std::memcpy(bytes, &ciphered, curr_block_size);

            output.write(bytes, curr_block_size);
        }
    }

    input.close();
    output.close();
}

void Decrypt(std::string inFile, std::string outFile) {
    std::ifstream input(inFile, std::ios::binary);
    std::ofstream output(outFile, std::ios::binary);

    if (!input) {
        std::cerr << "O arquivo de entrada especificado não foi encontrado!\n";
        std::exit(EXIT_FAILURE);
    }

    dword key = ReadKey();
    while(ReadNextChars(rdbuff, input, READ_BUFFER_SIZE)) {
        int bytesReaded = input.gcount();
        for (int i = 0; i < bytesReaded; i += BLOCK_SIZE)  {
            int curr_block_size = std::min(BLOCK_SIZE, bytesReaded - i);

            dword msg = 0;
            std::memcpy(&msg, rdbuff + i, curr_block_size);

            dword ciphered = DecipheringComputation(msg, key);

            char bytes[curr_block_size + 1] = {0};
            std::memcpy(bytes, &ciphered, curr_block_size);

            output.write(bytes, curr_block_size);
        }
    }

    input.close();
    output.close();
}
