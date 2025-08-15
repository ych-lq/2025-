#include <iostream>
#include <vector>
#include <iomanip>
#include <immintrin.h>

class SM3_Algorithm {
private:
    static const uint32_t initialVector[8];
    uint32_t state[8];
    uint64_t bitCount;
    std::vector<uint8_t> messageBuffer;

    static inline uint32_t   rotateLeft(uint32_t x, int n) {
        return (x << n) | (x >> (32 - n));
    }

    static inline uint32_t CalculateF(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z));
    }

    static inline uint32_t CalculateG(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j< 16) ? (x ^ y ^ z) : ((x & y) | (~x & z));
    }

    static inline uint32_t FunctionP0(uint32_t x) {
        return x ^   rotateLeft(x, 9) ^   rotateLeft(x, 17);
    }

    static inline uint32_t FunctionP1(uint32_t x) {
        return x ^   rotateLeft(x, 15) ^   rotateLeft(x, 23);
    }

    static inline uint32_t GetConstant(int index) {
        return (index < 16) ? 0x79cc4519 : 0x7a879d8a;
    }

    void ProcessMessageBlockSIMD(const uint8_t* input, uint32_t* W) {
        for (int i = 0; i < 16; i += 4) {
            __m128i vector = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input + i * 4));
            __m128i mask = _mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);
            __m128i shuffled = _mm_shuffle_epi8(vector, mask);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(W + i), shuffled);
        }
    }

    void ExpandW1SIMD(const uint32_t* W, uint32_t* W1) {
        for (int i = 0; i < 64; i += 4) {
            __m128i wVec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(W + i));
            __m128i w4Vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(W + i + 4));
            __m128i result = _mm_xor_si128(wVec, w4Vec);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(W1 + i), result);
        }
    }

    void ExecuteBlock(const uint8_t* input) {
        uint32_t W[68];
        ProcessMessageBlockSIMD(input, W);
        
        for (int i = 16; i < 68; ++i) {
            if (i < 29) {
                W[i] = FunctionP1(W[i - 16] ^ W[i - 9] ^   rotateLeft(W[i - 3], 15)) ^ 
                      rotateLeft(W[i - 13], 7) ^ W[i - 6];
            } else {
                W[i] = FunctionP1(W[i - 16] ^ W[i - 9] ^   rotateLeft(W[i - 3], 15)) ^ 
                      rotateLeft(W[i - 13], 7) ^ W[i - 6];
            }
        }

        uint32_t W1[64];
        ExpandW1SIMD(W, W1);

        uint32_t A = state[0], B = state[1], C = state[2], D = state[3];
        uint32_t E = state[4], F = state[5], G = state[6], H = state[7];

        for (int j = 0; j < 16; j += 4) {
            for (int k = 0; k < 4; ++k) {
                uint32_t SS1 =   rotateLeft(  rotateLeft(A, 12) + E +   rotateLeft(GetConstant(j + k), (j + k) % 32), 7);
                uint32_t SS2 = SS1 ^   rotateLeft(A, 12);
                uint32_t TT1 = CalculateF(A, B, C, j + k) + D + SS2 + W1[j + k];
                uint32_t TT2 = CalculateG(E, F, G, j + k) + H + SS1 + W[j + k];

                D = C; 
		C =   rotateLeft(B, 9); 
		B = A; 
		A = TT1;
                H = G; 
		G =   rotateLeft(F, 19); 
		F = E; 
		E = FunctionP0(TT2);
            }
        }

        for (int j = 16; j < 32; j += 4) {
            for (int k = 0; k < 4; ++k) {
                uint32_t SS1 =   rotateLeft(  rotateLeft(A, 12) + E +   rotateLeft(GetConstant(j + k), (j + k) % 32), 7);
                uint32_t SS2 = SS1 ^   rotateLeft(A, 12);
                uint32_t TT1 = CalculateF(A, B, C, j + k) + D + SS2 + W1[j + k];
                uint32_t TT2 = CalculateG(E, F, G, j + k) + H + SS1 + W[j + k];

                D = C; 
		C =   rotateLeft(B, 9); 
		B = A; 
		A = TT1;
                H = G; 
		G =   rotateLeft(F, 19); 
		F = E; 
		E = FunctionP0(TT2);
            }
        }

        for (int j = 32; j < 64; j += 4) {
            for (int k = 0; k < 4; ++k) {
                uint32_t SS1 =   rotateLeft(  rotateLeft(A, 12) + E +   rotateLeft(GetConstant(j + k), (j + k) % 32), 7);
                uint32_t SS2 = SS1 ^   rotateLeft(A, 12);
                uint32_t TT1 = CalculateF(A, B, C, j + k) + D + SS2 + W1[j + k];
                uint32_t TT2 = CalculateG(E, F, G, j + k) + H + SS1 + W[j + k];

                D = C; 
		C =   rotateLeft(B, 9); 
		B = A; 
		A = TT1;
                H = G; 
		G =   rotateLeft(F, 19); 
		F = E; 
		E = FunctionP0(TT2);
            }
        }

        for (int i = 0; i < 8; ++i) {
            state[i] ^= (i == 0 ? A : (i == 1 ? B : (i == 2 ? C : (i == 3 ? D : (i == 4 ? E : (i == 5 ? F : (i == 6 ? G : H)))))));
        }
    }

    void PadMessage() {
        size_t index = messageBuffer.size();
        size_t paddingLength = 56 - (index + 1) % 64;
        if (paddingLength > 56) {
            paddingLength += 64;
        }
        messageBuffer.push_back(0x80);
        for (size_t i = 0; i < paddingLength; ++i) {
            messageBuffer.push_back(0x00);
        }
        for (int i = 7; i >= 0; --i) {
            messageBuffer.push_back((bitCount >> (i * 8)) & 0xFF);
        }
    }

public:
    SM3_Algorithm() {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < 8; ++i) {
            state[i] = initialVector[i];
        }
        bitCount = 0;
        messageBuffer.clear();
    }

    void Update(const uint8_t* input, size_t length) {
        bitCount += length * 8;
        messageBuffer.insert(messageBuffer.end(), input, input + length);

        while (messageBuffer.size() >= 64) {
            ExecuteBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }
    }

    void Update(const std::string& str) {
        Update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    std::vector<uint8_t> Finalize() {
        PadMessage();
        while (messageBuffer.size() >= 64) {
            ExecuteBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }

        std::vector<uint8_t> hash(32);
        for (int i = 0; i < 8; ++i) {
            hash[4 * i] = (state[i] >> 24) & 0xFF;
            hash[4 * i + 1] = (state[i] >> 16) & 0xFF;
            hash[4 * i + 2] = (state[i] >> 8) & 0xFF;
            hash[4 * i + 3] = state[i] & 0xFF;
        }
        return hash;
    }

    static std::string ComputeHash(const std::string& input) {
        SM3_Algorithm hasher;
        hasher.Update(input);
        auto result = hasher.Finalize();

        std::stringstream hexStream;
        hexStream << std::hex << std::setfill('0');
        for (const auto& byte : result) {
            hexStream << std::setw(2) << static_cast<int>(byte);
        }
        return hexStream.str();
    }
};

const uint32_t SM3_Algorithm::initialVector[8] = {
    0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e
};

int main() 
{
    std::cout << "Test:" << std::endl;
    std::string hashEmpty = SM3_Algorithm::hash("");
    std::string expectedEmpty = "1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b";
    std::cout << "Empty string Test:" << std::endl;
    std::cout << "  SM3_Result: " << hashEmpty << std::endl;
    std::cout << "  Expected_Result: " << expectedEmpty << std::endl;
    
    std::string hashAbc = SM3_Algorithm::hash("abcd");
    std::string expectedAbc = "82ec580fe6d36ae4f81cae3c73f4a5b3b5a09c943172dc9053c69fd8e18dca1e";
    std::cout << "String test:" << std::endl;
    std::cout << "  SM3_Result: " << hashAbc << std::endl;
    std::cout << "  Expected_Result: " << expectedAbc << std::endl;
    return 0;
}
