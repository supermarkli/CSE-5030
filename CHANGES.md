# 2026-03-24

- 更新 `lab4/todo.md` 中的环境初始化路径，修正为从 `chipyard/env.sh` 加载实验环境，避免误写为仓库根目录下不存在的 `env.sh`。
- 更新根目录 `README.md` 为中文，并补充仓库定位、统一目录结构与跟踪策略说明。
- 调整 `lab2`、`lab3`、`lab4` 的目录结构：将实验源码整理到 `src/`，将题目材料整理到 `refs/`，并为各 lab 新增中文 `README.md`。
- 更新根目录 `.gitignore`，适配 `refs/` 下的课程 handout PDF 忽略规则。
- 新增根目录 `README.md`，说明仓库用途、目录结构、跟踪范围与忽略策略，明确该仓库定位为课程成果仓库而非完整工程环境快照。
- 新增根目录 `.gitignore`，忽略 `chipyard`、`NEMU`、`sustemu_lab`、`gem5_env` 等工程目录，以及常见构建产物、缓存目录和课程 handout PDF，便于将 `/home/lzh/CSE5030` 作为课程成果仓库管理。
- 更新 `lab4/note.md`，补充 `RocketCore.scala` 中与本次 lab 直接相关的信号说明，包括 `id_inst(0)`、`ex_reg_inst`、`ex_reg_pc`、`ex_pc_valid`、`id_raddr`、`ex_reg_rs_bypass`、`ex_reg_rs_lsb`、`ex_rs` 的含义与它们如何配合打印 BYPASS 日志。
- 更新 `lab4/note.md`，补充 Chipyard / Rocket Chip / RocketCore / Verilator 的工程背景，并解释 `Generator.scala`、`DigitalTop.scala`、`ChipTop.scala`、`ConfigFinder.scala` 等关键文件在整体链路中的位置。
- 新增 `lab4/note.md`，整理 Lab 4 相关概念笔记，覆盖 pipeline、三类 hazard、bypass、stall、branch prediction 与日志阅读方法。
- 新增 `lab4/todo.md`，按执行顺序拆解本次实验步骤，说明每步做什么、为什么做、命令是什么、命令含义是什么。
- 新增 `lab4/report.md`，生成可直接填写的实验报告模板，所有待补内容统一使用 `TODO` 占位。
