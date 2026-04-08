# 2026-04-07

- 进一步修订 `report.md` 的提交版表述：在 `Question 2`、`Question 3` 补入 `aligned(2048) = 2^11` 如何“改变 tag、不改变 BTB index”的位级解释，并在 `Question 5` 明确说明提交时会附带真实的 `src/cpu/bpred2.c` 源文件
- 修订 `report.md` 的提交风险项：补入学号姓名，说明 Q2 原始 `aligned(2048)` 数据的来源，修正多处 Markdown 反引号段落，并在 `Question 5` 增补 AI 提供的关键 insight 以及验证、debug 过程
- 更新 `report.md` 的 `Question 4` 和 `Question 5`，补入 cold/warm 测量结果、`bpred2` 实现代码、自定义 predictor 与 tournament baseline 的对比数据，并用中文写入原因分析
- 更新 `report.md` 的 `Question 3`，补入去掉 `aligned(2048)` 前后的 `loop_B` back-edge PC、BTB index、`BTB cold misses`、`cycles` 对照数据，并用中文说明 BTB aliasing 消失的原因
- 更新 `report.md` 的 `Question 2`，用中文写入 `loop_A`、`loop_B` 的 back-edge PC、BTB index、是否冲突，以及 `BTB tag mismatch` 的性能代价分析，并引用 Q2 baseline 的 `BTB cold misses` 与 `cycles` 结果作为佐证
- 新增 `note.md`，按零基础视角整理本次 Branch Predictor lab 的核心概念，覆盖 `BTB`、`BTB aliasing`、`2-bit saturating counter`、`local/global/tournament predictor`、`gshare`、`cold start`、`stale history`、`speculative update` 等内容
- 新增 `todo.md`，按 `Question 1` 到 `Question 5` 拆解完整实验流程，逐步说明每一步要做什么、为什么这么做、执行什么指令、指令含义是什么，以及应记录哪些结果
- 新增 `report.md`，整理成可直接填写的中文模板，只保留 PDF 明确要求提交的截图、结果表格和书面答案，所有答案统一用 `TODO` 占位
- 新增 `CHANGES.md`，记录本次在 `lab6` 目录下的文档整理工作
- 在文档中显式标注目录陷阱：PDF 的 `Question 5` 实际对应 `predictor_lab/Q4`，`predictor_lab/Q5` 是下一题 `Spectre v1`，避免后续跑错目录
- 补充 `note.md` 中关于 `BTB`、`predictor`、`snpc`、`dnpc` 的配合关系说明，加入一段 ASCII 流程图和“为什么 `BTB` 不能代替 local predictor”的直白解释
