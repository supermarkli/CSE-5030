# Lab 8 Memory Ordering Lab 零基础笔记

## 1. 这次 lab 到底在做什么

这次 lab 的主线是：

1. 用 `SUSTemu` 观察双核程序在 `TSO` 下为什么会出并发 bug
2. 理解这些 bug 为什么在 `SC` 下不会出现
3. 学会用 `fence rw, rw` 恢复正确的同步顺序

在正式看实验之前，先把下面这些前置词记住，不然后面的 `core`、`load`、`store`、`OOO` 会很容易看懵。

### 1.1 读前基础词

- `CPU`：中央处理器，负责执行程序。
- `core`：CPU 核心。一个 CPU 里可以有多个 `core`，你可以先把一个 `core` 看成一个能独立跑程序的“小 CPU”。
- `instruction`：指令。程序最终会变成一条一条机器指令交给 CPU 执行。
- `load`：读内存。把内存里的值读出来。
- `store`：写内存。把一个值写回内存。
- `register`：寄存器。CPU 内部很小但很快的存储位置，放当前计算要用的数据。
- `shared memory`：共享内存。多个 `core` 都能访问到的同一块内存。
- `cache`：缓存。位于 CPU 和内存之间，比内存快，用来临时存最近常用的数据。
- `pipeline`：流水线。CPU 会把一条指令拆成多个阶段，让多条指令重叠执行。
- `in-order core`：顺序执行核心。大体按程序书写顺序推进。
- `OOO core`：`Out-Of-Order core`，乱序执行核心。为了提速，后面的部分指令可能先执行。
- `atomic`：原子操作。要么整个做完，要么整个没做，中间不会被别人看到半成品。
- `mutex`：互斥锁。保证同一时刻只有一个线程进入关键代码。
- `critical section`：临界区。访问共享数据的那段关键代码。

你可以把这次 lab 理解成：

```text
两个 core 并发读写 shared memory
    ->
每个 core 前面都有一个 store buffer
    ->
store 先进入本地 buffer，不会立刻对另一个 core 可见
    ->
load 有机会绕过旧的 store 先执行
    ->
程序表面上“按顺序写的代码”在硬件上不一定按你想的顺序可见
    ->
Dekker 可能出现双 0
    ->
non-atomic counter 可能丢更新
    ->
Peterson mutex 可能失效
    ->
用 fence 强制顺序后再恢复正确性
```

你读这份笔记时可以先做两个最简单的脑内翻译：

```text
load  = 读
store = 写
```

```text
OOO core = 会为了提速而让部分后续指令先做的 CPU 核心
```

## 2. 整体 ASCII 流程图

```text
+------------------------------+
| 读懂题面与默认模拟器配置      |
| dual + ooo + per-core buffer |
+------------------------------+
              |
              v
+--------------------------------------+
| Step 1: Store-Load Reordering        |
| Dekker litmus test in order_lab/Q1   |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 跑 make run                           |
| 观察 r1 = 0 && r2 = 0 violation      |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 改成 in-order                         |
| 比较为什么 violation 消失             |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 跑 make run-fence                     |
| 理解 fence 建立 happens-before        |
+--------------------------------------+
              |
              v
+--------------------------------------+
| Step 2: Lost Updates in order_lab/Q2 |
| 两个 core 同时做 non-atomic ++        |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 实际值 < 2000                         |
| 分析 store buffer 为什么放大丢失      |
+--------------------------------------+
              |
              v
+--------------------------------------+
| Step 3: Peterson Lock in order_lab/Q3|
| 先跑 broken 版本，再补两个 fence      |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 分析 want[i] / turn 的可见性问题      |
| 解释为什么一个 fence 不够             |
+--------------------------------------+
              |
              v
+--------------------------------------+
| 整理 Q1-Q5 输出、时间线、解释到报告   |
+--------------------------------------+
```

## 3. 先记住这次 lab 要回答什么

题面一共要求 5 个问题：

1. `Question 1`
   - 跑 `make run`
   - 记录 violation 数量和百分比
   - 再改成 `make run-inorder`
   - 解释为什么 `in-order` 没有 violation
2. `Question 2`
   - 跑 `make run-fence`
   - 解释为什么 `fence rw, rw` 消除了双 0
   - 画出两个 core 的 `happens-before` 时间线
3. `Question 3`
   - 跑 `order_lab/Q2` 的 `make run`
   - 记录实际计数值、期望值 `2000`
   - 计算 lost updates
   - 解释为什么丢失量接近 `N_ITER`
4. `Question 4`
   - 跑 `order_lab/Q3` 的 `make run`
   - 记录实际计数值和期望值
   - 解释 `lock()` 在哪一步失效
   - 画出两个 core 同时进入 critical section 的时间线
5. `Question 5`
   - 打开 `peterson.c`
   - 在两个 `TODO` 位置加 `__asm__ volatile("fence rw, rw")`
   - 跑 `make run-fence`
   - 解释为什么必须是两个 fence

## 4. 这次 lab 的核心词汇

- `core`：CPU 核心，可以先把它当成一个能独立执行程序的小 CPU
- `instruction`：指令，也就是 CPU 真正执行的机器命令
- `shared memory`：共享内存，多个 core 都能看到的同一块内存
- `load`：读内存
- `store`：写内存
- `pipeline`：流水线，让多条指令的不同阶段重叠执行
- `OOO core`：乱序执行核心，后面的指令在条件满足时可能先做
- `in-order core`：顺序执行核心，大体按程序顺序推进
- `Memory Ordering`：内存顺序，指不同读写操作在其他核心眼里以什么顺序变得可见
- `SC`：`Sequential Consistency`，顺序一致性，最直观的理想模型
- `TSO`：`Total Store Order`，总存储顺序，比 `SC` 弱一些
- `Store-Load Reordering`：`store` 还没全局可见时，后面的 `load` 先完成
- `Store Buffer`：每个 core 前面的 store 缓冲区
- `Dekker litmus test`：经典并发测试，用来观察顺序违规
- `Violation`：这里特指本不该出现、但在 `TSO` 下出现的结果
- `Non-atomic`：非原子的，读改写不是一个不可分割的整体
- `Atomic`：原子的，整次操作像一个不可拆开的整体
- `Lost Update`：两个线程都做加一，但最后只保留了一次
- `Peterson's mutex`：一种只靠普通读写实现的软件互斥锁
- `Fence`：内存栅栏，强制某些操作不能乱序穿过它
- `happens-before`：先行发生关系，表示一个操作必须先于另一个操作生效
- `Critical Section`：临界区，同一时刻只允许一个线程进入的代码段

## 5. `SC` 和 `TSO` 到底差在哪里

### 5.1 `SC` 是什么

`SC` 可以先用最朴素的话理解。这里的“可见”你可以先理解成“别的 core 已经能观察到这次读写的结果”：

- 每个 core 自己写的程序顺序会被保留
- 所有 core 的内存操作好像被串成了一个总顺序
- 这个总顺序和每个 core 程序里的先后关系不冲突

所以在 `SC` 下，如果代码写成下面这样，意思就是“先写，再读”：

```text
store
load
```

你通常就会直觉地认为：

```text
别的 core 不应该先看到 load 的效果，再看到前面的 store
```

### 5.2 `TSO` 是什么

`TSO` 比 `SC` 弱，但不是完全乱来。这里所谓“弱”，不是说它错了，而是说它允许某些操作在别的 core 看来没有那么严格按代码顺序可见。

题面已经给了最关键的一句话：

```text
LOAD can pass an older STORE to a different address
```

意思是：

- 如果前面的 `store` 写地址 `A`
- 后面的 `load` 读地址 `B`
- 且 `A` 和 `B` 不同

那么在 `TSO` 下，后面的 `load` 可能先拿到结果，而前面的 `store` 还躺在 `store buffer` 里没对外可见。

### 5.3 `TSO` 不是所有顺序都打破

题面也强调了：

```text
Store-Store order is preserved
```

也就是说：

- `store -> store` 顺序仍然保留
- 真正危险的是 `store -> load`

所以这次 lab 的核心不是“乱序执行很复杂”，而是：

```text
为什么 store-load 这一种重排已经足够把并发程序搞坏
```

## 6. 什么是 `Store Buffer`

### 6.1 先理解它在干什么

每个 core 做 `store` 时，不一定立刻把值写到共享 cache 或 shared memory。这里的 shared memory 就是“两个 core 都能访问的同一块内存”。

更常见的做法是：

1. 先把这次 `store` 放进自己的 `store buffer`
2. core 继续往后执行
3. buffer 里的条目再慢慢 drain 到共享层次

这就是为什么：

- 这个 core 自己“觉得我已经 store 了”
- 另一个 core 却还“看不见这个 store”

### 6.2 这次实验里的配置

题面给的默认配置是。这里的 `OOO` 是 `Out-Of-Order`，意思是“允许后面的部分指令先做”；`drain` 可以理解成“把 store buffer 里的写操作慢慢排出去，真正送到共享层”：

- 两个 `OOO` core：`--dual --ooo --bpred -b`
- 每个 core 一个 `store buffer`
- `STORE_BUF_SIZE = 16`
- 每个 cycle 只 drain 一条 committed store 到共享 `L2`

你现在至少要抓住两个关键词：

1. `per-core`
   - 每个 core 都有自己的本地 buffer
2. `drain one entry per cycle`
   - store 不会瞬间对全世界可见

## 7. 为什么会出现 `Store-Load Reordering`

如果一个 core 里有这样的代码，也就是“先写 `x`，再读 `y`”：

```text
x = 1;
r1 = y;
```

在程序员脑中它当然是先写 `x` 再读 `y`。

但在 `TSO` + `store buffer` 下，真实可能发生的是：

1. `x = 1` 进入本 core 的 `store buffer`
2. 它还没 drain 到共享层
3. `r1 = y` 直接去共享层读
4. 这时另一个 core 还看不到 `x = 1`

这就形成了：

```text
程序顺序上 store 在前
全局可见性上 load 却先发生了效果
```

这就是题面说的 `Store-Load Reordering`。

## 8. `Dekker litmus test` 为什么能测出问题

题面里的 `Q1` 本质上是：

```text
Initially: x = 0, y = 0

Core 0: x = 1; [fence?] r1 = y
Core 1: y = 1; [fence?] r2 = x
```

### 8.1 在 `SC` 下

如果真按 `SC` 理解：

- 不可能两个 `store` 都还没对外可见
- 所以不可能同时读到：

```text
r1 = 0 && r2 = 0
```

### 8.2 在 `TSO` 下

如果两个 core 都把自己的 `store` 留在本地 `store buffer`：

- Core 0 的 `load y` 看到旧值 `0`
- Core 1 的 `load x` 也看到旧值 `0`

于是：

```text
r1 = 0 && r2 = 0
```

真的出现了。

这就是 `violation`。

## 9. 为什么 `in-order` 会没有 violation

题目要求你把 `Q1` 改成 `--inorder --bpred -b` 再跑。这里的 `in-order` 可以先理解成“CPU 不太会主动让后面的指令超到前面去做”。

核心理解是：

- `TSO` 只是允许 `Store-Load Reordering`
- 但不是说硬件一定会把它做出来
- 真正把这种可能性变成“高概率可观察现象”的，是 `OOO`

所以更直白地说：

```text
TSO 提供了“可以违规”的规则空间
OOO 提供了“真的把后面的 load 提前执行”的能力
```

而 `in-order pipeline` 不会积极地把后面的 `load` 提前穿过前面的 `store`，所以题面才说会出现零 violation。

## 10. `Fence` 为什么有用

`fence rw, rw` 可以先粗暴理解成：

```text
栅栏前面的读写没有完成并对外可见之前
栅栏后面的读写别想穿过去
```

放到 `Q1` 里：

```text
x = 1;
fence rw, rw;
r1 = y;
```

作用就是：

- 先让 `x = 1` 不再只是“躺在本地 buffer 里”
- 再允许后面的 `load y`

所以两个 core 不可能再同时看到对方的旧值 `0`。

## 11. 什么是 `happens-before`

这是写并发分析时特别有用的词。

你可以先把它理解成：

```text
如果 A happens-before B
那就表示 A 的效果必须先于 B 生效
```

在这次 lab 里：

- fence 的作用之一
- 就是人为建立某些必须遵守的 `happens-before`

例如：

```text
store(x=1)
    ->
fence
    ->
load(y)
```

表示：

- `load(y)` 不能再“越过 fence”先于前面的 `store(x=1)` 对外观察

## 12. 什么是 `Lost Update`

`Q2` 的程序是两个 core 都对同一个共享计数器做 `1000` 次加一。这里的“共享计数器”就是“两个 core 都在改同一个变量”。

但这个加一不是原子操作，而是：

```text
tmp = *counter;
tmp = tmp + 1;
*counter = tmp;
```

这其实是 3 步，不是一步。

### 12.1 为什么会丢更新

假设两个 core 同时看到：

```text
counter = 7
```

然后：

1. Core 0 读到 `7`
2. Core 1 也读到 `7`
3. 两边都算出 `8`
4. 两边都写回 `8`

最后共享值从 `7` 变成 `8`，而不是你期望的 `9`。

明明做了两次加一，但结果只保留了一次，这就是 `Lost Update`。

### 12.2 为什么题面说丢失量接近 `N_ITER`

因为这不是偶尔撞一次，而是实验环境故意把问题放大了：

- 两个 core 都在高速重复做同样的 non-atomic read-modify-write
- 每个 core 的 `store` 还要先经过自己的 `store buffer`
- drain 速度又有限

结果就是：

```text
两个 core 常常长时间都在看“差不多同一份旧值”
```

于是每一轮都很容易产生一次覆盖，导致丢失量接近 `N_ITER`，而不是只有零星几次。

## 13. 为什么 `++` 不是天然安全的

很多初学者看到：

```c
counter++;
```

会误以为它是“一条语句，所以是一个动作”。

但从并发和内存模型的角度看，它通常拆成：

1. `load`
2. `add`
3. `store`

只要这 3 步不是 `atomic`，两个 core 就可能互相覆盖。

所以这次 lab 不是在证明“写程序很难”，而是在证明：

```text
只要共享更新不是 atomic
store buffer 和 memory ordering 就足以把 bug 放大到肉眼可见
```

## 14. `Peterson's mutex` 是什么

`Peterson's algorithm` 是经典软件互斥算法。这里的“互斥”可以先理解成“同一时刻只允许一个线程进去”。

它不依赖原子指令，只靠普通读写变量：

- `want[i]`
- `want[j]`
- `turn`

来保证同一时间只有一个线程进入 `critical section`。

在 `SC` 下，它是正确的。

## 15. `Peterson` 在 `SC` 下为什么正确

每个线程进入锁时都会做：

```text
want[i] = 1;
turn = j;
while (want[j] && turn == j) {}
```

直觉上它的意思是：

1. 先宣布“我想进”
2. 再把优先权让给对方
3. 如果对方也想进，而且现在轮到对方，那我就等

在 `SC` 下：

- 这些写入会按你写代码的顺序被另一个 core 正确观察到
- 所以不可能两个 core 都错判“对方不想进”

## 16. `Peterson` 在 `TSO` 下为什么会坏

关键 bug 在这里：

```text
want[i] = 1;
```

它先进入本 core 的 `store buffer`，未必立刻对另一个 core 可见。

于是两边都可能发生：

1. 我把 `want[i] = 1` 写进了自己的 buffer
2. 但对方还看不见
3. 我去读 `want[j]`
4. 我看到的是旧值 `0`

结果是：

```text
两个 core 都以为“对方还没想进”
```

两边就可能同时穿过 `while`，一起进入 `critical section`，互斥失败。

## 17. 为什么 `Peterson` 这里要放两个 `fence`

题面明确说要在 `lock()` 里放两个 `fence rw, rw`：

```c
want[i] = 1;
/* fence 1 */
*turn = j;
/* fence 2 */
while (want[j] && *turn == j) {}
```

### 17.1 第一个 fence 的作用

放在 `want[i] = 1` 后面，是为了：

- 先把“我想进”的信号推出去
- 不要让后面读取 `want[j]` 时，对方还看不到我的意图

也就是：

```text
先保证 want[i] = 1 对外可见
再去看对方 want[j]
```

### 17.2 第二个 fence 的作用

放在 `turn = j` 后面，是为了：

- 防止 `turn` 的 store 被后面的读操作穿过去
- 确保检查 `while (want[j] && *turn == j)` 时，`turn` 已经按预期建立好顺序

### 17.3 为什么一个 fence 不够

因为这两个 fence 管的是两段不同的顺序关系：

1. `want[i]` 的可见性
2. `turn` 和后续读条件之间的顺序

只放一个 fence，通常只能堵住其中一处 `Store-Load Reordering`，另一处仍然可能漏掉。

## 18. `Barrier` 在 `Q1` 里是做什么的

题面提到有个 `rendezvous barrier`，用 monotonic sequence numbers 和 `fence rw, rw` 让两个 core 在 trial 之间同步。

这里你要特别注意：

```text
这个 barrier 只是为了让每次试验都从干净状态开始
它不是被测试的 litmus sequence 本身
```

也就是说：

- 你不能把 `Q1` 的 violation 归因到 barrier
- barrier 是实验控制变量，不是 bug 来源

## 19. 默认模拟器配置要记什么

题面给的默认配置里，最重要的是这些：

- `--dual`
  - 两个 core
- `--ooo`
  - `out-of-order` 执行
- `--bpred -b`
  - 开启分支预测相关配置
- `STORE_BUF_SIZE = 16`
  - 每个 core 有 16 项 store buffer
- `TSO`
  - 允许 `Store-Load Reordering`

你写报告时，不用把所有参数都展开成长篇解释，但要会抓住和现象最相关的点：

```text
dual + ooo + per-core store buffer + TSO
```

## 20. 这次 lab 最常见的理解误区

### 20.1 误区一：只要是按代码顺序写的，硬件就会按这个顺序让别人看见

错。

这次 lab 正是在证明：

```text
程序顺序 != 全局可见顺序
```

### 20.2 误区二：`++` 既然是一条语句，就是原子的

错。

如果没有 `atomic` 或锁，它通常只是 `load + add + store`。

### 20.3 误区三：`Peterson` 是经典算法，所以在哪种机器上都一定正确

错。

它依赖的前提是较强的内存顺序；题面已经明确说它在 `SC` 下正确，但在 `TSO` 下会坏。

### 20.4 误区四：加一个 fence 就能解决所有顺序问题

错。

要看你到底要禁止哪一段重排。题面特地要求你分析为什么这里必须是两个 fence。

## 21. 写报告时的最短答题逻辑

如果你后面写 `Q1-Q5` 卡住，可以统一按这个套路：

1. 先写观察到的现象
2. 再写底层原因
3. 再把原因和 `TSO` / `store buffer` / `fence` 关联起来
4. 最后说明为什么结果和 `SC` 不同

例如 `Q3` 可以按这个结构写：

```text
实际 counter 明显小于 2000，
说明两个 core 的 non-atomic read-modify-write 发生了大量覆盖；
由于 store 先进入 per-core store buffer，
另一个 core 在一段时间内仍可能读取到旧值，
于是双方反复基于同一个旧值计算并写回，
导致 lost updates 的数量接近 N_ITER。
```

## 22. 最小结论

这次 lab 最核心的 6 个结论是：

1. `TSO` 允许 `Store-Load Reordering`
2. `Store Buffer` 是这种现象能被观察到的直接硬件原因
3. `OOO` 会把这种潜在可能变成更容易观察到的 violation
4. `Non-atomic` 共享更新在并发下会出现 `Lost Update`
5. `Peterson's mutex` 依赖较强顺序，在 `TSO` 下会失效
6. `fence rw, rw` 的作用是强制顺序，恢复正确同步语义
