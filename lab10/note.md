# Lab 10 RISC-V Vector ISA Lab 零基础笔记

## 1. 这次 lab 到底在做什么

这次 lab 的主线是：

```text
配置支持 RVV 的 NEMU
    ->
用 Nexus-AM 编译 bare-metal 程序
    ->
阅读 NEMU 里 RVV 的实现位置
    ->
用 RVV C intrinsics 写两个向量函数
    ->
在 NEMU 上运行并和 scalar 版本对比
```

你要理解的不是“某一条命令怎么敲”，而是 `RVV` 这套向量机制如何从 C 代码、编译器、二进制程序一路落到 NEMU 模拟执行。

## 2. 这次 lab 的最终目标

题面要求你完成 3 类事情：

1. 搭环境
   - `NEMU`
   - `Nexus-AM`
   - `CONFIG_RVV=y`
2. 看源码
   - 找 `vector registers`
   - 找 `vl`
   - 找 `vector masks`
   - 找 `strided/indexed vector memory access`
3. 写程序
   - 用 RVV intrinsics 实现 `matrix-vector multiplication`
   - 用 RVV masks 实现 `branchless lowercase-to-uppercase string conversion`
   - 跑出两个 `PASSED`

## 3. 先记住整体 ASCII 流程图

```text
你的 hello.c
    |
    | make ARCH=riscv64-xs
    v
Nexus-AM build system
    |
    | 生成 bare-metal binary
    v
hello-riscv64-xs.bin
    |
    | NEMU -b 运行
    v
NEMU simulator
    |
    | 解释 RISC-V + RVV 指令
    v
RVV architectural state
    |
    | vector registers / vl / masks / memory access
    v
执行 matrix-vector 和 string conversion
    |
    | scalar result == vector result
    v
Task A PASSED + Task B PASSED
    |
    | whoami
    v
截图作为提交证据
```

## 4. 什么是 NEMU

`NEMU` 是一个模拟器。

在这次 lab 里，它负责：

- 读取你编译出来的 RISC-V binary
- 模拟 RISC-V CPU 的执行
- 模拟 RVV 指令的行为
- 输出程序里的 `printf`

你不是在真实 RISC-V 机器上跑程序，而是在 `NEMU` 里跑。

## 5. 什么是 Nexus-AM

`Nexus-AM` 是 `Abstract Machine` 风格的 bare-metal runtime。

你可以把它理解成：

- 帮你把普通 C 程序打包成 NEMU 能跑的裸机程序
- 提供最小运行环境
- 让 `printf`、`main`、链接脚本、启动代码这些东西能配合起来

所以这次关系是：

```text
Nexus-AM 负责编译和包装程序
NEMU 负责执行程序
```

## 6. 什么是 bare-metal

`bare-metal` 的意思是程序不跑在普通 Linux 用户态进程里。

它通常没有：

- 完整操作系统
- 普通文件系统
- 普通进程退出机制

所以 lab 里用 `Nexus-AM` 提供最小运行环境，再交给 `NEMU` 执行。

## 7. 什么是 ISA

`ISA = Instruction Set Architecture`。

它描述处理器能理解哪些指令，以及这些指令应该如何改变机器状态。

这次涉及两层：

1. `RISC-V`
   - 基础指令集
2. `RVV`
   - `RISC-V Vector Extension`
   - 让一条指令一次处理多个数据元素

## 8. 什么是 scalar 和 vector

`scalar` 是一次处理一个值。

例如：

```text
y = a * x
```

`vector` 是一次处理一组值。

例如：

```text
[y0, y1, y2, y3] = [a0, a1, a2, a3] * [x0, x1, x2, x3]
```

这次 lab 里的对比方式就是：

- scalar 版本作为正确性基准
- vector 版本用 RVV 加速思想实现
- 最后比较两个结果是否一致

## 9. 什么是 RVV

`RVV = RISC-V Vector Extension`。

它的核心思想是：

- 不固定一次必须处理多少个元素
- 程序通过 `vl` 告诉硬件这次实际处理多少个元素
- 硬件根据自己的向量长度能力执行

所以 RVV 不是简单的“固定 4 个一组”或“固定 8 个一组”，而是依赖 `vl`。

## 10. 什么是 vector register

`vector register` 是向量寄存器。

普通整数寄存器像 `x1`、`x2`，一次通常放一个整数。

向量寄存器可以放多个元素，例如：

```text
v0 = [1, 2, 3, 4]
v1 = [10, 20, 30, 40]
```

执行向量加法后：

```text
v2 = v0 + v1 = [11, 22, 33, 44]
```

NEMU 需要在源码里保存这些 vector register 的状态，才能模拟 RVV 指令。

## 11. 什么是 vl

`vl = vector length`。

它表示当前这条或这一组 RVV 指令实际处理多少个元素。

例如：

```text
vl = 4
```

表示后续向量指令只处理 4 个 lane。

这次 `matrix-vector multiplication` 里矩阵大小是 `4`，所以你会设置 32-bit 元素的 `vl` 为最多处理 `MATRIX_SIZE = 4` 个元素。

## 12. 什么是 SEW 和 LMUL

`SEW = Standard Element Width`。

它表示每个元素多少 bit。

常见例子：

- `e8`：每个元素 8 bit，适合处理 `char`
- `e32`：每个元素 32 bit，适合处理 `int32_t`

`LMUL` 表示一组向量操作使用多少个 vector register 的组合容量。

这次题面使用 `LMUL=1`，也就是最基础的配置。

## 13. 什么是 RVV C intrinsics

`RVV C intrinsics` 是 C 语言里的函数形式接口。

它看起来像普通 C 函数：

```c
__riscv_vle32_v_i32m1(...)
```

但编译器会把它翻译成 RVV 指令。

你可以把它理解为：

```text
C 函数写法
    ->
编译器生成 RVV 指令
    ->
NEMU 模拟执行这些 RVV 指令
```

## 14. Task A：matrix-vector multiplication

题目要你实现：

```text
y = A * x
```

矩阵是 `4 x 4`：

```text
A =
1   2   3   4
5   6   7   8
9   10  11  12
13  14  15  16

x = [1, 2, 3, 4]
```

每一行计算一个点积：

```text
y[i] = A[i][0] * x[0]
     + A[i][1] * x[1]
     + A[i][2] * x[2]
     + A[i][3] * x[3]
```

vector 版本的思路是：

```text
加载 A 的一整行
加载 x
逐元素相乘
把乘积向量 reduce 成一个 sum
写入 y[i]
```

## 15. Task A 需要的 intrinsics

常见对应关系：

```text
设置 vl
    -> __riscv_vsetvl_e32m1

加载 int32 向量
    -> __riscv_vle32_v_i32m1

逐元素乘法
    -> __riscv_vmul_vv_i32m1

向量求和 reduction
    -> __riscv_vredsum_vs_i32m1

取出 scalar 结果
    -> __riscv_vmv_x_s_i32m1_i32
```

其中 `reduction` 的意思是把多个元素合并成一个值。

例如：

```text
[1, 4, 9, 16] -> 30
```

## 16. Task B：branchless string case conversion

题目要你把字符串里的小写字母转成大写：

```text
'a' -> 'A'
'b' -> 'B'
```

ASCII 码里，小写字母和对应大写字母差 `32`。

所以 scalar 版本是：

```text
如果 c >= 'a' 且 c <= 'z'
    c = c - 32
```

vector 版本要一次处理多个字符。

## 17. 什么是 mask

`mask` 是“哪些 lane 要执行”的布尔向量。

例如：

```text
chars = ['H', 'e', '1', 'o']
mask  = [ 0,   1,   0,   1 ]
```

含义是：

- `H` 不是小写，不改
- `e` 是小写，改
- `1` 不是小写，不改
- `o` 是小写，改

这就是为什么 RVV 可以做到 `branchless`：

- 不需要每个字符都 `if`
- 用比较指令生成 mask
- 用 masked operation 只改 mask 为真的 lane

## 18. Task B 需要的 intrinsics

常见对应关系：

```text
设置 vl
    -> __riscv_vsetvl_e8m1

加载 uint8 字符
    -> __riscv_vle8_v_u8m1

生成全向量常量
    -> __riscv_vmv_v_x_u8m1

比较 c >= 'a'
    -> __riscv_vmsgeu_vv_u8m1_b8

比较 c <= 'z'
    -> __riscv_vmsleu_vv_u8m1_b8

mask 逻辑与
    -> __riscv_vmand_mm_b8

masked subtraction
    -> __riscv_vsub_vv_u8m1_m

存回字符串
    -> __riscv_vse8_v_u8m1
```

不同版本的 toolchain 可能对 intrinsic 名字支持略有差异，实际写代码时要以编译器报错和 `<riscv_vector.h>` 为准。

## 19. 什么是 strided vector memory access

`strided access` 是按固定步长访问内存。

普通连续访问：

```text
a[0], a[1], a[2], a[3]
```

strided 访问：

```text
a[0], a[2], a[4], a[6]
```

它适合处理数组里有规律但不连续的数据。

## 20. 什么是 indexed vector memory access

`indexed access` 是用一组 index 指定访问位置。

例如：

```text
index = [3, 0, 7, 2]
load a[3], a[0], a[7], a[2]
```

它适合处理不规则数据访问，比如 sparse matrix 或 gather/scatter。

## 21. 为什么 Q1-Q3 要看 NEMU 源码

题面不是只让你“知道概念”，而是让你确认 NEMU 里怎么实现。

所以报告里不能只写：

```text
vl controls vector length.
```

还要写：

```text
我在 NEMU 的某某文件/函数里看到 vl 被读取或更新；
它影响每条 RVV 指令实际循环处理的元素个数。
```

这就是 `source-code exploration`。

## 22. Source Exploration Record 是什么

这是题面要求的源码探索记录。

如果用了 AI 工具，需要写：

1. 用了什么工具和模型
2. 问了什么 prompt
3. AI 帮你找到了什么
4. 你自己在 NEMU 源码里验证了什么
5. AI 有什么错误、限制或经验教训

这部分不是可选项。Q1-Q3 后面题面明确要求报告。

## 23. 这次 lab 最容易错的地方

### 23.1 忘记 `CONFIG_RVV=y`

如果 NEMU 没开 RVV，后面 vector 程序可能无法正常运行。

### 23.2 环境变量换终端后丢失

题面提醒：

```text
export NEMU_HOME=${PWD}
export AM_HOME=${PWD}
```

要在同一个终端继续用，或者写进 shell profile。

### 23.3 `ARCH` 写错

这次目标是：

```text
ARCH=riscv64-xs
```

不是随便一个 RISC-V target。

### 23.4 把 `vl` 当成固定硬编码

RVV 的正确思维是：

```text
还有多少元素要处理
    ->
设置这次 vl
    ->
处理 vl 个
    ->
指针前进 vl
```

字符串任务尤其要这样写，因为字符串长度不一定刚好等于硬件最大向量长度。

### 23.5 masked operation 写成普通 operation

Task B 只能改小写字母。

如果直接对所有字符减 `32`，数字、空格、标点、大写字母都会被破坏。

## 24. 写报告时该怎么组织

最稳妥结构：

1. 环境和截图
2. Q1-Q3 源码定位与解释
3. Source Exploration Record
4. Q4 提交 `hello.c` 和 intrinsics 解释
5. Q5 成功运行截图
6. Q6 scalar vs vector mask 解释

不要额外塞太多无关背景，因为题面要求的是具体问题的答案。

## 25. 这次至少要掌握的词汇

- `NEMU`
- `Nexus-AM`
- `bare-metal`
- `RISC-V`
- `RVV`
- `vector register`
- `vl`
- `SEW`
- `LMUL`
- `intrinsics`
- `scalar`
- `vector`
- `mask`
- `masked operation`
- `branchless`
- `strided access`
- `indexed access`
- `reduction`
- `ARCH=riscv64-xs`
- `CONFIG_RVV=y`

## 26. 最后记住的结论

- `Nexus-AM` 负责编译裸机程序，`NEMU` 负责执行
- `RVV` 的核心是用 `vl` 控制一次处理多少元素
- `vector register` 存一组元素，不是一个元素
- `mask` 让 RVV 在不写分支的情况下只更新部分 lane
- `matrix-vector multiplication` 重点是 load、multiply、reduction
- `string conversion` 重点是 comparison、mask、masked subtraction、store
- 报告里的 Q1-Q3 要结合 NEMU 源码位置回答，不能只写概念
