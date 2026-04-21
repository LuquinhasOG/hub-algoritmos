#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>


using byte = uint8_t;
using word = uint32_t;
using MsgBlocks = std::vector<std::vector<word>>;
using Hash = word[8];

const int hash_size = 8;
const int words_in_block = 16;
const Hash initial_h = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

const word K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

word rotr(int n, word x);
word choose(word x, word y, word z);
word majority(word x, word y, word z);
word upper_sig0(word x);
word upper_sig1(word x);
word lower_sig0(word x);
word lower_sig1(word x);

std::string read_file(std::string path);
std::string padding_message(std::string message);
MsgBlocks parse_message(std::string padded_message);
MsgBlocks prepare_message_schedule(MsgBlocks message);
void computate_hash(MsgBlocks schedule, Hash output);
std::string hash_to_hexstr(Hash h);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Passe o caminho do arquivo de texto\n";
        std::exit(EXIT_FAILURE);
    }

    std::string message = read_file(argv[1]); // mensagem que irá se tornar o hash

    // pré-processamento da mensagem
    std::string padded_message = padding_message(message); // mensagem preenchida
    MsgBlocks blocks = parse_message(padded_message); // mensagem separada em blocos

    // computação do hash
    Hash result;
    MsgBlocks schedule = prepare_message_schedule(blocks);
    computate_hash(schedule, result);
    std::string hexstr = hash_to_hexstr(result);

    std::cout << "SHA-256: " << hexstr << std::endl;

    return 0;
}

// rotação para a direita (circular right shift)
word rotr(int n, word x) {
    return (x >> n) | (x << (32 - n));
}

// função de escolha
word choose(word x, word y, word z) {
    return (x & y) ^ (~x & z);
}

// função maioria
word majority(word x, word y, word z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

word upper_sig0(word x) {
    return rotr(2, x) ^ rotr(13, x) ^ rotr(22, x);
}

word upper_sig1(word x) {
    return rotr(6, x) ^ rotr(11, x) ^ rotr(25, x);
}

word lower_sig0(word x) {
    return rotr(7, x) ^ rotr(18, x) ^ (x >> 3);
}

word lower_sig1(word x) {
    return rotr(17, x) ^ rotr(19, x) ^ (x >> 10);
}

// lê um arquivo txt
std::string read_file(std::string path) {
    std::ifstream file(path); // abre o arquivo
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << path << "\nArquivo corrompido ou inexistente" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // lê todo o conteúdo do arquivo
    std::string content = buffer.str();

    return content;
}

// processo de preenchimento da mensagem
std::string padding_message(std::string message) {
    uint64_t l = message.size() * 8; // tamanho da mensagem em bits
    int padding = 56 - ((l/8) % 64); // calculando o padding (preenchimento) em bytes

    if (padding < 0)
        padding += 64;

    // preenchimento de zeros
    if (padding > 0) {
        message += (char) 0x80;

        for (int i = 1; i < padding; i++)
            message += (char) 0x00;
    }

    // transformando l em bytes
    byte l_bytes[8];
    std::memcpy(l_bytes, &l, sizeof(l));

    // preenchendo como o valor de l no final
    for (int i = 7; i >= 0; i--)
        message += l_bytes[i];

    return message;
}

// separa a mensagem em blocos; linhas são blocos e colunas palavras do bloco
MsgBlocks parse_message(std::string padded_message) {
    uint64_t l = padded_message.size(); // tamanho mensagem em bytes
    int blocks_n = l / 64; // número de blocos de 512 bits

    MsgBlocks blocks;
    for (int i = 0; i < blocks_n; i++) {
        std::vector<word> words;
        for (int j = 0; j < words_in_block; j++) {
            int index = i*64 + j*4;

            // faz a concatenação dos bytes em 4 bytes
            // tem que transformar primeiro em uint8_t para não confundir com número negativo e preencher os bits a esquerda com 1's
            word byte1 = ((word) ((byte) padded_message[index])) << 24;
            word byte2 = ((word) ((byte) padded_message[index+1])) << 16;
            word byte3 = ((word) ((byte) padded_message[index+2])) << 8;
            word byte4 = (word) ((byte) padded_message[index+3]);

            word word = byte1 | byte2 | byte3 | byte4;
            words.push_back(word);
        }

        blocks.push_back(words);
    }

    return blocks;
}

// prepara a programação de mensagem
MsgBlocks prepare_message_schedule(MsgBlocks message) {
    MsgBlocks schedule(message.begin(), message.end());

    for (int i = 0; i < schedule.size(); i++) {
        for (int j = 16; j < 64; j++) {
            schedule[i].push_back(lower_sig1(schedule[i][j-2]) + schedule[i][j-7] + lower_sig0(schedule[i][j-15]) + schedule[i][j-16]);
        }
    }

    return schedule;
}

void computate_hash(MsgBlocks schedule, Hash output) {
    Hash Hi = { initial_h[0], initial_h[1], initial_h[2], initial_h[3], initial_h[4], initial_h[5], initial_h[6], initial_h[7] };
    word a, b, c, d, e, f, g, h;

    for (int i = 0; i < schedule.size(); i++) {
        a = Hi[0]; b = Hi[1]; c = Hi[2]; d = Hi[3];
        e = Hi[4]; f = Hi[5]; g = Hi[6]; h = Hi[7];

        for (int t = 0; t < 64; t++) {
            word T1 = h + upper_sig1(e) + choose(e, f, g) + K[t] + schedule[i][t];
            word T2 = upper_sig0(a) + majority(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        Hi[0] += a;
        Hi[1] += b;
        Hi[2] += c;
        Hi[3] += d;
        Hi[4] += e;
        Hi[5] += f;
        Hi[6] += g;
        Hi[7] += h;
    }

    for (int t = 0; t < hash_size; t++) {
        output[t] = Hi[t];
    }
}

// gera a string do hash em hexadecimal
std::string hash_to_hexstr(Hash h) {
    std::stringstream stream;
    stream << std::hex << h[0] << h[1] << h[2] << h[3] << h[4] << h[5] << h[6] << h[7];
    std::string result( stream.str() );

    return result;
}
