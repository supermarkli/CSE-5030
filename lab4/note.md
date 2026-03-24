# Lab 4 笔记：Pipeline Hazards Visualization

## 1. 这次 lab 到底要做什么

这次 lab 不是让你设计一个新的 CPU，而是让你：

1. 在 RocketCore 里加一点调试输出，让 CPU 在仿真时把 `bypass` 信息打印出来。
2. 自己写一段 RISC-V 汇编，人为制造 3 类典型流水线冲突：
   - Data Hazard
   - Load-Use Data Hazard
   - Control Hazard
3. 跑仿真，看日志，解释 CPU 为什么会停顿、为什么会旁路、分支默认预测到底是什么。

你最后要交的东西，本质上只有两类：

1. 证据
   - 你的汇编代码截图
   - 仿真日志截图
2. 分析
   - 每种 hazard 发生了什么
   - 为什么会这样
   - 你怎么从日志里判断出来

## 2. 先建立最小背景：什么是流水线

CPU 不会傻等一条指令完全做完再做下一条，而是把一条指令拆成几个阶段，让多条指令重叠执行，这就叫 `pipeline`。

可以把它理解成工厂流水线：

```text
指令1: IF -> ID -> EX -> MEM -> WB
指令2:      IF -> ID -> EX -> MEM -> WB
指令3:           IF -> ID -> EX -> MEM -> WB
```

常见 5 个阶段：

1. `IF`
   - Instruction Fetch
   - 取指令
2. `ID`
   - Instruction Decode
   - 解码、读寄存器
3. `EX`
   - Execute
   - 做算术运算、算地址、比较分支
4. `MEM`
   - 访问数据内存
   - `lw/sw` 之类主要在这里工作
5. `WB`
   - Write Back
   - 把结果写回寄存器

理想情况：每个周期都能推进一条指令。

但现实里会有冲突，导致：

1. 停顿 `stall`
2. 旁路 `bypass` / `forwarding`
3. 错误路径指令被清掉 `flush/kill`

这就是这次 lab 的重点。

## 3. Hazard 是什么

`hazard` 就是“如果照常推进流水线，会出错或者效率下降”的情况。

这次只关注 3 类。

### 3.1 Data Hazard

后一条指令要读一个寄存器，但前一条指令刚好要写这个寄存器。

例子：

```asm
addi t0, zero, 5
add  t1, t0, t0
```

第二条 `add` 需要读 `t0`，但 `t0` 是第一条刚算出来的。

问题：如果等到第一条真的在 `WB` 阶段写回寄存器，第二条就太晚了。

解决办法之一：`bypass`

也就是不等写回寄存器，直接把前面阶段刚算出来的值“转发”给后面的指令。

### 3.2 Load-Use Data Hazard

这是 data hazard 的特殊版本，也是更麻烦的一种。

例子：

```asm
lw   t0, 0(s0)
add  t1, t0, t0
```

这里 `t0` 来自 `lw`。问题在于：

1. `lw` 的数据不是在 `EX` 阶段就得到
2. 它通常要到 `MEM` 阶段访问内存后才真正拿到值

所以紧跟着的下一条指令太早了，旁路也来不及。

结果：通常必须 `stall` 一个周期或更多周期。

### 3.3 Control Hazard

分支指令会改变 PC，导致 CPU 不知道“下一条应该取哪条指令”。

例子：

```asm
beq  t0, t1, label
addi t2, zero, 1
label:
```

在分支结果还没确定前，CPU 先猜一下，这叫 `branch prediction`。

如果猜错了，就要把错路径上的指令清掉，再跳去正确位置。

这就叫 control hazard。

## 4. 什么是 bypass

`bypass` 或 `forwarding` 的意思是：

前一条指令的结果还没正式写回寄存器，但后一条已经急着要用了，于是 CPU 直接从流水线里的某个阶段把结果送过去。

ASCII 理解：

```text
正常:
生产结果 -> 写回寄存器 -> 下一条再读寄存器

bypass:
生产结果 -> 直接转给下一条使用
```

这次你加的调试代码会打印：

```text
[BYPASS] PC=0x..., rs0=x10 (src=EX)
```

含义：

1. `PC=0x...`
   - 当前“接收被旁路数据”的这条指令的 PC
2. `rs0=x10`
   - 表示这条指令的源寄存器之一用了 `x10`
   - `rs0` / `rs1` 是代码里打印时对第 0 个或第 1 个源操作数的标记，不是 RISC-V 正式寄存器名的一部分
3. `src=EX/WB/MEM`
   - 数据是从哪个流水段转发过来的

### 为什么会有 EX / MEM / WB 三种来源

因为不同指令在不同阶段才产生可用结果。

1. `EX`
   - 算术类指令通常在 EX 阶段就算出来
2. `MEM`
   - 某些值可能在 MEM 阶段可用
3. `WB`
   - 更晚的时候才写回，也可能从这里转发

你要结合你的指令顺序判断为什么是这个来源。

## 5. 什么是 stall

`stall` 就是流水线暂停一下，不让后面的指令继续推进。

原因很简单：数据还没准备好，硬推会算错。

判断 stall 最直接的方法：

1. 看日志里的周期号，比如 `C0: 37`、`C0: 38`
2. 比较相关指令在 `WB` 阶段出现的时间差

如果两条本该连续推进的指令之间多了额外周期，通常就是发生了 stall。

### 这次 lab 里怎么观察 stall

重点看 `load-use hazard`：

```asm
lw   t0, 0(s0)
add  t1, t0, t0
```

如果第二条依赖第一条刚加载出来的数据，通常会产生停顿。

## 6. Control Hazard 里要看什么

题目明确说：

1. RocketCore 使用动态分支预测
2. 但在分支历史还没建立前，会回退到静态预测
3. 你要分析这个静态预测默认是 `taken` 还是 `not taken`

### taken / not taken 是什么

1. `taken`
   - 分支成立，跳转到目标地址
2. `not taken`
   - 分支不跳转，继续执行下一条顺序指令

### 为什么题目让你同时构造 taken branch 和 not taken branch

因为只看一种情况，很难判断“默认猜法”到底是什么。

你需要同时构造：

1. 一个实际会跳的分支
2. 一个实际不会跳的分支

然后看日志表现，判断 CPU 最开始默认猜的是哪边。

### 为什么题目说不推荐 loop

因为循环会快速形成分支历史，动态预测器学会之后，你看到的就不再是“默认静态策略”，而是“训练后的动态策略”。

题目想让你分析的是“还没历史时”的默认行为，所以不要用 loop。

## 7. 这次会改哪个文件，为什么

要改：

`chipyard/generators/rocket-chip/src/main/scala/rocket/RocketCore.scala`

原因：

你不是在改 CPU 功能，而是在加 `printf` 调试输出，让仿真时看到 bypass 信息。

题目给的代码核心作用是：

1. 从当前 EX 阶段指令里取出 `rs1` 和 `rs2`
2. 检查这两个源操作数是不是用了 bypass
3. 如果用了，就打印来源是 `EX`、`WB` 还是 `MEM`

## 8. 先补工程背景：Chipyard、Rocket Chip、RocketCore 到底是什么

如果你以前主要写的是普通程序，第一次看到这类路径：

`chipyard/generators/rocket-chip/src/main/scala/rocket/RocketCore.scala`

很容易懵，因为它看起来不像普通软件项目。

这很正常。这个工程本质上是一个“硬件生成工程”，不是单纯的 C/C++ 或 Python 项目。

### 8.1 Chipyard 是什么

`Chipyard` 是一个用来搭建和生成 RISC-V SoC 的开源框架。

可以把它理解成：

```text
Chipyard = 一个大平台
         = 帮你组合 CPU、缓存、总线、外设、仿真环境
         = 最后生成可仿真的硬件设计
```

题目仓库里的 `chipyard/README.md` 也明确说明了这一点：

1. Chipyard 是一个用于开发基于 Chisel 的 SoC 的框架
2. 它会使用 Rocket Chip SoC generator
3. 它支持软件 RTL 仿真，比如 Verilator

所以这次 lab 不是让你在“应用程序”里加日志，而是在“CPU 硬件描述”里加调试打印，再重新生成仿真器。

### 8.2 Rocket Chip 是什么

`Rocket Chip` 是 Berkeley 的一个 RISC-V SoC 生成器项目。

它里面包含：

1. Rocket 核心
2. cache
3. 总线互联
4. 一些 SoC 级组件

在这个工程里，Rocket Chip 作为 Chipyard 的一个核心子模块存在，所以你会看到：

`chipyard/generators/rocket-chip/...`

意思就是：

这是 Chipyard 这个大框架下面的一个生成器，其中包含 Rocket 相关实现。

### 8.3 RocketCore 是什么

`RocketCore.scala` 不是“整个芯片”，也不是“整个仿真器”，它更接近：

```text
单个 Rocket CPU 核心的实现文件
```

你这次改它，是因为：

1. bypass 发生在 CPU 流水线内部
2. 最适合在核心内部打印调试信息
3. 所以要在 `RocketCore.scala` 里加 `printf`

换句话说：

```text
Chipyard     是整个平台
Rocket Chip  是其中一套 SoC/CPU 生成器
RocketCore   是其中一个具体 CPU 核心实现文件
```

### 8.4 为什么文件后缀是 `.scala`

因为这里不是直接手写 Verilog，而是用 `Chisel` 写硬件。

`Chisel` 是一个基于 Scala 的硬件描述语言/硬件构造语言。

你可以先这样粗理解：

```text
Scala/Chisel 源码
    ->
生成硬件电路
    ->
再变成 Verilog / 仿真器
```

所以这次你虽然改的是 `.scala` 文件，但实际影响的是 CPU 的硬件行为和仿真输出。

### 8.5 这次 lab 的工作链路是什么

最重要的是把整个链路想通：

```text
你修改 RocketCore.scala
    ->
Chipyard 重新生成/编译相关硬件仿真产物
    ->
Verilator 重新构建模拟器
    ->
模拟器运行你的 test.elf
    ->
输出 CPU 执行日志和 BYPASS 日志
```

所以 `make run-binary-debug ...` 不是单纯“运行程序”，它通常包含：

1. 必要时重新编译硬件生成结果
2. 重新构建 Verilator 模拟器
3. 用模拟器运行指定 RISC-V 程序

### 8.6 为什么改一个 Scala 文件就要重新编译这么久

因为你改的不是普通业务逻辑，而是 CPU 核心本身。

这会触发一条比较重的链：

```text
Scala/Chisel
    ->
硬件生成
    ->
Verilog
    ->
C++/仿真器构建
    ->
最终 simulator 可执行文件
```

所以时间长是正常的，不代表你操作错了。

## 9. 这次 lab 里几个关键目录怎么理解

先把最常见的几个路径记住：

### 9.1 `chipyard/`

整个硬件生成与仿真工程根目录。

你可以把它看成这门课里“处理 CPU/SoC 的主仓库”。

### 9.2 `chipyard/generators/`

放各种“生成器”。

这里的“生成器”不是随机生成代码的意思，而是：

```text
根据配置拼装出不同硬件系统的模块集合
```

你会看到很多子目录，比如：

1. `rocket-chip`
2. `gemmini`
3. `cva6`
4. 其他核或加速器

这说明 Chipyard 不只支持一种 CPU/加速器。

### 9.3 `chipyard/generators/rocket-chip/`

Rocket Chip 相关实现所在目录。

这次 lab 最重要的 CPU 核心文件就在这里。

### 9.4 `chipyard/generators/chipyard/src/main/scala/`

这一层更像“Chipyard 平台自身”的顶层组织代码。

你现在不一定要进去改，但知道它的角色有帮助。

从仓库里可以看到几个典型入口文件：

1. `Generator.scala`
   - 定义生成流程入口
2. `DigitalTop.scala`
   - 描述数字顶层系统
3. `ChipTop.scala`
   - 更靠近芯片/顶层封装
4. `ConfigFinder.scala`
   - 帮助找到和处理配置

它们更偏“把系统拼起来”，而不是像 `RocketCore.scala` 那样直接写单个流水线细节。

### 9.5 `chipyard/sims/verilator/`

这里是 Verilator 仿真目录。

你在这里执行：

```bash
make run-binary-debug BINARY=...
```

就是在调用 Verilator 相关的构建和运行流程。

这个目录下常见的东西：

1. `Makefile`
   - 定义如何构建和运行仿真
2. `generated-src/`
   - 生成出来的中间代码/源码
3. `output/`
   - 仿真输出日志目录
4. `simulator-chipyard.harness-RocketConfig-debug`
   - 最终生成的 debug 仿真器可执行文件

## 10. 这次 lab 涉及的关键文件分别是干什么的

### 10.1 `RocketCore.scala`

CPU 核心实现文件。这里只是加调试打印，不是让你重写 CPU。

更准确地说，它是：

```text
Rocket 五级流水线核心实现的重要文件之一
```

你现在会在这个文件里接触到：

1. 指令流水寄存器
2. EX 阶段相关信号
3. bypass 相关信号
4. `printf` 调试输出

为什么老师让你改这里：

因为这正是最接近 hazard 发生位置的地方。

### 10.2 `test.S`

你写的 RISC-V 汇编文件。核心任务都在这里完成。

它的角色不是“功能程序”，而是“实验刺激输入”。

也就是说：

```text
RocketCore.scala 负责让你看见现象
test.S           负责制造现象
```

这两个文件配合起来，才构成这次 lab。

### 10.3 `link.ld`

链接脚本，决定你的程序最终放到内存的什么地址。

题目里给了：

```ld
. = 0x80000000;
```

所以你的代码从 `0x80000000` 开始。

这就是为什么报告里要求你：

只分析你自己代码那部分 PC，不要分析初始化代码 `0x10000`。

### 10.4 `test.elf`

汇编编译后的可执行文件，Verilator 仿真时真正运行的是它。

它可以理解成：

```text
你写的 test.S + link.ld
    ->
经过 RISC-V 交叉编译
    ->
得到 test.elf
```

然后这个 ELF 被送进 Verilator 仿真器里执行。

### 10.5 `test.out`

仿真日志文件。你分析 hazard 的主要证据就在这里。

它不是你的程序主动打印出来的普通 stdout，而是“CPU 执行过程的调试日志 + 你加的 BYPASS 日志”。

所以它对这次 lab 很关键。

### 10.6 `Generator.scala`

路径：

`chipyard/generators/chipyard/src/main/scala/Generator.scala`

仓库里可以看到它定义了 `object Generator extends StageMain(new ChipyardStage)`。

你可以先把它理解成：

```text
Chipyard 生成流程的程序入口
```

它更像“启动整个硬件生成过程”的入口，而不是某个流水线细节实现。

### 10.7 `DigitalTop.scala`

路径：

`chipyard/generators/chipyard/src/main/scala/DigitalTop.scala`

这个文件对应数字系统顶层。

你可以粗理解为：

```text
把 CPU、总线、外设等数字部分组装到一起
```

它属于系统集成层。

### 10.8 `ChipTop.scala`

路径：

`chipyard/generators/chipyard/src/main/scala/ChipTop.scala`

这个文件更接近“芯片顶层封装”概念。

和 `DigitalTop.scala` 相比，它更偏整体顶层组织，而不是具体流水线逻辑。

### 10.9 `ConfigFinder.scala`

路径：

`chipyard/generators/chipyard/src/main/scala/ConfigFinder.scala`

这个文件主要和配置查找/管理有关。

你暂时不用深入它，但知道有这层很重要：

```text
同一个 Chipyard 工程
可以通过不同配置
生成不同系统
```

所以你看到的 `RocketConfig`，本质上就是一种配置选择。

## 11. 一张图把这次 lab 用到的工程层次串起来

```text
Chipyard
├── generators/chipyard/...           <- 平台顶层、配置、系统集成
├── generators/rocket-chip/...        <- Rocket Chip 生成器
│   └── src/main/scala/rocket/
│       └── RocketCore.scala          <- 这次要改的 CPU 核心文件
└── sims/verilator/                   <- 仿真目录
    ├── Makefile
    ├── generated-src/
    ├── output/
    └── simulator-...-debug

你写的 test.S / link.ld
    ->
编译出 test.elf
    ->
交给 sims/verilator 里的模拟器运行
    ->
得到 test.out
```

只要把这张图想通，你就不会再把这些文件看成毫无关系的碎片。

## 12. 再补一层：`RocketCore.scala` 里这次会碰到的信号到底是什么

这一节只讲这次 lab 真会用到的信号。

目标不是让你读完整个 `RocketCore.scala`，而是让你能看懂老师要求插入的那段代码。

### 12.1 先记住一个事实：这份文件在描述流水线寄存器和阶段传递

在 `RocketCore.scala` 里，你会经常看到这种命名：

1. `id_xxx`
2. `ex_xxx`
3. `mem_xxx`
4. `wb_xxx`
5. `ex_reg_xxx`
6. `mem_reg_xxx`

可以这样理解：

1. `id`
   - Decode 阶段正在使用的信号
2. `ex`
   - Execute 阶段正在使用的信号
3. `mem`
   - Memory 阶段正在使用的信号
4. `wb`
   - Write Back 阶段正在使用的信号
5. `ex_reg_xxx`
   - 从前一阶段锁存下来，进入 EX 阶段后保存的寄存器值

所以名字里带 `reg`，通常表示：

```text
这个值不是“临时线网”
而是跨周期保存下来的阶段寄存器
```

### 12.2 `id_inst(0)` 是什么

你会看到：

```scala
val id_ctrl = Wire(new IntCtrlSigs).decode(id_inst(0), decode_table)
```

以及：

```scala
ex_reg_inst := id_inst(0)
```

这里的 `id_inst(0)` 可以先粗理解成：

```text
当前在 ID 阶段的那条指令
```

因为这个 RocketCore 配置里一次只解一条主指令，所以 `(0)` 可以先理解成“第 0 条槽位”。

对这次 lab 来说，最重要的是：

1. `id_inst(0)` 是还在 ID 阶段的指令
2. `ex_reg_inst` 是这条指令被送进 EX 阶段后保存下来的版本

### 12.3 `ex_reg_inst` 是什么

在源码里可以看到：

```scala
val ex_reg_inst = Reg(Bits())
...
ex_reg_inst := id_inst(0)
```

这表示：

```text
把当前 ID 阶段的指令
锁存到 EX 阶段寄存器里
```

所以当一条指令进入 EX 阶段后，`ex_reg_inst` 就代表：

```text
当前 EX 阶段这条指令的机器码
```

这就是为什么老师的代码能写：

```scala
val ex_rs1 = ex_reg_inst(19, 15)
val ex_rs2 = ex_reg_inst(24, 20)
```

因为 RISC-V 指令格式里：

1. `rs1` 常在 bit `[19:15]`
2. `rs2` 常在 bit `[24:20]`

所以这是在直接从机器码里切出两个源寄存器编号。

### 12.4 `ex_reg_pc` 是什么

在源码里有：

```scala
val ex_reg_pc = Reg(UInt())
...
ex_reg_pc := ibuf.io.pc
```

它表示：

```text
当前进入 EX 阶段这条指令对应的 PC
```

所以 BYPASS 日志里打印：

```scala
printf("[BYPASS] PC=0x%x ...", ex_reg_pc, ...)
```

意思就是：

打印“哪一条 EX 阶段指令收到了旁路数据”。

### 12.5 `ex_pc_valid` 是什么

源码里有：

```scala
val ex_pc_valid = ex_reg_valid || ex_reg_replay || ex_reg_xcpt_interrupt
```

你不用追所有细节，只要抓住它的大意：

```text
EX 阶段当前有一条有效指令需要被当真处理
```

所以老师的打印代码会先判断：

```scala
when (ex_pc_valid && ...)
```

含义就是：

```text
只有当前 EX 阶段这条指令真的有效时，才打印 bypass
```

否则可能会把空泡、无效状态也打印出来，日志就会很乱。

### 12.6 `id_raddr` 是什么

源码里有：

```scala
val id_raddr = IndexedSeq(id_raddr1, id_raddr2)
```

这表示：

```text
当前 ID 阶段这条指令的两个源寄存器地址
```

也就是：

1. 第 0 个源寄存器
2. 第 1 个源寄存器

所以后面很多逻辑会写：

```scala
for (i <- 0 until id_raddr.size) { ... }
```

因为它其实就是在遍历：

1. `rs1`
2. `rs2`

### 12.7 `ex_reg_rs_bypass(i)` 是什么

源码里可以看到：

```scala
val ex_reg_rs_bypass = Reg(Vec(id_raddr.size, Bool()))
```

以及：

```scala
ex_reg_rs_bypass(i) := do_bypass
```

它的含义很直接：

```text
EX 阶段这条指令的第 i 个源操作数
是否采用了 bypass
```

如果为真，就表示：

```text
这次不是老老实实从寄存器堆里读旧值
而是从流水线别的阶段转发一个更新的值过来
```

所以老师打印代码里的这句：

```scala
when (ex_pc_valid && ex_reg_rs_bypass(i) && ...)
```

核心就是：

```text
只有真的发生 bypass，才打印日志
```

### 12.8 `ex_reg_rs_lsb(i)` 是什么

这个名字最容易让人困惑。

源码里有：

```scala
val ex_reg_rs_lsb = Reg(Vec(id_raddr.size, UInt(log2Ceil(bypass_sources.size).W)))
```

以及：

```scala
val bypass_src = PriorityEncoder(id_bypass_src(i))
ex_reg_rs_lsb(i) := bypass_src
```

你不用死抠 `lsb` 这个命名本身，只要理解它在这次 lab 里的用途：

```text
它保存了“这个源操作数当前选中了哪个 bypass 来源”
```

也就是一个“来源编号”。

在 RocketCore 的源码里，`bypass_sources` 是这样排的：

1. `0` 对应寄存器堆正常值
2. `1` 对应 EX 相关来源
3. `2` 对应 WB 相关来源
4. `3` 对应 MEM 相关来源

所以老师给的打印代码才会写：

```scala
when (ex_reg_rs_lsb(i) === 1.U) { ... src=EX ... }
.elsewhen (ex_reg_rs_lsb(i) === 2.U) { ... src=WB ... }
.elsewhen (ex_reg_rs_lsb(i) === 3.U) { ... src=MEM ... }
```

这不是随便写的，而是和源码里 `bypass_sources` 的顺序对应起来的。

### 12.9 `ex_rs(0)` 和 `ex_rs(1)` 是什么

源码里有：

```scala
val ex_rs = for (i <- 0 until id_raddr.size)
  yield Mux(ex_reg_rs_bypass(i), bypass_mux(ex_reg_rs_lsb(i)), ...)
```

它的核心意思是：

```text
EX 阶段真正拿来参与运算的源操作数值
```

也就是说：

1. 如果需要 bypass，就从 `bypass_mux(...)` 取值
2. 如果不需要 bypass，就用正常寄存器值

所以：

```text
ex_reg_rs_bypass(i) 决定“要不要旁路”
ex_reg_rs_lsb(i)    决定“从哪一段旁路”
ex_rs(i)            是最后真正送去运算的值
```

这三个是连在一起的。

### 12.10 这次老师插入的 BYPASS 代码到底在做什么

你可以把它翻译成下面这段自然语言：

```text
对于当前 EX 阶段这条有效指令：
  检查它的两个源寄存器 rs1 / rs2
  如果某个源寄存器使用了 bypass
    就打印：
    1. 当前指令 PC
    2. 是第几个源寄存器
    3. 这个源寄存器编号是多少
    4. bypass 来自 EX / WB / MEM 哪一段
```

也就是说，这段代码不是在“改变 bypass 行为”。

它只是：

```text
读取 RocketCore 里已经存在的 bypass 判定结果
把它打印出来给你看
```

### 12.11 一张小图把这些信号串起来

```text
ID 阶段
  id_inst(0)        当前解码中的指令
  id_raddr          当前指令要读的 rs1/rs2 编号
      |
      | 锁存到 EX 阶段
      v
EX 阶段
  ex_reg_inst       进入 EX 后保存的指令机器码
  ex_reg_pc         这条指令的 PC
  ex_pc_valid       这条 EX 指令是否有效
  ex_reg_rs_bypass  某个源操作数是否用了 bypass
  ex_reg_rs_lsb     bypass 来源编号
  ex_rs             最终真正参与运算的源操作数值
```

### 12.12 你读这段代码时最容易卡住的点

1. 以为 `ex_reg_inst` 是“上一条指令”
   - 更准确说，它是“当前在 EX 阶段的那条指令”
2. 以为老师在新增 bypass 逻辑
   - 不是，老师只是让你把已有 bypass 状态打印出来
3. 看到 `lsb` 就想追位级细节
   - 这次没必要，你只要把它当“来源编号”就够了
4. 看到 `rs0` / `rs1` 以为是寄存器 `x0/x1`
   - 不是，这里是“第 0 个源操作数”和“第 1 个源操作数”的意思

### 12.13 这一节你真正该记住的 6 个句子

1. `id_inst(0)` 是当前 ID 阶段指令
2. `ex_reg_inst` 是当前 EX 阶段指令
3. `ex_reg_pc` 是当前 EX 阶段指令的 PC
4. `ex_pc_valid` 表示当前 EX 阶段指令有效
5. `ex_reg_rs_bypass(i)` 表示第 `i` 个源操作数用了 bypass
6. `ex_reg_rs_lsb(i)` 表示这个 bypass 来自哪一段

如果这 6 句你能说顺，这次插入的 Scala 代码你就已经基本看懂了。

## 13. 汇编模板最后那段 return 代码在干什么

题目模板最后这段：

```asm
lw a0, result
sll a0, a0, 1
or a0, a0, 1
write_tohost:
    sw a0, tohost, t5
    sw zero, tohost + 4, t5
    j write_tohost
```

它的作用不是让你算算法题，而是把结果写到 `tohost`，让仿真环境知道程序执行完了。

你不用动它。

题目已经明确说了：指定区域外不要改。

## 14. 日志怎么看

日志里有两类信息。

### 14.1 BYPASS 日志

示例：

```text
[BYPASS] PC=0x000001002c, rs0=x10 (src=EX)
```

含义前面讲过，重点看：

1. 哪条指令在用旁路
2. 用的是哪个源寄存器
3. 数据来自哪个阶段

### 14.2 指令日志

示例：

```text
C0: 44 [1] pc=[000000000001002c] ... is_br(taken)=[0(0)] ...
```

对这次 lab 最重要的字段：

1. `C0: 44`
   - 周期号
2. `pc=[...]`
   - 指令地址
3. `W[...]`
   - 写回了哪个寄存器和值
4. `R[...]`
   - 读了哪些寄存器和值
5. `is_ld/st=[x/y]`
   - 是否 load/store
6. `is_br(taken)=[预测结果(实际结果)]`
   - 这是 control hazard 分析的关键

### 关于 `is_br(taken)=[0(1)]` 的直观理解

可以先这样理解：

1. 前面的值表示预测相关信息
2. 括号里的值表示实际是否 taken

你要结合具体日志、PC 跳转情况、后续执行路径一起判断。

最安全的做法不是只盯一个字段，而是：

1. 看分支指令本身的日志
2. 看下一条真正执行的是顺序下一条还是目标地址
3. 再结合 `is_br(taken)` 去交叉验证

## 15. 这次 lab 的三个核心观察点

### 15.1 Data Hazard

你要证明：

1. 确实有数据相关
2. 是否发生 stall
3. 如果没 stall，是因为 bypass 解决了
4. bypass 来自 EX / MEM / WB 哪个阶段

典型现象：

1. 有 `[BYPASS]`
2. 周期推进比较紧凑
3. 说明数据及时被转发，没有额外停顿

### 15.2 Load-Use Hazard

你要证明：

1. 后一条马上使用 `lw` 的结果
2. 这里通常会 stall
3. 从 WB 周期号或相邻指令间隔能看出多了空档

典型现象：

1. 相关指令之间周期差异常
2. 相比普通 ALU 数据相关，多了停顿

### 15.3 Control Hazard

你要证明：

1. 默认静态预测是 `taken` 还是 `not taken`
2. 你是怎么从日志推断出来的

最稳妥的判断方式：

1. 写一个实际 `taken` 的分支
2. 写一个实际 `not taken` 的分支
3. 看哪一种情况更像“无需纠正”，哪一种更像“走错再修正”

## 16. 你真正要回答老师的几个问题

### Data Hazards

老师要你回答：

1. 这个 hazard 有没有导致 stall？
2. 为什么会或不会 stall？
3. bypass 来自哪个阶段？
4. 为什么会从那个阶段来？

### Control Hazards

老师要你回答：

1. 默认静态分支预测是 `taken` 还是 `not taken`？
2. 你怎么从日志推出来的？

## 17. 最容易踩的坑

1. 分析错了地址范围
   - 只看你自己的代码，PC 从 `0x80000000` 开始
   - 不要分析初始化代码 `0x10000`
2. 用 loop 做分支实验
   - 会训练动态预测器，干扰结论
3. 只写 hazard，不留可观察证据
   - 最终报告必须能对应到日志和截图
4. 只说“有 bypass / 有 stall”，但解释不清原因
   - 老师要的是“为什么”
5. 改了模板不该改的部分
   - 题目明确说只改指定区域

## 18. 用一句话概括每个概念

1. `pipeline`
   - 多条指令分阶段重叠执行
2. `hazard`
   - 会让流水线出错或降速的冲突
3. `data hazard`
   - 后一条要用前一条刚产生的数据
4. `load-use hazard`
   - 刚 `lw` 出来的数据立刻被下一条使用，常需要 stall
5. `control hazard`
   - 分支改变 PC，CPU 需要预测下一条走哪
6. `bypass`
   - 不等写回寄存器，直接把中间结果转发给后面指令
7. `stall`
   - 暂停流水线，等数据准备好
8. `branch prediction`
   - 分支结果未确定前先猜一下
9. `taken`
   - 分支跳转
10. `not taken`
   - 分支不跳转

## 19. 你做题时脑子里只保留这条主线就够了

```text
先加调试打印
    ->
写 3 段会制造 hazard 的汇编
    ->
编译成 test.elf
    ->
跑 Verilator
    ->
看 test.out 里的 BYPASS 和周期号
    ->
回答：有没有 stall、有没有 bypass、来自哪一段、默认预测是什么
```

如果你把这条链条想通，这次 lab 就已经过半了。
