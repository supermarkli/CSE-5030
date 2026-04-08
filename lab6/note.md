# Lab 5 Branch Predictor Lab 零基础笔记

## 1. 这次 lab 到底在做什么

这次 lab 的主线不是“把程序跑起来”这么简单，而是理解：

`一条 branch 指令 -> 预测 taken / not-taken -> 预测 target -> CPU 先按预测继续跑 -> 真正执行后检查对不对`

如果预测错了：

- 前面按错误方向投机执行的指令要被清掉
- `IPC` 会下降
- `cycles` 会上升

所以这次 lab 本质上是在训练你理解：

1. `branch predictor` 为什么能影响性能
2. `BTB` 为什么会发生 aliasing
3. `gshare` 为什么会有 cold start penalty
4. `OOO` 流水线里，为什么“历史寄存器更新太晚”会伤害预测器
5. 怎样设计一个更适合当前 workload 的预测器

## 2. 先记住这次 lab 的四个任务

### 2.1 Question 1

比较：

- `make run-nobpred`
- `make run-bpred`

看开启 predictor 之后：

- `IPC` 怎么变
- `mispredictions` 怎么变

### 2.2 Question 2 / Question 3

分析 `BTB aliasing`：

- 找到 `loop_A` 和 `loop_B` 的回边 branch 地址
- 计算它们的 `BTB index`
- 判断是否冲突
- 解释为什么 `aligned(2048)` 会制造冲突
- 去掉对齐后再跑一次，解释为什么冲突消失

### 2.3 Question 4

分析 `gshare` 的 `cold start`：

- 为什么 `cold` 比 `warm` 慢很多
- 为什么 `reset_ghr()` 后，两次测量会落到同一批 `PHT` 槽位上

### 2.4 Question 5

自己设计一个 `bpred2`：

- 修改 `src/cpu/bpred2.c`
- 目标是准确率超过 tournament predictor 的 `82.44%`
- 解释核心思路和验证过程

## 3. branch 是什么

`branch` 就是“根据条件决定要不要跳转”的指令。

例如：

- 条件成立：跳到目标地址
- 条件不成立：顺序执行下一条

常见两类相关控制流指令：

- `conditional branch`
- `jump / call / return`

这次 lab 重点主要在条件分支的方向预测，以及 taken 分支的目标地址预测。

## 4. 为什么 branch prediction 很重要

CPU 取指时还没真正算出 branch 条件，但为了不让流水线停住，必须先猜：

- 这条 branch 会不会跳
- 如果跳，跳到哪里

这就叫 `branch prediction`。

如果猜对了：

- 后面的取指不会断
- `IPC` 更高

如果猜错了：

- CPU 要 flush 错路径上的指令
- 白跑的指令越多，损失越大

在 `out-of-order (OOO)` 处理器里，这个损失通常比 `in-order` 更明显，因为 OOO 会让更多指令提前在飞。

## 5. 这次 lab 里的几个核心指标

### 5.1 IPC

`IPC = Instructions Per Cycle`

含义：

- 平均每个周期退休多少条指令

一般来说：

- 越大越好

### 5.2 cycles

程序总共跑了多少周期。

一般来说：

- 越少越好

### 5.3 mispredictions

预测错误次数。

一般来说：

- 越少越好

### 5.4 BTB cold misses

`BTB` 里原本没有这条 taken branch 的目标信息，所以第一次遇到它时预测不了目标。

这类 miss 常出现在：

- 第一次执行
- 冲突把旧项顶掉之后再次执行

## 6. 什么是 2-bit saturating counter

这是最经典的方向预测器元件。

一个 2-bit 饱和计数器通常有 4 个状态：

```text
0: strongly not-taken
1: weakly not-taken
2: weakly taken
3: strongly taken
```

你可以把它理解成“朝 taken 或 not-taken 倾斜”的小状态机。

更新规则很简单：

- 实际是 `taken`，计数器加 1，最多到 `3`
- 实际是 `not-taken`，计数器减 1，最少到 `0`

预测规则通常是：

- `>= 2` 预测 `taken`
- `< 2` 预测 `not-taken`

## 7. 为什么严格交替的 T/NT/T/NT 很难预测

如果一个 branch 的结果是：

```text
T, NT, T, NT, T, NT ...
```

2-bit counter 往往会在中间两种弱状态来回摇摆。

于是会发生：

- 刚学到 `taken`
- 下一次就变成 `not-taken`
- 再下一次又反过来

所以这种模式会导致很高的误预测率。

这就是 Q1 要你观察的核心现象。

重点不是“开 predictor 一定更快”，而是：

有些模式本身就和简单 predictor 不匹配。

## 8. 什么是 BTB

`BTB = Branch Target Buffer`

它主要回答：

> 如果这条 branch 预测为 taken，那我要跳到哪里？

也就是说，方向预测器回答“跳不跳”，BTB 回答“跳去哪”。

这次 lab 里的默认 BTB 配置是：

- `512` entries
- `index = PC[10:2]`
- `tag = PC >> 11`

## 9. BTB 的 index 和 tag 怎么理解

一条 branch 指令地址是 `PC`。

BTB 不会把整个 `PC` 都拿来当数组下标，而是：

- 用 `PC[10:2]` 选表项位置
- 用更高位 `PC >> 11` 做 `tag`

这样做的原因是：

- 表项数量有限
- 只能用部分位索引

但这会带来冲突问题。

### 9.1 BTB index 的计算

如果题目要你算某条 branch 的 BTB index，公式就是：

```text
BTB index = (PC >> 2) & 0x1FF
```

原因：

- `512 = 2^9`
- 所以 index 只需要 `9` 位
- 正好就是 `PC[10:2]`

### 9.2 tag 的作用

如果两条不同地址的 branch 恰好落到同一个 index：

- 只靠 index 无法分辨它们
- 所以还要再检查 tag

如果 tag 不匹配，就说明：

- 这个槽位里存的是“别的 branch”的信息
- 当前 branch 不能直接用这条 BTB 记录

## 10. 什么是 BTB aliasing

`BTB aliasing` 可以理解成：

- 两条不同的 branch
- 落到了同一个 BTB index

如果它们交替出现，就可能互相覆盖 BTB 表项。

结果就是：

- 一会儿槽位里是 `loop_A`
- 一会儿又被 `loop_B` 改写
- 下次回来时 tag 不匹配
- 造成额外的 cold miss 或错误目标预测

这就是 Q2 / Q3 的主线。

## 11. 为什么 `aligned(2048)` 会导致 aliasing

题目里 `loop_B` 带了：

```c
__attribute__((aligned(2048)))
```

`2048 = 2^11`

这件事很关键，因为：

- BTB index 用的是 `PC[10:2]`
- 如果两个函数地址刚好相差 `2048` 的整数倍
- 那它们的 `PC[10:2]` 可能完全一样

换句话说：

```text
地址高位变了
但 index 那 9 位没变
```

于是：

- `index` 冲突
- `tag` 不同
- 形成 `BTB aliasing`

这就是题目要你分析“function alignment causes aliasing”的意思。

## 12. 什么是 back-edge branch

循环末尾通常会有一条“跳回循环开头”的 branch。

这条 branch：

- 目标地址更小
- 控制下一轮循环

它就叫 `back-edge branch`。

在 Q2 里你要在 `bench.dis` 里找到：

- `loop_A` 的回边 branch 地址
- `loop_B` 的回边 branch 地址

因为真正进入 BTB 的，通常就是这些实际执行的 branch 指令。

## 13. BTB tag mismatch 的性能代价是什么

如果 `index` 碰撞，但 `tag` 不同：

- 当前槽位虽然“有人占着”
- 但不是当前 branch 的记录

这时的性能代价通常是：

1. 这次 taken branch 拿不到正确 target
2. 前端不能像命中 BTB 那样立即跳到正确地址
3. 会出现额外 fetch/redirect 开销
4. 分支预测统计里的 `BTB cold misses` 会增加

虽然严格说它不是“第一次见到这条 branch”，但从当前 branch 的视角看，效果和冷启动很像。

## 14. local predictor 是什么

`local predictor` 按“每个 PC 自己的历史”来预测。

这次配置里：

- `LHT` 有 `1024` 项
- 每项是 `10-bit` 历史
- 这个历史再去索引 `LPHT`

你可以把它理解成：

1. 先看“这条 branch 过去 10 次大概什么模式”
2. 再根据这个历史去查一个 2-bit counter

它擅长：

- 针对某一条固定 branch 的重复模式

## 15. global predictor / gshare 是什么

`global predictor` 看的是“最近全局发生过哪些 taken / not-taken”。

这次是 `gshare`：

- `GHR` 宽度是 `12 bits`
- 用 `GHR XOR PC` 去索引 `GPHT`

它的特点是：

- 会把不同 branch 之间的关联也编码进去
- 适合“当前 branch 受前面别的 branch 影响”的情况

### 15.1 GHR 是什么

`GHR = Global History Register`

它是一串最近分支结果的位串，比如：

```text
... 1 0 1 1 0
```

其中：

- `1` 表示 taken
- `0` 表示 not-taken

每来一条新 branch，就左移再塞入最新结果。

## 16. tournament predictor 是什么

这次默认 predictor 不是只用 local 或只用 global，而是 `tournament predictor`。

它的组成是：

- `local predictor`
- `global predictor (gshare)`
- `meta selector`

`meta selector` 负责决定：

- 当前这次更相信 local
- 还是更相信 global

PDF 里给的规则是：

- `meta >= 2` 选 global
- `meta < 2` 选 local

所以它本质上是“两个专家 + 一个裁判”。

## 16.5 `BTB`、`predictor`、`snpc`、`dnpc` 到底怎么配合

如果你从 `local predictor` 开始觉得绕，通常是因为这几个东西混在一起了。

先强行拆开：

- `predictor` 负责猜：这次 `taken` 还是 `not-taken`
- `BTB` 负责给：如果 `taken`，目标地址 `target` 是多少
- `snpc` 表示顺序执行时“下一条本来该去哪里”
- `dnpc` 表示这条指令真正执行完后，“最后实际去哪里”

可以先看这张图：

```text
当前取到一条 branch，地址是 PC
        |
        v
+----------------------+
| predictor 看历史信息 |
| 猜 taken / not-taken |
+----------------------+
        |
        +--------------------+
        |                    |
   预测 not-taken        预测 taken
        |                    |
        v                    v
   预测下一条 = snpc      去 BTB 查 target
                             |
                             +--------------------+
                             |                    |
                         BTB hit              BTB miss
                             |                    |
                             v                    v
                      预测下一条 = target     目标地址拿不到
                             |
                             v
                 后端真正执行这条 branch
                             |
                             v
            得到真实结果：这次实际应该去 dnpc
                             |
                             v
          比较“预测的下一条” 和 “真实 dnpc” 是否一致
```

### `snpc` 是什么

`snpc` 可以理解成：

```text
如果什么跳转都不发生，
按顺序执行的话，
下一条指令地址应该是多少
```

对于大多数普通指令来说：

- 下一条就是顺序地址
- 所以通常 `dnpc == snpc`

对于 branch 指令来说：

- 如果条件不成立，不跳
- 那么 `dnpc == snpc`
- 如果条件成立并发生跳转
- 那么 `dnpc == target`，通常就不等于 `snpc`

### `dnpc` 是什么

`dnpc` 是：

```text
这条指令真正执行结束后，
CPU 最终应该去的下一条地址
```

所以它才是“真实答案”。

### 一个具体例子

假设当前有一条 branch：

```text
PC = 0x1040
snpc = 0x1044
target = 0x1000
```

如果这次条件成立：

- 实际会跳回循环头
- 所以 `dnpc = 0x1000`

如果这次条件不成立：

- 实际不跳
- 所以 `dnpc = 0x1044`

### predictor 和 BTB 分别在干嘛

这时候 CPU 要先猜。

#### 情况 1：predictor 猜 `not-taken`

那它会先按：

```text
预测下一条 = snpc = 0x1044
```

继续取指。

这时根本不用 `BTB`。

#### 情况 2：predictor 猜 `taken`

那它会说：

```text
这次应该跳
```

但“跳到哪里”它自己不知道，所以要去问 `BTB`。

如果 `BTB` 命中，就能拿到：

```text
target = 0x1000
```

于是 CPU 先去 `0x1000` 取指。

### 为什么 `BTB` 不能代替 predictor

因为 `BTB` 只回答：

```text
如果要跳，跳到哪里
```

它不回答：

```text
这次到底该不该跳
```

所以不是“根据 BTB 来跳转”，而是：

1. 先由 predictor 猜方向
2. 如果猜 `taken`，再由 BTB 给目标地址

你可以记成：

```text
predictor = 要不要跳
BTB       = 跳去哪
```

### 为什么 local predictor 还是必要的

假设 `BTB` 已经记住某条 branch 的 target 是 `0x1000`。

这并不代表：

- 这条 branch 每次都要去 `0x1000`

因为 branch 有时跳，有时不跳。

例如循环尾部 branch：

- 前 9 次可能都跳回去
- 最后 1 次退出循环就不跳了

如果没有方向预测器，只靠 BTB，你根本不知道：

- 这次该走 `target`
- 还是该走 `snpc`

所以 `local predictor` / `global predictor` 的意义就在于：

- 它们根据历史，先猜这次是 `taken` 还是 `not-taken`

### 再用一句最短的话总结

```text
BTB 不是“分支预测器的全部”
BTB 只是 taken 分支的目标地址缓存
真正决定 taken / not-taken 的，是 local / global / tournament predictor
```

## 17. 什么是 cold start

`cold start` 指 predictor 刚开始时还没学到模式。

例如：

- `PHT` 里计数器全部初始化为 `0`
- 也就是 `strongly not-taken`

如果实际 branch 都是 `taken`，那一开始会连续猜错很多次。

等运行几轮之后：

- 这些 counter 被训练到 `2` 或 `3`
- 再预测同样的 branch 就会快很多

这就是 Q4 中 `cold` 和 `warm` 的差别来源。

## 18. 为什么 Q4 的 cold 比 warm 慢很多

Q4 的 `probe_branches()` 里有 `32` 个 always-taken branches。

而初始状态下：

- 所有 2-bit counters 都是 `0`
- 默认强烈偏向 `not-taken`

所以第一次测量时：

- 前面很多 branch 都会错
- 每次错都要 recover
- 周期数明显升高

而 warm pass 前已经做了 8 轮 warm-up：

- 对应槽位基本被训练到 `3`
- 再测量时大部分预测都会命中

所以 warm 会明显更快。

## 19. 为什么 `reset_ghr()` 很重要

如果不重置 `GHR`，那么：

- 同一批 branch 在两次测量时
- 可能会因为 GHR 不同而落到不同的 `GPHT` 槽位

这样 cold 和 warm 就不公平了。

`reset_ghr()` 的作用就是：

- 在 cold 和 warm 两次正式测量前
- 都把全局历史清成同样状态

于是两次访问的是同一批 gshare 表项。

这样你观察到的差别就主要来自：

- 这些计数器有没有被训练过

## 20. OOO 流水线里为什么会出现“stale history”

这是这次 lab 最关键也最容易绕晕的点。

Q5 的题面已经把根因说出来了：

- 同一条 branch 的多个实例会同时在飞
- 但 `history register` 只在 `EX stage` 才更新

结果是：

- 第 1 个实例还没执行到 EX
- 第 2 个实例就已经来预测了
- 第 2 个实例读到的还是旧历史
- 第 3 个实例也可能还是旧历史

于是连续几个预测看到的是同一份 `stale history`。

## 21. 为什么 stale history 会把交替模式预测坏

假设某条 branch 真正模式是：

```text
T, NT, T, NT, ...
```

理想情况下：

- 每次预测前都应该知道上一次真实结果

但如果历史更新太晚：

- 连续几个 in-flight 实例都看到了同一个旧历史
- 那它们就会做出同一个方向预测

问题在于：

- 真实模式是交替
- 连续预测却给了同样方向

所以在条件 branch 上会逼近 `50%` miss rate。

这也是 tournament baseline 在这个 workload 上只有大约 `82%` 准确率的核心原因。

## 22. 什么叫 speculative update

`speculative update` 的意思是：

- branch 还没真正执行完
- 但 predictor 先假设自己的预测是对的
- 把“预测出来的结果”先写进一个 speculative history

这样下一条同类 branch 来预测时，就不会总读到旧历史。

如果后面发现预测错了：

- 再把 speculative history 拉回已提交状态

这就是 Q5 里非常自然的一种设计方向。

## 23. `bpred2.c` 里 skeleton 已经在暗示什么

当前 `src/cpu/bpred2.c` 里已经有：

- `static uint32_t spec_lht[LHT_SIZE];`

而且有两个 TODO：

1. 在 `bpred2_predict()` 里 speculative update `spec_lht`
2. 在 `bpred2_update()` 里 misprediction 时把 `spec_lht` 同步回 committed `lht`

这说明出题人已经在引导你：

- 用“投机本地历史”去解决 OOO 下历史更新太晚的问题

它不是让你凭空发明一整套很复杂的新 predictor，而是要你抓住：

```text
关键 bug / 瓶颈在历史更新时机
```

## 24. 为什么短历史有时比长历史更适合这个题

`bpred2.h` 文件注释已经提示了一个方向：

- 用更短的 per-PC history
- 让同一类模式更快重复命中同一批 PHT 槽位

原因是：

- 交替模式本身只需要很短的历史就能描述
- 历史过长反而会把访问分散到很多槽位
- 训练会变慢

所以对这个题来说，关键不是“历史越长越高级”，而是：

- 历史长度要匹配 workload 的规律

## 25. 你在 Q5 里真正要证明什么

不是证明“我代码能跑”就够了，而是证明：

1. baseline 的弱点在哪里
2. 你的设计怎样针对这个弱点
3. 改完后 `Accuracy`、`Mispredictions`、`IPC` 有什么变化
4. 这些变化为什么合理

也就是说，Q5 的核心是：

```text
问题定位 -> 设计思路 -> 实现 -> 验证
```

## 26. 这次 lab 的输出里你要重点看什么

### 26.1 Q1

重点看：

- `IPC`
- `Predictions`
- `Mispredictions`
- `Accuracy`
- `BTB cold misses`

### 26.2 Q2 / Q3

重点看：

- `bench.dis`
- `loop_A` / `loop_B` 回边地址
- 算出来的 `BTB index`
- 改掉 `aligned(2048)` 前后的 `BTB cold misses`

### 26.3 Q4

重点看：

- `cold: cycles=...`
- `warm: cycles=...`

### 26.4 Q5

重点看：

- baseline `run-bpred`
- 自定义 `run-bpred2`
- `Accuracy`
- `Mispredictions`
- `IPC`

## 27. 一个容易踩坑的目录问题

这次资料里有一个很容易混淆的地方：

- PDF 的 `Step 4 / Question 5` 对应的是 `predictor_lab/Q4`
- 但源码里还存在 `predictor_lab/Q5`

根据当前仓库实际内容：

- `predictor_lab/Q4/Makefile` 才有 `run-bpred2`
- 所以这次要做的自定义 predictor 任务在 `Q4`
- `predictor_lab/Q5` 是下一题 `Spectre v1`，不是本次 lab 的提交范围

做实验时不要跑错目录。

## 28. 你做这次 lab 时的正确理解顺序

建议按这个顺序理解：

1. 先理解 branch prediction 在解决什么问题
2. 再理解 2-bit counter 为什么怕交替模式
3. 再理解 BTB 的 index / tag 和 aliasing
4. 再理解 local / global / tournament 各自看什么历史
5. 再理解 cold start 为什么本质上是“还没训练好”
6. 最后理解 OOO 下 stale history 为什么会打坏 predictor

## 29. 你写报告时最常用的论证顺序

每题都建议按这个顺序写：

1. 先说现象
2. 再说底层原因
3. 再把原因和配置参数对应起来
4. 最后说明为什么结果合理

比如 Q2 的答题顺序可以是：

```text
先给 back-edge PC
-> 再算 BTB index
-> 再说明 index 冲突 / tag 不同
-> 再解释为什么这会增加 BTB cold misses
```

## 30. 这次 lab 最该记住的结论

- branch predictor 不是永远都准，模式不匹配时会明显失效
- `BTB` 负责 taken branch 的目标地址，不只是“要不要跳”
- `PC[10:2]` 相同会导致 BTB index 冲突
- `aligned(2048)` 之所以危险，是因为 `2048 = 2^11`，正好跨过 tag 边界但不改变 index
- `cold start` 的本质是 predictor 还没被训练
- `gshare` 依赖 `GHR`，所以 `reset_ghr()` 是公平比较 cold / warm 的关键
- OOO 下多个 in-flight branches 会读到 `stale history`
- Q5 的设计重点不是堆复杂结构，而是让历史在预测时更接近真实未来

## 31. 最小词汇表

- `branch prediction`：分支预测
- `taken / not-taken`：跳转 / 不跳转
- `BTB`：Branch Target Buffer
- `aliasing`：冲突映射
- `tag mismatch`：标签不匹配
- `local predictor`：局部预测器
- `global predictor`：全局预测器
- `gshare`：GHR 与 PC 异或索引的全局预测器
- `GHR`：Global History Register
- `LHT`：Local History Table
- `LPHT`：Local Pattern History Table
- `GPHT`：Global Pattern History Table
- `META`：选择 local / global 的元预测器
- `cold start`：冷启动
- `speculative update`：投机更新
- `stale history`：过时历史
- `back-edge branch`：循环回边分支
- `IPC`：每周期退休指令数
- `misprediction`：误预测

## 32. 后续怎么用这份笔记

建议这样用：

1. 做 Q1 前先看第 6 到第 7 节
2. 做 Q2 / Q3 前重点看第 8 到第 13 节
3. 做 Q4 前重点看第 17 到第 19 节
4. 做 Q5 前重点看第 20 到第 24 节
5. 写报告时反复对照第 29 节的论证顺序
