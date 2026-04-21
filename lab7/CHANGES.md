# 2026-04-14

- 将本地 `markdown-to-pdf` skill 的默认样式入口从 `default-style.css` 切换到 `github-style.css`，使未显式传入 `--css` 时默认导出为 GitHub 风格；随后用该 skill 将 `report.md` 导出为 PDF。
- 补充 `report.md` 中 `3.4` 的优化代码片段，加入线程分别访问 `counter0.value` / `counter1.value` 的关键逻辑；同时为 `3.5.2`、`3.6.2`、`3.7` 补充 `L1Dcache` 统计采用汇总口径的说明。
- 根据实测 good version 仿真结果填写 `report.md` 中 `3.4`、`3.6.2`、`3.7`、`3.8`，并将 good 截图占位路径规范为 `assets/image-3.png`。
- 将 `false_sharing_mt.c` 从 bad version 改为 good version：为两个热点计数器分别使用独立 cache line，并改用 `aligned_alloc(64, ...)` 做对齐分配，减少 false sharing。
- 根据 bad version 运行输出汇总非零 `L1Dcache.m_demand_*` 统计，填写 `report.md` 中 `3.5.2 L1D Cache Data`，并计算 miss rate 为 `10.52%`。
- 根据已验证的 `ProtocolTrace` 日志填写 `report.md` 中 `2.2 Observed State Transitions`，改为中文描述 `Store -> GETX -> Exclusive_Data` 的状态转换过程。
- 根据 `false_sharing_mt.c` 的 bad version 实现填写 `report.md` 中 `3.1`、`3.2`、`3.3`，补全 `false sharing` 的成因、优化方法和 `stats.txt` 观察思路。
- 根据已验证的本机输出，填写 `report.md` 中 `1.2 Verification Result` 的 `whoami` 为 `lzh`。
- 按 `lab7.pdf` 的原始提交要求重写 `report.md`，仅保留 `gem5 Installation Verification`、`Protocol Trace Analysis`、`False Sharing Analysis` 三个提交模块及其对应占位符。
- 删除 `report.md` 中不属于 PDF 明确要求的冗余占位结构，并统一截图占位为同一终端中包含命令输出与 `whoami` 的形式。
- 调整 `todo.md` 的步骤结构，删除当前机器不适用的源码编译相关步骤，并将剩余步骤重新顺延编号。
- 将 `todo.md` 中原“处理老师给的预编译版本”的备用路径表述改为当前机器实际使用的 `gem5_package` 主工作流表述。
- 更新 `todo.md` 中与当前机器不符的路径，统一改为 `/home/lzh/CSE5030/chipyard`、`/home/lzh/CSE5030/gem5_package`、`/home/lzh/CSE5030/gem5_package/lab7` 和 `/home/lzh/CSE5030/lab7`。
- 将 `todo.md` 中依赖 `/home/gem5` 与 `build_opts/` 的步骤改写为当前机器实际使用的 `gem5_package` 预编译工作流，并补充 `LD_LIBRARY_PATH` 的真实运行方式。

# 2026-04-14

- 新增 `note.md`，按零基础视角整理本次 `Cache Coherence Lab` 的核心概念，覆盖 `gem5`、`Ruby`、`SLICC`、`MOESI`、`directory-based coherence`、`ProtocolTrace`、`stats.txt`、`false sharing` 等内容，并补充整体 ASCII 流程图
- 新增 `todo.md`，按照实验实际流程拆解为可执行步骤，逐步说明每一步做什么、为什么这么做、该执行什么指令、每条指令的含义是什么
- 新增 `report.md`，整理成中文提交模板，只保留 PDF 明确要求的提交项，所有待填写内容统一使用 `TODO` 占位
- 新增 `CHANGES.md`，记录本次 `lab7` 目录下的文档整理工作
