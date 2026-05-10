# Lab 9 XiangShan Micro-Architecture 零基础笔记

## 1. 这次 lab 到底在做什么

这次 `lab9` 的主线不是“自己从零写复杂硬件”，而是：

1. 搭一个最小的硬件验证环境
2. 准备一个带有故意缺陷的 `Adder` 电路
3. 把这个 `RTL` 电路导出成 Python 可调用的验证工程
4. 手写测试用例，并用非 AI 工具运行仿真
5. 根据仿真结果和代码，定位 `Adder` 里的 bug

你可以先把这次实验粗略理解成：

```text
写一个有 bug 的加法器
    ->
用工具把它变成“软件能调用”的形式
    ->
手写测试并跑仿真
    ->
对比 expected / actual
    ->
指出 bug 在哪、为什么错
```

这次题面原始版本要求借助 `UCAgent + OpenCode + MCP` 做自动化验证，但你补充说老师要求“不能借助 AI 工具”，且截图不用提交。因此正式实验中不要使用 `UCAgent`、`OpenCode` 或任何大模型。这里的“手工”不是不用仿真工具，而是测试设计、命令执行、结果分析和报告结论都由你自己完成。

## 2. 整体 ASCII 流程图

```text
+--------------------------------------+
| 读题面，明确本次交付物                |
| 手工记录 / bug 分析 / workflow 总结   |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 1 搭环境                         |
| Verilator / Picker                   |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 2 准备 DUT                      |
| 创建 Adder.v 和 README.md            |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 3 导出验证工程                   |
| picker export -> output/Adder        |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 4 手写测试用例                   |
| 覆盖普通加法 / 进位 / 边界值          |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 5 运行仿真                       |
| 用 Python / Verilator 执行测试        |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 6 记录失败 case                  |
| 对比 expected / actual                |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 7 手工分析 Adder width bug       |
| 结合仿真结果说明 sum 位宽少 1 位      |
+--------------------------------------+
                  |
                  v
+--------------------------------------+
| Step 8 整理手工记录、bug 解释、总结   |
| 说明未使用任何 AI 工具                |
+--------------------------------------+
```

## 3. 先记住这次要交什么

原题最后要求回答 3 个问题，但老师已经取消截图并要求纯手工，因此你现在更应该提交这些内容：

1. 手工执行记录
   - 说明你没有使用 `UCAgent`、`OpenCode` 或大模型
   - 记录你实际使用的非 AI 工具、关键命令和仿真输出
2. Bug 分析
   - 对照 `Adder/README.md` 和 `Adder/Adder.v`
   - 记录能暴露问题的测试输入、expected / actual
   - 说明 `sum` 位宽少 1 位导致结果被截断
3. Workflow 总结
   - 说明你如何手工完成环境准备、工程导出、测试设计、仿真运行和 bug 分析
   - 简要说明原题 AI 自动化流程与本次手工流程的区别

## 4. 这次 lab 的核心概念

## 4.1 `RTL`

- 全称：`Register-Transfer Level`
- 直观理解：用硬件描述语言写电路逻辑
- 这次文件：`Adder/Adder.v`

你可以先把 `RTL` 理解成“不是普通软件程序，而是描述电路输入、输出和组合逻辑的代码”。

参考：

- Verilator overview: https://verilator.org/guide/latest/overview.html

## 4.2 `Verilog`

- 一种常见硬件描述语言
- 用来描述模块、端口、连线、寄存器和组合逻辑
- 这次 `Adder.v` 就是 `Verilog`

这次题目里的 `Adder` 本质上是一个参数化加法器模块：

```text
输入：a, b, cin
输出：sum, cout
功能：a + b + cin
```

参考：

- Verilator language support: https://verilator.org/guide/latest/languages.html

## 4.3 `DUT`

- 全称：`Design Under Test`
- 中文可理解成“被测设计”
- 这次 `DUT` 就是 `Adder`

做验证时，你不是验证整个世界，而是只验证这一个目标模块是否满足规格。

参考：

- UCAgent README: https://github.com/XS-MLVP/UCAgent/blob/main/README.zh.md

## 4.4 `Specification`

- 中文：规格说明、需求说明
- 意思是“这个模块应该做什么”
- 这次规格写在 `Adder/README.md`

为什么要写这个文件：

1. 让验证流程知道模块的输入输出语义
2. 让后续测试围绕“加法功能”展开
3. 避免验证内容跑偏到无关方向

参考：

- UCAgent README: https://github.com/XS-MLVP/UCAgent/blob/main/README.zh.md

## 4.5 `Verilator`

- 一个把 `Verilog/SystemVerilog` 转成可执行仿真模型的工具
- 可以理解成“硬件世界里的编译/仿真基础设施”
- 这次 `Picker` 依赖它

题面特别要求 `Verilator >= 5.020`，因为本 lab 使用的 `Picker` 版本要求这个下限。

参考：

- 官方安装文档: https://verilator.org/guide/latest/install.html

## 4.6 `Picker`

- 一个把 `RTL` 导出成高层语言接口的工具
- 题面里它的作用是：把 `Adder.v` 导出成 Python 可调用的验证工程
- 你可以理解成“给硬件模型包一层软件接口”

`picker export` 做完后，你会得到一个输出目录，里面包含：

1. 编译后的仿真相关内容
2. Python 调用接口
3. 波形文件等验证辅助产物

参考：

- Picker 仓库: https://github.com/XS-MLVP/picker
- Picker 安装与导出说明: https://open-verify.cc/mlvp/en/docs/quick-start/installer/

## 4.7 `Toffee`

- 一个 Python 硬件验证框架
- 作用是：用 Python 驱动硬件、写测试、比较结果
- 在这次 lab 中，它是验证环境的一部分

你不用把它理解得太深，只要知道：

```text
Adder RTL
    ->
Picker 导出 Python 接口
    ->
Toffee / Python 验证逻辑调用这个接口
```

参考：

- Toffee 仓库: https://github.com/XS-MLVP/mlvp

## 4.8 `UCAgent`

- 一个面向硬件单元测试验证的 AI agent
- 它能组织验证阶段、调用工具、生成测试、运行测试、整理报告
- 这次题面要求它以 `MCP server` 方式暴露能力
- 因为老师要求纯手工且不能使用 AI 工具，所以本次正式实验不要安装、启动或使用它

你只需要理解它在原题里的角色：

```text
它不是 DUT 本身
它是“验证流程组织者”
```

参考：

- UCAgent README: https://github.com/XS-MLVP/UCAgent/blob/main/README.zh.md
- UCAgent 论文: https://arxiv.org/pdf/2603.25768

## 4.9 `MCP`

- 全称：`Model Context Protocol`
- 可以理解成“让 AI 工具和外部工具/资源通信的标准接口”
- 这次题面里，`OpenCode` 通过 `MCP` 去连接 `UCAgent`
- 本次非 AI 正式流程不需要配置或使用 `MCP`

你可以先把它想成：

```text
OpenCode 是客户端
UCAgent 是服务端
MCP 是双方说话的协议
```

参考：

- MCP 官方介绍: https://modelcontextprotocol.io/docs/getting-started/intro
- MCP 规范: https://modelcontextprotocol.io/specification/2025-03-26

## 4.10 `OpenCode`

- 一个代码 agent / CLI 工具
- 题面里它被当成主要的外部 agent，用来连接 `UCAgent`
- 配置文件是 `opencode.json`
- 因为它是 AI agent，本次正式实验不要安装、启动或使用它

这一步本质上不是验证逻辑本身，而是“把 agent 和 MCP server 连起来”。

参考：

- OpenCode config 文档: https://opencode.ai/docs/config/

## 4.11 `TUI`

- 全称：`Text User Interface`
- 就是终端里的交互式文字界面
- 原题要求截图 `UCAgent TUI` 正常运行的画面
- 现在老师取消截图要求，所以本次不用启动 `UCAgent TUI`

所以你至少要会识别：

```text
不是普通一行 shell 输出
而是一个持续运行、可交互的终端界面
```

参考：

- UCAgent README: https://github.com/XS-MLVP/UCAgent/blob/main/README.zh.md

## 4.12 `HTML test report`

- 原题中由自动化验证流程生成的网页报告
- 原题路径是：`output/uc_test_report/index.html`
- 本次非 AI 正式流程不依赖这个报告来定位 bug

你可以把它理解成：

```text
测试有没有过
失败在哪
为什么怀疑这个模块有 bug
```

都会在这份报告里体现。

## 5. 这次故意注入的 bug 是什么

题面已经把 bug 说出来了，只是你写报告时不能只抄一句话，要理解它。

原题说的是：

```text
output bit width modified to 63 bits
```

对应代码是：

```verilog
output [WIDTH-2:0] sum,
```

如果 `WIDTH = 64`：

- 正常应该是 `sum[63:0]`，共 `64` 位
- 现在却写成了 `sum[62:0]`，只有 `63` 位

这会带来什么问题：

1. 低 63 位还能保留
2. 原本应属于 `sum` 的最高位被挤掉
3. 某些加法结果会被截断
4. 导致输出结果与真正的 64 位加法不一致

你可以把它记成一句最关键的话：

```text
这不是“加法公式写错了”，而是“sum 这个输出端口的位宽少了 1 位”。
```

## 6. 为什么 `assign {cout, sum} = a + b + cin;` 会暴露这个 bug

这一句的意思是：

```text
把加法结果拼接后分配给 cout 和 sum
```

正常情况下，如果 `sum` 是 `64` 位，`cout` 是 `1` 位，那么：

- 总共会接收 `65` 位结果
- `cout` 接最高位
- `sum` 接低 `64` 位

但现在 `sum` 只有 `63` 位：

- `cout + sum` 一共只有 `64` 位
- 真正的 `65` 位加法结果无法完整容纳

所以你会看到一种典型现象：

```text
某些本来应该落在 sum 最高位的位置
会被截掉或者错位
```

## 7. 为什么这次既有 `sum` 又有 `cout`

这是初学者容易困惑的点。

做二进制加法时：

- `sum` 保存主要结果位
- `cout` 保存最高位进位

例如 64 位加法：

```text
64 位 + 64 位 + cin
可能产生 65 位结果
```

所以：

- 低 64 位给 `sum`
- 第 65 位给 `cout`

这也是为什么 `sum` 的位宽少 1 位会出问题，因为电路本来就需要严格区分“结果位”和“进位位”。

## 8. `picker export` 到底做了什么

题面命令：

```bash
picker export Adder/Adder.v --rw 1 --sname Adder --tdir output/ -c -w
```

你先不用死背全部参数，只要理解 3 个关键动作：

1. 读取 `Adder.v`
2. 识别顶层模块 `Adder`
3. 在 `output/` 下生成可用于后续验证的工程

所以它的本质是：

```text
把一个 RTL 文件
导出成一个更容易被 Python/验证框架调用的工程目录
```

## 9. 为什么还要复制 `README.md` 到 `output/Adder/`

题面要求：

```bash
cp Adder/README.md output/Adder/README.md
```

原因不是“为了好看”，而是为了把规格说明一起放进导出的工程里。

这样后续验证流程能直接在输出目录读到：

1. 模块要做什么
2. 验证目标是什么
3. bug 分析时应该参考哪个源码
4. 文档语言要求是什么

## 10. 原题为什么要开两个 terminal

原题要求：

1. terminal A 运行 `ucagent`
2. terminal B 运行 `opencode`

这是因为它们职责不同：

- `ucagent` 负责启动 MCP server 和验证上下文
- `opencode` 负责作为客户端接入并驱动任务

你可以理解成：

```text
terminal A = 服务端
terminal B = 客户端
```

本次非 AI 正式流程不使用 `ucagent` 和 `opencode`，所以不需要按这个双终端结构执行。

## 11. 原题为什么要求 `whoami` 出现在截图里

原题要求截图中带 `whoami` 输出，通常是为了证明：

1. 截图是你自己机器上的真实运行结果
2. 不是别人转发的图
3. 不是只截了一个静态页面

但你已经确认截图不用提交，所以本次不需要执行这一步。

原题截图顺序通常是：

```text
先运行 ucagent
    ->
确认 TUI 已经起来
    ->
同一终端或旁边终端输入 whoami
    ->
一起截图
```

## 12. 如果老师要求纯手工完成，怎么理解这次实验

你要把原题的“AI 自动化验证”翻译成非 AI 手动验证流程：

```text
原题的 AI 自动生成测试
    ->
你手写测试用例

原题的 AI 自动执行验证
    ->
你自己运行 Picker / Verilator / Python 仿真

原题的 AI 自动读报告和定位 bug
    ->
你手工对比 expected / actual，并分析代码错误原因

原题的 AI 自动总结 workflow
    ->
你手工描述自己完成了哪些阶段
```

也就是说，你报告里可以写：

- 本实验原始流程依赖 `UCAgent/OpenCode`
- 但根据老师要求，本次没有使用任何 AI 工具
- `UCAgent/OpenCode/MCP` 只作为原题背景，不参与正式实验执行

## 13. 最短理解版

如果你时间很紧，只记下面这几句也够用：

1. `Adder.v` 是被测电路 `DUT`
2. bug 不是加法逻辑错，而是 `sum` 位宽少了 1 位
3. `Picker` 的作用是把 `RTL` 导出成 Python 可调用工程
4. `UCAgent` 和 `OpenCode` 是原题中的 AI 自动化工具，本次不能用
5. 本次核心是手写测试、运行仿真、记录失败 case，再说明位宽 bug

## 14. 参考资料汇总
