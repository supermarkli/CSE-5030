# Lab 7 Cache Coherence Lab 零基础笔记

## 1. 这次 lab 到底在做什么

这次 lab 的主线不是单纯把 `gem5` 跑起来，而是把下面 4 件事连成一条线：

1. 用 `gem5` 搭一个 `RISC-V` 多核系统
2. 让这个系统使用 `MOESI_CMP_directory` cache coherence protocol
3. 跑一个会触发 `false sharing` 的多线程程序
4. 从 `stats.txt` 和 `ProtocolTrace` 里观察 cache coherence 带来的行为和代价

你可以把这次 lab 理解成：

```text
多线程程序在多核上同时访问内存
    ->
多个 core 的 cache 里会出现同一份数据的多个副本
    ->
必须靠 cache coherence protocol 保证这些副本不会彼此打架
    ->
如果程序写法不好，就会出现 false sharing
    ->
coherence traffic 变多，性能变差
    ->
我们用 gem5 把这个过程看清楚
```

## 2. 先记住这次 lab 要交什么

PDF 最后要求提交 3 类东西：

1. `gem5` 已经用 `MOESI_CMP_directory` 编译成功的截图
2. 开启 `ProtocolTrace` 后能看到状态转换的截图
3. `false sharing` 的解释、优化代码、以及 bad/good 两个版本的数据对比

所以这次 lab 不是让你空谈概念，而是要你做到：

1. 会确认环境
2. 会运行仿真
3. 会读统计结果
4. 会解释 `false sharing`
5. 会修改代码减少 `false sharing`

## 3. 整体 ASCII 流程图

```text
+----------------------+
| 准备工具链和 gem5 环境 |
+----------------------+
           |
           v
+-------------------------------+
| 确认 gem5 编译协议是 MOESI    |
| PROTOCOL = MOESI_CMP_directory |
+-------------------------------+
           |
           v
+-----------------------------+
| 编译 false_sharing_mt.c     |
| 生成 bad version 可执行文件 |
+-----------------------------+
           |
           v
+-----------------------------------+
| 用 riscv_moesi_riscv.py 跑仿真    |
| 多核 RISC-V + Ruby + MOESI        |
+-----------------------------------+
           |
           v
+-------------------------------+
| 读取 m5out/stats.txt          |
| 看 L1D/L1I cache 统计         |
+-------------------------------+
           |
           v
+----------------------------------+
| 开启 --debug-flags=ProtocolTrace |
| 观察状态机转换                   |
+----------------------------------+
           |
           v
+--------------------------------+
| 分析 false sharing 为什么出现 |
+--------------------------------+
           |
           v
+----------------------------------+
| 修改代码让不同线程避免共用同一 |
| cache line                       |
+----------------------------------+
           |
           v
+----------------------------------+
| 重新编译 good version 并重跑仿真 |
+----------------------------------+
           |
           v
+-----------------------------------+
| 对比 bad / good 的 cache 统计    |
| 写 report 并准备截图提交         |
+-----------------------------------+
```

## 4. 这次 lab 的核心词汇先背下来

- `cache coherence`：缓存一致性，保证多个 cache 看到的数据不会互相矛盾
- `cache line`：缓存按块管理数据，常见大小是 `64B`
- `MOESI`：一种 coherence protocol，状态是 `Modified`、`Owned`、`Exclusive`、`Shared`、`Invalid`
- `directory-based coherence`：目录式一致性，由目录记录谁持有某个 cache line
- `gem5`：计算机体系结构模拟器
- `Ruby`：`gem5` 里更详细、更灵活的 memory system
- `Classic`：`gem5` 里较简单的 cache system
- `SLICC`：描述 cache coherence protocol 状态机的语言
- `ProtocolTrace`：打印协议事件和状态转换的 debug 输出
- `false sharing`：逻辑上没共享变量，但物理上落在同一 cache line，导致 coherence 开销变大

## 5. `gem5` 是什么

`gem5` 是一个体系结构模拟器。它不只是“跑程序”，而是让你在软件里搭建一个虚拟硬件系统，然后观察 CPU、cache、memory、interconnect 的行为。

这次 lab 里你主要把它当成：

- 一个能模拟多核 `RISC-V` 的平台
- 一个能切换 cache coherence protocol 的平台
- 一个能输出详细 cache 统计和状态转换日志的平台

## 6. `Classic` 和 `Ruby` 有什么区别

PDF 里特别强调 `gem5` 有两套 cache 子系统：

### 6.1 `Classic`

特点：

- 上手快
- 配置简单
- 适合快速搭简单 cache hierarchy

缺点：

- 灵活性较差
- protocol 和 cache 实现耦合比较紧
- 不适合细致研究 coherence protocol

### 6.2 `Ruby`

特点：

- 更细致
- 更模块化
- 更适合研究 memory subsystem
- 支持多种 coherence protocol

这次 lab 必须用 `Ruby`，因为 `MOESI_CMP_directory` 就是在 `Ruby` 体系里工作的。

## 7. `SLICC` 是什么

`SLICC` 全称是 `Specification Language including Cache Coherence`。

你可以把它理解成：

- 不是普通应用代码语言
- 而是描述 cache coherence 状态机的专用语言

它的作用是：

1. 用高层方式描述状态和转换
2. 自动生成对应的 `C++` 模拟代码

所以在这次 lab 里，`SLICC_HTML=y` 的意义不是性能，而是帮你生成状态机文档，方便理解协议。

## 8. `SE` 和 `FS` 模式有什么区别

`gem5` 有两个常见运行模式：

### 8.1 `SE` mode

`SE = Syscall Emulation`

特点：

- 不模拟完整操作系统
- 由 `gem5` 拦截 system call，再交给宿主机处理
- 比较轻量
- 适合跑用户程序

这次 lab 用的就是 `SE mode`。

### 8.2 `FS` mode

`FS = Full-System`

特点：

- 模拟完整系统
- 可以跑真正的操作系统
- 支持中断、异常、特权级、I/O 设备
- 更重，但更完整

## 9. 为什么这次 lab 要用多核

因为 `cache coherence` 本来就是多核问题。

如果只有一个 core：

- 不存在多个私有 cache 之间的数据副本同步
- 也就很难体现 coherence protocol 的意义

一旦有多个 core：

- 同一个内存地址可能被多个 core 的 cache 同时持有
- 某个 core 一写，别的 core 的副本就要被更新或失效

这时就需要 `MOESI` 这类协议来管住整个过程。

## 10. `MOESI` 五个状态怎么理解

`MOESI` 的 5 个字母分别是：

- `M = Modified`
- `O = Owned`
- `E = Exclusive`
- `S = Shared`
- `I = Invalid`

先用最直白的话理解：

### 10.1 `Invalid`

这条 cache line 在当前 cache 里无效，不能用。

### 10.2 `Shared`

这条 cache line 可能被多个 cache 同时持有，而且内容和内存一致，通常是只读共享。

### 10.3 `Exclusive`

只有当前 cache 持有这条 line，但它还没被改写，所以和内存一致。

### 10.4 `Modified`

只有当前 cache 持有这条 line，并且已经被改过，内容比内存新。

### 10.5 `Owned`

这是 `MOESI` 比 `MESI` 多出来的一个状态。

可以粗略理解成：

- 这条 line 是共享的
- 但当前 cache 负责持有最新数据
- 其他共享者拿到的是这个最新版本的副本

你现在不需要死记每个边角转换，先抓住重点：

`MOESI` 的目的就是在“多个 cache 有副本”的情况下，保证读写结果仍然正确。

## 11. 什么是 `directory-based coherence`

目录式一致性可以理解成：

- 系统里有一个目录
- 目录知道“哪几个 cache 持有某条 cache line”
- 当某个 core 想读或写这条 line 时，目录决定该通知谁、失效谁、转发给谁

和 `snooping` 相比，它更像“有组织地查表和协调”，而不是所有人一直广播监听。

这次 lab 用的协议名叫 `MOESI_CMP_directory`，名字里已经告诉你：

- 协议是 `MOESI`
- 一致性方式是 `directory`

## 12. 这次给你的 Python 配置脚本在做什么

PDF 里让你建立 `riscv_moesi_riscv.py`，它的本质工作是：

1. 创建一个 `System`
2. 设置时钟、电压、内存大小
3. 建立多个 `TimingSimpleCPU`
4. 调用 `Ruby.create_system()` 建立 `Ruby` memory system
5. 指定协议是编译时已经选好的 `MOESI_CMP_directory`
6. 把 benchmark 程序作为 workload 放进去
7. `m5.instantiate()` 后开始仿真

你可以把这段脚本理解成“这台虚拟机器的装机清单”。

## 13. `TimingSimpleCPU` 是什么

这是 `gem5` 里的一种 CPU 模型。

它不是最复杂的 CPU，但比完全功能型模型更接近时序行为，所以更适合这次用来观察 cache activity。

你现在只要记住：

- 它能产生比较真实的 memory access 行为
- 足够支撑 coherence 观察

## 14. `stats.txt` 为什么重要

跑完仿真以后，结果不会凭空出现在屏幕上，真正完整的数据在 `m5out/stats.txt`。

这份文件里会有：

- cache hit/miss
- Ruby 相关统计
- 各类 performance numbers

所以实验时你要有一个习惯：

```text
终端输出只是“运行情况”
stats.txt 才是“分析依据”
```

## 15. `L1Icache` 和 `L1Dcache` 是什么

- `L1Icache`：一级指令缓存，存放 instruction
- `L1Dcache`：一级数据缓存，存放 data

在这次 `false sharing` 分析里，你更关注 `L1Dcache`，因为冲突来自多个线程对数据的访问和写入。

## 16. `m_demand_hits` / `m_demand_misses` / `m_demand_accesses` 是什么

这几个统计量是本次最常见的分析入口：

- `m_demand_hits`：需求访问命中的次数
- `m_demand_misses`：需求访问 miss 的次数
- `m_demand_accesses`：需求访问总次数

它们关系很直接：

```text
m_demand_accesses = m_demand_hits + m_demand_misses
```

如果你后面要算 miss rate，最朴素的思路就是：

```text
miss rate = misses / accesses
```

## 17. `ProtocolTrace` 是什么

如果只看最终统计，你会知道“结果变差了”，但不一定知道“中间发生了什么”。

`ProtocolTrace` 的作用就是把 coherence protocol 的状态转换过程打印出来，比如：

- 谁发起了请求
- 哪个组件处理了这个请求
- 状态从什么变成什么
- 访问的是哪条地址

所以它像是“协议状态机的运行日志”。

## 18. 协议 trace 每一列怎么看

PDF 里给了一个 trace 示例，你不用逐字背，但要知道这些列的大意：

- `Timestamp`：仿真时刻
- `CPU ID`：哪个核发起的
- `Component`：哪个组件在处理，例如 `L1Cache`、`L2Cache`、`Directory`
- `Event`：发生了什么事件，例如 `GETS`
- `State Transition`：状态怎么变，比如 `IS>S`
- `Address`：涉及的地址或 cache line

你看到 `I>IS_M`、`IS>S`、`M>I` 这类输出时，重点不是全部读懂，而是知道：

```text
状态真的在变
协议真的在工作
```

这就足够支撑题目要求的截图了。

## 19. 什么是 `false sharing`

这是本次 lab 最重要的概念。

`false sharing` 的意思是：

- 线程 A 和线程 B 并没有真的共享同一个变量
- 但它们访问的不同变量碰巧落在同一个 `cache line`
- coherence protocol 只能按 `cache line` 粒度管理
- 所以会把它们当成“共享冲突”来处理

这就是“逻辑上没共享，物理上像共享”。

## 20. 为什么 `false sharing` 会让性能变差

假设两个线程分别写：

- `counter0`
- `counter1`

如果这两个变量在同一条 `64B` cache line 里，就会发生这种事：

1. 线程 0 写 `counter0`
2. 该 cache line 在 core 0 变成更独占的状态
3. 线程 1 再写 `counter1`
4. coherence protocol 发现另一个 core 也持有同一条 line
5. 于是发生 invalidation 或 transfer
6. 两边来回抢这条 line

结果就是：

- coherence traffic 增多
- cache miss 或重取数开销变多
- 程序变慢

虽然两个线程没碰同一个变量，但它们在“抢同一条 line”。

## 21. 这次实验代码里的 `BadCounter` 在哪里坏了

PDF 里的 bad 版本结构体大致是：

```c
struct BadCounter {
    volatile atomic_int counter0;
    volatile atomic_int counter1;
};
```

问题在于：

- `counter0` 和 `counter1` 紧挨着放
- 很大概率在同一个 cache line
- 两个线程又分别不停写它们

于是这就是一个典型的 `false sharing` 演示程序。

## 22. 为什么这是“多线程写不同变量”却仍然有问题

关键点在于 cache coherence 不认识“你的 C 变量概念”，它只认识：

- 地址
- cache line
- 状态

所以它不会说：

```text
这是 counter0，那是 counter1，它们逻辑不同，没事
```

它只会看到：

```text
这两个写操作都落在同一条 cache line
```

于是还是得做一致性维护。

## 23. 怎么消除 `false sharing`

最常见的办法就是让不同线程高频写的变量不要落在同一条 cache line。

思路包括：

1. 加 `padding`
2. 调整结构体布局
3. 给变量做 cache-line alignment
4. 每个线程写自己独立的一整块数据

本质目标只有一个：

```text
把“不同线程频繁写的变量”拆到不同 cache line
```

## 24. 你在这次 lab 里应该怎么理解 bad / good 两个版本

### bad version

- 两个计数器挨得太近
- 两个线程分别写不同计数器
- 但在同一 cache line
- 发生 `false sharing`

### good version

- 通过布局调整或 padding
- 让两个线程写的数据分居不同 cache line
- coherence 冲突减少

所以你后面在报告里不是只写“good 更快”，而是要写：

```text
good version 减少了同一 cache line 在多个 core 之间来回转移
```

## 25. 为什么这次实验要你看 `L1Dcache` 统计

因为 `false sharing` 主要影响的是数据访问。

如果 bad 版本存在严重 coherence 干扰，通常你会看到：

- `L1Dcache` 的行为更差
- miss 或相关访问统计更难看

而 good 版本改善后，相关统计会更健康。

题目没要求你一定只看一个数字，而是要你从数据中说明：

```text
bad 和 good 的 cache 行为确实不同
```

## 26. 如何从 `stats.txt` 里观察 `false sharing`

你不用幻想 `stats.txt` 里会直接跳出一句“here is false sharing”。

正确理解是：

- `false sharing` 是原因
- `stats.txt` 里的 cache miss / access 变化是证据

所以分析步骤应是：

1. 先知道 bad 版本的内存布局容易触发 `false sharing`
2. 再看 `L1Dcache.m_demand_*` 统计
3. 对比 bad 和 good
4. 解释为什么 good 版本更少出现无谓 coherence 开销

## 27. `whoami` 为什么也要截图

这不是技术核心，但它是提交要求的一部分。

老师要求你在同一个终端里跑 `whoami`，通常是为了证明：

- 结果是你自己的环境跑出来的
- 截图不是随手借来的

所以你后面整理截图时，要记得别漏掉这一行。

## 28. 这次 lab 最常见的坑

### 28.1 协议编错了

如果 `--build-info` 里看到的是 `MI_example`，而不是 `MOESI_CMP_directory`，那后面所有实验方向都错了。

### 28.2 内存开太大导致 `mmap` 失败

PDF 提示过，如果出现：

```text
fatal: Could not mmap ...
```

可以把 `--mem-size` 调小。

### 28.3 只看终端，不看 `stats.txt`

这样会导致报告里只有“我跑了”，没有“我分析了”。

### 28.4 把 `ProtocolTrace` 当成必须全部读懂

不是。你只需要知道它展示了状态机转换，并挑出能证明转换发生的截图。

## 29. 这次 lab 的正确理解顺序

建议按这个顺序学：

1. 先理解 `gem5` 是干什么的
2. 再理解为什么这次必须用 `Ruby`
3. 再理解 `MOESI` 和 `directory-based coherence`
4. 再理解 `ProtocolTrace` 和 `stats.txt` 各负责什么
5. 最后理解 `false sharing` 为什么会让 bad version 变慢

## 30. 写报告时最短的答题逻辑

如果你后面写报告卡住，可以直接套这个逻辑：

1. 先说实验现象
2. 再说底层原因
3. 再说你从哪个输出或统计看出来
4. 最后说优化后为什么变好

例如分析 `false sharing` 时可以写成：

```text
bad version 中两个线程虽然写不同变量，
但变量位于同一 cache line，
导致 coherence protocol 频繁失效和转移该 line，
从而带来额外 cache 开销；
good version 把变量拆开后，
L1D cache 相关统计改善，因此性能更好。
```

## 31. 最小结论

这次 lab 你至少要带走 5 个结论：

1. `gem5` 是用来观察体系结构行为的，不只是跑程序
2. `Ruby` 比 `Classic` 更适合研究 cache coherence
3. `MOESI_CMP_directory` 是目录式缓存一致性协议
4. `ProtocolTrace` 看状态转换，`stats.txt` 看统计结果
5. `false sharing` 的本质是“不同线程写不同变量，但它们落在同一 cache line”
