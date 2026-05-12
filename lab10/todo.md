# Lab 10 Todo List

## 1. 总流程

```text
检查工具和路径
    ->
准备 NEMU
    ->
确认 CONFIG_RVV=y
    ->
准备 Nexus-AM
    ->
跑 coremark 验证环境
    ->
阅读 NEMU RVV 源码
    ->
填写 hello.c 的 8 个 TODO
    ->
编译 hello
    ->
用 NEMU 运行 hello
    ->
截图 + whoami
    ->
整理 report.md
```

## 2. 最终要交什么

按题面，最终至少要有：

1. `Question 1`
   - vector registers 和 `vl` 的源码位置/函数
   - 解释 `vl` 如何影响一条 RVV 指令执行的元素数量
2. `Question 2`
   - vector masks 的源码位置/函数
   - 解释 masked operation 和 unmasked operation 在 NEMU 中有什么区别
3. `Question 3`
   - strided 或 indexed vector memory access 的源码位置/函数
   - 解释这些访问模式为什么适合 data-level parallelism
4. `Source Exploration Record`
   - 源码探索花费时间
   - 如果用 AI，写工具、模型、prompt、帮助、人工验证、错误和经验
5. `Question 4`
   - 完成后的 `hello.c`
   - 简述 load、multiply、reduction、comparison、masked subtraction、store 使用的 RVV intrinsics
6. `Question 5`
   - 成功运行截图
   - 同一终端里包含 `whoami`
7. `Question 6`
   - 比较 scalar 和 vector string conversion
   - 解释 RVV 为什么能避免 branches 但只更新小写字母

## 3. Step 1：确认路径和基础工具

### 要做什么

确认当前目录、PDF、基础构建工具是否存在。

### 为什么这么做

后面要克隆、编译、搜索源码。先预检能避免中途才发现基础工具缺失。

### 指令

```bash
cd /home/lzh/CSE5030
ls -l /home/lzh/CSE5030/lab10/lab10.pdf
command -v git
command -v make
command -v gcc
command -v g++
command -v grep
```

### 指令含义

- `cd /home/lzh/CSE5030`：进入课程仓库根目录
- `ls -l ...lab10.pdf`：确认题目 PDF 存在
- `command -v 工具名`：查看该工具是否能在当前 `PATH` 中找到

## 4. Step 2：准备 NEMU

### 要做什么

如果本地还没有可用的 OpenXiangShan NEMU，就按题面克隆指定仓库和 commit。

### 为什么这么做

题面指定了 NEMU commit，报告和源码定位最好基于同一版本，避免文件名或实现细节不一致。

### 指令

```bash
cd /home/lzh/CSE5030
git clone git@github.com:OpenXiangShan/NEMU.git
cd NEMU
git checkout 0860de24253ef2097663fe64095b0fc329515489
git submodule update --init --recursive
export NEMU_HOME=${PWD}
```

### 指令含义

- `git clone ...`：下载 NEMU 源码
- `git checkout ...`：切换到题面指定 commit
- `git submodule update --init --recursive`：下载 NEMU 依赖的子模块
- `export NEMU_HOME=${PWD}`：把当前 NEMU 路径保存为环境变量

## 5. Step 3：安装 NEMU 构建依赖

### 要做什么

安装题面列出的依赖包。

### 为什么这么做

NEMU 构建会用到编译器、readline、SDL2、zstd、bison、flex 等库或工具。

### 指令

```bash
sudo apt install build-essential man gcc gdb git libreadline-dev libsdl2-dev zstd libzstd-dev bison flex
```

### 指令含义

- `sudo apt install`：用系统包管理器安装软件包
- `build-essential`：提供常用 C/C++ 编译工具
- `libreadline-dev`：提供命令行交互库
- `libsdl2-dev`：提供 SDL2 开发库
- `zstd libzstd-dev`：提供压缩相关库
- `bison flex`：提供语法/词法生成工具

## 6. Step 4：配置并编译 NEMU

### 要做什么

使用 `riscv64-xs_defconfig` 配置 NEMU，然后编译。

### 为什么这么做

题面目标是 `ARCH=riscv64-xs`，NEMU 要先按对应配置编好。

### 指令

```bash
cd ${NEMU_HOME}
make riscv64-xs_defconfig
make
```

### 指令含义

- `make riscv64-xs_defconfig`：生成适合 `riscv64-xs` 的默认配置
- `make`：按当前配置编译 NEMU

## 7. Step 5：确认 RVV 已开启

### 要做什么

检查 `.config` 里是否有 `CONFIG_RVV=y`。

### 为什么这么做

这次 lab 依赖 RVV。没有打开 RVV，后面的 vector intrinsic 程序没有实验意义。

### 指令

```bash
cd ${NEMU_HOME}
grep "RVV" -ri .config
```

### 指令含义

- `grep "RVV" -ri .config`：在 `.config` 中查找所有包含 `RVV` 的配置行

### 期望看到

```text
CONFIG_RVV=y
CONFIG_RVV_AGNOSTIC=y
```

## 8. Step 6：准备 Nexus-AM

### 要做什么

克隆 `nexus-am` 并设置 `AM_HOME`。

### 为什么这么做

Nexus-AM 负责把应用程序编译成 NEMU 可运行的 bare-metal binary。

### 指令

```bash
cd /home/lzh/CSE5030
git clone https://github.com/OpenXiangShan/nexus-am.git --depth=1
cd nexus-am
export AM_HOME=${PWD}
```

### 指令含义

- `git clone ... --depth=1`：只下载最新历史，速度更快
- `export AM_HOME=${PWD}`：把当前 Nexus-AM 路径保存为环境变量

## 9. Step 7：用 coremark 验证 Nexus-AM 和 NEMU

### 要做什么

先编译并运行 `apps/coremark`。

### 为什么这么做

在写 RVV 程序前，先证明基础环境能编译、能被 NEMU 跑起来。

### 指令

```bash
cd ${AM_HOME}/apps/coremark
make ARCH=riscv64-xs
${NEMU_HOME}/build/riscv64-nemu-interpreter -b ./build/coremark-riscv64-xs.bin
```

### 指令含义

- `make ARCH=riscv64-xs`：按 `riscv64-xs` 目标编译 coremark
- `${NEMU_HOME}/build/riscv64-nemu-interpreter`：运行 NEMU 解释器
- `-b`：batch 模式运行，不进入交互调试
- `./build/coremark-riscv64-xs.bin`：要运行的裸机二进制文件

## 10. Step 8：准备源码探索记录

### 要做什么

在开始看 NEMU 源码前，记录开始时间、搜索关键词、最终核对文件。

### 为什么这么做

题面要求 `Source Exploration Record`。后面补写不如现在边做边记准确。

### 建议记录

```text
Start time: TODO
Search keywords:
- vl
- mask
- vreg
- strided
- indexed
- segment
- load
- store
Files checked:
- TODO
```

## 11. Step 9：搜索 vector registers 和 vl

### 要做什么

在 NEMU 源码中搜索 `vl`、`vreg`、`vector` 等关键词。

### 为什么这么做

Q1 要你定位 vector registers 和 `vl` 的实现位置。

### 指令

```bash
cd ${NEMU_HOME}
rg -n "vl|vreg|vector|vstart|vtype" src
```

### 指令含义

- `rg -n`：递归搜索并显示行号
- `"vl|vreg|vector|vstart|vtype"`：正则关键词，查找 RVV 相关状态
- `src`：只在源码目录中搜索

### 你要写进报告的内容

```text
Q1:
- Source files/functions: TODO
- Explanation: TODO
```

## 12. Step 10：理解 vl 的作用

### 要做什么

找到 RVV 指令执行循环里哪里使用 `vl` 控制 lane 数量。

### 为什么这么做

Q1 不只问 `vl` 存在哪，还问它如何影响一条 RVV 指令处理多少元素。

### 指令

```bash
cd ${NEMU_HOME}
rg -n "for .*vl|vl .*for|vlmax|vsetvl|vsetvli" src
```

### 指令含义

- 搜索设置或使用 `vl` 的代码位置
- 重点看 `vsetvl/vsetvli` 以及 vector instruction 执行循环

## 13. Step 11：搜索 vector masks

### 要做什么

搜索 mask 相关实现。

### 为什么这么做

Q2 要你解释 masked operation 和 unmasked operation 在 NEMU 中的区别。

### 指令

```bash
cd ${NEMU_HOME}
rg -n "mask|vmask|v0|masked|unmasked" src
```

### 指令含义

- `mask`：查找掩码逻辑
- `v0`：RVV 中常见的 mask register
- `masked/unmasked`：查找是否有显式分支或宏区分两类操作

### 你要重点理解

```text
unmasked operation:
    每个 active lane 都执行

masked operation:
    只有 mask 对应 bit 为 true 的 lane 执行
    mask 为 false 的 lane 按 policy 保持或变成 agnostic
```

## 14. Step 12：搜索 strided/indexed memory access

### 要做什么

搜索 RVV load/store 中 strided 和 indexed 相关实现。

### 为什么这么做

Q3 要你找到 NEMU 在哪里处理这些访问模式，并解释用途。

### 指令

```bash
cd ${NEMU_HOME}
rg -n "strid|stride|index|indexed|gather|scatter|vlse|vsse|vlux|vloxe|vsux|vsox" src
```

### 指令含义

- `stride/strid`：查找步长访问
- `index/indexed`：查找索引访问
- `vlse/vsse`：RVV strided load/store 指令名线索
- `vlux/vlox/vsux/vsox`：RVV indexed load/store 指令名线索

## 15. Step 13：整理 Q1-Q3 答案草稿

### 要做什么

把源码位置和解释先写进 `report.md` 的 TODO 位置。

### 为什么这么做

源码定位最容易忘，应该趁刚看完时记录。

### 建议格式

```text
Source files/functions:
- TODO

Explanation:
TODO
```

## 16. Step 14：准备 hello.c

### 要做什么

进入 Nexus-AM 的 `apps/hello`，按题面替换或创建 `hello.c`。

### 为什么这么做

Step 3 要求在 Nexus-AM application 目录中编译运行 RVV 程序。

### 指令

```bash
cd ${AM_HOME}/apps/hello
ls -l
```

### 指令含义

- `cd ${AM_HOME}/apps/hello`：进入 hello 应用目录
- `ls -l`：确认目录内容，避免写错路径

## 17. Step 15：填写 Task A 的 TODO 1-3

### 要做什么

实现 matrix-vector vector 版本的加载、乘法、reduction 前半部分。

### 为什么这么做

Task A 的核心是把一行矩阵和向量 `x` 同时加载进 vector register，然后逐元素相乘。

### 需要填的逻辑

```text
TODO 1:
设置 e32m1 的 vl，元素数是 MATRIX_SIZE

TODO 2:
加载 A[i] 这一行
加载 x

TODO 3:
vec_row 和 vec_x 做逐元素乘法
```

### 常用 intrinsics

```text
__riscv_vsetvl_e32m1
__riscv_vle32_v_i32m1
__riscv_vmul_vv_i32m1
```

## 18. Step 16：理解 Task A 的 reduction

### 要做什么

确认题面 skeleton 里的 reduction 代码含义。

### 为什么这么做

逐元素乘法后得到的是一个向量，矩阵乘法需要的是一个 sum。

### 代码含义

```text
__riscv_vmv_s_x_i32m1(0, vl)
    创建初始 sum = 0

__riscv_vredsum_vs_i32m1(...)
    把 vec_mul 的所有 active lane 加起来

__riscv_vmv_x_s_i32m1_i32(...)
    从向量结果里取出 scalar sum
```

## 19. Step 17：填写 Task B 的 TODO 4-8

### 要做什么

实现 vector string conversion。

### 为什么这么做

Task B 要展示 RVV mask 的用法：不写分支，也只修改小写字母。

### 需要填的逻辑

```text
TODO 4:
按剩余 len 设置 e8m1 的 vl

TODO 5:
从 str 加载 vl 个字符

TODO 6:
生成 mask:
    c >= 'a'
    c <= 'z'
    两个 mask 做 and

TODO 7:
只对 mask 为 true 的 lane 减 32

TODO 8:
把字符向量存回 str
```

### 常用 intrinsics

```text
__riscv_vsetvl_e8m1
__riscv_vle8_v_u8m1
__riscv_vmsgeu_vv_u8m1_b8
__riscv_vmsleu_vv_u8m1_b8
__riscv_vmand_mm_b8
__riscv_vsub_vv_u8m1_m
__riscv_vse8_v_u8m1
```

## 20. Step 18：编译 hello

### 要做什么

用 Nexus-AM 编译 hello。

### 为什么这么做

这一步会把 C 代码和 RVV intrinsics 编译成 NEMU 可运行的 binary。

### 指令

```bash
cd ${AM_HOME}/apps/hello
make ARCH=riscv64-xs
```

### 指令含义

- `make`：调用 Nexus-AM 的构建系统
- `ARCH=riscv64-xs`：指定目标平台

## 21. Step 19：运行 hello

### 要做什么

用 NEMU 跑编译出的 hello binary。

### 为什么这么做

题面要求比较 scalar 和 vector 实现，并截图证明成功运行。

### 指令

```bash
${NEMU_HOME}/build/riscv64-nemu-interpreter -b ./build/hello-riscv64-xs.bin
```

### 指令含义

- `${NEMU_HOME}/build/riscv64-nemu-interpreter`：NEMU 可执行文件
- `-b`：batch 模式运行
- `./build/hello-riscv64-xs.bin`：要执行的 hello 程序

### 期望输出

```text
Task A PASSED: Scalar and vector implementations match!
Task B PASSED: Scalar and vector implementations match!
```

## 22. Step 20：截图前运行 whoami

### 要做什么

程序运行成功后，在同一个终端立刻执行 `whoami`。

### 为什么这么做

题面 Q5 明确要求截图里包含成功输出和 `whoami`。

### 指令

```bash
whoami
```

### 截图检查点

截图中应清楚包含：

```text
Task A PASSED
Task B PASSED
whoami
lzh
```

## 23. Step 21：填写 Question 4

### 要做什么

在报告中说明你用了哪些 RVV intrinsics。

### 为什么这么做

Q4 不只要求交 `hello.c`，还要求简要解释 intrinsics 分工。

### 建议填写结构

```text
Load:
TODO

Multiply:
TODO

Reduction:
TODO

Comparison:
TODO

Masked subtraction:
TODO

Store:
TODO
```

## 24. Step 22：填写 Question 6

### 要做什么

解释 scalar 和 vector 字符串转换的区别。

### 为什么这么做

Q6 是对 mask 理解的总结。

### 建议答题逻辑

```text
scalar:
    每个字符执行 if 判断
    是小写才减 32

vector:
    一次加载多个字符
    用比较生成 mask
    masked subtraction 只修改小写字母 lane
    所以避免了逐字符 branch
```

## 25. Step 23：整理 Source Exploration Record

### 要做什么

把源码探索过程写入 report。

### 为什么这么做

题面在 Q1-Q3 后明确要求提交这部分。

### 建议填写

```text
Total time spent:
TODO

Tool name and model:
OpenAI Codex / TODO

Prompts:
TODO

What AI helped find:
TODO

What I verified manually:
TODO

Mistakes, limitations, lessons:
TODO
```

## 26. Step 24：最终复查 report

### 要做什么

对照 PDF 检查 `report.md` 是否只包含题面要求内容。

### 为什么这么做

用户要求 report 不冗余，所以不应该加入过长背景笔记。

### 自查清单

```text
[ ] 有 ID / NAME
[ ] Q1 有源码位置和 vl 解释
[ ] Q2 有 mask 源码位置和 masked/unmasked 解释
[ ] Q3 有 strided/indexed memory access 源码位置和用途解释
[ ] 有 Source Exploration Record
[ ] Q4 有 hello.c 提交说明和 intrinsics 解释
[ ] Q5 有成功截图占位和 whoami 要求
[ ] Q6 有 scalar vs vector string conversion 解释
[ ] 所有未完成答案都用 TODO 占位
[ ] 没有加入 PDF 没要求的长篇背景
```

## 27. 最短执行命令汇总

```bash
cd /home/lzh/CSE5030
git clone git@github.com:OpenXiangShan/NEMU.git
cd NEMU
git checkout 0860de24253ef2097663fe64095b0fc329515489
git submodule update --init --recursive
export NEMU_HOME=${PWD}
make riscv64-xs_defconfig
make
grep "RVV" -ri .config

cd /home/lzh/CSE5030
git clone https://github.com/OpenXiangShan/nexus-am.git --depth=1
cd nexus-am
export AM_HOME=${PWD}

cd ${AM_HOME}/apps/coremark
make ARCH=riscv64-xs
${NEMU_HOME}/build/riscv64-nemu-interpreter -b ./build/coremark-riscv64-xs.bin

cd ${AM_HOME}/apps/hello
# 按题面填写 hello.c
make ARCH=riscv64-xs
${NEMU_HOME}/build/riscv64-nemu-interpreter -b ./build/hello-riscv64-xs.bin
whoami
```
