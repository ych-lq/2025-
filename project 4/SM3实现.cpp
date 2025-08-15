#include <iostream>
#include <vector>
#include <iomanip>


class SM3_Algorithm {
private:
    static const uint32_t initialVector[8];
    uint32_t state[8];
    uint64_t bitCount;
    std::vector<uint8_t> messageBuffer;
    static inline uint32_t rotateLeft(uint32_t x, int n) {
        return (x << n) | (x >> (32 - n));
    }
    static inline uint32_t functionFF(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j >= 0 && j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z));
    }
    
    static inline uint32_t functionGG(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j >= 0 && j < 16) ? (x ^ y ^ z) : ((x & y) | (~x & z));
    }
    
    static inline uint32_t functionP0(uint32_t x) {
        return x ^ rotateLeft(x, 9) ^ rotateLeft(x, 17);
    }
    
    static inline uint32_t functionP1(uint32_t x) {
        return x ^ rotateLeft(x, 15) ^ rotateLeft(x, 23);
    }
    
    static inline uint32_t getT(int j) {
        return (j >= 0 && j < 16) ? 0x79cc4519 : 0x7a879d8a;
    }
    
    void processBlock(const uint8_t* data) {
        uint32_t W[68];
        
        for (int i = 0; i < 16; ++i) {
            W[i] = (static_cast<uint32_t>(data[4*i]) << 24) |
                   (static_cast<uint32_t>(data[4*i+1]) << 16) |
                   (static_cast<uint32_t>(data[4*i+2]) << 8) |
                   static_cast<uint32_t>(data[4*i+3]);
        }
        
        for (int i = 16; i < 68; ++i) {
            W[i] = functionP1(W[i-16] ^ W[i-9] ^ rotateLeft(W[i-3], 15)) ^ 
                             rotateLeft(W[i-13], 7) ^ W[i-6];
        }
        
        uint32_t W1[64];
        for (int i = 0; i < 64; ++i) {
            W1[i] = W[i] ^ W[i+4];
        }
        
        uint32_t A = state[0], B = state[1], C = state[2], D = state[3];
        uint32_t E = state[4], F = state[5], G = state[6], H = state[7];
        
        for (int j = 0; j < 64; ++j) {
            uint32_t SS1 = rotateLeft(rotateLeft(A, 12) + E + rotateLeft(getT(j), j % 32), 7);
            uint32_t SS2 = SS1 ^ rotateLeft(A, 12);
            
            uint32_t TT1 = functionFF(A, B, C, j) + D + SS2 + W1[j];
            uint32_t TT2 = functionGG(E, F, G, j) + H + SS1 + W[j];
            
            D = C;
            C = rotateLeft(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = rotateLeft(F, 19);
            F = E;
            E = functionP0(TT2);
        }
        
        state[0] ^= A; state[1] ^= B; state[2] ^= C; state[3] ^= D;
        state[4] ^= E; state[5] ^= F; state[6] ^= G; state[7] ^= H;
    }
    
    void padMessage() {
        size_t index = messageBuffer.size();
        size_t padLength = 56 - (index + 1) % 64;
        if (padLength > 56) {
            padLength += 64;
        }
        
        messageBuffer.push_back(0x80);
        messageBuffer.insert(messageBuffer.end(), padLength, 0x00);
        
        for (int i = 7; i >= 0; --i) {
            messageBuffer.push_back((bitCount >> (i * 8)) & 0xFF);
        }
    }

public:
    SM3_Algorithm() { 
        reset(); 
    }
    
    void reset() {
        for (int i = 0; i < 8; ++i) {
            state[i] = initialVector[i];
        }
        bitCount = 0;
        messageBuffer.clear();
    }
    
    void update(const uint8_t* data, size_t len) {
        bitCount += len * 8;
        messageBuffer.insert(messageBuffer.end(), data, data + len);
        
        while (messageBuffer.size() >= 64) {
            processBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }
    }
    
    void update(const std::string& str) {
        update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }
    
    std::vector<uint8_t> finalize() {
        padMessage();
        
        while (messageBuffer.size() >= 64) {
            processBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }
        
        std::vector<uint8_t> hash(32);
        for (int i = 0; i < 8; ++i) {
            hash[4*i] = (state[i] >> 24) & 0xFF;
            hash[4*i+1] = (state[i] >> 16) & 0xFF;
            hash[4*i+2] = (state[i] >> 8) & 0xFF;
            hash[4*i+3] = state[i] & 0xFF;
        }
        return hash;
    }
    
    static std::string hash(const std::string& input) {
        SM3_Algorithm hashAlg;
        hashAlg.update(input);
        auto hashResult = hashAlg.finalize();
        
        std::stringstream ss;
        for (auto b : hashResult) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        return ss.str();
    }
};

const uint32_t SM3_Algorithm::initialVector[8] = {
    0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e
};


int main() {
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
