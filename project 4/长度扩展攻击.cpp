#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstdint>

class SM3_Algorithm {
private:
    static const uint32_t initialVector[8];
    uint32_t state[8];
    uint64_t bitCount;
    std::vector<uint8_t> messageBuffer;

    static inline uint32_t rotateLeft(uint32_t x, int  n) {
        return (x << n) | (x >> (32 - n));
    }

    static inline uint32_t funcFF(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j < 16) ? (x ^ y ^ z) : ((x & y) | (y & z) | (z & x));
    }

    static inline uint32_t funcGG(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j < 16) ? (x ^ y ^ z) : ((x & z) | (~x & y));
    }

    static inline uint32_t transformP0(uint32_t x) {
        return x ^ rotateLeft(x, 9) ^ rotateLeft(x, 17);
    }

    static inline uint32_t transformP1(uint32_t x) {
        return x ^ rotateLeft(x, 15) ^ rotateLeft(x, 23);
    }

    static inline uint32_t constantT(int round) {
        return (round < 16) ? 0x79cc4519 : 0x7a879d8a;
    }

    void processMessageBlock(const uint8_t* block) {
        uint32_t W[68];
        for (int i = 0; i < 16; ++i) {
            W[i] = (static_cast<uint32_t>(block[4*i]) << 24) |
                   (static_cast<uint32_t>(block[4*i + 1]) << 16) |
                   (static_cast<uint32_t>(block[4*i + 2]) << 8) |
                   static_cast<uint32_t>(block[4*i + 3]);
        }
        
        for (int i = 16; i < 68; ++i) {
            W[i] = transformP1(W[i - 16] ^ W[i - 9] ^ rotateLeft(W[i - 3], 15)) ^ 
                   rotateLeft(W[i - 13], 7) ^ W[i - 6];
        }
        
        uint32_t W1[64];
        for (int i = 0; i < 64; ++i) {
            W1[i] = W[i] ^ W[i + 4];
        }

        uint32_t A = state[0], B = state[1], C = state[2], D = state[3];
        uint32_t E = state[4], F = state[5], G = state[6], H = state[7];

        for (int j = 0; j < 64; ++j) {
            uint32_t SS1 = rotateLeft(rotateLeft(A, 12) + E + rotateLeft(constantT(j), j % 32), 7);
            uint32_t SS2 = SS1 ^ rotateLeft(A, 12);
            uint32_t TT1 = funcFF(A, B, C, j) + D + SS2 + W1[j];
            uint32_t TT2 = funcGG(E, F, G, j) + H + SS1 + W[j];

            D = C;
            C = rotateLeft(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = rotateLeft(F, 19);
            F = E;
            E = transformP0(TT2);
        }

        state[0] ^= A; state[1] ^= B; state[2] ^= C; state[3] ^= D;
        state[4] ^= E; state[5] ^= F; state[6] ^= G; state[7] ^= H;
    }

    void padData() {
        size_t currentLength = messageBuffer.size();
        size_t paddingSize = (currentLength % 64 < 56) ? (56 - currentLength % 64) : (120 - currentLength % 64);
        
        messageBuffer.push_back(0x80);
        messageBuffer.insert(messageBuffer.end(), paddingSize - 1, 0x00);

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

    void update(const uint8_t* inputData, size_t length) {
        bitCount += length * 8;
        messageBuffer.insert(messageBuffer.end(), inputData, inputData + length);

        while (messageBuffer.size() >= 64) {
            processMessageBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }
    }

    std::vector<uint8_t> finalize() {
        padData();
        while (messageBuffer.size() >= 64) {
            processMessageBlock(messageBuffer.data());
            messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + 64);
        }

        std::vector<uint8_t> output(32);
        for (int i = 0; i < 8; ++i) {
            output[4*i] = (state[i] >> 24) & 0xFF;
            output[4*i + 1] = (state[i] >> 16) & 0xFF;
            output[4*i + 2] = (state[i] >> 8) & 0xFF;
            output[4*i + 3] = state[i] & 0xFF;
        }
        return output;
    }

    static std::string computeHash(const std::string& input) {
        SM3_Algorithm hasher;
        hasher.update(reinterpret_cast<const uint8_t*>(input.data()), input.size());
        auto hashBytes = hasher.finalize();
        
        std::ostringstream hexStream;
        for (const auto& byte : hashBytes) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return hexStream.str();
    }
};

const uint32_t SM3_Algorithm::initialVector[8] = {
    0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e
};

class Attack {
public:
    static std::string executeAttack(
        const std::string& hashString,
        size_t originalLength,
        const std::string& additionalData) {

        std::vector<uint8_t> hashBytes(32);
        for (size_t i = 0; i < hashString.length(); i += 2) {
            std::string byteString = hashString.substr(i, 2);
            hashBytes[i / 2] = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        }

        size_t paddingSize = 56 - (originalLength + 1) % 64;
        if (paddingSize > 56) {
            paddingSize += 64;
        }

        size_t totalBytes = originalLength + 1 + paddingSize + 8;
        uint64_t totalBits = totalBytes * 8;

        SM3_Algorithm attacker;
        attacker.update(reinterpret_cast<const uint8_t*>(hashBytes.data()), hashBytes.size());
        attacker.update(reinterpret_cast<const uint8_t*>(additionalData.data()), additionalData.size());

        auto forgedHash = attacker.finalize();
        std::ostringstream forgedHexStream;
        for (const auto& byte : forgedHash) {
            forgedHexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        return forgedHexStream.str();
    }
};

int main() {
    std::string key = "SECRET_KEY";
    std::string originalData = "user_data=123456";
    std::string completeMessage = key + originalData;
    std::string additional = "&admin=true";

    std::cout << "Secret_key: " << key << std::endl;
    std::cout << "Message: " << originalData << std::endl;
    std::cout << "Combi_Meassage: " << completeMessage << std::endl;

    std::string originalHash = SM3_Algorithm::computeHash(completeMessage);
    std::cout << "Hash_message: " << originalHash << std::endl;

    std::string forgedMessage = completeMessage + additional;
    std::string forgedHash = Attack::executeAttack(originalHash, completeMessage.length(), additional);
    std::cout << "Fake_hash: " << forgedHash << std::endl;

    return 0;
}
