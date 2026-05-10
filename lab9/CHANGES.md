# 2026-05-09

- 调整 `markdown-to-pdf` skill 默认样式：缩小 PDF 页边距和整体缩放，压缩正文、表格、代码块字号，并启用长表格/长代码自动换行以避免内容溢出裁切。
- 生成 `report.pdf`：将 `report.md` 转换为 8 页 PDF，并用 `pdfinfo`、`pdftotext` 验证 PDF 存在且包含关键报告内容。
- 更新 `report.md`：从用时分析表删除没有正文对应的“与原题 AI 流程的区别”和 “Personal Reflection”两项，保持报告结构一致。
- 重构 `report.md`：修复 `4.1` 缺失和章节顺序问题，补充两个失败 case 的完整分析，为后半章节补充时间开销，并新增总用时分析。
- 完成 `todo.md` Step 17/18/19：在 `report.md` 补齐 bug 结论后的 workflow summary、原题 AI 流程对比和 personal reflection，移除报告剩余 `TODO`。
- 完成 `todo.md` Step 15/16：新增 `output/Adder/manual_test.py` 手写测试脚本，运行 Python 仿真得到 2 个失败 case，并在 `report.md` 回填仿真命令、输出、失败记录和 bug 分析。
- 完成 `todo.md` Step 14：手工对照 `README.md`、`Adder.v` 和 `signals.json`，在 `report.md` 补充人工测试点、expected 计算方式和预期结果。
- 完成 `todo.md` Step 12：运行 `picker export` 生成 `output/Adder`，复制 `README.md`，并在 `report.md` 记录真实命令、`-w` 参数修正、成功输出和关键产物。
- 更新 `note.md`、`todo.md`、`report.md`：将流程从“手工检查规格和 RTL”修正为“不使用 AI，但手写测试并用 Verilator / Picker / Python 运行仿真”，补充测试设计、仿真输出和 expected / actual 记录要求。
- 修正 `note.md`、`todo.md`：将残留“纯手工流程”改为“非 AI 正式流程”，并修正 `todo.md` 后半段标题编号。
- 更新 `report.md`：补充 `3.3 Picker Export`，对应 `todo.md` Step 12/13，记录 `picker export`、输出目录检查和复制规格文件。
- 调整 `Adder/Adder.v`：将 `Intentional bug injected here` 注释位置改回题面样式，保持 `sum` 位宽 bug 不变。
- 新增 `Adder/Adder.v` 和 `Adder/README.md`：完成 `todo.md` Step 10/11，创建带有故意 `sum` 位宽 bug 的 64 位加法器和对应中文规格说明。
- 更新 `report.md`：在 `3.2 Lab Tool Check` 补充 Verilator / Picker 的主要安装命令和真实参考 URL。
- 安装并验证课程级工具链：`Verilator 5.020` 安装到 `/home/lzh/CSE5030/tools/verilator`，`Picker` 安装到 `/home/lzh/CSE5030/tools/picker`，并更新 `report.md` 的 `3.2 Lab Tool Check`。
- 填写 `report.md`：完成 `3.2 Lab Tool Check`，记录当前环境中 `Verilator` 和 `Picker` 未在 `PATH` 中，并补充人工安装用时估计 `35 到 65 分钟`。
- 调整 `report.md`：将 `3.1 Basic Command Check` 从命令路径改为版本信息，报告可读性更强。
- 填写 `report.md`：根据 `command -v git/pip3/python3/make/g++` 的真实输出完成 `3.1 Basic Command Check`。
- 修正 `report.md`：将环境检查拆成 `Basic Command Check` 和 `Lab Tool Check`，使基础命令预检与 `todo.md` Step 3 对齐，并修正后续章节编号。
- 更新 `note.md`：按“完全不使用 AI 工具”的新要求重写整体流程和交付说明，明确 `UCAgent`、`OpenCode`、`MCP`、`TUI` 截图和 HTML report 都只属于原题背景，不参与正式执行。
- 更新 `todo.md`：移除正式流程中的 `UCAgent` 安装/启动、`OpenCode/MCP` 配置、截图、`whoami` 和 HTML report 依赖，改为 `Verilator / Picker` 加手工对照 `README.md` 与 `Adder.v` 的纯手工流程。
- 重写 `report.md`：改成纯手工报告模板，保留环境检查、关键命令、手工 bug 分析、workflow summary 和“未使用 AI 工具”的说明。
- 更新 `note.md`：将整体 ASCII 流程图改为老师要求下的手工执行版，移除正式流程中的 `OpenCode` 驱动步骤，强调手工观察、手工打开报告和手工分析 bug。
- 更新 `todo.md`：把 `OpenCode / MCP` 从正式执行步骤调整为原题背景理解内容，删除安装、配置和启动 `OpenCode` 的实际执行要求，并同步更新总流程、自查清单和最短路径命令。
- 修正 `todo.md`：将残留的 `terminal B 启动 OpenCode` 标题改为跳过正式执行，并移除不再需要的 `npm` 相关预检和假设。

# 2026-05-07

- 更新 `todo.md`：补充“先阅读 `note.md`、再按参考 URL 检索背景资料”的前置步骤，并增加对应人工估时、自查项与最短路径提示。
- 细化 `report.md`：为各处 `TODO` 补充填写提示，方便后续按真实实验结果手工回填。
- 新增 `note.md`：整理 `lab9` 涉及的核心概念、整体 ASCII 流程图、手工完成视角下的实验理解，以及官方/项目参考链接。
- 新增 `todo.md`：按人工执行方式拆解完整实验步骤，补充每步目的、命令、命令含义与人工估时。
- 新增 `report.md`：生成中文报告模板，保留英文术语，并用 `TODO` 预留最终实验结果与答案位置。
