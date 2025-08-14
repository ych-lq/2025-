from gmssl import sm2, func
import base64
import secrets

class SM2Utils:
    def __init__(self, private_key=None, public_key=None):
        self.private_key = private_key
        self.public_key = public_key
        self.sm2_crypt = sm2.CryptSM2(public_key=public_key or '', private_key=private_key or '')

    @staticmethod
    def generate_keypair():
        """生成SM2密钥对"""
        private_key = func.random_hex(64)
        sm2_tmp = sm2.CryptSM2(private_key=private_key, public_key='')
        public_key = sm2_tmp._kg(int(private_key, 16), sm2_tmp.ecc_table['g'])
        return private_key, public_key

    def sign(self, data: bytes) -> str:
        """签名数据，返回签名（hex）"""
        # 生成随机数k，长度为32字节（256位）
        rand_k = func.random_hex(64)
        try:
            # 尝试使用新版本gmssl的签名方法
            signature = self.sm2_crypt.sign(data, rand_k)
        except TypeError:
            # 如果失败，尝试旧版本方法
            try:
                signature = self.sm2_crypt.sign(data, rand_k, user_id='1234567812345678')
            except:
                # 最后尝试无user_id参数的方法
                signature = self.sm2_crypt.sign(data, rand_k)
        return signature

    def verify(self, data: bytes, signature: str) -> bool:
        """验证签名"""
        try:
            # 尝试使用新版本gmssl的验证方法
            return self.sm2_crypt.verify(signature, data)
        except TypeError:
            # 如果失败，尝试旧版本方法
            try:
                return self.sm2_crypt.verify(signature, data, user_id='1234567812345678')
            except:
                # 最后尝试无user_id参数的方法
                return self.sm2_crypt.verify(signature, data)

    def encrypt(self, data: bytes) -> str:
        """SM2 加密，返回 base64 编码"""
        try:
            encrypted_data = self.sm2_crypt.encrypt(data)
            return base64.b64encode(encrypted_data).decode()
        except Exception as e:
            print(f"加密失败: {e}")
            return ""

    def decrypt(self, cipher_base64: str) -> bytes:
        """SM2 解密，输入 base64 编码密文"""
        try:
            cipher_data = base64.b64decode(cipher_base64)
            return self.sm2_crypt.decrypt(cipher_data)
        except Exception as e:
            print(f"解密失败: {e}")
            return b""


if __name__ == "__main__":
    try:
        # 生成密钥对
        private_key, public_key = SM2Utils.generate_keypair()
        print("私钥:", private_key)
        print("公钥:", public_key)

        # 创建SM2工具实例
        sm2_tool = SM2Utils(private_key=private_key, public_key=public_key)

        # 测试消息
        msg = "你好，SM2".encode('utf-8')
        print("原始消息:", msg.decode('utf-8'))

        # 签名测试
        signature = sm2_tool.sign(msg)
        print("签名:", signature)

        # 验证签名
        is_valid = sm2_tool.verify(msg, signature)
        print("验签结果:", is_valid)

        # 加密测试
        encrypted = sm2_tool.encrypt(msg)
        print("加密结果:", encrypted)

        # 解密测试
        if encrypted:
            decrypted = sm2_tool.decrypt(encrypted)
            print("解密结果:", decrypted.decode('utf-8'))
            print("解密成功:", decrypted == msg)
        else:
            print("加密失败，无法进行解密测试")

    except Exception as e:
        print(f"程序执行出错: {e}")
        import traceback
        traceback.print_exc()
