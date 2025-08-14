## Project 1: SM4的软件实现和优化

### 任务描述
- **a)** 从基本实现出发，优化SM4的软件执行效率，至少应该覆盖以下内容：
  - T-table：使用查找表加速加密过程。
  - AES-NI：利用AES指令集提升性能。
  - 最新的指令集：如GFNI、VPROLD等，进一步优化软件性能。
  
- **b)** 基于SM4的实现，进行SM4-GCM工作模式的软件优化实现。

## Project 2: 基于数字水印的图片泄露检测

### 任务描述
- 编程实现图片水印的嵌入和提取（可依托开源项目进行二次开发）。
- 进行鲁棒性测试，包括但不限于以下操作：
  - 翻转
  - 平移
  - 截取
  - 调对比度等

## Project 3: 用circom实现poseidon2哈希算法的电路

### 任务描述
1. poseidon2哈希算法参数参考文档1的Table1，用(n,t,d)=(256,3,5)或(256,2,5)。
2. 电路的公开输入为poseidon2哈希值，隐私输入为哈希原象，哈希算法的输入只考虑一个block即可。
3. 使用Groth16算法生成证明。

### 参考文献
1. [poseidon2哈希算法](https://eprint.iacr.org/2023/323.pdf)
2. [circom说明文档](https://docs.circom.io/)
3. [circom电路样例](https://github.com/iden3/circomlib)

## Project 4: SM3的软件实现与优化

### 任务描述
- **a)** 与Project 1类似，从SM3的基本软件实现出发，参考付勇老师的PPT，不断对SM3的软件执行效率进行改进。
- **b)** 基于SM3的实现，验证length-extension attack。
- **c)** 基于SM3的实现，根据RFC6962构建Merkle树（10万叶子节点），并构建叶子的存在性证明和不存在性证明。

## Project 5: SM2的软件实现优化

### 任务描述
- **a)** 考虑到SM2用C语言来做比较复杂，可以考虑用Python来做SM2的基础实现以及各种算法的改进尝试。
- **b)** 针对20250713-wen-sm2-public.pdf中提到的签名算法的误用，基于做POC验证，给出推导文档以及验证代码。
- **c)** 伪造中本聪的数字签名。

## Project 6: Google Password Checkup验证

### 任务描述
- 参考刘巍然老师的报告，尝试实现Google Password Checkup协议，参考论文 [Google Password Checkup](https://eprint.iacr.org/2019/723.pdf) 的Section 3.1，即Figure 2中展示的协议。

