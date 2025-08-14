import hashlib
from sympy import randprime
import secrets
import random
from phe import paillier
from functools import lru_cache
import math

class CryptoBase:  # 基础密码类
    prime_number = None  # 大素数
    _prime_cache = {}  # 素数缓存

    @classmethod
    def _generate_prime_safely(cls, bit_length):  # 安全生成指定位数的素数
        # 使用缓存避免重复生成
        if bit_length in cls._prime_cache:
            return cls._prime_cache[bit_length]
        
        # 确定素数范围
        min_val = 2 ** (bit_length - 1)
        max_val = 2 ** bit_length - 1
        
        # 生成素数并验证
        prime_candidate = randprime(min_val, max_val)
        
        # 二次验证确保是素数
        if not cls._verify_prime(prime_candidate):
            raise ValueError("生成的候选数不是素数")
        
        cls._prime_cache[bit_length] = prime_candidate
        return prime_candidate

    @staticmethod
    def _verify_prime(num, certainty=5):
        """使用Miller-Rabin方法验证素数"""
        if num < 2:
            return False
        if num in (2, 3):
            return True
        if num % 2 == 0:
            return False
        
        # 分解为 d * 2^s
        exponent, remainder = num - 1, 0
        while exponent % 2 == 0:
            exponent //= 2
            remainder += 1
        
        # 进行多次测试
        for _ in range(certainty):
            base = secrets.randbelow(num - 3) + 2
            result = pow(base, exponent, num)
            if result in (1, num - 1):
                continue
            for __ in range(remainder - 1):
                result = pow(result, 2, num)
                if result == num - 1:
                    break
            else:
                return False
        return True

    @classmethod
    def initialize(cls, bit_length=1024):  # 初始化大素数
        cls.prime_number = cls._generate_prime_safely(bit_length)

    # 使用缓存优化幂运算
    @lru_cache(maxsize=1024)
    def modular_exponentiation(self, base, exponent):  # base^exponent mod prime
        """高效计算模幂"""
        return pow(base, exponent, self.prime_number)

    def create_secret_key(self):  # 生成秘密值
        # 使用安全随机源
        if self.prime_number is None:
            raise ValueError("请先调用 initialize() 初始化素数")
        
        # 生成 [1, p-2] 范围内的安全随机数
        secret = secrets.randbelow(self.prime_number - 2) + 1
        return secret

    def credential_to_int(self, credential):  # 凭证转整数
        if self.prime_number is None:
            raise ValueError("请先调用 initialize() 初始化素数")
        
        # 使用更安全的哈希算法
        hashed = hashlib.blake2b(credential.encode(), digest_size=32).digest()
        return int.from_bytes(hashed, 'big') % self.prime_number

    def process_credentials(self, credentials):  # 处理凭证集
        if self.prime_number is None:
            raise ValueError("请先调用 initialize() 初始化素数")
        return [self.credential_to_int(cred) for cred in credentials]

    def create_crypto_keys(self):  # 生成加密密钥对
        return paillier.generate_paillier_keypair()

    def encrypt_data(self, data, public_key):  # 加密数据
        return public_key.encrypt(data)

    def decrypt_data(self, encrypted_data, private_key):  # 解密数据
        return private_key.decrypt(encrypted_data)


class Initiator(CryptoBase):
    def __init__(self, credentials={"securePass123"}):
        self.user_credentials = credentials
        self.secret_value = self.create_secret_key()
        # 预计算哈希值
        self.hashed_credentials = self.process_credentials(self.user_credentials)
        # 预计算H(cred)^secret
        self.precomputed_values = [self.modular_exponentiation(h, self.secret_value) for h in self.hashed_credentials]

    def first_step(self):
        # 返回预计算值
        computed_list = self.precomputed_values.copy()
        random.shuffle(computed_list)
        return computed_list

    def receive_response(self, response_set, processed_list):  # 接收响应
        self.response_set = response_set
        self.processed_list = processed_list

    def final_computation(self):
        # 合并计算和验证
        encrypted_total = None
        for item in self.processed_list:  # item = (H(cred')^secret2, Enc(value))
            # 计算 (H(cred')^secret2)^secret1
            computed_val = self.modular_exponentiation(item[0], self.secret_value)
            
            # 如果匹配，累加加密值
            if computed_val in self.response_set:
                if encrypted_total is None:
                    encrypted_total = item[1]
                else:
                    encrypted_total += item[1]  # 同态加法
        return encrypted_total


class Responder(CryptoBase):
    def __init__(self,
                 credentials_data={("用户身份验证", 15), ("访问权限", 25), ("系统登录", 75)}):
        self.credentials_data = credentials_data
        self.secret_value = self.create_secret_key()
        self.public_key, self.private_key = self.create_crypto_keys()  # 生成密钥对
        
        # 预计算哈希值
        self.hashed_credentials = {cred[0]: self.credential_to_int(cred[0]) for cred in self.credentials_data}
        # 预计算H(cred')^secret
        self.precomputed_vals = {cred: self.modular_exponentiation(h, self.secret_value) for cred, h in self.hashed_credentials.items()}

    def process_request(self, request_list):  # 处理请求
        self.request_list = request_list

    def prepare_response(self):  # 准备响应
        # 计算响应集
        response_set = [self.modular_exponentiation(i, self.secret_value) for i in self.request_list]
        random.shuffle(response_set)
        
        # 准备处理列表
        processed_data = []
        for cred_data in self.credentials_data:
            cred_text = cred_data[0]
            # 使用预计算值
            computed_hash = self.precomputed_vals[cred_text]
            # 加密关联值
            encrypted_value = self.encrypt_data(cred_data[1], self.public_key)
            processed_data.append((computed_hash, encrypted_value))
        
        random.shuffle(processed_data)
        return set(response_set), processed_data

    def interpret_result(self, encrypted_result):  # 解释结果
        if encrypted_result is None:
            return 0
        total_value = self.decrypt_data(encrypted_result, self.private_key)
        return total_value


def execute_protocol():
    CryptoBase.initialize()
    initiator = Initiator()
    responder = Responder()
    step1_result = initiator.first_step()  # 第一步
    responder.process_request(step1_result)  # 处理请求
    print("协议第一步完成")
    response_set, processed_list = responder.prepare_response()  # 准备响应
    initiator.receive_response(response_set, processed_list)  # 接收响应
    print("协议第二步完成")
    encrypted_result = initiator.final_computation()  # 最终计算
    final_value = responder.interpret_result(encrypted_result)  # 解释结果
    print("协议完成")
    return initiator.user_credentials, responder.credentials_data, final_value


if __name__ == "__main__":
    creds1, creds2, result = execute_protocol()
    print("发起方凭证:", creds1, "\n响应方凭证:", creds2, "\n计算结果:", result)
