# Lab 5 Todo List

## 1. 总流程先看一眼

```text
克隆并编译 SUSTemu
    ->
运行 Q1 的两个 benchmark（in-order / OOO）
    ->
记录 IPC 表格并分析 critical path
    ->
运行 Q2 的两个 benchmark（in-order / OOO）
    ->
根据汇编和 latency 计算理论最短 cycles/iteration
    ->
对比实测 OOO 结果并写分析
    ->
运行 Q3 的两个 benchmark
    ->
修改 LAT_L2_HIT 为 20、40，分别重新编译并重跑
    ->
记录 CPI 与 speedup 表格
    ->
恢复 LAT_L2_HIT=8 并重新编译
    ->
整理截图和 report
```

## 2. 先明确这次要交什么

题目要求提交这些内容：

1. `Q1`
   - 一张截图
   - 内容要同时包含编译结束的 `over` 和 `whoami` 结果
2. `Q2`
   - 一张完成后的表格
   - 一段书面分析
3. `Q3`
   - 一张完成后的表格
   - 一段书面分析
4. `Q4`
   - 一张完成后的表格
5. `Q5`
   - 一段书面分析

## 3. Step 1：进入环境并检查工具

### 要做什么

确认你在正确目录，并检查基本工具是否存在。

### 为什么这么做

因为后面要用 `git`、`make`、`gcc`、`whoami`。先检查工具，能避免做到一半才发现环境没装好。

### 指令

```bash
cd /home/lzh/CSE5030
command -v git
command -v make
command -v gcc
command -v whoami
ls -l
```

### 指令含义

1. `cd /home/lzh/CSE5030`
   - 切到课程工作目录
2. `command -v git`
   - 检查 `git` 是否可用，并打印其路径
3. `command -v make`
   - 检查 `make` 是否可用
4. `command -v gcc`
   - 检查编译器是否可用
5. `command -v whoami`
   - 检查 `whoami` 是否可用
6. `ls -l`
   - 查看当前目录内容和权限

### 做完如何确认

如果这些命令都能打印出路径，说明基础工具没问题。

## 4. Step 2：克隆 SUSTemu 仓库

### 要做什么

按题目要求克隆仓库。

### 为什么这么做

因为后面的 benchmark、配置文件、Makefile 都在这个仓库里。

### 指令

```bash
git clone git@github.com:Compass-All/SUSTemu.git
cd SUSTemu
ls -l
```

### 指令含义

1. `git clone ...`
   - 从 GitHub 拉取仓库到本地
2. `cd SUSTemu`
   - 进入仓库根目录
3. `ls -l`
   - 确认仓库内容已经下载下来

### 可能遇到的问题

- 如果 SSH key 没配好，`git clone git@...` 可能失败
- 这时通常需要先配置 GitHub SSH key，或者改用 HTTPS 地址

## 5. Step 3：编译模拟器

### 要做什么

清理旧结果并重新编译。

### 为什么这么做

题目明确要求先 build simulator，后面的 `run-inorder` / `run-ooo` 都依赖它。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
make clean && make
```

### 指令含义

1. `make clean`
   - 删除旧的编译产物
2. `&&`
   - 前一条成功后再执行后一条
3. `make`
   - 按 Makefile 重新编译整个项目

### 做完如何确认

看终端最后是否出现题目要求的 `over`。

## 6. Step 4：在同一个终端运行 whoami，并准备 Q1 截图

### 要做什么

编译成功后，不要换终端，直接执行 `whoami`。

### 为什么这么做

题目要求一张截图同时包含：

1. 编译输出结尾的 `over`
2. `whoami` 的结果

### 指令

```bash
whoami
```

### 指令含义

- `whoami`
  - 打印当前登录用户名

### 做完如何确认

终端窗口里同时能看到：

- build 的结尾
- `whoami` 输出

然后截图保存。

## 7. Step 5：做 Q1，运行 ILP 两个 benchmark

### 要做什么

进入 `ooo_lab/Q1/ilp`，分别跑 `in-order` 和 `OOO`。

### 为什么这么做

Q1/Q2 要比较：

- `kernel_serial`
- `kernel_parallel`

在两种处理器模式下的表现差异。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/ooo_lab/Q1/ilp
ls -l
make run-inorder
make run-ooo
```

### 指令含义

1. `cd .../ooo_lab/Q1/ilp`
   - 进入 Q1 的实验目录
2. `ls -l`
   - 看目录里是否有对应源码和 Makefile
3. `make run-inorder`
   - 用 5-stage in-order pipeline 运行
4. `make run-ooo`
   - 用 OOO engine 运行

### 这一步你要记录什么

你需要从输出里记录表格中的结果：

- `serial` 在 `In-Order` 下的结果
- `serial` 在 `OOO` 下的结果
- `parallel` 在 `In-Order` 下的结果
- `parallel` 在 `OOO` 下的结果

如果题目输出的是 `IPC`，就按 `IPC` 填表；如果显示的是总 cycle 或 CPI，就按目录说明换算或直接摘录老师要求的指标。

## 8. Step 6：理解 Q2 到底要回答什么

### 要做什么

在写答案前，先盯着题目给的两段汇编，找每个 kernel 的依赖链。

### 为什么这么做

Q2 不是问“哪个更快”，而是问：

1. 为什么 `kernel_parallel` 的 `OOO IPC` 更高
2. 两个 kernel 的 `critical path` 分别是什么
3. OOO 调度器分别被什么限制住

### 推荐做法

先把两段循环各自写成依赖链：

```text
kernel_serial:
sum -> and -> sll -> add(addr) -> ld -> add(sum) -> next iter

kernel_parallel:
load aa -> add sa
load bb -> add sb
两条链可并行推进
```

### 这一步写答案时要抓住的关键词

- `dependency chain`
- `critical path`
- `look-ahead`
- `independent accumulators`
- `scheduler`
- `issue width`

## 9. Step 7：做 Q2 的表格和书面分析

### 要做什么

把 Step 5 的结果填入 Q2 表格，并写一段解释。

### 为什么这么做

因为 Q2 明确要求：

- completed table
- written answer

### 推荐写法

先写 `kernel_serial`：

- 地址依赖上一轮 `sum`
- load 地址无法提前知道
- 可并行空间很小
- OOO 很难把 IPC 拉高

再写 `kernel_parallel`：

- 两条独立累加链
- 两个 load 互不依赖
- OOO 能同时利用更多 issue slot 和 functional units

## 10. Step 8：做 Q3，运行 Tomasulo 两个 benchmark

### 要做什么

进入 `ooo_lab/Q2/tomasulo`，跑两个 benchmark。

### 为什么这么做

Q3 要比较：

- `bench_diamond`
- `bench_chain`

在依赖结构不同的情况下，OOO 的调度效果差异。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/ooo_lab/Q2/tomasulo
ls -l
make run-inorder && make run-ooo
```

### 指令含义

1. `cd .../ooo_lab/Q2/tomasulo`
   - 进入 Tomasulo 实验目录
2. `ls -l`
   - 查看目录内容
3. `make run-inorder`
   - 先跑顺序处理器
4. `&&`
   - 前一条成功后再执行后一条
5. `make run-ooo`
   - 再跑 OOO

### 这一步要记录什么

记录表格里 `diamond` 和 `chain` 的测量结果，尤其是 OOO 下每轮迭代的 cycle 表现。

## 11. Step 9：手算 Q3 的理论最短 cycles/iteration

### 要做什么

根据题目给的汇编和默认 latency 配置，分别算出：

- `bench_diamond` 一轮最少要多少 cycle
- `bench_chain` 一轮最少要多少 cycle

### 为什么这么做

Q3 的关键不是只抄测量值，而是对比：

- 理论最短值
- 实测 OOO 值

再解释 OOO scheduler 怎么处理这两种结构。

### 默认 latency

题目给的默认值是：

```text
LAT_INT_ADD = 1
LAT_INT_MUL = 3
LAT_L1_HIT = 1
```

### 推荐分析方法

1. 先标出每条指令依赖谁
2. 画依赖图
3. 沿最长链累计 latency
4. 并行分支取较慢的一支

### 你要特别注意

`bench_diamond` 里：

- `mul` 和 `add +1` 都依赖最开始的 `ld`
- 但它们彼此不依赖

`bench_chain` 里：

- `add +1` 之后才能 `mul`
- `mul` 之后才能最后的 `add`

所以 `chain` 的关键路径通常更长。

## 12. Step 10：写 Q3 的表格和书面分析

### 要做什么

把测量值和理论值一起整理。

### 为什么这么做

Q3 明确要求：

- completed table
- written answer

### 推荐回答结构

1. 先写 `diamond` 的依赖关系
2. 再写它的理论最短 cycles/iteration
3. 再写它和实测 OOO 的差距
4. 最后解释差距说明了什么

然后同样写 `chain`。

### 最后要落到的结论

重点不是“OOO 很强”，而是：

- `diamond` 这种结构给了 OOO 可挖掘的并行空间
- `chain` 这种结构把调度器绑死在长依赖链上

## 13. Step 11：做 Q4，运行 Memory-Level Parallelism benchmark

### 要做什么

进入 `ooo_lab/Q3/memlat`，先在默认 `LAT_L2_HIT = 8` 下跑一次。

### 为什么这么做

Q4 的表格第一部分就是默认值 `8`。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/ooo_lab/Q3/memlat
ls -l
make run-inorder && make run-ooo
```

### 指令含义

和前面类似：

1. 进入对应实验目录
2. 确认文件存在
3. 跑 in-order
4. 跑 OOO

### 这一步要记录什么

记录：

- `parallel` 的 `In-Order CPI`
- `parallel` 的 `OOO CPI`
- `serial` 的 `In-Order CPI`
- `serial` 的 `OOO CPI`
- 对应 speedup

如果输出里没有直接给 speedup，你就自己算：

```text
speedup = In-Order CPI / OOO CPI
```

## 14. Step 12：修改 LAT_L2_HIT 为 20

### 要做什么

打开 `include/cpu/exec_cfg.h`，把 `LAT_L2_HIT` 改为 `20`。

### 为什么这么做

题目要你观察更大的 L2 latency 对两种模式的影响。

### 推荐操作

先搜索目标行：

```bash
cd /home/lzh/CSE5030/SUSTemu
rg -n 'LAT_L2_HIT' include/cpu/exec_cfg.h
```

### 指令含义

1. `rg -n`
   - 搜索文本并显示行号
2. `'LAT_L2_HIT'`
   - 查找这个配置项

### 修改后必须执行

```bash
make clean && make
```

### 为什么必须重新编译

题目特别提醒：

- `include/` 下改了任何文件
- 都必须 `make clean && make`

因为这是编译期配置，不重编译不会生效。

## 15. Step 13：在 LAT_L2_HIT=20 下重跑 Q4

### 要做什么

再次跑 `memlat` 目录下的两个 benchmark。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/ooo_lab/Q3/memlat
make run-inorder && make run-ooo
```

### 要记录什么

记录 `20` 这一组表格的：

- `parallel`
- `serial`

两行数据。

## 16. Step 14：修改 LAT_L2_HIT 为 40

### 要做什么

把 `include/cpu/exec_cfg.h` 里的 `LAT_L2_HIT` 再改成 `40`。

### 为什么这么做

这是 Q4 的第三组实验条件。

### 修改后必须执行

```bash
cd /home/lzh/CSE5030/SUSTemu
make clean && make
```

### 含义

- 重新按新 latency 配置编译模拟器

## 17. Step 15：在 LAT_L2_HIT=40 下重跑 Q4

### 要做什么

再次重跑 `memlat` benchmark。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu/ooo_lab/Q3/memlat
make run-inorder && make run-ooo
```

### 要记录什么

记录 `40` 这一组表格的：

- `parallel`
- `serial`

两行数据。

## 18. Step 16：恢复 LAT_L2_HIT=8

### 要做什么

把 `LAT_L2_HIT` 改回 `8`，然后重新编译。

### 为什么这么做

题目明确要求最后恢复默认值，避免把仓库留在改坏的状态。

### 指令

```bash
cd /home/lzh/CSE5030/SUSTemu
rg -n 'LAT_L2_HIT' include/cpu/exec_cfg.h
make clean && make
```

### 最后如何确认

再次搜索 `LAT_L2_HIT`，确认它已经回到 `8`。

## 19. Step 17：写 Q5，分析 speedup 是否收敛

### 要做什么

根据 Q4 的表格，判断：

1. OOO speedup 是否持续增长
2. 还是逐渐收敛
3. 收敛上限由什么硬件资源决定
4. 最终趋近于多少

### 为什么这么做

Q5 要的不是描述现象，而是解释“为什么有上限”。

### 推荐思路

先观察：

- `LAT_L2_HIT` 从 `8 -> 20 -> 40`
- in-order CPI 增长趋势
- OOO CPI 增长趋势
- speedup 变化趋势

然后再结合硬件资源解释：

- 题目默认配置里 `LSU units = 2`
- 如果每轮只有两次独立 load 可并行，那么 OOO 最多也只能把两次长延迟尽量重叠
- 所以 speedup 会向某个常数收敛，而不是无限增长

## 20. 最后整理 report 前的自查清单

```text
[ ] 已截图：编译输出结尾 over + whoami
[ ] 已完成 Q2 表格
[ ] 已写完 Q2 的 critical path 分析
[ ] 已完成 Q3 表格
[ ] 已写完 Q3 的理论最短 cycle 与实测对比分析
[ ] 已完成 Q4 表格（LAT_L2_HIT = 8 / 20 / 40）
[ ] 已写完 Q5 的收敛分析
[ ] 已把 LAT_L2_HIT 恢复为 8
[ ] 已重新 make clean && make
```

## 21. 如果你只想按最短路径做完

直接照这个顺序执行：

```bash
cd /home/lzh/CSE5030
git clone git@github.com:Compass-All/SUSTemu.git
cd SUSTemu
make clean && make
whoami

cd ooo_lab/Q1/ilp
make run-inorder
make run-ooo

cd ../..
cd Q2/tomasulo
make run-inorder && make run-ooo

cd ../..
cd Q3/memlat
make run-inorder && make run-ooo

cd /home/lzh/CSE5030/SUSTemu
# 把 LAT_L2_HIT 改成 20
make clean && make
cd ooo_lab/Q3/memlat
make run-inorder && make run-ooo

cd /home/lzh/CSE5030/SUSTemu
# 把 LAT_L2_HIT 改成 40
make clean && make
cd ooo_lab/Q3/memlat
make run-inorder && make run-ooo

cd /home/lzh/CSE5030/SUSTemu
# 把 LAT_L2_HIT 改回 8
make clean && make
```

你只要按这条链跑通，再把结果填进 `report.md`，这次 lab 就能完整交上去。
