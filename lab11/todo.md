# Lab 11 Todo List

## 1. 总流程先看一眼

```text
检查工具和路径
    ->
编译 Ventus simulator
    ->
编译 driver library
    ->
准备 Q1 截图
    ->
阅读 driver 源码并回答 Q2/Q3
    ->
写 my_link.ld / my_vecadd.s / my_vecmul.s
    ->
编译 .riscv kernel
    ->
写 my_test.cpp
    ->
复制 kernel 到 driver/build
    ->
编译 my_test
    ->
运行并验证 Task A / Task B
    ->
截图 + 整理 report
```

## 2. 这次要交什么

按题面，这次最终至少要交这些：

1. `Question 1`
   - 一张截图
   - 需要同时看到：
     - simulator `make install` 成功
     - driver build artifacts
       - `libspike_driver.so`
       - `spike_test`
     - `whoami`
2. `Question 2`
   - `Ventus driver API flow` 的 8 个步骤
   - 每一步对应的硬件/模拟器层解释
3. `Question 3`
   - `meta_data` 字段解释
   - `Source Exploration Record`
4. `Question 4`
   - `my_test.cpp`
   - `my_vecadd.s`
   - `my_vecmul.s`
   - 一张成功运行截图
   - 截图里要有 `whoami`
   - 还要简述 API 函数及其 simulator-level 作用
5. `Question 5`
   - 解释为什么要写 `tohost`
   - 解释 `HTIF` 怎么处理
   - 解释漏掉 `tohost` 会怎样

## 3. Step 1：确认路径与基础工具

### 要做什么

先确认你所在目录、PDF 所在目录，以及本机是否已有题目要求的工具。

### 为什么这么做

因为这次 lab 同时依赖：

- `git`
- `gcc-11`
- `g++-11`
- `cmake`
- `make`
- `riscv64-unknown-elf-*`

先检查，能避免后面做到一半才发现环境不对。

### 指令

```bash
cd /home/lzh/CSE5030
command -v git
command -v make
command -v cmake
command -v gcc-11
command -v g++-11
command -v riscv64-unknown-elf-as
command -v riscv64-unknown-elf-ld
ls -l
```

### 指令含义

1. `cd /home/lzh/CSE5030`
   - 进入课程工作目录
2. `command -v xxx`
   - 检查某个命令是否存在，并打印其路径
3. `ls -l`
   - 查看当前目录内容和权限

### 做完如何确认

如果这些命令都能打印路径，说明基本工具已就绪。

## 4. Step 2：检查 RISC-V toolchain 和依赖

### 要做什么

按题目要求检查 `RISC-V GNU Toolchain` 和 `device-tree-compiler`。

### 为什么这么做

因为 simulator 和 kernel 编译都依赖这些工具。

### 指令

```bash
riscv64-unknown-elf-as --version
riscv64-unknown-elf-ld --version
gcc-11 --version
sudo apt-get install device-tree-compiler
```

### 指令含义

1. `riscv64-unknown-elf-as --version`
   - 看汇编器是否可用
2. `riscv64-unknown-elf-ld --version`
   - 看链接器是否可用
3. `gcc-11 --version`
   - 确认题目指定的 GCC 11 存在
4. `sudo apt-get install device-tree-compiler`
   - 安装构建 simulator 需要的依赖

### 注意

如果 `gcc-11` 不存在，而只有较新的 GCC，后面 simulator 可能编不过。

## 5. Step 3：克隆 Ventus simulator 仓库

### 要做什么

把 `ventus-gpgpu-isa-simulator` 仓库拉到本地。

### 为什么这么做

因为后面的 simulator、driver、testcase 都在这个仓库里。

### 指令

```bash
cd /home/lzh/CSE5030
git clone https://github.com/THU-DSP-LAB/ventus-gpgpu-isa-simulator --depth=1
cd ventus-gpgpu-isa-simulator
mkdir spike
ls -l
```

### 指令含义

1. `git clone ... --depth=1`
   - 浅克隆仓库，只拉最近历史
2. `cd ventus-gpgpu-isa-simulator`
   - 进入仓库
3. `mkdir spike`
   - 创建安装目标目录
4. `ls -l`
   - 检查目录内容

## 6. Step 4：设置环境变量

### 要做什么

在同一个终端里设置：

- `SPIKE_SRC_DIR`
- `SPIKE_TARGET_DIR`

### 为什么这么做

因为题目明确要求后面命令会反复用这两个变量。

### 指令

```bash
export SPIKE_SRC_DIR=${PWD}
export SPIKE_TARGET_DIR=${PWD}/spike
echo ${SPIKE_SRC_DIR}
echo ${SPIKE_TARGET_DIR}
```

### 指令含义

1. `export SPIKE_SRC_DIR=${PWD}`
   - 把当前仓库根目录记录成源码根目录
2. `export SPIKE_TARGET_DIR=${PWD}/spike`
   - 把安装目标目录记录下来
3. `echo ...`
   - 打印变量，确认它们真被设置了

### 注意

题目提醒：

- 后续最好继续用同一个终端
- 否则这些变量可能丢失

## 7. Step 5：编译 Ventus simulator

### 要做什么

用 `autoconf` 风格流程编 simulator。

### 为什么这么做

因为这是整个 lab 的设备执行环境。

### 指令

```bash
cd ${SPIKE_SRC_DIR}
mkdir -p build
cd build
../configure --prefix=$SPIKE_TARGET_DIR --enable-commitlog
make CC=gcc-11 CXX=g++-11 -j$(nproc)
make install
```

### 指令含义

1. `mkdir -p build`
   - 建立构建目录
2. `../configure --prefix=... --enable-commitlog`
   - 生成 Makefile，并指定安装路径
3. `make CC=gcc-11 CXX=g++-11 -j$(nproc)`
   - 用 GCC 11 并行编译
4. `make install`
   - 把编好的结果安装到 `spike/`

### 做完如何确认

编译结束后再运行：

```bash
${SPIKE_TARGET_DIR}/bin/spike --help
```

如果能正常打印帮助信息，说明 simulator 基本可用。

## 8. Step 6：编译 driver library

### 要做什么

进入 `gpgpu-testcase/driver` 编译 driver。

### 为什么这么做

因为 host 端测试程序后面要链接这个库。

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver
cmake -S . -B build
cmake --build build
ls -l build
```

### 指令含义

1. `cmake -S . -B build`
   - 在 `build/` 里生成构建系统
2. `cmake --build build`
   - 真正执行编译
3. `ls -l build`
   - 看目标文件有没有出来

### 做完如何确认

你要在 `driver/build/` 里看到：

1. `libspike_driver.so`
2. `spike_test`

## 9. Step 7：准备 Q1 截图

### 要做什么

在同一个终端里保留 simulator `make install` 成功信息、driver build 结果，再执行 `whoami`。

### 为什么这么做

因为题目要求一张截图同时包含这些内容。

### 指令

```bash
whoami
ls -l ${SPIKE_SRC_DIR}/gpgpu-testcase/driver/build
```

### 指令含义

1. `whoami`
   - 打印当前用户名
2. `ls -l .../driver/build`
   - 再次显示两个 build artifacts

### 截图检查点

截图里最好同时能看到：

- `make install` 成功的结尾
- `libspike_driver.so`
- `spike_test`
- `whoami`

## 10. Step 8：可选地验证 driver

### 要做什么

用题目给的 `spike_test` 做一次简单验证。

### 为什么这么做

虽然是 optional，但能帮助你确认：

- driver
- `LD_LIBRARY_PATH`
- spike 动态库

这条链没有明显问题。

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver/build
cp ../../vsw_testcase/vecadd.riscv .
LD_LIBRARY_PATH=.:${SPIKE_TARGET_DIR}/lib ./spike_test
```

### 指令含义

1. `cp ...`
   - 把测试 kernel 复制到当前目录
2. `LD_LIBRARY_PATH=.:${SPIKE_TARGET_DIR}/lib ./spike_test`
   - 运行测试时显式指定动态库搜索路径

## 11. Step 9：开始做 Q2，梳理 driver API 8 步流程

### 要做什么

阅读 `driver` 相关源码和头文件，整理：

- 从 `vt_dev_open` 到 `vt_buf_free` 的标准流程

### 为什么这么做

因为 Q2 不是让你只抄函数名，而是要解释：

- 每一步在硬件/模拟器层面发生了什么

### 推荐先搜的内容

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver
rg -n 'vt_dev_open|vt_buf_alloc|vt_copy_to_dev|vt_upload_kernel_file|vt_start|vt_copy_from_dev|vt_buf_free'
```

### 指令含义

- `rg -n '...'`
  - 搜索这些 API 的定义和调用位置，并显示行号

### 你要整理出的核心逻辑

```text
open device
    ->
alloc buffers
    ->
copy input to device
    ->
upload kernel
    ->
start execution
    ->
copy result back
    ->
free buffers
```

## 12. Step 10：继续做 Q3，弄懂 `meta_data`

### 要做什么

重点看：

- `wf_size`
- `wg_size`
- `metaDataBaseAddr`
- `ldsSize`
- `pdsSize`
- `pdsBaseAddr`

### 为什么这么做

因为 Q3 要你解释字段含义和 GPU 存储层次关系。

### 推荐命令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase
rg -n 'struct meta_data|wf_size|wg_size|ldsSize|pdsSize|metaDataBaseAddr|pdsBaseAddr'
```

### 指令含义

- 搜索 `meta_data` 定义和这些字段被使用的位置

### 你重点要想清楚的 3 件事

1. `wf_size` 是每个 warp 多少线程
2. `wg_size` 是每个 workgroup 多少 warp
3. `metaDataBaseAddr` / `pdsBaseAddr` 都是在告诉设备“去哪里找运行时数据”

## 13. Step 11：记录 `Source Exploration Record`

### 要做什么

把你读源码花了多久、看了哪些位置、是否用了 AI 工具记下来。

### 为什么这么做

因为题目明确要求在 Q2-Q3 后提交这一段记录。

### 建议记录模板

```text
Time spent: TODO
Tool/model: TODO
Prompts/questions: TODO
AI helped with: TODO
Manually verified in source: TODO
Mistakes/limitations/lessons: TODO
```

## 14. Step 12：写 `my_link.ld`

### 要做什么

创建 linker script，保证代码、`tohost`、`fromhost` 布局符合题目要求。

### 为什么这么做

因为 kernel 会被加载到固定地址，`tohost` 也必须保留符号。

### 你要写进去的关键点

1. `ENTRY(_start)`
2. `.text` 从 `0x80000000` 开始
3. 保留 `.tohost`
4. 保留 `.fromhost`

### 做完如何确认

后面用 `nm` 检查时，必须能看到：

- `tohost`
- `fromhost`

## 15. Step 13：写 `my_vecadd.s`

### 要做什么

补完题目给的 6 个 TODO。

### 为什么这么做

这是 Task A 的 kernel。

### 你真正要做的动作

1. 设 `sp`
2. `vsetvli` 配置 8 个 `e32`
3. `vle32.v` 载入 A、B
4. `vadd.vv`
5. `vse32.v` 写回 C
6. 写 `tohost`

### 相关英文术语

- `stack pointer`
- `vector length`
- `load vector`
- `element-wise addition`
- `store result`
- `signal completion`

## 16. Step 14：写 `my_vecmul.s`

### 要做什么

从 `my_vecadd.s` 改出 `my_vecmul.s`。

### 为什么这么做

题目明确说除了运算指令外其余逻辑保持一致。

### 关键差异

只把：

```text
vadd.vv
```

换成：

```text
vmul.vv
```

### 你要避免的错误

不要把地址、`vsetvli`、`tohost` 这些也乱改了。

## 17. Step 15：编译两个 kernel

### 要做什么

把 `.s` 编成 `.o`，再链接成 `.riscv`。

### 为什么这么做

driver 上传的是 `.riscv` 二进制，不是源文件。

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase
mkdir -p build
riscv64-unknown-elf-as -march=rv64gcv -o build/my_vecadd.o my_vecadd.s
riscv64-unknown-elf-as -march=rv64gcv -o build/my_vecmul.o my_vecmul.s
riscv64-unknown-elf-ld -T my_link.ld build/my_vecadd.o -o build/my_vecadd.riscv
riscv64-unknown-elf-ld -T my_link.ld build/my_vecmul.o -o build/my_vecmul.riscv
```

### 指令含义

1. `as -march=rv64gcv`
   - 用支持 `RVV` 的目标架构汇编
2. `ld -T my_link.ld`
   - 按自定义链接脚本链接

## 18. Step 16：验证 kernel 二进制

### 要做什么

确认二进制里真的有你要的向量指令和 `tohost/fromhost` 符号。

### 为什么这么做

因为这一步能很快发现：

- 指令没装进去
- 链接脚本没生效
- `tohost` 符号丢了

### 指令

```bash
riscv64-unknown-elf-objdump -d build/my_vecadd.riscv | grep -E 'vsetvli|vle32|vadd|vse32|tohost'
riscv64-unknown-elf-nm build/my_vecadd.riscv | grep -E 'tohost|fromhost'
```

### 指令含义

1. `objdump -d`
   - 反汇编，看机器码里到底有哪些指令
2. `grep -E ...`
   - 筛出你关心的指令名
3. `nm`
   - 查看符号表

### 做完如何确认

你至少要看到：

- `vsetvli`
- `vle32`
- `vadd`
- `vse32`
- `tohost`
- `fromhost`

## 19. Step 17：写 `my_test.cpp`

### 要做什么

补完 host 端测试程序里的所有 TODO。

### 为什么这么做

因为这是把 host 和 device 串起来的核心程序。

### 你要完成的点

1. `vt_dev_open`
2. 三次 `vt_buf_alloc`
3. 两次 `vt_copy_to_dev`
4. `vt_upload_kernel_file`
5. `vt_start`
6. `vt_copy_from_dev`

### 你要理解的逻辑

`main()` 只负责：

- 准备输入
- 调两次 `run_kernel`
- 检查输出

`run_kernel()` 负责：

- 分配设备内存
- 传数据
- 上传 kernel
- 启动执行
- 读回结果

## 20. Step 18：把 `.riscv` 复制到 `driver/build/`

### 要做什么

按题目要求，把两个 kernel 文件复制到 `driver/build/`。

### 为什么这么做

因为后面 `my_test` 是在 `driver/build/` 目录里运行的，当前目录找不到 kernel 文件就会失败。

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase
cp build/my_vecadd.riscv driver/build/
cp build/my_vecmul.riscv driver/build/
ls -l driver/build
```

### 指令含义

1. `cp`
   - 复制文件
2. `ls -l driver/build`
   - 确认两个 `.riscv` 已经在运行目录中

## 21. Step 19：编译 `my_test`

### 要做什么

在 `driver/build/` 目录里编译 host 端测试程序。

### 为什么这么做

因为它要链接当前目录下的 `libspike_driver.so`。

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver/build
g++ -std=c++17 \
    -I../include -L. \
    -o my_test \
    ../../my_test.cpp \
    -lspike_driver \
    -lpthread
```

### 指令含义

1. `-std=c++17`
   - 使用 C++17 标准
2. `-I../include`
   - 指定头文件目录
3. `-L.`
   - 指定当前目录为库搜索目录
4. `-lspike_driver`
   - 链接 driver 动态库
5. `-lpthread`
   - 链接线程库

## 22. Step 20：运行测试程序

### 要做什么

设置动态库路径，运行 `my_test`。

### 为什么这么做

因为运行时需要同时找到：

- `libspike_driver.so`
- spike 安装目录下的相关动态库

### 指令

```bash
cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver/build
LD_LIBRARY_PATH=.:${SPIKE_TARGET_DIR}/lib ./my_test
```

### 指令含义

1. `LD_LIBRARY_PATH=.:${SPIKE_TARGET_DIR}/lib`
   - 临时指定运行时动态库搜索路径
2. `./my_test`
   - 运行你的 host 端测试程序

### 期望输出

你希望看到：

1. `Task A: PASSED`
2. `Task B: PASSED`
3. `All tests passed!`

## 23. Step 21：检查运行日志

### 要做什么

如果程序能跑通，再去看 `.log`。

### 为什么这么做

这样能帮助你确认：

- 向量指令真的执行了

### 指令

```bash
cat my_vecadd.riscv.log
grep -E 'vle32|vadd|vmul|vse32' my_vecadd.riscv.log
```

### 指令含义

1. `cat ...log`
   - 查看完整日志
2. `grep -E ...`
   - 筛出向量相关指令

## 24. Step 22：准备 Q4 截图

### 要做什么

程序成功后，立刻在同一终端执行 `whoami`，然后截图。

### 为什么这么做

因为题目要求截图里同时包含：

- `Task A` 成功
- `Task B` 成功
- `whoami`

### 指令

```bash
whoami
```

### 截图检查点

截图里最好清楚看到：

- `C = A + B: 3 5 7 9 11 13 15 17`
- `Task A: PASSED`
- `C = A * B: 2 6 12 20 30 42 56 72`
- `Task B: PASSED`
- `All tests passed!`
- `whoami`

## 25. Step 23：写 Q4 的 API 解释

### 要做什么

在报告里简述：

- 你调用了哪些 API
- 它们在 simulator 层做了什么

### 为什么这么做

因为 Q4 不只是交代码和截图，还要解释运行过程。

### 建议回答逻辑

1. `vt_dev_open`
   - 初始化模拟设备
2. `vt_buf_alloc`
   - 在设备内存映射区分配缓冲区
3. `vt_copy_to_dev`
   - 把输入数据写入设备地址空间
4. `vt_upload_kernel_file`
   - 把 `.riscv` kernel 装到代码区
5. `vt_start`
   - 按 `meta_data` 启动 kernel
6. `vt_copy_from_dev`
   - 从设备内存读回结果
7. `vt_buf_free`
   - 重置分配，准备下一次运行

## 26. Step 24：写 Q5 的 `tohost` 解释

### 要做什么

回答 3 个问题：

1. 为什么普通 `return` 不行
2. `HTIF` 如何检测 `tohost`
3. 如果漏掉 `tohost` 会怎样

### 为什么这么做

这是这次 lab 对执行模型理解的收尾题。

### 建议答题骨架

```text
(a) bare-metal kernel 没有普通进程返回路径
(b) spike 通过 HTIF 监测 tohost 被写入
(c) 如果不写 tohost，simulator 不知道 kernel 已结束，可能一直跑或超时
```

## 27. 最后的自查清单

```text
[ ] 已检查 gcc-11 / g++-11 / riscv toolchain
[ ] 已克隆 ventus-gpgpu-isa-simulator
[ ] 已设置 SPIKE_SRC_DIR
[ ] 已设置 SPIKE_TARGET_DIR
[ ] 已成功 make install simulator
[ ] 已成功 cmake/build driver
[ ] 已完成 Q1 截图
[ ] 已梳理 Q2 的 8 步 API flow
[ ] 已解释 Q3 的 meta_data 字段
[ ] 已写好 Source Exploration Record
[ ] 已完成 my_link.ld
[ ] 已完成 my_vecadd.s
[ ] 已完成 my_vecmul.s
[ ] 已编出 my_vecadd.riscv
[ ] 已编出 my_vecmul.riscv
[ ] 已完成 my_test.cpp
[ ] 已把两个 .riscv 复制到 driver/build
[ ] 已成功编译 my_test
[ ] 已看到 Task A PASSED
[ ] 已看到 Task B PASSED
[ ] 已完成 Q4 截图
[ ] 已完成 Q5 解释
[ ] 已整理 report.md
```

## 28. 如果你只想按最短路径完成

```bash
cd /home/lzh/CSE5030
git clone https://github.com/THU-DSP-LAB/ventus-gpgpu-isa-simulator --depth=1
cd ventus-gpgpu-isa-simulator
mkdir spike
export SPIKE_SRC_DIR=${PWD}
export SPIKE_TARGET_DIR=${PWD}/spike

riscv64-unknown-elf-as --version
riscv64-unknown-elf-ld --version
gcc-11 --version

mkdir build && cd build
../configure --prefix=$SPIKE_TARGET_DIR --enable-commitlog
make CC=gcc-11 CXX=g++-11 -j$(nproc)
make install

cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver
cmake -S . -B build
cmake --build build

cd ${SPIKE_SRC_DIR}/gpgpu-testcase
mkdir -p build

# 写 my_link.ld / my_vecadd.s / my_vecmul.s / my_test.cpp

riscv64-unknown-elf-as -march=rv64gcv -o build/my_vecadd.o my_vecadd.s
riscv64-unknown-elf-as -march=rv64gcv -o build/my_vecmul.o my_vecmul.s
riscv64-unknown-elf-ld -T my_link.ld build/my_vecadd.o -o build/my_vecadd.riscv
riscv64-unknown-elf-ld -T my_link.ld build/my_vecmul.o -o build/my_vecmul.riscv

cp build/my_vecadd.riscv driver/build/
cp build/my_vecmul.riscv driver/build/

cd ${SPIKE_SRC_DIR}/gpgpu-testcase/driver/build
g++ -std=c++17 -I../include -L. -o my_test ../../my_test.cpp -lspike_driver -lpthread
LD_LIBRARY_PATH=.:${SPIKE_TARGET_DIR}/lib ./my_test
whoami
```

然后把源码、截图和 `report.md` 一起整理提交。
