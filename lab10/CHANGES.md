# Change Log

## 2026-05-12

- 按最终提交要求同步 `/home/lzh/CSE5030/nexus-am/apps/hello/hello.c` 到 `lab10/hello.c`；更新 `report.md` 的 `Question 4` 提交路径为 `lab10/hello.c`，并精简 `Question 6` 中关于 `vsub + vmerge` 的冗余解释。
- 修正 `report.md` 的 `Question 6` 表述：明确 Task B 当前实现是先用 `vsub` 生成 uppercase candidate，再用 `vmerge` 按 lowercase mask 选择写回；补充说明没有直接采用 masked subtraction 的原因是 masked-off lane 必须正确保留原字符，否则可能导致非小写字符被破坏或 `strcmp` 验证失败。
- 按 Lab 10 的 RVV 要求重新构建并验证 `apps/hello`：使用 `MARCH=rv64gcv_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt` 打开 `__riscv_vector`，在裸机程序中设置 `mstatus.VS`，将字符串转换改为 `vsub` 生成候选值后用 `vmerge` 按 mask 选择；NEMU 运行确认 `Task A PASSED`、`Task B PASSED`、`vst count = 5`，并同步更新 `lab10/hello.c` 附件和 `report.md` 的 Q4 intrinsic 说明。
- 修正 `report.md` 中 `Question 1` 的 `vreg.h` 路径，将 RVV vector register 宏位置改为 `src/isa/riscv64/instr/rvv/vreg.h`；复制 `/home/lzh/CSE5030/nexus-am/apps/hello/hello.c` 到 `lab10/hello.c` 作为提交附件，并确认两者 MD5 一致。
- 完成 `todo.md` Step 15 到 Step 21 的核心实现与验证：将 `nexus-am/apps/hello/hello.c` 替换为 RVV lab 程序，实现 `matrix-vector multiplication` 和 branchless string uppercase conversion，使用 `riscv64-unknown-elf-` 编译生成 `build/hello-riscv64-xs.bin`，并用 NEMU 运行确认 `Task A PASSED`、`Task B PASSED` 和 `HIT GOOD TRAP`。
- 更新 `report.md` 的 `Question 4` 和 `Question 6`：写入已完成的 `hello.c` 路径、使用的 RVV intrinsics 说明，以及 scalar 和 vector string conversion 的中文对比说明；`Question 5` 截图位置继续保留 `TODO`。
- 完成 `todo.md` Step 14：确认 `/home/lzh/CSE5030/nexus-am/apps/hello` 存在，目录内当前有 `Makefile` 和默认 `hello.c`；`Makefile` 使用 `NAME = hello`、`SRCS = hello.c` 并包含 `$(AM_HOME)/Makefile.app`，默认 `hello.c` 仅打印 `Hello, XiangShan!`，后续 Step 15 才需要替换为题面 RVV skeleton。
- 按用户反馈修正 `report.md` 语言：将已回填的 `Question 1` 到 `Question 3` 和 `Source Exploration Record` 改为中文表述，保留英文术语、源码路径、函数名和 TODO 占位。
- 完成 `todo.md` Step 8 到 Step 13：阅读 NEMU RVV 源码，定位 `vector registers`、`vl`、`vector masks`、`strided/indexed vector memory access` 的关键实现，并回填 `report.md` 的 `Question 1` 到 `Question 3` 与 `Source Exploration Record`。
- 完成 PDF `Step 1 - Environment Setup`：复用本地 `/home/lzh/CSE5030/NEMU`，确认 commit 为 `0860de24253ef2097663fe64095b0fc329515489`，`.config` 包含 `CONFIG_RVV=y` 和 `CONFIG_RVV_AGNOSTIC=y`，并确认 `build/riscv64-nemu-interpreter` 已存在。
- 克隆 `nexus-am` 到 `/home/lzh/CSE5030/nexus-am`，使用本机已有的 `riscv64-unknown-elf-` bare-metal 工具链编译 `apps/coremark`，生成 `build/coremark-riscv64-xs.bin`，并用 NEMU 成功运行，输出 `HIT GOOD TRAP` 和 `CoreMark Iterations/Sec 402.06`。
- 记录一次环境报错复盘：直接运行 `make ARCH=riscv64-xs` 会因未传 `AM_HOME` 报 `/Makefile.app` 缺失；改传 `AM_HOME` 后默认寻找 `riscv64-unknown-linux-gnu-gcc`，本机不存在；改用 `CROSS_COMPILE=riscv64-linux-gnu-` 又因缺少 `libc6-dev-riscv64-cross` 报 `bits/libc-header-start.h` 缺失；最终改用已安装的 `riscv64-unknown-elf-` 裸机工具链完成编译。
- 新增 `note.md`，按零基础视角整理 `RISC-V Vector ISA Lab` 的核心概念，覆盖 `NEMU`、`Nexus-AM`、`RVV`、`vl`、`vector register`、`mask`、`intrinsics`、`reduction`、`strided/indexed access`，并加入整体 ASCII 流程图。
- 新增 `todo.md`，按题面整理详细执行步骤，逐步说明要做什么、为什么做、命令是什么以及命令含义。
- 新增 `report.md`，按 PDF 的 `Question 1` 到 `Question 6` 和 `Source Exploration Record` 生成中文报告模板，所有答案保留为 `TODO` 占位，只包含题面要求提交的内容。
