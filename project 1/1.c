#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static inline uint32_t rotl32(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
static inline uint32_t rotr32(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t bswap32(uint32_t x) {
    return ((x & 0x000000FFu) << 24) |
           ((x & 0x0000FF00u) << 8) |
           ((x & 0x00FF0000u) >> 8) |
           ((x & 0xFF000000u) >> 24);
}

static inline uint32_t load_be32(const void* p) {
    uint32_t x; memcpy(&x, p, 4); return bswap32(x);
}

static inline void store_be32(void* p, uint32_t x) {
    x = bswap32(x); memcpy(p, &x, 4);
}

static const uint32_t FK[4] = {
    0xA3B1BAC6u, 0x56AA3350u, 0x677D9197u, 0xB27022DCu
};

static const uint32_t CK[32] = {
    0x00070E15u, 0x1C232A31u, 0x383F464Du, 0x545B6269u,
    0x70777E85u, 0x8C939AA1u, 0xA8AFB6BDu, 0xC4CBD2D9u,
    0xE0E7EEF5u, 0xFC030A11u, 0x181F262Du, 0x343B4249u,
    0x50575E65u, 0x6C737A81u, 0x888F969Du, 0xA4ABB2B9u,
    0xC0C7CED5u, 0xDCE3EAF1u, 0xF8FF060Du, 0x141B2229u,
    0x30373E45u, 0x4C535A61u, 0x686F767Du, 0x848B9299u,
    0xA0A7AEB5u, 0xBCC3CAD1u, 0xD8DFE6EDu, 0xF4FB0209u,
    0x10171E25u, 0x2C333A41u, 0x484F565Du, 0x646B7279u
};

static const uint8_t SBOX[256] = {
    // Fill in the SBOX values
};

static uint32_t T[4][256];

static uint32_t L32(uint32_t x) {
    return x ^ rotl32(x, 2) ^ rotl32(x, 10) ^ rotl32(x, 18) ^ rotl32(x, 24);
}

static void sm4_build_Ttables(void) {
    for (int b = 0; b < 256; b++) {
        uint8_t s = SBOX[b];
        uint32_t x = ((uint32_t)s) << 24;       
        uint32_t t = L32(x);
        T[0][b] = t;
        T[1][b] = rotl32(t, 8);
        T[2][b] = rotl32(t, 16);
        T[3][b] = rotl32(t, 24);
    }
}

typedef struct { uint32_t rk[32]; uint32_t drk[32]; } sm4_key_t;

static void sm4_key_schedule(sm4_key_t* ks, const uint8_t key[16]) {
    uint32_t K[4];
    for (int i = 0; i < 4; i++) {
        K[i] = load_be32(key + 4 * i) ^ FK[i];
    }
    for (int i = 0; i < 32; i++) {
        uint32_t t = K[1] ^ K[2] ^ K[3] ^ CK[i];
        ks->rk[i] = K[0] ^ t;
        K[0] = K[1]; K[1] = K[2]; K[2] = K[3]; K[3] = ks->rk[i];
    }
    for (int i = 0; i < 32; i++) {
        ks->drk[i] = ks->rk[31 - i];
    }
}

static inline void sm4_round(uint32_t* X, uint32_t rk) {
    uint32_t t = X[1] ^ X[2] ^ X[3] ^ rk;
    X[0] ^= T[0][(t >> 24) & 0xFF] ^ T[1][(t >> 16) & 0xFF] ^
             T[2][(t >> 8) & 0xFF] ^ T[3][t & 0xFF];
}

static void sm4_process_block(const sm4_key_t* ks, const uint8_t in[16], uint8_t out[16], int decrypt) {
    uint32_t X[4];
    X[0] = load_be32(in + 0); X[1] = load_be32(in + 4); 
    X[2] = load_be32(in + 8); X[3] = load_be32(in + 12);
    
    for (int i = 0; i < 32; i++) {
        sm4_round(X, decrypt ? ks->drk[i] : ks->rk[i]);
        uint32_t tmp = X[0]; X[0] = X[1]; X[1] = X[2]; X[2] = X[3]; X[3] = tmp;
    }
    
    store_be32(out + 0, X[3]); store_be32(out + 4, X[2]);
    store_be32(out + 8, X[1]); store_be32(out + 12, X[0]);
}

static inline void inc_be128(uint8_t ctr[16]) {
    for (int i = 15; i >= 0; --i) { if (++ctr[i]) break; }
}

void sm4_ctr_encrypt(const sm4_key_t* ks, const uint8_t iv[16], const uint8_t* in, uint8_t* out, size_t len) {
    uint8_t ctr[16]; memcpy(ctr, iv, 16);

    while (len >= 16) {
        uint8_t ksblk[16];
        sm4_process_block(ks, ctr, ksblk, 0);
        for (size_t i = 0; i < 16 && len > 0; i++, len--) {
            out[i] = in[i] ^ ksblk[i];
        }
        in += 16; out += 16; inc_be128(ctr);
    }
}

int main(int argc, char** argv) {
    sm4_build_Ttables();
    return 0;
}
