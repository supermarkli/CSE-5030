# Lab 7 Todo List

## 1. 总流程先看一眼

```text
确认工具链和路径
    ->
确认 gem5 编译协议是 MOESI_CMP_directory
    ->
编译 false_sharing_bad_mt
    ->
运行多核 RISC-V + Ruby + MOESI 仿真
    ->
查看 m5out/stats.txt 中的 cache 统计
    ->
开启 ProtocolTrace 截图状态转换
    ->
修改代码消除 false sharing
    ->
编译 false_sharing_good_mt 并重跑
    ->
比较 bad / good 版本的 L1D cache 统计
    ->
整理截图和 report
```

## 2. 先明确这次要交什么

根据 PDF，这次 lab 最终要交：

1. `gem5 Installation Verification`
   - `--build-info | grep PROTOCOL` 的截图
   - 同一终端里的 `whoami` 截图
2. `Protocol Trace Analysis`
   - 开启 `--debug-flags=ProtocolTrace` 的状态转换截图
   - 同一终端里的 `whoami` 截图
3. `False Sharing Analysis`
   - 3 个问答题
   - 你优化后的代码片段
   - bad / good 版本的数据对比
   - 同一终端里的 `whoami` 截图

## 3. Step 1：先确认目录和文件

### 要做什么

确认 `lab7`、`gem5` 工作目录、源码文件、配置脚本文件的位置。

### 为什么这么做

这次 lab 牵涉：

- `chipyard` 环境
- `gem5_package` 预编译目录
- `lab7` 工作目录
- `false_sharing_mt.c`
- `riscv_moesi_riscv.py`

如果路径一开始就错，后面命令会连续报错。

### 指令

```bash
cd /home/lzh/CSE5030/lab7
ls -l
ls -l /home/lzh/CSE5030/gem5_package
ls -l /home/lzh/CSE5030/gem5_package/build/RISCV_MOESI
```

### 指令含义

1. `cd /home/lzh/CSE5030/lab7`
   - 进入这次实验自己的工作目录
2. `ls -l`
   - 查看当前目录里有哪些文件
3. `ls -l /home/lzh/CSE5030/gem5_package`
   - 确认预编译 `gem5` 包目录存在
4. `ls -l /home/lzh/CSE5030/gem5_package/build/RISCV_MOESI`
   - 确认 `MOESI` 版本二进制目录存在

## 4. Step 2：预检工具

### 要做什么

确认本次需要的工具链都能用。

### 为什么这么做

PDF 里会用到：

- `riscv64-unknown-linux-gnu-gcc`
- `scons`
- `grep`
- `tar`

先检查，能节省后面排错时间。

### 指令

```bash
command -v riscv64-unknown-linux-gnu-gcc
command -v scons
command -v grep
command -v tar
```

### 指令含义

1. `command -v riscv64-unknown-linux-gnu-gcc`
   - 检查 RISC-V Linux 交叉编译器是否安装
2. `command -v scons`
   - 检查 `gem5` 编译工具是否安装
3. `command -v grep`
   - 检查搜索工具是否存在
4. `command -v tar`
   - 检查压缩包解压工具是否存在

## 5. Step 3：加载环境并确认交叉编译器

### 要做什么

进入 `chipyard` 并加载环境，然后查看编译器版本。

### 为什么这么做

PDF 先让你确认工具链，因为后面编译 benchmark 要依赖这个交叉编译器。

### 指令

```bash
cd /home/lzh/CSE5030/chipyard
source env.sh
riscv64-unknown-linux-gnu-gcc --version
```

### 指令含义

1. `cd /home/lzh/CSE5030/chipyard`
   - 进入 `chipyard` 目录
2. `source env.sh`
   - 把环境变量加载进当前 shell
3. `riscv64-unknown-linux-gnu-gcc --version`
   - 输出编译器版本，确认命令可用

## 6. Step 4：编译 bad version 程序

### 要做什么

把 `false_sharing_mt.c` 编译成 bad 版本程序，例如 `false_sharing_bad_mt`。

### 为什么这么做

后面仿真必须有一个可执行 benchmark，而这一步生成的就是 bad version 基准程序。

### 指令

```bash
cd /home/lzh/CSE5030/lab7
riscv64-unknown-linux-gnu-gcc -pthread -static -Wall -O2 -o false_sharing_bad_mt false_sharing_mt.c
ls -l false_sharing_bad_mt
```

### 指令含义

1. `-pthread`
   - 打开 `pthread` 多线程支持
2. `-static`
   - 静态链接，方便放进 `gem5` 里跑
3. `-Wall`
   - 打开常见 warning
4. `-O2`
   - 做常规优化
5. `-o false_sharing_bad_mt`
   - 指定输出文件名
6. `ls -l false_sharing_bad_mt`
   - 确认可执行文件真的生成了

## 7. Step 5：检查 gem5 当前编译协议

### 要做什么

看当前 `gem5` 二进制是不是已经按 `MOESI_CMP_directory` 编译。

### 为什么这么做

这一步是整个实验的前置条件。如果看到的是 `MI_example`，说明协议编错了。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package
LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH ./build/RISCV_MOESI/gem5.opt --build-info | grep PROTOCOL
```

### 指令含义

1. `LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH ./build/RISCV_MOESI/gem5.opt --build-info`
   - 在加载预编译包依赖库后，输出当前 `gem5` 的编译信息
2. `| grep PROTOCOL`
   - 只筛出和协议有关的行

### 做完后怎么判断

你想看到的是：

```text
PROTOCOL = MOESI_CMP_directory
```

如果不是，说明你拿到的预编译包不对，或者你实际上跑的是别的 `gem5` 二进制。

## 8. Step 6：再次确认协议编译成功

### 要做什么

再次检查 `PROTOCOL`。

### 为什么这么做

没有这一步，你无法证明“现在跑的二进制”确实是对的。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package
LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH ./build/RISCV_MOESI/gem5.opt --build-info | grep PROTOCOL
whoami
```

### 指令含义

1. `LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH ./build/RISCV_MOESI/gem5.opt --build-info | grep PROTOCOL`
   - 只看协议相关编译信息
2. `whoami`
   - 输出当前用户名，用于截图提交

### 这一步要截图什么

同一个终端里一起截到：

- `PROTOCOL = MOESI_CMP_directory`
- `RUBY_PROTOCOL_MOESI_CMP_directory = True`
- `whoami`

## 9. Step 7：确认预编译 `gem5_package`

### 要做什么

确认老师给的预编译包已经解压好，并能作为当前机器的主工作流使用。

### 为什么这么做

这台机器当前就是通过这套预编译包来完成 lab，不再依赖 `/home/gem5` 的源码编译路径。

### 指令

```bash
cd /home/lzh/CSE5030
tar -xzf /home/lzh/CSE5030/gem5_package.tar.gz
cd /home/lzh/CSE5030/gem5_package
export LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH
```

### 指令含义

1. `cd /home/lzh/CSE5030`
   - 进入课程目录根路径
2. `tar -xzf /home/lzh/CSE5030/gem5_package.tar.gz`
   - 解压老师提供的 `gem5` 预编译包
3. `cd /home/lzh/CSE5030/gem5_package`
   - 进入解压后的预编译包目录
4. `export LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH`
   - 把包里的动态库目录加到运行时搜索路径

## 10. Step 8：创建 `lab7` 的 Python 配置脚本

### 要做什么

确认 `riscv_moesi_riscv.py` 在当前机器上的真实可运行位置。

### 为什么这么做

这就是后面运行多核 `RISC-V` + `Ruby` + `MOESI` 仿真的入口脚本，而且它依赖相对路径找到 `../configs`。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
ls -l
```

### 这一步实际要做的文件内容

当前机器上真正直接可运行的脚本是：

```text
/home/lzh/CSE5030/gem5_package/lab7/riscv_moesi_riscv.py
```

### 为什么脚本里要写 `configs_dir = os.path.abspath('../configs')`

因为脚本需要从 `gem5_package/lab7` 回到 `gem5_package/configs`，才能正确导入 `ruby.Ruby`。

## 11. Step 9：运行 bad version 仿真

### 要做什么

用 `MOESI` 配置运行 bad 版本 benchmark。

### 为什么这么做

这是后面所有分析的 baseline。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt ./riscv_moesi_riscv.py \
    --benchmark /home/lzh/CSE5030/lab7/false_sharing_bad_mt \
    --benchmark-args 100000 \
    --mem-size=8GiB \
    --num-cpus 4
```

### 指令含义

1. `LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt`
   - 使用刚编好的 `MOESI` 版本 `gem5`
2. `./riscv_moesi_riscv.py`
   - 使用预编译包里那份能正确找到 `../configs` 的系统配置脚本
3. `--benchmark /home/lzh/CSE5030/lab7/false_sharing_bad_mt`
   - 指定要运行的 bad 版本程序
4. `--benchmark-args 100000`
   - 给 benchmark 传入迭代次数参数
5. `--mem-size=8GiB`
   - 指定模拟内存大小
6. `--num-cpus 4`
   - 模拟 4 个 CPU core

### 如果这里报 `Could not mmap`

说明内存开太大了，可以改小 `--mem-size`，例如尝试 `4GiB` 或更低。

## 12. Step 10：查看 `m5out` 输出目录

### 要做什么

确认仿真结束后产物已经生成。

### 为什么这么做

你后面分析的数据都在 `m5out/` 里。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
ls -lh m5out/
```

### 指令含义

- `ls -lh m5out/`
  - 列出输出目录里的关键文件和大小

### 这一步重点看什么

- `stats.txt`
- `config.ini`
- `config.json`

## 13. Step 11：提取 L1 cache 统计

### 要做什么

从 `stats.txt` 提取 `L1I` 和 `L1D` 的 demand 访问统计。

### 为什么这么做

这是后面写 `false sharing` 数据分析的直接依据。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
grep 'L1Icache.m_demand' m5out/stats.txt
grep 'L1Dcache.m_demand' m5out/stats.txt
```

### 指令含义

1. `grep 'L1Icache.m_demand' m5out/stats.txt`
   - 找出 L1 instruction cache 的 demand 统计
2. `grep 'L1Dcache.m_demand' m5out/stats.txt`
   - 找出 L1 data cache 的 demand 统计

### 这一步要记录什么

记下 bad version 的：

- `m_demand_hits`
- `m_demand_misses`
- `m_demand_accesses`

## 14. Step 12：开启 `ProtocolTrace`

### 要做什么

重新运行仿真，但打开协议跟踪。

### 为什么这么做

题目要求你提交状态转换截图。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt --debug-flags=ProtocolTrace \
    ./riscv_moesi_riscv.py \
    --benchmark /home/lzh/CSE5030/lab7/false_sharing_bad_mt \
    --benchmark-args 10 \
    --mem-size=8GiB \
    --num-cpus 4
```

### 指令含义

1. `--debug-flags=ProtocolTrace`
   - 打开协议 trace 输出
2. `--benchmark-args 10`
   - 用较小迭代次数快速产生 trace

### 这一步怎么截图

看到有状态转换就可以截图，例如包含：

- `I>IS_M`
- `IS>S`
- `M>I`

之后可以 `Ctrl+C` 停掉。

### 别忘了

截图后在同一终端再执行：

```bash
whoami
```

## 15. Step 13：回答 `ProtocolTrace` 的含义

### 要做什么

理解 trace 里的每列表示什么。

### 为什么这么做

报告里不能只贴图不解释。

### 你至少要会说

1. `Timestamp` 是仿真时刻
2. `CPU ID` 表示哪个核
3. `Component` 是处理事件的模块
4. `Event` 是协议事件
5. `State Transition` 表示状态转换
6. `Address` 表示涉及的地址或 cache line

## 16. Step 14：解释 `false sharing` 为什么会发生

### 要做什么

先写出 bad 版本为什么会出现 `false sharing`。

### 为什么这么做

这是实验问答题 `a)` 的核心。

### 应该怎么理解

bad 版本里两个线程分别写不同的计数器，但两个计数器很可能在同一条 `cache line`，于是 coherence protocol 会把它们当成同一块数据来协调，导致频繁 invalidation 或 cache line 转移。

## 17. Step 15：修改代码消除 `false sharing`

### 要做什么

把 bad 版本改成 good 版本，让不同线程写的数据分离到不同 `cache line`。

### 为什么这么做

这是实验问答题 `b)` 和提交代码片段的核心。

### 可以怎么改

典型思路有两种：

1. 在两个计数器之间加 `padding`
2. 把每个线程自己的计数器放到单独对齐的结构里

### 一个常见思路示例

```c
struct PaddedCounter {
    volatile atomic_int counter;
    char padding[64];
};
```

### 修改目标

不是为了代码好看，而是为了：

```text
让 thread 0 和 thread 1 高频写的数据不要落在同一 cache line
```

## 18. Step 16：编译 good version

### 要做什么

把优化后的代码重新编译成 `false_sharing_good_mt`。

### 为什么这么做

你需要一个和 bad version 对照的新版可执行文件。

### 指令

```bash
cd /home/lzh/CSE5030/lab7
riscv64-unknown-linux-gnu-gcc -pthread -static -O2 -Wall -o false_sharing_good_mt false_sharing_mt.c
ls -l false_sharing_good_mt
```

### 指令含义

- 与编译 bad version 基本相同，只是输出文件名换成了 `false_sharing_good_mt`

## 19. Step 17：运行 good version 仿真

### 要做什么

在同样的 `gem5` 配置下运行优化后的程序。

### 为什么这么做

只有保持其他条件不变，bad/good 对比才有意义。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt ./riscv_moesi_riscv.py \
    --benchmark /home/lzh/CSE5030/lab7/false_sharing_good_mt \
    --benchmark-args 100000 \
    --mem-size=8GiB \
    --num-cpus 4
```

### 指令含义

- 和 bad version 的仿真命令相同，只把 `--benchmark` 换成 good 版本程序

## 20. Step 18：提取 good version 的 L1D 统计

### 要做什么

再次读取 `stats.txt` 里的 `L1Dcache.m_demand` 统计。

### 为什么这么做

这是回答实验问答题 `c)` 和 bad/good 数据对比的依据。

### 指令

```bash
cd /home/lzh/CSE5030/gem5_package/lab7
grep 'L1Dcache.m_demand' m5out/stats.txt
whoami
```

### 指令含义

1. `grep 'L1Dcache.m_demand' m5out/stats.txt`
   - 抽取 good version 的数据 cache 统计
2. `whoami`
   - 同终端显示用户名，便于截图提交

## 21. Step 19：整理 bad / good 对比表

### 要做什么

把 bad 和 good 两组数据放进一张表。

### 为什么这么做

老师要的是“比较”和“分析”，不是两份孤立数字。

### 建议表头

```text
Version | L1D hits | L1D misses | L1D accesses | miss rate | 说明
```

### 这一节要怎么解释

可以按这个顺序写：

1. bad version 为什么更容易引发 `false sharing`
2. good version 做了什么布局调整
3. 哪些 `L1D` 统计变好了
4. 为什么这些变化说明 coherence 开销下降了

## 22. Step 20：完成报告中的 3 个问答题

### 题目 `a)` 为什么会出现 `false sharing`

回答重点：

- 不同线程访问不同变量
- 但变量在同一 `cache line`
- coherence protocol 按 line 管，不按变量管

### 题目 `b)` 怎么改善或消除 `false sharing`

回答重点：

- padding
- alignment
- 分离每个线程的写热点数据

### 题目 `c)` 怎么从 `stats.txt` 观察 `false sharing`

回答重点：

- 比较 bad/good 的 `L1Dcache.m_demand_*`
- 看 miss 和 access 的变化
- 结合程序内存布局解释

## 23. Step 21：最终截图清单

你至少需要准备这些截图：

1. `build-info | grep PROTOCOL` + `whoami`
2. `ProtocolTrace` 状态转换 + `whoami`
3. bad version 的 `L1D` 统计或运行结果 + `whoami`
4. good version 的 `L1D` 统计或运行结果 + `whoami`

## 24. 最后的自查清单

```text
[ ] 已确认工具链可用
[ ] 已确认 gem5 编译协议是 MOESI_CMP_directory
[ ] 已保存 riscv_moesi_riscv.py
[ ] 已编译 false_sharing_bad_mt
[ ] 已运行 bad version 仿真
[ ] 已提取 bad version 的 L1D 统计
[ ] 已截图 ProtocolTrace 和 whoami
[ ] 已修改代码消除 false sharing
[ ] 已编译 false_sharing_good_mt
[ ] 已运行 good version 仿真
[ ] 已提取 good version 的 L1D 统计
[ ] 已完成 3 个问答题
[ ] 已整理 report.md
```

## 25. 如果你只想按最短路径做完

按这条链走：

```bash
cd /home/lzh/CSE5030/chipyard
source env.sh
riscv64-unknown-linux-gnu-gcc --version

cd /home/lzh/CSE5030/gem5_package
LD_LIBRARY_PATH=$(pwd)/libs:$LD_LIBRARY_PATH ./build/RISCV_MOESI/gem5.opt --build-info | grep PROTOCOL

cd /home/lzh/CSE5030/gem5_package/lab7
LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt ./riscv_moesi_riscv.py --benchmark /home/lzh/CSE5030/lab7/false_sharing_bad_mt --benchmark-args 100000 --mem-size=8GiB --num-cpus 4
grep 'L1Dcache.m_demand' m5out/stats.txt

LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt --debug-flags=ProtocolTrace ./riscv_moesi_riscv.py --benchmark /home/lzh/CSE5030/lab7/false_sharing_bad_mt --benchmark-args 10 --mem-size=8GiB --num-cpus 4

# 修改代码，编译 good version

LD_LIBRARY_PATH=/home/lzh/CSE5030/gem5_package/libs:$LD_LIBRARY_PATH ../build/RISCV_MOESI/gem5.opt ./riscv_moesi_riscv.py --benchmark /home/lzh/CSE5030/lab7/false_sharing_good_mt --benchmark-args 100000 --mem-size=8GiB --num-cpus 4
grep 'L1Dcache.m_demand' m5out/stats.txt
whoami
```

然后把数据和截图填进 `report.md` 就行。
