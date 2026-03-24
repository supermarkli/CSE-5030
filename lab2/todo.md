### Step 1.1 Prerequisites: Enabling Detailed CPU Logging

#### 1. PDF 要求与任务

- 涉及文件：
  - `/opt/ext1/lzh/CSE5030/chipyard/generators/rocket-chip/src/main/scala/rocket/RocketCore.scala`
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/rv64ui-p-simple.out`
- 在约 `1234` 行附近加入：
  - `monitor_imm`
  - `monitor_opcode`
  - `monitor_funct3`
  - `monitor_br_taken`
  - `monitor_j`
  - `monitor_is_load`
  - `monitor_is_store`
- 在约 `1272` 行附近替换原有 `printf`
- 执行：
  - `make run-binary-debug BINARY=$RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-simple -j`
- 检查输出文件：
  - `output/chipyard.harness.TestHarness.RocketConfig/rv64ui-p-simple.out`

#### 2. TODO List

- [x] 我做了什么：在 `RocketCore.scala` 中加入了 PDF 要求的 7 个监控信号
- [x] 我做了什么：替换了 trace `printf`，日志中已输出 `pc`、`opcode`、`funct3`、`imm`、`is_ld/st`、`is_br(taken)` 等字段
- [x] 我做了什么：运行验证后，在 `rv64ui-p-simple.out` 中确认目标字段已经出现
- [ ] 要做什么：如果后续提交实验，需要保留一段可截图的 `make` 输出和 trace 输出

### Step 1.2 Writing a Simple RISC-V Assembly Program

#### 1. PDF 要求与任务

- 涉及文件：
  - `/home/lzh/CSE5030/lab2/test.S`
  - `/home/lzh/CSE5030/lab2/link.ld`
  - `/home/lzh/CSE5030/lab2/test.elf`
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/test.out`
- 新建 `test.S`
- 在 `_start:` 的 TODO 区域实现数组求和逻辑
- 新建 `link.ld`
- 执行交叉编译命令生成 `test.elf`
- 运行：
  - `make run-binary-debug BINARY=test.elf`
- 观察 `test.out`

#### 2. TODO List

- [x] 我做了什么：按 PDF 模板创建了 `test.S`
- [x] 我做了什么：在 TODO 区域实现了 `ld`、`add`、`addi`、`bnez` 组成的循环求和逻辑
- [x] 我做了什么：按 PDF 段布局创建了 `link.ld`
- [x] 我做了什么：编译生成了 `test.elf`
- [x] 我做了什么：运行仿真并确认 `test.out` 中出现了数组求和相关指令，结果为 `10`
- [ ] 要做什么：如果实验要求提交过程证明，还需要准备这一步的命令行截图

### Step 1.3 Observing Instruction Semantics

#### 1. PDF 要求与任务

- 涉及文件：
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/test.out`
  - 实验报告文档
- 从 `test.out` 中挑 4 条代表性指令
- 对每条指令分析：
  - `pc`
  - `opcode`
  - `funct3`
  - `W[...]`
  - `R[...]`
  - `imm`
  - `is_ld/st`
  - `is_br(taken)`
- 整理成实验报告内容

#### 2. TODO List

- [x] 我做了什么：已经生成并核对了 `test.out`，其中包含 `ld`、`add`、`addi`、`bnez`、`sd`、`lw` 等可用于分析的指令
- [x] 我做了什么：从 `test.out` 中正式选定了 `add`、`addi`、`ld`、`bnez` 4 条指令作为报告样例
- [x] 我做了什么：分别写出了 4 条指令的 ISA 语义与日志字段对应关系，整理到 `report.md`
- [x] 我做了什么：把 Step 1.3 分析整理进实验报告草稿 `report.md`

### Step 2.1 Creating the C Program

#### 1. PDF 要求与任务

- 涉及文件：
  - `/home/lzh/CSE5030/lab2/hello.c`
- 按 PDF 模板写出 `hello.c`

#### 2. TODO List

- [x] 我做了什么：创建了 `hello.c`
- [x] 我做了什么：按 PDF 模板实现了 `f` 和 `main`
- [x] 我做了什么：把源码保存到 `lab2` 目录

### Step 2.2 Compilation and Execution

#### 1. PDF 要求与任务

- 涉及文件：
  - `/home/lzh/CSE5030/lab2/hello.c`
  - `/home/lzh/CSE5030/lab2/hello.elf`
- 执行：
  - `riscv64-unknown-elf-gcc -fno-common -fno-builtin-printf -specs=htif_nano.specs -static -o hello.elf hello.c`
- 执行：
  - `make run-binary-debug BINARY=hello.elf`
- 观察串口输出

#### 2. TODO List

- [x] 我做了什么：编译生成了 `hello.elf`
- [x] 我做了什么：运行 `hello.elf` 并在 `hello.log` 中确认输出 `Hello, World!`
- [x] 我做了什么：保留了 `hello.log`、`hello.out`、`hello.dump` 用于截图或报告

### Step 2.3 Examining Generated Assembly

#### 1. PDF 要求与任务

- 涉及文件：
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/hello.dump`
- 找到 `hello.dump`
- 分析 `f` 和 `main` 的汇编实现

#### 2. TODO List

- [x] 我做了什么：生成并定位了 `hello.dump`
- [x] 我做了什么：查看并摘录了函数 `f` 的汇编
- [x] 我做了什么：查看并摘录了 `main` 中调用 `f` 的汇编

### 实验报告与提交

#### 1. PDF 要求与任务

- 涉及文件：
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/test.out`
  - `/opt/ext1/lzh/CSE5030/chipyard/sims/verilator/output/chipyard.harness.TestHarness.RocketConfig/hello.dump`
  - 实验报告文档
- 基于 `hello.dump` 分析函数调用与寄存器保存/恢复
- 分析 Step 1 和 Step 2 的编译参数与 bare-metal 场景的关系
- 补齐 Step 2 后整理最终截图与报告

#### 2. TODO List

- [x] 我做了什么：从 `hello.dump` 中确认调用 `f` 的跳转指令形式是 `jal`
- [x] 我做了什么：分析了 `f` 的 prologue/epilogue 涉及的寄存器
- [x] 我做了什么：分析了 `main` 的 call site 涉及的寄存器
- [x] 我做了什么：说明了 `riscv64-unknown-elf-gcc` 和系统 `gcc` 的目标平台差异
- [x] 我做了什么：逐项分析了 Step 1 汇编编译参数
- [x] 我做了什么：逐项分析了 Step 2 C 编译参数
- [x] 我做了什么：Step 1 的代码和日志已经具备，可用于截图
- [x] 我做了什么：完成 Step 2 后补齐了 `make` 输出对应的日志与产物文件
- [x] 我做了什么：执行了 `whoami` 并记录输出为 `lzh`
- [x] 我做了什么：已将 `report.md` 润色为中文成稿，并加入截图占位符
- [x] 我做了什么：整理了最终提交材料路径和报告内容到 `report.md`
- [ ] 要做什么：按课程要求手动截取 `make`、`test.out` 和 `whoami` 的截图
