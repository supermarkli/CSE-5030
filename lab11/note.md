# Lab 11 零基础笔记

## 1. 这次 lab 到底在做什么

这次 lab 的主线不是单纯“编译一个程序然后截图”，而是让你把下面这整条链路串起来：

```text
在主机端写 C++ driver
    ->
把输入数据复制到 GPU 设备内存
    ->
把你写的 RISC-V Vector kernel 上传到模拟器
    ->
启动 kernel 执行
    ->
kernel 在设备侧做向量计算
    ->
把结果复制回主机检查
```

这其实就是一个最小版的 `GPGPU` 编程流程。

你会同时接触 3 类东西：

1. `Ventus GPGPU ISA Simulator`
   - 它是一个 GPU 风格的 RISC-V 模拟器
2. `driver API`
   - 它像一个简化版 `CUDA runtime`
3. `RVV kernel`
   - 也就是你自己写的 RISC-V Vector 汇编程序

## 2. 先记住这次 lab 的最终目标

题目想让你会 4 件事：

1. 搭环境并编译 `Ventus` 模拟器和 driver
2. 写两个向量 kernel
   - `vector addition`
   - `vector multiplication`
3. 用 driver API 跑通一次完整执行流程
4. 能解释这套模型和 `CUDA` / `OpenCL` 有什么相似点

## 3. 读前基础词

### 3.1 什么是 GPGPU

`GPGPU = General-Purpose computing on GPU`

意思是：

- 不把 GPU 只当画图硬件
- 而是让 GPU 做通用并行计算

### 3.2 什么是 kernel

`kernel` 在这里不是操作系统内核。

这里的意思是：

- 跑在 GPU 设备上的那段计算程序

你可以把它理解成：

- “真正干活的函数”

### 3.3 什么是 host 和 device

这次 lab 里有两个角色：

1. `host`
   - 就是你的 C++ 测试程序运行的一侧
   - 一般理解成 CPU 这一侧
2. `device`
   - 就是被 driver 控制的 GPU 模拟器这一侧

所以：

- `host` 负责准备数据、启动任务、取回结果
- `device` 负责真正做并行计算

### 3.4 什么是 simulator

`simulator` 就是模拟器。

它不一定是真实硬件，但会尽量模拟硬件行为。

这次你不是在真实 GPU 上跑，而是在 `Ventus GPGPU ISA Simulator` 上跑。

### 3.5 什么是 ISA

`ISA = Instruction Set Architecture`

意思是：

- 处理器能识别哪些指令
- 指令是怎么定义的

这里的重点是：

- `RISC-V`
- `RVV (RISC-V Vector Extension)`

### 3.6 什么是 vector instruction

普通标量指令一次处理一个数。

`vector instruction` 一次能处理一组数。

例如：

```text
C[i] = A[i] + B[i]
```

如果用向量指令，就可以一次处理多个 `i`，而不是一条条做。

## 4. 这次 lab 的整体结构

```text
Step 1: Build simulator + driver
    ->
Step 2: 看懂 driver API 和 meta_data
    ->
Step 3: 写两个 RVV kernel
    ->
Step 4: 写 host 端 my_test.cpp
    ->
Step 5: 跑通 vecadd / vecmul 并截图
    ->
回答 Q2 / Q3 / Q5 的原理问题
```

## 5. 整体 ASCII 流程图

```text
Host(C++ test program)
    |
    | vt_dev_open
    v
Open simulated GPU device
    |
    | vt_buf_alloc
    v
Allocate device memory
0x90000000 -> A
0x90001000 -> B
0x90002000 -> C
    |
    | vt_copy_to_dev
    v
Copy host data A/B to device memory
    |
    | vt_upload_kernel_file
    v
Load kernel binary to device code area
0x80000000
    |
    | vt_start(meta_data)
    v
Simulator starts GPU-style kernel execution
    |
    | RVV instructions
    | vle32.v / vadd.vv or vmul.vv / vse32.v
    v
Kernel writes result to C buffer
    |
    | sw 1 -> tohost
    v
HTIF sees completion signal
sim->run() returns
    |
    | vt_copy_from_dev
    v
Copy result C back to host
    |
    | compare expected output
    v
PASS / FAIL
```

## 6. Step 1 的本质：先把“舞台”搭起来

题目先让你编：

1. `Ventus GPGPU ISA Simulator`
2. `driver library`

为什么分成两部分？

因为它们不是一个东西。

### 6.1 simulator 是什么

`simulator` 负责：

- 模拟设备执行
- 跑你的 `.riscv` kernel
- 解释 `tohost` 信号

### 6.2 driver library 是什么

`driver library` 负责：

- 提供 C++ API
- 帮你在 host 端分配设备内存
- 传数据
- 传 kernel
- 启动执行

所以你可以把关系记成：

```text
driver = 控制器
simulator = 被控制的设备模型
kernel = 设备上运行的程序
```

## 7. 为什么题目强调 GCC 11

题目明确写了：

- `GCC 11 required`
- `GCC 13+ may produce build errors`

这说明：

- 这个项目对编译器版本比较敏感
- 如果系统默认是新版本 GCC，可能会编不过

所以题目才要求：

```text
make CC=gcc-11 CXX=g++-11
```

这不是“多余写法”，而是在锁定可工作的编译器版本。

## 8. 为什么 simulator 用 autoconf，而 driver 用 cmake

这是这次 lab 很容易踩坑的点。

### 8.1 simulator

题目明确说：

- simulator 用 `autoconf`
- 不是 `cmake`

所以它的流程是：

```text
configure -> make -> make install
```

### 8.2 driver

driver 则是：

```text
cmake -S . -B build
cmake --build build
```

所以不要混用构建系统。

## 9. Step 2 的本质：看懂 driver API 在干什么

题目问你：

- `vt_dev_open` 到 `vt_buf_free` 的 8 个步骤是什么
- 每一步在硬件/模拟器层面发生了什么

你可以先记住标准流程：

1. `vt_dev_open`
2. `vt_buf_alloc`
3. `vt_copy_to_dev`
4. `vt_upload_kernel_file`
5. `vt_start`
6. `vt_copy_from_dev`
7. `vt_buf_free`
8. 下一次运行时重新分配和重新上传

严格来说，题面说“from `vt_dev_open` to `vt_buf_free`”，核心链条就是这几步。

### 9.1 `vt_dev_open`

作用：

- 打开并初始化一个模拟设备句柄

直白理解：

- 相当于“连上这块虚拟 GPU”

### 9.2 `vt_buf_alloc`

作用：

- 在设备内存里分配缓冲区

这次题目给了很关键的信息：

- 设备数据内存从 `0x90000000` 一带分配

所以你会看到：

- `A @ 0x90000000`
- `B @ 0x90001000`
- `C @ 0x90002000`

### 9.3 `vt_copy_to_dev`

作用：

- 把主机端数组复制到设备内存

直白理解：

- 把 CPU 这边准备好的输入数据送到“GPU 那边”

### 9.4 `vt_upload_kernel_file`

作用：

- 把 `.riscv` 二进制装到设备代码区

题目给的信息是：

- kernel code loaded at `0x80000000`

所以链接脚本 `my_link.ld` 也把 `.text` 放到 `0x80000000`。

### 9.5 `vt_start`

作用：

- 按 `meta_data` 指定的配置启动 kernel

直白理解：

- 告诉模拟器“现在用这些线程组织方式和资源配置开始跑”

### 9.6 `vt_copy_from_dev`

作用：

- 把设备侧结果复制回主机侧

也就是把 `C` 取回来，再和预期答案比较。

### 9.7 `vt_buf_free`

作用：

- 释放设备缓冲区

题目还特别提醒：

- 在两次 run 之间调用它，避免旧分配影响新运行

## 10. 为什么题目不建议 `vt_dev_close`

题目明确说：

- `Avoid vt_dev_close`
- 因为 `known double-free bug after vt_start`

意思是：

- 这个实验环境里设备关闭逻辑有已知 bug
- 所以这次最稳妥的做法是：
  - 设备只打开一次
  - 两个 kernel 复用同一个 device handle
  - 最后不要显式 `vt_dev_close`

这不是你写法不规范，而是题目给你的规避策略。

## 11. Step 3 的本质：你要写“设备侧汇编程序”

这一步你要写两个文件：

1. `my_vecadd.s`
2. `my_vecmul.s`

它们本质上都是：

- 从设备内存读入 A、B
- 用 RVV 指令做元素级运算
- 把结果写回 C
- 最后写 `tohost` 通知模拟器结束

## 12. 为什么 linker script 很重要

`my_link.ld` 的作用是告诉链接器：

- 代码放哪里
- 数据段放哪里
- `tohost` / `fromhost` 符号放哪里

这里最重要的是：

1. `.text` 从 `0x80000000` 开始
2. 保留 `.tohost`
3. 保留 `.fromhost`

如果没有 `tohost` / `fromhost`：

- `spike` 会告警
- 程序也不能按题目预期干净退出

## 13. 为什么要先设置 `sp`

题目第一条 TODO 是：

- 设置 `stack pointer`

原因很简单：

- 程序开始执行时，不该假设 `sp` 已经是有效值
- 哪怕当前 kernel 代码很短，也应该给它一个合法栈地址

你可以把它理解成：

- 先把运行时最基本的执行环境搭好

## 14. 什么是 `vsetvli`

`vsetvli` 是 RVV 里最关键的配置指令之一。

它用来设置：

- 这次向量操作准备处理多少元素
- 元素宽度是多少
- 向量模式是什么

题目要求：

- `8` elements
- `e32`
- `ta`
- `ma`

所以这一步的核心意思是：

```text
告诉向量单元：
这次按 32-bit 元素来处理
目标向量长度是 8
```

## 15. 什么是 `wf_size`，什么是 `wg_size`

这是 `Question 3` 的重点。

### 15.1 `wf_size`

`wf_size = threads per warp`

意思是：

- 一个 `warp` 里有多少线程

题目里写：

- `wf_size = 8`

### 15.2 `wg_size`

`wg_size = warps per workgroup`

意思是：

- 一个 `workgroup` 里有多少个 warp

所以两者不是一个层级：

```text
thread < warp < workgroup
```

### 15.3 为什么 `wf_size` 决定 `VLEN`

题目给出：

```text
VLEN = wf_size × 32
```

直觉理解：

- 每个线程对应 32-bit lane
- 一个 warp 里有多少线程，就有多少个 32-bit lane

如果：

- `wf_size = 8`

那么：

- `VLEN = 8 × 32 = 256 bits`

这也是为什么题目注释里写：

- `wf_size = 8 -> VLEN = 256`

## 16. 什么是 `ldsSize` 和 `pdsSize`

这两个是 GPU 内存层次里的概念。

### 16.1 `ldsSize`

`LDS = Local Data Share`

可以把它理解成：

- 一个工作组内部可共享的小块本地存储

类似理解：

- GPU 上给同组线程协作用的共享空间

它更像 `shared memory`。

### 16.2 `pdsSize`

`PDS = Private Data Share`

可以粗略理解成：

- 给线程私有状态或私有数据使用的空间

它更像：

- 每个线程自己单独用的那部分存储

### 16.3 为什么题目要你解释它们

因为这正体现了 GPU 的层次化存储思想：

1. 有全局内存
2. 有工作组共享存储
3. 有线程私有存储

不是所有数据都在同一层。

## 17. 什么是 `metaDataBaseAddr` 和 `pdsBaseAddr`

题目要求你解释：

- 为什么 `metaDataBaseAddr` 指向设备内存中的 metadata buffer
- 同时还要设置 `pdsBaseAddr`

你可以这么理解：

### 17.1 `metaDataBaseAddr`

它告诉设备：

- 去哪里读这次 kernel 启动所需的元信息

比如：

- grid/workgroup 配置
- 资源配置
- 运行参数布局

### 17.2 `pdsBaseAddr`

它告诉设备：

- 线程私有数据区域从哪开始

所以这两个地址都像“入口指针”：

- 一个指向元数据
- 一个指向私有数据区

## 18. 什么是 `sgprUsage` 和 `vgprUsage`

从字面就能看出：

1. `sgprUsage`
   - `scalar general-purpose registers` 的使用量
2. `vgprUsage`
   - `vector general-purpose registers` 的使用量

题目虽然没有让你深挖实现，但你要知道它们是在描述：

- 这个 kernel 大概要消耗多少寄存器资源

这和 GPU 能同时调度多少工作项有关。

## 19. 为什么 `tohost` 这么重要

这是 `Question 5` 的核心。

### 19.1 为什么不能直接 `ret`

因为这不是一个“正常函数调用返回”的环境。

这是一个 `bare-metal GPGPU execution model`。

这里没有：

- 操作系统帮你收尾
- 普通用户态进程退出机制
- 完整的函数调用上下文在 host 上等你返回

所以 kernel 跑完后，不能指望“像 C 函数一样 return 回去”。

### 19.2 `tohost` 是什么

`tohost` 是 `spike` / `HTIF` 约定使用的一个通信位置。

kernel 往这里写一个值，相当于告诉模拟器：

```text
我执行完了
```

### 19.3 如果不写 `tohost`

最直观的结果就是：

- 模拟器不知道 kernel 已结束
- `sim->run()` 可能一直等
- 程序可能表现成卡住或超时

## 20. 什么是 HTIF

`HTIF = Host-Target Interface`

你可以把它理解成：

- host 和被模拟目标之间的一种约定接口

这次题目里：

- host 是你的 driver / 测试程序这一边
- target 是 spike 里跑的 kernel 那一边

kernel 写 `tohost`，HTIF 就能检测到这个完成信号。

## 21. 为什么这次是“GPU 风格执行”

虽然你写的是 RISC-V 汇编，但执行模型更像 GPU：

1. 先把 kernel 装到设备侧
2. 再配置工作组织方式
3. 再启动大量数据并行计算

这和普通 CPU 程序直接 `main()` 开始跑很不一样。

## 22. `vecadd` 和 `vecmul` 真正在验证什么

表面上它们只是：

1. `C = A + B`
2. `C = A * B`

但实际上它们在验证整条链是否打通：

1. 汇编是否正确
2. 链接地址是否正确
3. kernel 是否成功上传
4. 设备内存分配是否正确
5. 输入数据是否正确复制到设备
6. 结果是否正确复制回 host
7. `tohost` 退出机制是否正确

所以如果最后 `PASS`，说明不是只对了一条指令，而是整套流程基本通了。

## 23. 这次 lab 和 CUDA / OpenCL 有什么相似点

题目最后让你比较编程模型，最核心的相似点是：

1. 都有 `host` 和 `device` 分工
2. 都要先分配 device memory
3. 都要 host -> device 拷数据
4. 都要上传或指定 kernel
5. 都要设置执行配置
6. 都要启动 kernel
7. 都要把结果取回 host 验证

所以你完全可以把 Ventus driver API 理解成：

- 一个简化版的 `CUDA/OpenCL` 风格运行时

## 24. 这次 lab 最容易错的地方

### 24.1 构建系统用错

- simulator 用 `configure + make`
- driver 用 `cmake`

### 24.2 GCC 版本不对

- 要优先用 `gcc-11/g++-11`

### 24.3 环境变量没在同一终端保留

题目提醒：

- 设完 `SPIKE_SRC_DIR`、`SPIKE_TARGET_DIR` 后最好在同一终端继续

### 24.4 忘了复制 `.riscv` 到 `driver/build/`

题目明确要求：

- 先把 `my_vecadd.riscv` 和 `my_vecmul.riscv` 复制到 `driver/build/`

否则 `my_test` 在当前目录可能找不到 kernel 文件。

### 24.5 忘了 `tohost`

- 会导致 spike 不能正常结束

### 24.6 错把 `vt_dev_close` 当成必须步骤

- 这次题目明确建议不要调它

## 25. 你写报告时该怎么组织答案

建议始终按下面逻辑写：

1. 先说 API 或机制做了什么
2. 再说模拟器/硬件层面发生了什么
3. 再说为什么这样设计
4. 最后用这次 `vecadd/vecmul` 例子落地

例如回答 `Question 5` 时，不要只写：

- “因为题目这么要求”

而要写成：

```text
没有操作系统返回路径
    ->
kernel 必须显式通知完成
    ->
HTIF 通过 tohost 检测完成
    ->
否则 sim->run() 不知道何时退出
```

## 26. 这次 lab 你至少要掌握的词汇

- `GPGPU`
- `host`
- `device`
- `kernel`
- `simulator`
- `driver API`
- `RVV`
- `vsetvli`
- `vle32.v`
- `vadd.vv`
- `vmul.vv`
- `vse32.v`
- `warp`
- `workgroup`
- `wf_size`
- `wg_size`
- `VLEN`
- `LDS`
- `PDS`
- `HTIF`
- `tohost`
- `bare-metal`

## 27. 现在最该记住的结论

- 这次 lab 的核心不是公式，而是把 `host -> device -> kernel -> result` 这条链打通
- `driver API` 像简化版 `CUDA/OpenCL runtime`
- 设备内存和代码区是分开的：数据在 `0x90000000` 一带，代码在 `0x80000000`
- `wf_size` 决定一个 warp 的线程数，因此也决定向量长度
- `wg_size` 决定一个 workgroup 里有多少个 warp
- `ldsSize` 和 `pdsSize` 对应 GPU 存储层次里的共享/私有空间概念
- kernel 结束不能靠普通 `return`，而要靠写 `tohost`
- `vecadd` 和 `vecmul` 表面简单，但实际上是在验证整套执行链路是否正确

## 28. 这份笔记怎么用最有效

建议顺序：

1. 先看第 1 到第 10 节，建立整体图景
2. 写 `Question 2/3` 前重点看第 9、15、16、17、18 节
3. 写 `my_vecadd.s` / `my_vecmul.s` 前重点看第 11、12、13、14、19 节
4. 写 `Question 5` 前重点看第 19、20、25 节

如果你读完后能自己回答下面 4 句，说明这次 lab 的主线你已经抓住了：

1. 为什么这次需要同时写 host 程序和 device kernel
2. 为什么 `wf_size` 和 `wg_size` 不是一回事
3. 为什么 kernel 结束要写 `tohost`
4. 为什么 `vecadd` 成功不只是说明加法对了，而是说明整条驱动链路通了
