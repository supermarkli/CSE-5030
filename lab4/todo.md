# Lab 4 Todo List

## 1. 总流程先看一眼

```text
修改 RocketCore.scala 加调试输出
    ->
重新编译并跑一个官方测试确认改动生效
    ->
写自己的 test.S
    ->
写 link.ld
    ->
编译成 test.elf
    ->
运行 test.elf 仿真
    ->
打开 test.out 分析三类 hazard
    ->
截图 + 写 report
```

## 2. Step 1：进入环境

### 要做什么

进入 `chipyard` 工程目录，并加载环境变量。

### 为什么这么做

因为后面要用到 `riscv64-unknown-elf-gcc`、`make`、仿真脚本等工具，通常都依赖 `chipyard/env.sh` 设置路径。

### 指令

```bash
cd /home/lzh/CSE5030/chipyard
source ./env.sh
```

### 指令含义

1. `cd /home/lzh/CSE5030/chipyard`
   - 切到 `chipyard` 工程根目录
2. `source ./env.sh`
   - 在当前 shell 中加载环境变量
   - 这里加载的是 `chipyard/env.sh`
   - 注意：不能直接 `./env.sh`，否则变量可能不会留在当前终端里

### 做完如何确认

```bash
which riscv64-unknown-elf-gcc
which verilator
```

如果能打印出路径，说明环境基本可用。

## 3. Step 2：修改 RocketCore.scala，加入 bypass 调试打印

### 要做什么

打开：

`chipyard/generators/rocket-chip/src/main/scala/rocket/RocketCore.scala`

在题目指定位置加入给出的 `printf` 调试代码。

### 为什么这么做

因为默认日志里不一定会把 bypass 来源打印出来。你需要自己加打印，后面才能观察：

1. 哪条指令用了 bypass
2. 用的是哪个源寄存器
3. 数据来自 EX / MEM / WB 哪一段

### 具体动作

找到题目说的那段：

```scala
when (!ctrl_killd || csr.io.interrupt || ibuf.io.inst(0).bits.replay) {
  ...
}

// replay inst in ex stage?
val ex_pc_valid = ex_reg_valid || ex_reg_replay || ex_reg_xcpt_interrupt
```

然后把题目 PDF 里给的 `ADD CODE BELOW` 那一整段插到 `val ex_pc_valid = ...` 后面。

### 做完为什么还不能直接继续

因为 Scala 源码改了，但仿真器和生成的 Verilog 还没重编译。

## 4. Step 3：先用官方测试重编译并验证调试代码生效

### 要做什么

切到：

`chipyard/sims/verilator`

运行题目给的官方测试。

### 为什么这么做

这是最小验证。

如果连官方测试都没有出现 `[BYPASS]` 日志，那说明：

1. 代码可能没加对
2. 没有重新编译成功
3. 环境有问题

先在这里排错，比直接拿自己的汇编排错更省时间。

### 指令

```bash
cd /home/lzh/CSE5030/chipyard/sims/verilator
make run-binary-debug BINARY=$RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-simple -j
```

### 指令含义

1. `make run-binary-debug`
   - 执行带 debug 输出的仿真目标
2. `BINARY=...`
   - 指定仿真运行哪个 ELF 程序
3. `-j`
   - 并行编译，加快速度

### 做完如何确认

打开题目说的输出文件，找 `[BYPASS]`：

```bash
rg '\[BYPASS\]' output/chipyard.harness.TestHarness.RocketConfig/rv64ui-p-simple.out
```

### 指令含义

1. `rg`
   - ripgrep，快速搜索文本
2. `'\[BYPASS\]'`
   - 搜索包含 `[BYPASS]` 的行

如果能搜到，说明调试打印已经生效。

## 5. Step 4：新建你的汇编文件 `test.S`

### 要做什么

在合适目录下新建 `test.S`，基于题目模板只补 `TODO` 区域。

### 为什么这么做

这一步是整个 lab 的核心。你需要用汇编人为构造：

1. 普通 data hazard
2. load-use hazard
3. control hazard

### 推荐目录

```bash
mkdir -p /home/lzh/CSE5030/lab4/work
cd /home/lzh/CSE5030/lab4/work
```

### 指令含义

1. `mkdir -p`
   - 创建目录；如果已存在也不会报错

### 这一步真正该怎么写

建议你把三类 hazard 分成三个小段，段与段之间用注释隔开。

推荐思路：

1. `Data Hazard`
   - 先写一条 ALU 指令生成寄存器值
   - 紧跟一条立刻使用这个寄存器的 ALU 指令
   - 目的是观察 bypass，通常不应出现额外 stall
2. `Load-Use Hazard`
   - 先从 `array` 里 `lw/ld` 一个值
   - 紧跟一条立刻使用这个寄存器的 ALU 指令
   - 目的是观察 stall
3. `Control Hazard`
   - 写一个实际 taken 的 branch
   - 再写一个实际 not taken 的 branch
   - 不要用 loop

### 为什么这样设计

因为老师不是要看你汇编写得花哨，而是要你让三种现象尽量“干净、好观察、好解释”。

## 6. Step 5：新建链接脚本 `link.ld`

### 要做什么

把题目给的链接脚本原样写进 `link.ld`。

### 为什么这么做

因为它规定了程序从 `0x80000000` 开始放置，而报告分析就依赖这个地址范围。

### 指令

```bash
touch link.ld
```

然后把 PDF 里的链接脚本内容粘进去。

### 关键理解

其中最重要的一行是：

```ld
. = 0x80000000;
```

它决定你的代码起始 PC。

## 7. Step 6：编译 `test.S`

### 要做什么

把汇编编译成 `test.elf`。

### 指令

```bash
riscv64-unknown-elf-gcc \
    -static \
    -march=rv64imafd \
    -mcmodel=medany \
    -fvisibility=hidden \
    -nostdlib \
    -nostartfiles \
    -T link.ld \
    test.S \
    -o test.elf
```

### 指令含义

1. `riscv64-unknown-elf-gcc`
   - RISC-V 交叉编译器
2. `-static`
   - 静态链接
3. `-march=rv64imafd`
   - 指定目标指令集
4. `-mcmodel=medany`
   - 适配该平台常见地址访问模型
5. `-fvisibility=hidden`
   - 控制符号可见性
6. `-nostdlib`
   - 不链接标准库
7. `-nostartfiles`
   - 不用系统默认启动文件
8. `-T link.ld`
   - 使用你自己的链接脚本
9. `test.S`
   - 输入汇编源文件
10. `-o test.elf`
   - 输出 ELF 文件名

### 做完如何确认

```bash
ls -l test.elf
file test.elf
```

如果 ELF 成功生成，就能继续。

## 8. Step 7：运行你的程序

### 要做什么

在 `chipyard/sims/verilator` 里运行你刚编好的 `test.elf`。

### 指令

```bash
cd /home/lzh/CSE5030/chipyard/sims/verilator
make run-binary-debug BINARY=/home/lzh/CSE5030/lab4/work/test.elf
```

### 为什么这么做

这样 Verilator 会运行你自己的 hazard 测试程序，并在日志中输出：

1. 你的指令执行记录
2. bypass 调试打印
3. 分支行为信息

### 做完如何确认

如果终端最后出现类似：

```text
Verilog $finish
```

通常说明程序跑完了。

如果太慢，也可以先停掉，再直接去分析输出日志。题目允许这样做。

## 9. Step 8：打开日志，先只看你自己的代码

### 要做什么

分析：

`output/chipyard.harness.TestHarness.RocketConfig/test.out`

### 为什么这么做

报告要求明确说了：只分析你自己的代码，不分析初始化代码。

### 先筛选关键内容

```bash
rg -n 'BYPASS|pc=\[' output/chipyard.harness.TestHarness.RocketConfig/test.out
```

### 指令含义

1. `-n`
   - 显示行号
2. `BYPASS|pc=\[`
   - 同时搜索 bypass 行和指令日志行

### 你要重点盯住什么

1. PC 是否在 `0x80000000` 附近
2. 哪些行是你写的汇编
3. 哪些地方出现 `[BYPASS]`
4. 哪些地方 `is_br(taken)` 能体现分支预测
5. 相关指令的周期号差值

## 10. Step 9：分析 Data Hazard

### 要做什么

找到你那两条存在寄存器依赖的 ALU 指令。

### 你要回答什么

1. 有没有 stall
2. 如果没有，为什么
3. bypass 来自哪一段
4. 为什么来自那一段

### 分析方法

1. 看两条相关指令的 WB 周期号是否紧挨着
2. 看中间有没有异常空档
3. 看对应 `[BYPASS]` 的 `src=EX/WB/MEM`

### 常见结论模板

```text
这组普通 data hazard 没有明显 stall，因为后一条指令通过 bypass 直接获得前一条结果。
从日志可见 src=EX，说明前一条 ALU 指令在 EX 阶段生成结果后就被转发给了后继指令。
```

注意：这只是分析模板，不是你的最终答案。最终要以你的日志为准。

## 11. Step 10：分析 Load-Use Hazard

### 要做什么

找到 `load` 指令和紧跟着使用其结果的那条指令。

### 你要回答什么

1. 有没有 stall
2. 为什么 load-use 比普通 data hazard 更容易 stall

### 分析方法

1. 看 `lw/ld` 和下一条依赖指令之间的周期差
2. 和普通 ALU data hazard 的周期间隔做对比
3. 看日志能否体现数据准备更晚

### 常见结论模板

```text
这组 load-use hazard 出现了 stall，因为 load 的数据要到更晚的流水段才能得到，
紧随其后的指令无法像普通 ALU 相关那样及时从 EX 阶段拿到结果，因此流水线需要暂停等待。
```

同样，这只是模板，最终要以你自己的日志证据为准。

## 12. Step 11：分析 Control Hazard

### 要做什么

分别分析：

1. 你写的 taken branch
2. 你写的 not taken branch

### 你要回答什么

1. 默认静态预测是 taken 还是 not taken
2. 你是怎么判断出来的

### 分析方法

1. 找到分支指令所在日志
2. 看分支后的实际执行路径
3. 看 `is_br(taken)` 字段
4. 比较 taken 和 not taken 两种情况哪个更像“默认路径”

### 最关键提醒

不要只看一个字段就下结论。

最稳妥的是三重证据：

1. 分支指令日志
2. 下一条真正执行的 PC
3. `is_br(taken)` 字段

## 13. Step 12：截屏

### 要做什么

准备报告里要用的截图。

### 需要哪些截图

1. 三类 hazard 对应的汇编代码截图
2. 三类 hazard 对应的日志截图
3. 能体现 bypass/stall/prediction 的关键日志截图

### 为什么这么做

老师明确要求在报告里提交这些作为证据。

## 14. Step 13：写报告

### 要做什么

把以下内容写进报告：

1. 三种 hazard 的实现结果
2. 每种 hazard 的日志分析
3. Control hazard 的默认预测结论

### 建议写法

报告结构建议：

1. 实验目的
2. 实现代码
3. 运行结果与日志截图
4. Hazard 分析
5. 结论

## 15. 最后自查清单

在交作业前逐项确认：

```text
[ ] RocketCore.scala 已加入 BYPASS 调试输出
[ ] 官方测试能看到 [BYPASS]
[ ] test.S 中实现了 3 类 hazard
[ ] control hazard 同时包含 taken 和 not taken 分支
[ ] 没有用 loop 干扰静态预测分析
[ ] 成功生成 test.elf
[ ] 成功运行 make run-binary-debug BINARY=test.elf
[ ] 找到 test.out
[ ] 只分析了 0x80000000 开始的自定义代码
[ ] 截好了代码和日志
[ ] report 已写完
```

## 16. 如果你只想最短路径完成

直接照这个最短版执行：

```bash
cd /home/lzh/CSE5030
cd chipyard
source ./env.sh

cd chipyard/sims/verilator
make run-binary-debug BINARY=$RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-simple -j

mkdir -p /home/lzh/CSE5030/lab4/work
cd /home/lzh/CSE5030/lab4/work

# 新建 test.S 和 link.ld
# 写入你的 hazard 测试代码和链接脚本

riscv64-unknown-elf-gcc \
    -static \
    -march=rv64imafd \
    -mcmodel=medany \
    -fvisibility=hidden \
    -nostdlib \
    -nostartfiles \
    -T link.ld \
    test.S \
    -o test.elf

cd /home/lzh/CSE5030/chipyard/sims/verilator
make run-binary-debug BINARY=/home/lzh/CSE5030/lab4/work/test.elf

rg -n 'BYPASS|pc=\[' output/chipyard.harness.TestHarness.RocketConfig/test.out
```

你只要跑通这条链，再按 `report.md` 填内容，就能完成这次 lab。
