# Lab 8 Todo List

## 1. 总流程先看一眼

```text
确认题面和代码目录
    ->
确认 SUSTemu 仓库根目录
    ->
运行 Q1 观察 Store-Load violation
    ->
改成 in-order 对比结果
    ->
运行 Q1 fence 版本并画 happens-before 时间线
    ->
运行 Q2 记录 counter 实际值与 lost updates
    ->
运行 Q3 broken Peterson 版本
    ->
修改 peterson.c 的两个 TODO 加 fence
    ->
从仓库根目录 make clean && make
    ->
运行 Q3 fence 版本验证恢复正确
    ->
整理 Q1-Q5 的输出、解释和时间线到 report
```

## 2. 先明确这次要交什么

根据 `lab8.pdf`，这次最终要完成并写入报告的是：

1. `Question 1`
   - `make run` 的 violation 数量和百分比
   - `make run-inorder` 或等价修改 `FLAGS` 后的结果
   - 为什么 `OOO + TSO` 会违反、为什么 `in-order` 不会
2. `Question 2`
   - `make run-fence` 的结果
   - 为什么 `fence rw, rw` 消除了 `r1 = 0 && r2 = 0`
   - 两个 core 的 `happens-before` 时间线
3. `Question 3`
   - `Q2` 的实际 counter 值
   - 期望值 `2000`
   - lost updates 数量
   - 为什么丢失量接近 `N_ITER`
4. `Question 4`
   - `Q3` 无 fence 时的实际值和期望值
   - `lock()` 在 `TSO` 下哪一步失效
   - 两个 core 同时进入 `critical section` 的时间线
5. `Question 5`
   - `peterson.c` 两个 `TODO` 位置加 fence 的代码
   - `make run-fence` 后验证结果
   - 为什么必须两个 fence，而不是一个

## 3. 隐含前提先说清楚

### 3.1 当前从题面确认到的事实

1. 实验使用 `SUSTemu`
2. benchmark 目录叫 `order_lab/Q1`、`order_lab/Q2`、`order_lab/Q3`
3. 默认配置是 `--dual --ooo --bpred -b`
4. 修改 `include/` 下文件后，要回仓库根目录执行 `make clean && make`

### 3.2 当前仓库里的风险点

我后来已经从远端最新 `origin/main` 验证到，当前可用的实验目录在独立 worktree 里，真实路径是：

```text
/opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab
```

现在这台机器上，`SUSTemu_order_main` 已经可以直接单独使用了，因为它自己的 simulator 也已经编出来了：

```text
/opt/ext1/lzh/CSE5030/SUSTemu_order_main/build/sustemu
```

所以这次实验现在可以统一成单目录工作流：

```text
源码目录：/opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab
模拟器：  /opt/ext1/lzh/CSE5030/SUSTemu_order_main/build/sustemu
```

## 4. Step 1：先确认实验目录

### 要做什么

确认 `lab8.pdf` 说的 `order_lab` 和 `SUSTemu` 仓库根目录到底在哪。

### 为什么这么做

后面所有 `make run` 都依赖正确路径；如果路径错了，后面全部命令都会报错。

### 指令

```bash
cd /home/lzh/CSE5030
find . -maxdepth 4 -type d | grep 'order_lab'
find . -maxdepth 3 -type d | grep 'SUSTemu'
```

### 指令含义

1. `cd /home/lzh/CSE5030`
   - 进入课程仓库根目录
2. `find . -maxdepth 4 -type d | grep 'order_lab'`
   - 搜索题面提到的 `order_lab` 目录
3. `find . -maxdepth 3 -type d | grep 'SUSTemu'`
   - 搜索 `SUSTemu` 仓库根目录

### 做完后你要得到什么

你需要明确两件事：

1. 仓库根目录是什么
2. `order_lab/Q1`、`Q2`、`Q3` 的真实路径是什么

## 5. Step 2：预检工具

### 要做什么

检查本次实验最基本的工具是否可用。

### 为什么这么做

至少你会用到：

- `make`
- `gcc`
- `grep`

### 指令

```bash
command -v make
command -v gcc
command -v grep
```

### 指令含义

1. `command -v make`
   - 看 `make` 是否存在
2. `command -v gcc`
   - 看本机 C 编译器是否存在
3. `command -v grep`
   - 看文本搜索工具是否存在

## 6. Step 3：确认题面默认配置

### 要做什么

先把这次实验默认配置记下来。

### 为什么这么做

后面解释所有现象时都要回到这些硬件前提。

### 题面给出的默认配置

```text
Two OOO cores: --dual --ooo --bpred -b
per-core store buffer: STORE_BUF_SIZE = 16
drain rate: one committed store entry per cycle
memory model: TSO
```

### 这一步你需要会说什么

你要能复述：

```text
这是一个 dual-core、OOO、带 per-core store buffer 的 TSO 实验环境
```

## 7. Step 4：进入 `Q1` 跑基础版本

### 要做什么

先在 `SUSTemu_order_main` 根目录编好 simulator，然后进入 `order_lab/Q1` 跑无 fence 版本。

### 为什么这么做

这是 `Question 1` 的第一部分，要观察 `r1 = 0 && r2 = 0` violation。`Q1/Q2/Q3` 的 `Makefile` 默认会去找 `../../../build/sustemu`，所以第一次实验前必须先把根目录的 simulator 编出来。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main
make
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q1
make run
```

### 指令含义

1. `cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main`
   - 进入这次 lab8 统一使用的 `SUSTemu` 根目录
2. `make`
   - 在根目录编译出 `build/sustemu`
3. `cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q1`
   - 进入 `Dekker litmus test` 目录
4. `make run`
   - 运行无 fence 的默认版本

### 跑完后要记录什么

把输出里的这些内容记下来：

1. violation 的数量
2. violation 的百分比
3. 题面总共跑了多少个 trial

## 8. Step 5：改成 `in-order` 再跑 `Q1`

### 要做什么

按题面要求，把 `Makefile` 里的 `FLAGS` 从 `--ooo --bpred -b` 改成 `--inorder --bpred -b`。

### 为什么这么做

这是 `Question 1` 的第二部分，要证明 `in-order` 不会观察到 violation。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q1
grep 'FLAGS' Makefile
```

### 你要改什么

把类似：

```make
FLAGS = --dual --ooo --bpred -b
```

改成：

```make
FLAGS = --dual --inorder --bpred -b
```

### 修改后运行

```bash
make run
```

### 跑完后要记录什么

记录：

1. violation 数量
2. 是否为 `0`
3. 你如何解释 `OOO + TSO` 才会让 violation 变得可观察

## 9. Step 6：恢复 `Q1` 的 OOO 配置并跑 fence 版本

### 要做什么

把 `Q1` 恢复到默认 `OOO` 配置，然后跑 `make run-fence`。

### 为什么这么做

这是 `Question 2`，要验证 `fence rw, rw` 会消除双 0。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q1
make run-fence
```

### 指令含义

- `make run-fence`
  - 运行插入 `fence rw, rw` 的版本

### 跑完后你要做什么

1. 记录是否还有 violation
2. 用文字解释为什么 fence 消除了 `r1 = 0 && r2 = 0`
3. 画一张双核时间线

### 时间线应该抓什么

你要表达的是：

```text
Core 0: store x
    ->
fence
    ->
load y

Core 1: store y
    ->
fence
    ->
load x
```

也就是 fence 阻止后面的 `load` 越过前面的 `store`。

## 10. Step 7：进入 `Q2` 跑 Lost Updates

### 要做什么

进入 `order_lab/Q2`，运行 shared counter 的 non-atomic increment 程序。

### 为什么这么做

这是 `Question 3` 的实验数据来源。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q2
make run
```

### 指令含义

- `make run`
  - 跑两个 core 对共享 counter 做 non-atomic increment 的版本

### 跑完后要记录什么

1. `actual counter value`
2. `expected value`
3. lost updates 数量

### 公式

```text
lost updates = expected value - actual counter value
```

题面里：

```text
expected value = 2000
```

## 11. Step 8：解释为什么丢失量接近 `N_ITER`

### 要做什么

写 `Question 3` 的分析部分。

### 为什么这么做

题目不是只要你算差值，还要你解释为什么这个差值会这么大。

### 你应该抓住的点

1. increment 不是 atomic
2. `store` 先进入本 core 的 `store buffer`
3. 另一个 core 在一段时间内仍能基于旧值做计算
4. 两边反复写回同一个 `k + 1`
5. 因为 drain latency 存在，覆盖不是偶发，而是高频

## 12. Step 9：进入 `Q3` 跑无 fence 的 Peterson

### 要做什么

进入 `order_lab/Q3`，先跑 `USE_FENCE=0` 的 broken 版本。

### 为什么这么做

这是 `Question 4` 的数据来源，要先证明 Peterson 在 `TSO` 下会失效。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q3
make run
```

### 指令含义

- `make run`
  - 跑无 fence 的 Peterson lock 版本

### 跑完后要记录什么

1. `actual counter value`
2. `expected value`
3. 是否出现小于期望值的结果

### 这说明了什么

如果实际值小于期望值，说明两个 core 至少在某些轮次里同时进入了 `critical section`，互斥失败。

## 13. Step 10：定位 `peterson.c` 里的两个 `TODO`

### 要做什么

打开 `order_lab/Q3/peterson.c`，找到 `lock()` 里的两个 `TODO (Q5)`。

### 为什么这么做

这是 `Question 5` 的改代码位置。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q3
grep -n 'TODO (Q5)' peterson.c
```

### 指令含义

- `grep -n 'TODO (Q5)' peterson.c`
  - 定位 `TODO` 所在行号

### 你会看到的结构

```c
want[i] = 1;
/* TODO (Q5): add fence rw,rw here */
*turn = j;
/* TODO (Q5): add fence rw,rw here */
while (want[j] && *turn == j) {}
```

## 14. Step 11：在两个 `TODO` 位置加入 fence

### 要做什么

在两个 `TODO` 位置都加上：

```c
__asm__ volatile("fence rw, rw");
```

### 为什么这么做

题目明确要求你验证两个 fence 才能恢复正确的顺序保证。

### 修改后的目标结构

```c
want[i] = 1;
__asm__ volatile("fence rw, rw");
*turn = j;
__asm__ volatile("fence rw, rw");
while (want[j] && *turn == j) {}
```

## 15. Step 12：如果改到了 `include/`，回仓库根目录重编译

### 要做什么

题面明确说，只要你改了 `include/` 下文件，就必须回仓库根目录执行：

```bash
make clean && make
```

### 为什么这么做

因为头文件变化不一定会被局部目标完整追踪，回根目录 clean rebuild 才稳。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main
make clean
make
```

### 指令含义

1. `make clean`
   - 清掉旧的编译产物
2. `make`
   - 从仓库根目录完整重编译

### 注意

如果你这次只改的是 `order_lab/Q3/peterson.c`，而不是 `include/` 下文件，这一步通常不需要，因为 `peterson.c` 是 benchmark 源码，不是 simulator 主程序源码。

当前这台机器跑 lab8 的最稳做法是全程只用：

1. `/opt/ext1/lzh/CSE5030/SUSTemu_order_main`
2. 其中 `labs/order_lab` 放 benchmark
3. 其中 `build/sustemu` 放 simulator

只有你真的改了 `SUSTemu_order_main/include/` 或 `SUSTemu_order_main/src/` 下的 simulator 源码，才需要回 `SUSTemu_order_main` 根目录重新 `make clean && make`。

## 16. Step 13：运行 `Q3` 的 fence 版本

### 要做什么

再次进入 `Q3` 跑 `make run-fence`。

### 为什么这么做

这是 `Question 5` 的验证步骤，要确认 counter 恢复到正确值。

### 指令

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q3
make run-fence
```

### 指令含义

- `make run-fence`
  - 跑启用 fence 的 Peterson 版本

### 跑完后要记录什么

1. `actual counter value`
2. `expected value`
3. 是否恢复到题面期望值

题面写的是：

```text
expected result is 2 * N_ITER if the mutex is correct
```

但 `Question 5` 又写了：

```text
verify the counter equals N_ITER
```

### 这里怎么处理

报告里不要自行替老师修题，直接写：

1. 题面背景说期望是 `2 * N_ITER`
2. `Q5` 文字又写成 `equals N_ITER`
3. 你实际以程序输出和代码常量为准填写

这样最稳。

## 17. Step 14：画 `Q4` 的失败时间线

### 要做什么

把无 fence 的 Peterson 为什么会失效画成 ASCII 时间线。

### 为什么这么做

题面明确要求你“Draw a timeline”。

### 可以按这个结构画

```text
Core 0                         Core 1
want[0] = 1  (buffered)
                               want[1] = 1  (buffered)
turn = 1
                               turn = 0
read want[1] == 0
                               read want[0] == 0
pass while
                               pass while
enter critical section
                               enter critical section
```

### 这张图想证明什么

想证明：

```text
双方写入的 want[i] 还没对外可见时
双方都基于旧值通过了 spin-wait
```

## 18. Step 15：画 `Q2` 的成功时间线

### 要做什么

把加了 fence 后为什么不会双 0，也画成 ASCII 时间线。

### 为什么这么做

这是 `Question 2` 的核心解释材料。

### 可以按这个结构画

```text
Core 0                         Core 1
store x = 1
fence rw, rw
                               store y = 1
                               fence rw, rw
load y
                               load x
```

### 这张图想表达什么

想表达：

```text
load 不能再越过本 core 前面的 store
所以双方不可能同时读到旧值 0
```

## 19. Step 16：整理报告中的最终材料

### 要做什么

把 `Q1-Q5` 的输出、解释和时间线全部整理进 `report.md`。

### 为什么这么做

老师要的不是零散运行记录，而是结构化答案。

### 建议整理顺序

1. `Q1` 无 fence 的 violation 数据
2. `Q1` 改成 `in-order` 的对比结论
3. `Q2` 加 fence 后的解释和时间线
4. `Q3` lost updates 的数据和分析
5. `Q4` Peterson broken 版本数据和失败时间线
6. `Q5` 加 fence 后的代码、结果和分析

## 20. 最后的自查清单

```text
[ ] 已确认 order_lab 的真实路径
[ ] 已确认 SUSTemu 仓库根目录
[ ] 已运行 Q1 make run
[ ] 已记录 Q1 violation 数量和百分比
[ ] 已改成 in-order 再跑 Q1
[ ] 已解释为什么 in-order 为 0
[ ] 已运行 Q1 make run-fence
[ ] 已画出 Q1 的 happens-before 时间线
[ ] 已运行 Q2 make run
[ ] 已记录 actual / expected / lost updates
[ ] 已解释为什么 loss 接近 N_ITER
[ ] 已运行 Q3 make run
[ ] 已定位 peterson.c 的两个 TODO
[ ] 已加入两个 fence
[ ] 已运行 Q3 make run-fence
[ ] 已解释为什么两个 fence 都需要
[ ] 已整理 report.md
```

## 21. 如果你只想按最短路径做完

```bash
cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main
make

cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q1
make run

# 改 Makefile: --ooo -> --inorder
make run

# 恢复 OOO
make run-fence

cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q2
make run

cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q3
make run
grep -n 'TODO (Q5)' peterson.c

# 在两个 TODO 位置加入:
# __asm__ volatile("fence rw, rw");

cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main
make clean
make

cd /opt/ext1/lzh/CSE5030/SUSTemu_order_main/labs/order_lab/Q3
make run-fence
```

然后把所有输出和解释填进 `report.md` 就行。
