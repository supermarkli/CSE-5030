# Lab 5 Todo List

## 1. 总流程先看一眼

```text
确认环境和工具
    ->
在 SUSTemu 根目录完成一次 make clean && make
    ->
Q1 跑 run-nobpred / run-bpred，截图 IPC 和分支统计
    ->
Q2 跑 run-bpred，再做 disasm，算 loop_A / loop_B 的 BTB index
    ->
Q3 去掉 aligned(2048)，重编译，比较 BTB cold misses 变化
    ->
Q4 跑 cold / warm，记录 cycles 并解释 cold start
    ->
Q5 修改 src/cpu/bpred2.c，跑 baseline 和 bpred2，对比准确率与 IPC
    ->
整理 report 和截图
```

## 2. 先明确这次要交什么

根据 PDF，这次要交：

1. `Question 1`
   - 两张截图
   - 内容要能看到 `IPC` 和 `Branch Prediction Statistics`
2. `Question 2`
   - `loop_A` 和 `loop_B` 的 back-edge `PC`
   - 各自的 `BTB index`
   - 是否冲突
   - `BTB tag mismatch` 的性能代价解释
3. `Question 3`
   - 去掉 `aligned(2048)` 前后的对比
   - 至少比较 `BTB cold misses`
   - 解释为什么 aliasing 消失
4. `Question 4`
   - `cold` 和 `warm` 的 cycle 数
   - 解释为什么 `cold` 更慢
5. `Question 5`
   - `bpred2.c`
   - `make run-bpred` 和 `make run-bpred2` 的截图
   - 比较 `misprediction count` 与 `IPC`
   - 说明设计核心思路

## 3. 先认清目录，避免跑错

### 要做什么

确认这次 predictor lab 在哪里，尤其是 Q5 的实现目录。

### 为什么这么做

因为当前仓库里：

- PDF 的 `Question 5` 实际对应 `predictor_lab/Q4`
- `predictor_lab/Q5` 是另一题 `Spectre v1`

如果目录跑错，后面分析和截图都会错位。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
ls -l
ls -l predictor_lab
ls -l predictor_lab/Q4
```

### 指令含义

1. `cd /home/lzh/CSE5030/SUSTemu`
   - 切到模拟器仓库根目录
2. `ls -l`
   - 看当前目录内容和权限
3. `ls -l predictor_lab`
   - 确认 Q1 到 Q5 子目录都存在
4. `ls -l predictor_lab/Q4`
   - 确认这题对应目录里有 `Makefile`、`bench.c`

## 4. Step 1：预检工具和路径

### 要做什么

检查这次实验会用到的工具。

### 为什么这么做

题目里会用到：

- `make`
- `riscv64-unknown-elf-gcc`
- `riscv64-unknown-elf-objdump`
- `rg`

先确认，避免做到一半卡住。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
command -v make
command -v riscv64-unknown-elf-gcc
command -v riscv64-unknown-elf-objdump
command -v rg
ls -l build/sustemu
```

### 指令含义

1. `command -v make`
   - 检查 `make` 是否可用
2. `command -v riscv64-unknown-elf-gcc`
   - 检查 RISC-V 交叉编译器是否安装
3. `command -v riscv64-unknown-elf-objdump`
   - 检查反汇编工具是否安装
4. `command -v rg`
   - 检查快速搜索工具是否安装
5. `ls -l build/sustemu`
   - 看模拟器二进制是否已经存在

## 5. Step 2：先在仓库根目录完整编译一次

### 要做什么

在 `SUSTemu` 根目录执行完整编译。

### 为什么这么做

这次后面会多次运行 benchmark；如果模拟器本身还没编好，后面每一步都会失败。

另外，PDF 明确提醒：

- 修改 `include/` 下面任何文件后
- 都要回仓库根目录执行 `make clean && make`

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
make clean && make
```

### 指令含义

1. `make clean`
   - 清理旧编译产物
2. `&&`
   - 前一条成功后才执行后一条
3. `make`
   - 重新构建模拟器

### 做完如何确认

如果编译成功，后续 `predictor_lab/Q1~Q4` 里的 `make run-*` 才能正常调用 `build/sustemu`。

## 6. Step 3：做 Question 1，比较有无 predictor

### 要做什么

在 `Q1` 分别跑：

- `run-nobpred`
- `run-bpred`

### 为什么这么做

Q1 要你直接观察：

- predictor 开和不开时
- `IPC` 和 `Branch Prediction Statistics` 有什么差异

这个 benchmark 的 branch 是严格交替的 `T/NT/T/NT`，很适合用来展示简单预测器的局限。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q1
ls -l
make run-nobpred
make run-bpred
```

### 指令含义

1. `cd .../Q1`
   - 进入 Q1 目录
2. `ls -l`
   - 确认 `Makefile` 和 `bench.c` 存在
3. `make run-nobpred`
   - 用 OOO pipeline 跑，但分支一律预测 not-taken
4. `make run-bpred`
   - 用 OOO pipeline 跑，并开启 tournament predictor

### 这一步要记录什么

从两次输出里记下：

- `cycles`
- `IPC` 或 `IPC*100`
- `Predictions`
- `Mispredictions`
- `Accuracy`
- `BTB cold misses`

### 这一步做完后要做什么

把两次终端输出截图，后面放进报告的 `Question 1`。

## 7. Step 4：做 Question 2，先跑一次 BTB aliasing benchmark

### 要做什么

先在 `Q2` 原始版本下运行一次。

### 为什么这么做

因为你需要先拿到“有 `aligned(2048)` 时”的 baseline，再去分析 aliasing 和后续变化。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q2
ls -l
make run-bpred
```

### 指令含义

1. `cd .../Q2`
   - 进入 Q2 实验目录
2. `ls -l`
   - 查看文件是否齐全
3. `make run-bpred`
   - 在开启 tournament predictor 的情况下运行基准程序

### 这一步要记录什么

至少要记：

- `cycles`
- `BTB cold misses`
- 其他 branch prediction statistics

## 8. Step 5：生成反汇编并找到 back-edge branch 地址

### 要做什么

生成 `bench.dis`，找到 `loop_A` 和 `loop_B` 的回边 branch。

### 为什么这么做

Q2 不是只让你看运行结果，而是要你从指令地址出发，真正算出：

- 两条 branch 的 `PC`
- 它们的 `BTB index`

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q2
make disasm
rg -n 'loop_A|loop_B|bne|blt|beq|jal' bench.dis
```

### 指令含义

1. `make disasm`
   - 生成带源码注释的反汇编文件 `bench.dis`
2. `rg -n 'loop_A|loop_B|bne|blt|beq|jal' bench.dis`
   - 搜索函数名和分支指令，并显示行号，方便定位循环回边

### 如何找到 back-edge

你要找的是“循环尾部跳回循环头”的那条 branch。它的特点通常是：

- 位于 `loop_A:` 或 `loop_B:` 函数内部
- 目标地址比当前地址小
- 常见是 `bne` / `blt`

### 算 BTB index 的公式

```text
BTB index = (PC >> 2) & 0x1FF
```

### 为什么是这个公式

因为 PDF 已经给出：

- BTB 用 `PC[10:2]` 做 index
- `512` entries 需要 `9` 位

## 9. Step 6：填写 Question 2 的分析

### 要做什么

把 `loop_A` 和 `loop_B` 的：

- back-edge `PC`
- `BTB index`
- 是否冲突

写进报告。

### 为什么这么做

Q2 真正要考的是：

- 你是否理解 `index` 和 `tag` 的角色
- 你是否能把函数对齐和 BTB aliasing 联系起来

### 推荐写法

按这个顺序组织：

1. 给出 `loop_A` 的 back-edge `PC` 和 index
2. 给出 `loop_B` 的 back-edge `PC` 和 index
3. 判断两个 index 是否相同
4. 如果相同，再说明 tag 是否不同
5. 解释 `BTB tag mismatch` 的性能代价

### `BTB tag mismatch` 应该怎么解释

可以写成：

- BTB 槽位被另一条 branch 占据
- 当前 branch 在同一个 index 上看到的是错误 tag
- 因此不能直接使用该 target
- 会产生额外 target redirection / fetch 开销，并增加 BTB cold misses

## 10. Step 7：做 Question 3，去掉 `aligned(2048)`

### 要做什么

修改 `predictor_lab/Q2/bench.c`，去掉 `loop_B` 上的 `aligned(2048)`。

### 为什么这么做

这是题目用来证明 aliasing 根因的实验对照组。

如果冲突真的是由 `2048` 字节对齐制造的，那么去掉这个属性后：

- `loop_B` 的地址会变化
- 回边 branch 的 index 很可能不再与 `loop_A` 重合
- `BTB cold misses` 会下降

### 修改位置

原始代码大致是：

```c
__attribute__((noinline, aligned(2048)))
static long loop_B(void)
```

修改后应变成：

```c
__attribute__((noinline))
static long loop_B(void)
```

### 为什么不要顺手改别的

因为这题要做的是“单变量对照”：

- 只改对齐属性
- 这样前后结果才有解释力

## 11. Step 8：重编译并重跑 Question 3

### 要做什么

改完 `bench.c` 后，重新运行 Q2 benchmark。

### 为什么这么做

需要拿“去掉对齐后”的新结果，与原始版本对比。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q2
make clean
make run-bpred
make disasm
```

### 指令含义

1. `make clean`
   - 删除 Q2 目录下旧的 `.o`、`.elf`、`.bin`、`.dis`
2. `make run-bpred`
   - 用更新后的 `bench.c` 重新编译并运行
3. `make disasm`
   - 重新生成新的 `bench.dis`，用于确认 branch 地址是否变化

### 这一步要记录什么

重点记录：

- 新的 `BTB cold misses`
- 新的 `loop_B` back-edge `PC`
- 新的 `BTB index`

## 12. Step 9：写 Question 3 的结论

### 要做什么

把改动前后的数据整理成对照。

### 为什么这么做

Q3 的关键不是“我改了一个 attribute”，而是证明：

- aliasing 的根因是地址映射冲突
- 而不是程序逻辑本身

### 推荐写法

先写现象：

- 去掉 `aligned(2048)` 后，`BTB cold misses` 从 `TODO` 变成 `TODO`

再写原因：

- `loop_B` 的回边地址变化了
- 计算出的 `BTB index` 不再与 `loop_A` 冲突
- 因此同一个 BTB 槽位不再被交替覆盖

## 13. Step 10：做 Question 4，测 cold / warm

### 要做什么

运行 `Q3` 的 cold-start benchmark。

### 为什么这么做

这一步要直接测出：

- `cold: cycles=...`
- `warm: cycles=...`

并解释 predictor 从“未训练”到“训练完成”时的性能差别。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q3
ls -l
make run-ooo
```

### 指令含义

1. `cd .../Q3`
   - 进入 cold start 实验目录
2. `ls -l`
   - 确认文件存在
3. `make run-ooo`
   - 在 OOO + predictor 的模式下运行 benchmark

### 这一步要记录什么

从输出里抄下：

- `cold` 的 cycles
- `warm` 的 cycles
- `result`

### 这一步的解释要点

建议写：

1. 初始时 PHT 全为 `0`，偏向 `strongly not-taken`
2. `probe_branches()` 里的 32 条 branch 实际都是 `taken`
3. 所以 cold pass 前期会出现大量 misprediction
4. warm-up 后相关 counters 饱和到 `3`
5. 再测 warm 时，大多数 branch 都能被正确预测
6. `reset_ghr()` 保证两次访问的是同一批 gshare 槽位

## 14. Step 11：做 Question 5 之前，先确认要改哪些文件

### 要做什么

确认 `bpred2` 相关文件位置。

### 为什么这么做

Q5 明确说：

- 要改的是 `src/cpu/bpred2.c`
- 数据结构在 `include/cpu/bpred2.h`
- 不要改 `bench.c`

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
ls -l src/cpu/bpred2.c include/cpu/bpred2.h predictor_lab/Q4/bench.c
```

### 指令含义

- `ls -l` 同时检查 3 个关键文件是否存在

## 15. Step 12：先跑 baseline predictor

### 要做什么

先在 `predictor_lab/Q4` 跑 baseline。

### 为什么这么做

没有 baseline，就无法证明你自己的 predictor 更好。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q4
ls -l
make run-bpred
```

### 指令含义

1. `cd .../Q4`
   - 进入自定义 predictor 这题的实验目录
2. `ls -l`
   - 确认 `Makefile`、`bench.c` 存在
3. `make run-bpred`
   - 使用默认 tournament predictor 运行，作为 baseline

### 这一步要记录什么

至少记录：

- `IPC`
- `Predictions`
- `Mispredictions`
- `Accuracy`
- `BTB cold misses`

## 16. Step 13：实现你自己的 `bpred2`

### 要做什么

修改 `src/cpu/bpred2.c` 里的：

- `bpred2_predict`
- `bpred2_update`

### 为什么这么做

这题的目标不是泛泛“改个 predictor”，而是针对题面给出的弱点：

- OOO 下多个 in-flight branches 共用 stale history

### 推荐思路

优先考虑题目 skeleton 已经提示的路线：

1. 维护 `spec_lht`
2. 在 `predict` 阶段按预测结果投机更新 `spec_lht`
3. 在 `update` 阶段按真实结果更新 committed `lht`
4. 如果 mispredict，就把对应 `spec_lht` 拉回 committed 状态

### 为什么这条路线合理

因为问题根因是：

- 历史更新发生在 EX 太晚

所以更直接的补救办法就是：

- 让预测阶段就先拥有“未来一拍的历史”

### 额外提醒

- 不要改 `predictor_lab/Q4/bench.c`
- 如果你改了 `include/` 下的任何头文件，必须回仓库根目录执行 `make clean && make`
- 如果只改 `src/cpu/bpred2.c`，通常重新运行该题的 `make run-bpred2` 就会触发重编译

## 17. Step 14：运行你自己的 predictor

### 要做什么

执行 `make run-bpred2`。

### 为什么这么做

Q5 要求你提交自定义 predictor 的实测结果。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/predictor_lab/Q4
make run-bpred2
```

### 指令含义

- `make run-bpred2`
  - 用你实现的 `bpred2_predict / bpred2_update` 运行 benchmark

### 这一步要记录什么

与 baseline 同样记录：

- `IPC`
- `Predictions`
- `Mispredictions`
- `Accuracy`
- `BTB cold misses`

## 18. Step 15：写 Question 5 的分析

### 要做什么

比较 baseline 和 `bpred2`。

### 为什么这么做

老师要看的不是“你改了代码”，而是：

- 你是否抓住了 baseline 的弱点
- 你的设计是否真的改善了该弱点

### 推荐写法

按这个顺序写：

1. baseline 的问题是什么
2. 你设计的核心 insight 是什么
3. 你在 `bpred2.c` 里做了什么
4. `Accuracy`、`Mispredictions`、`IPC` 变成了什么
5. 为什么这些变化说明你的设计有效

### 可以用的关键词

- `stale history`
- `speculative local history`
- `alternating pattern`
- `multiple in-flight instances`
- `EX-stage update too late`

## 19. Step 16：截图和整理材料

### 要做什么

准备最终提交截图。

### 为什么这么做

PDF 明确要求截图，不是只交文字说明。

### 至少需要的截图

1. `Q1`
   - `run-nobpred`
   - `run-bpred`
2. `Q5`
   - `run-bpred`
   - `run-bpred2`

### 截图时注意什么

- 要把终端窗口放大
- 保证 `IPC` 和分支统计完整可见
- 不要截掉题目要求的关键行

## 20. 最后整理 report 前的自查清单

```text
[ ] 已完成一次仓库根目录 build
[ ] 已完成 Q1 两次运行并截图
[ ] 已完成 Q2 的 disasm 和 back-edge PC / BTB index 计算
[ ] 已完成 Q3 去掉 aligned(2048) 前后对比
[ ] 已记录 Q4 的 cold / warm cycles
[ ] 已修改并保存 src/cpu/bpred2.c
[ ] 已运行 Q5 的 baseline 和 bpred2
[ ] 已对比 Accuracy / Mispredictions / IPC
[ ] 已把结果填进 report.md
```

## 21. 如果你只想按最短路径做完

直接按这条链执行：

```bash
cd /home/lzh/CSE5030/SUSTemu
make clean && make

cd predictor_lab/Q1
make run-nobpred
make run-bpred

cd ../Q2
make run-bpred
make disasm

# 去掉 loop_B 上的 aligned(2048)
make clean
make run-bpred
make disasm

cd ../Q3
make run-ooo

cd ../Q4
make run-bpred

# 修改 src/cpu/bpred2.c
make run-bpred2
```

然后把：

- 截图
- `PC / BTB index`
- `cold / warm cycles`
- `bpred2` 对比结果

填进 `report.md` 即可。
