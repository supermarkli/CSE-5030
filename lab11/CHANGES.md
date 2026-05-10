# 2026-05-08

- 为 `report.pdf` 新增本地导出样式 `report-pdf.css` 和导出脚本 `export-report-pdf.cjs`，把 PDF 左右页边距从默认 `2cm` 缩小到 `1.1cm`，用于重新导出更紧凑的提交版 `report.pdf`。
- 使用 `markdown-to-pdf` skill 将 `report.md` 导出为 `report.pdf`，并创建 `submission/` 目录，把 `report.pdf`、`my_test.cpp`、`my_vecadd.s`、`my_vecmul.s` 整理进去，作为本次作业提交集合。
- 根据 `Question 4` 的提交要求，将 `ventus-gpgpu-isa-simulator/gpgpu-testcase/` 下的 `my_test.cpp`、`my_vecadd.s`、`my_vecmul.s` 复制到当前 `lab11/` 目录，便于和 `report.md` 一起打包提交。
- 继续按题面最小集压缩 `report.md`：删除 `Question 1` 的命令、结果记录和备注，仅保留截图；删除 `Question 4` 的文字输出记录，仅保留提交文件、截图和简短 API 说明。
- 精简 `report.md` 以贴近题面评分点：删除 `Question 2` 的重复总述、删除 `Question 4` 中冗余的设计笔记/构建命令/运行命令/appendix，仅保留提交文件、成功输出、截图和简短 API 说明；同时收紧 `Question 1` 的截图备注，去掉“若老师要求可再补拍”的自我提示。
- 复查 `report.md` 后，收紧 `Question 1` 的截图说明：不再把现有三张图表述成“单张同屏证据”，改为“同一终端工作流中的顺序证据”，并注明若老师要求严格单图同屏，应补拍一张合并截图。
- 新增 `note.md`，按零基础视角整理 `Ventus GPGPU Driver Lab` 的核心概念，覆盖 `host/device`、`driver API`、`RVV kernel`、`wf_size`、`wg_size`、`VLEN`、`LDS/PDS`、`HTIF`、`tohost`，并补充整体 ASCII 流程图。
- 新增 `todo.md`，按题面执行顺序拆解环境检查、编译 simulator、编译 driver、源码探索、编写 kernel、编译 `.riscv`、编写 `my_test.cpp`、运行验证和截图整理步骤，逐步说明做什么、为什么做、命令是什么、命令含义是什么。
- 新增 `report.md`，整理 `Question 1` 到 `Question 5` 的中文提交模板，保留英文术语，并将实验结果、截图说明、源码分析与问答内容统一留为 `TODO` 占位。
- 更新 `report.md` 的 `Question 2`，基于 `gpgpu-testcase/driver/include/ventus.h`、`gpgpu-testcase/driver/ventus.cpp`、`gpgpu-testcase/driver/test.cpp` 以及 `spike_main/spike_device.cc` 的实际实现，补全 `vt_dev_open` 到 `vt_buf_free` 的 8 步 API flow，并写明每一步在 simulator 层到底做了什么。
- 更新 `report.md` 的 `Question 3`，基于 `meta_data` 结构定义、`driver/test.cpp` 的默认参数以及 `spike_device::run` 对 `--varch` / `--gpgpuarch` 的构造方式，补全 `wf_size`、`wg_size`、`VLEN`、`ldsSize`、`pdsSize`、`metaDataBaseAddr` 和 `pdsBaseAddr` 的解释。
- 更新 `report.md` 的 `Source Exploration Record`，补写本次 `Q2-Q3` 源码探索的大致耗时、使用的 AI 工具和提示词、人工核对的源码位置，以及一次把 `spike_device.cc` 误判成 `spike_main.cc` 的复盘结论。
- 在 `ventus-gpgpu-isa-simulator/gpgpu-testcase/` 下新增 `my_link.ld`、`my_vecadd.s`、`my_vecmul.s` 和 `my_test.cpp`，实现题面要求的 bare-metal RVV vecadd/vecmul kernel 与 host 端 driver 测试程序。
- 完成代码验证：成功汇编和链接 `my_vecadd.riscv`、`my_vecmul.riscv`，确认二进制中包含 `vsetvli`、`vle32`、`vadd`/`vmul`、`vse32` 以及 `tohost` / `fromhost` 符号；随后成功编译并运行 `driver/build/my_test`，实测 `Task A` 与 `Task B` 均 `PASSED`。
- 更新 `report.md` 的 `Question 4`，补入已完成的提交文件路径、关键代码设计说明、真实 build / run 命令、实测输出记录，以及本次 `my_test.cpp` 实际调用的 API 与 simulator-level 含义。
- 更新 `report.md` 的 `Question 5`，基于 `fesvr/htif.cc`、`riscv/sim.cc` 和本次 `my_link.ld` / `my_vecadd.s` / `my_vecmul.s` 的实际写法，补全“为什么不能普通 return、HTIF 如何检测 `tohost`、省略 `tohost` 会怎样”的解释。
- 继续收尾 `report.md`：补入 `ID/NAME`、已验证的 `Q1` 构建结果与 `whoami`、appendix 中的日志证据，以及一段简短的 `Ventus` 与 `CUDA/OpenCL` 编程模型对比说明。
