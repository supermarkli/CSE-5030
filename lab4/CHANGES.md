# 2026-03-24

- 修正 `report.md` 中 Control Hazard 对 `is_br(taken)` 字段的错误解读，改为“是否控制流指令(是否实际 taken)”
- 修正默认静态预测策略结论为 `predict not taken`，并补充顺序路径与目标路径的日志证据
- 补充 Data Hazard 的 WB 周期号证据，明确说明没有发生 stall
- 为三类 hazard 的代码截图和日志截图补充说明文字，明确每张图对应的观察目标与结论
- 统一实验报告措辞与结构，使正文更接近正式提交版本
