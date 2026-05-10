# Lab 9 Todo List

## 1. 先明确这次的执行原则

这次 `lab9.pdf` 原始要求包含 `UCAgent + OpenCode + MCP` 的自动化验证流程，但你老师额外要求“不能借助 AI 工具”，并且截图不用提交。所以这份 `todo` 采用下面这个原则：

1. 你手工输入所有命令
2. 你手写测试用例
3. 你用 Verilator / Picker / Python 运行仿真
4. 你手工对比 expected / actual 并分析 bug
5. 你手工写最终答案
6. 不使用 `UCAgent`、`OpenCode` 或任何大模型

这意味着：

```text
可以使用 Verilator / Picker / Python / shell / make 这类非 AI 工具
不能使用 UCAgent / OpenCode 这类 AI agent
```

## 2. 总流程先看一眼

```text
确认题面与目录
    ->
先阅读 note.md 建立整体认识
    ->
按 note.md 的 URL 检索相关资料了解实验背景
    ->
预检工具和路径
    ->
安装 Verilator
    ->
安装 Picker
    ->
创建 Adder/Adder.v
    ->
创建 Adder/README.md
    ->
运行 picker export
    ->
复制 README 到 output/Adder
    ->
手工对照 README 和 Adder.v，设计测试点
    ->
手写测试脚本
    ->
运行仿真
    ->
记录 expected / actual
    ->
人工定位 bug
    ->
整理手工执行记录和 bug 分析到 report
```

## 3. 隐含假设先列出来

1. 你在 Linux 环境中做实验。来源：从 PDF 命令验证得到。
2. 你有 `sudo` 权限。来源：从 PDF 里的安装命令验证得到。
3. 你能正常联网拉取 GitHub 仓库，并通过 `apt` / `pip` 安装必要工具。来源：从 PDF 里的安装流程验证得到。
4. 原题里的 `UCAgent` 属于 AI 工具，本次正式实验不能使用。来源：用户明确说明。
5. 原题截图不用提交，本次报告不需要 `UCAgent TUI + whoami` 截图。来源：用户明确说明。
6. Verilator、Picker、Python、shell 属于非 AI 工具，可以用于编译、导出和仿真。来源：用户明确纠正“手工不是纯手工，是不用 AI 工具”。

## 4. Step 0：确认实验目录与 PDF

### 要做什么

确认你当前工作目录和 `lab9.pdf` 路径都正确。

### 为什么这么做

路径一旦错，后面创建文件和运行命令都会偏。

### 指令

```bash
pwd
ls -l /home/lzh/CSE5030/lab9/lab9.pdf
ls -l
```

### 指令含义

1. `pwd`
   - 显示当前目录
2. `ls -l /home/lzh/CSE5030/lab9/lab9.pdf`
   - 检查 PDF 是否真的存在
3. `ls -l`
   - 查看当前目录已有文件

### 人工估时

- `3` 分钟

## 5. Step 1：先阅读 `note.md`

### 要做什么

完整阅读当前目录下的 `note.md`。

### 为什么这么做

这次实验不是单纯照着敲命令。你如果先不知道：

1. 原题中的 `UCAgent` 为什么不能用于本次正式流程
2. `Picker` 为什么要导出 Python 工程
3. `Adder` 的 bug 为什么是位宽 bug
4. 最后到底要怎么手工解释 `Adder` 的位宽 bug

那后面就很容易出现“命令会跑，但不知道自己在做什么”的情况。先读 `note.md`，就是先建立整体实验图景。

### 你至少要读懂什么

1. 整体 ASCII 流程图
2. `RTL / DUT / Verilator / Picker`
3. `Adder` 的故意注入 bug 是什么
4. `UCAgent / OpenCode / MCP` 为什么只作为原题背景

### 人工估时

- `15` 到 `25` 分钟

## 6. Step 2：按 `note.md` 的 URL 检索相关资料

### 要做什么

根据 `note.md` 最后给出的参考链接，手工打开并浏览相关资料，建立实验背景知识。

### 为什么这么做

老师要求纯手工完成，所以你不能只背结论。你至少要知道哪些工具可以用、哪些工具不能用，这样你后面写报告时才不会把：

- `Picker`
- `Verilator`
- `UCAgent`
- `OpenCode`

混成一团。

### 建议优先看的资料

1. `Picker GitHub`
2. `Verilator install`
3. `Verilator overview`
4. `UCAgent README`
5. `OpenCode config`

### 建议怎么读

不是要逐字通读全部文档，而是抓住与你这次 lab 最相关的点：

1. 这个工具是干什么的
2. 这次 lab 用它做哪一步
3. 它和其他工具的关系是什么

### 建议记录

建议你自己手写或在草稿里记 5 条最关键结论，例如：

```text

```

### 人工估时

- `20` 到 `35` 分钟

## 7. Step 3：预检基础工具

### 要做什么

确认题面会用到的核心命令存在。

### 为什么这么做

题面命令很多，如果工具压根没装，后面会连续报错。

### 指令

```bash
command -v git
command -v pip3
command -v python3
command -v make
command -v g++
```

### 指令含义

1. `command -v xxx`
   - 检查某个命令是否在 `PATH` 里

### 通过标准

这些命令都应返回实际路径，而不是空。

### 人工估时

- `5` 分钟

## 8. Step 4：安装 Verilator

### 要做什么

安装题面要求的 `Verilator >= 5.020`。

### 为什么这么做

`Picker` 依赖它，没有它就没法把 `RTL` 导出成可验证工程。

### 指令

```bash
sudo apt-get install git help2man perl python3 make g++ libgz libfl2 libfl-dev zlibc zlib1g zlib1g-dev
git clone https://github.com/verilator/verilator
cd verilator
git checkout v5.020
autoconf
./configure
make
sudo make install
verilator --version
```

### 指令含义

1. `sudo apt-get install ...`
   - 安装编译 `Verilator` 所需依赖
2. `git clone ...`
   - 下载 `Verilator` 源码
3. `git checkout v5.020`
   - 切到题面指定版本
4. `autoconf`
   - 生成配置脚本
5. `./configure`
   - 检查环境并生成 `Makefile`
6. `make`
   - 编译源码
7. `sudo make install`
   - 安装到系统路径
8. `verilator --version`
   - 验证安装成功并确认版本

### 你要记录什么

记录最终版本号，确认不低于 `5.020`。

### 人工估时

- `20` 到 `35` 分钟

## 9. Step 5：跳过 UCAgent

### 要做什么

不要安装或启动 `UCAgent`。

### 为什么这么做

`UCAgent` 是题面原始流程中的 AI-powered automated hardware verification agent。老师要求纯手工且不能借助 AI 工具，所以正式实验中不要使用它。

### 你要记录什么

在报告里明确写：`UCAgent used: No`。

### 人工估时

- `1` 分钟

## 10. Step 6：安装 Picker

### 要做什么

安装 `Picker` 和其依赖。

### 为什么这么做

后面 `picker export` 是本实验从 `RTL` 进入 Python 验证工程的关键一步。

### 指令

```bash
sudo apt install cmake build-essential
pip3 install swig
git clone https://github.com/XS-MLVP/picker.git
cd picker
make init
make
sudo -E make install
picker --help
```

### 指令含义

1. `sudo apt install cmake build-essential`
   - 安装构建工具链
2. `pip3 install swig`
   - 安装 `swig`
3. `git clone ...`
   - 下载 `Picker`
4. `make init`
   - 初始化构建环境
5. `make`
   - 编译 `Picker`
6. `sudo -E make install`
   - 安装到系统环境，`-E` 表示保留环境变量
7. `picker --help`
   - 检查命令是否可用

### 人工估时

- `15` 到 `30` 分钟

## 11. Step 7：跳过 OpenCode / MCP

### 要做什么

不要安装、配置或启动 `OpenCode`，也不要配置 `MCP`。

### 为什么这么做

`OpenCode` 是原题里的 AI 客户端，`MCP` 是它连接 `UCAgent` 的协议。老师要求纯手工时，这条链路整体不能用于正式实验。

### 人工动作

1. 不安装 `OpenCode`
2. 不创建 `opencode.json`
3. 不输入 prompt 给任何 AI agent

### 可以在报告中怎么写

```text
原题设计中，OpenCode 可通过 MCP 连接 UCAgent 并驱动自动验证。
但本次根据老师要求，未使用 OpenCode、MCP 或 UCAgent。
```

### 人工估时

- `3` 到 `5` 分钟

## 12. Step 8：确认 AI 工具未使用

### 要做什么

确认本次正式流程没有使用 `UCAgent`、`OpenCode`、`MCP` 或其他大模型工具。

### 为什么这么做

这一步是为了让最终报告的执行口径和老师要求一致。

### 你要记录什么

记录自己只使用非 AI 工具和手工分析。

### 人工估时

- `1` 分钟

## 13. Step 9：创建工作目录 `Adder`

### 要做什么

创建这次 `DUT` 的目录。

### 为什么这么做

后面的 `Adder.v` 和 `README.md` 都要放在这里。

### 指令

```bash
mkdir -p Adder
ls -l
```

### 指令含义

1. `mkdir -p Adder`
   - 创建目录，已存在也不会报错
2. `ls -l`
   - 确认目录已创建成功

### 人工估时

- `2` 分钟

## 14. Step 10：手工创建 `Adder/Adder.v`

### 要做什么

把题面给出的 `Verilog` 模块手工写入文件。

### 为什么这么做

这是被测设计 `DUT`，没有它后续验证无从谈起。

### 题面代码

```verilog
// A verilog 64-bit full adder with carry in and carry out
module Adder #(
parameter WIDTH = 64
) (
input [WIDTH-1:0] a,
input [WIDTH-1:0] b,
input cin,
output [WIDTH-2:0] sum,
output cout

// Intentional bug injected here

);
assign {cout, sum} = a + b + cin;
endmodule
```

### 为什么这里故意有 bug

因为老师就是要你通过后面的验证流程把它找出来。

### 人工估时

- `5` 到 `8` 分钟

## 15. Step 11：手工创建 `Adder/README.md`

### 要做什么

把题面要求的规格说明手工写入 `README.md`。

### 为什么这么做

验证流程需要这份规格；而且题面明确要求所有文档和注释用中文。

### 题面内容

```markdown
### Adder 64 位加法器
输入 a, b, cin 输出 sum，cout
实现 sum = a + b + cin
cin 是进位输入
cout 是进位输出
### 验证目标
只要验证加法相关的功能，其他验证（例如波形、接口等）不需要出现。
### bug 分析
在 bug 分析时，请参考源码：examples/MyAdder/Adder.v。
### 其他
所有的文档和注释都用中文编写。
```

### 人工估时

- `5` 分钟

## 16. Step 12：运行 `picker export`

### 要做什么

把 `Adder.v` 导出到 `output/` 目录。

### 为什么这么做

这是把 `RTL` 变成后续验证工程的关键转换步骤。

### 指令

```bash
picker export Adder/Adder.v --rw 1 --sname Adder --tdir output/ -c -w
ls -l output/Adder
```

### 指令含义

1. `picker export ...`
   - 执行导出
2. `Adder/Adder.v`
   - 指定输入 RTL 文件
3. `--rw 1`
   - 题面给定参数，按题面照做
4. `--sname Adder`
   - 指定顶层模块名
5. `--tdir output/`
   - 指定输出目录
6. `-c -w`
   - 题面给定参数，按题面照做
7. `ls -l output/Adder`
   - 查看输出目录是否真的生成

### 你要检查什么

至少确认 `output/Adder` 目录存在，并且里面出现导出产物。

### 人工估时

- `8` 到 `15` 分钟

## 17. Step 13：复制规格文件到输出目录

### 要做什么

把 `Adder/README.md` 复制到 `output/Adder/README.md`。

### 为什么这么做

后续验证流程会在输出目录下读取规格说明。

### 指令

```bash
cp Adder/README.md output/Adder/README.md
ls -l output/Adder/README.md
```

### 指令含义

1. `cp ...`
   - 复制文件
2. `ls -l ...`
   - 验证复制结果

### 人工估时

- `2` 分钟

## 18. Step 14：手工对照规格和 RTL，设计测试点

### 要做什么

打开 `Adder/README.md` 和 `Adder/Adder.v`，手工检查接口规格和实际声明是否一致，并设计能覆盖普通加法、进位和边界值的测试点。

### 为什么这么做

老师要求不能使用 AI，所以测试点应由你自己设计，而不是由 AI agent 自动生成。

### 指令

```bash
rg -n "WIDTH|sum|assign" Adder/README.md Adder/Adder.v
```

### 指令含义

1. 在规格和 RTL 中查找 `WIDTH`
2. 在 RTL 中查找 `sum` 的声明
3. 在 RTL 中查找 `assign` 加法语句
4. 手工列出至少 3 类输入：普通不进位、产生 cout、结果最高位为 1

### 人工估时

- `5` 到 `8` 分钟

## 19. Step 15：手写测试脚本

### 要做什么

根据导出工程的真实目录结构，手写 Python 测试脚本或修改导出示例中的测试入口，覆盖 Step 14 设计的测试点。

### 为什么这么做

本次不能让 AI 自动生成测试，但仍然应该用仿真证明问题确实存在。

### 指令

```bash
ls -l output/Adder
rg -n "python|class|dut|Adder|Makefile|example" output/Adder
```

### 记录要求

在报告里写清楚：

1. 测试脚本文件名
2. 主要测试输入
3. 每组输入的 expected 计算方式
4. 脚本由本人手写，未使用 AI 生成

### 人工估时

- `10` 到 `20` 分钟

## 20. Step 16：运行仿真并记录输出

### 要做什么

运行手写测试，记录仿真命令、成功或失败输出，以及至少一个能暴露 bug 的 case。

### 为什么这么做

报告不能只写“看代码发现 bug”，需要有非 AI 工具跑出来的验证证据。

### 指令

```bash
cd /home/lzh/CSE5030/lab9/output/Adder

# 按导出工程里的 README、Makefile 或 Python 示例运行你手写的测试脚本
# 具体命令以 output/Adder 的真实文件结构为准
```

### 记录要求

在报告里至少记录：

```text
input a = ...
input b = ...
input cin = ...
expected sum = ...
actual sum = ...
expected cout = ...
actual cout = ...
```

### 人工估时

- `8` 到 `15` 分钟

## 21. Step 17：手工定位 `sum` 位宽 bug

### 要做什么

结合仿真失败 case 和 `Adder.v` 的代码说明 bug 在哪里。

### 为什么这么做

这是本次实验的核心，不是“跑出报告”，而是你能不能把仿真现象和代码原因对应起来。

### 标准结论

你最终应能写出类似结论：

```text
Adder 的错误在 sum 输出位宽。
当 WIDTH = 64 时，sum 被写成 output [WIDTH-2:0]，
实际只有 63 位，而正确设计应为 64 位。
这会导致加法结果的最高有效结果位被截断，
从而在部分输入下出现错误输出。
```

### 人工估时

- `8` 到 `15` 分钟

## 22. Step 18：整理手工 workflow summary

### 要做什么

手工总结你自己完成了哪些阶段，以及原题 AI 流程为什么没有使用。

### 为什么这么做

这是为了回应老师“纯手工完成”的要求。

### 你可以从这几个角度总结

1. 阅读题面
2. 排除 AI 工具
3. 准备 `Adder` 文件
4. 使用 `Picker` 导出工程
5. 手写测试用例
6. 用 Verilator / Picker / Python 运行仿真
7. 手工对比 expected / actual
8. 手工定位位宽 bug

### 注意

报告里要明确写：本次未使用 `UCAgent`、`OpenCode` 或大模型。

### 人工估时

- `8` 到 `12` 分钟

## 23. Step 19：填写 `report.md`
### 要做什么

把你的真实输出、bug 分析、workflow 总结填入报告模板。

### 为什么这么做

最终提交不是命令历史，而是结构化报告。

### 人工估时

- `20` 到 `35` 分钟

## 24. 最后的自查清单

```text
[ ] 已确认 PDF 与路径正确
[ ] 已完整阅读 note.md
[ ] 已按 note.md 的 URL 检索相关资料
[ ] 已预检 git / pip3 / make / g++
[ ] 已安装 Verilator 并确认版本 >= 5.020
[ ] 已安装 Picker
[ ] 已确认正式实验不使用 UCAgent
[ ] 已确认正式实验不使用 OpenCode / MCP
[ ] 已创建 Adder/Adder.v
[ ] 已创建 Adder/README.md
[ ] 已执行 picker export
[ ] 已复制 README 到 output/Adder
[ ] 已手工对照 README 和 Adder.v 设计测试点
[ ] 已手写测试脚本
[ ] 已运行仿真
[ ] 已记录 expected / actual
[ ] 已手工定位 Adder 的位宽 bug
[ ] 已总结非 AI workflow
[ ] 已把真实结果填入 report.md
```

## 25. 最短路径版

如果你只想按最短路径完成：

```bash
cd /home/lzh/CSE5030/lab9

# 先看 note.md，再按其中 URL 手工查背景资料

command -v git
command -v pip3

sudo apt-get install git help2man perl python3 make g++ libgz libfl2 libfl-dev zlibc zlib1g zlib1g-dev
git clone https://github.com/verilator/verilator
cd verilator
git checkout v5.020
autoconf
./configure
make
sudo make install
verilator --version

sudo apt install cmake build-essential
pip3 install swig
git clone https://github.com/XS-MLVP/picker.git
cd picker
make init
make
sudo -E make install
picker --help

mkdir -p Adder

# 手工写入 Adder/Adder.v 和 Adder/README.md

picker export Adder/Adder.v --rw 1 --sname Adder --tdir output/ -c -w
cp Adder/README.md output/Adder/README.md

rg -n "WIDTH|sum|assign" Adder/README.md Adder/Adder.v
ls -l output/Adder
rg -n "python|class|dut|Adder|Makefile|example" output/Adder
```

然后根据导出工程的真实结构手写测试脚本，运行仿真，记录 expected / actual，再手工分析 `sum` 位宽 bug 并写答案。
