Poseidon2 哈希算法电路实现：
本项目使用 Circom 2.1.5 实现 Poseidon2 哈希算法的零知识证明电路，适用于 Groth16 证明系统。
本实验的电路接受两个输入，并生成一个哈希值，利用 Poseidon 哈希函数进行计算。
由Poseidon2 论文中的参数：
n = 256（有限域大小）
t = 2（输入数量）
d = 5（S-box 度数）
poseidon2\_hash.circom：主电路实现文件
test.js:电路测试用例
input.json：用于生成证明的输入文件
verify.sh：验证文件
参考文档：
Poseidon2 论文：https://eprint.iacr.org/2023/323.pdf
Circom 文档：https://docs.circom.io/
SnarkJS 文档：https://github.com/iden3/snarkjs

