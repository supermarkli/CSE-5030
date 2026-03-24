# CSE-5030

`CSE-5030` 课程成果仓库，用来整理各次 lab 的笔记、源码、报告和截图。

## 仓库定位

这个仓库只追踪“课程成果”相关内容，不追踪完整开发环境。

因此以下大型工程目录和本地环境目录不会进入版本控制：

- `chipyard/`
- `NEMU/`
- `sustemu_lab/`
- `gem5_env/`

这些目录仍然保留在本地，用于编译、仿真和实验，但不会上传到这个轻量仓库中。

## 目录结构

```text
CSE5030/
├── README.md
├── CHANGES.md
├── prompt.md
├── assets/
├── lab2/
├── lab3/
└── lab4/
```

### 根目录说明

- `README.md`
  - 仓库总览
- `CHANGES.md`
  - 根目录级别的改动记录
- `prompt.md`
  - 工作过程中的上下文提示记录
- `assets/`
  - 跨 lab 共用的截图或图片资源

## 各 Lab 的统一组织方式

从当前版本开始，每个 lab 目录尽量按下面的思路组织：

```text
labX/
├── README.md
├── CHANGES.md
├── note.md
├── todo.md
├── report.md
├── report.pdf
├── src/
├── assets/
└── refs/
```

各目录职责如下：

- `README.md`
  - 说明这一题做了什么、目录里每个文件是什么
- `CHANGES.md`
  - 记录该 lab 内部的修改历史
- `note.md`
  - 概念笔记、背景知识、日志解释
- `todo.md`
  - 可执行步骤、命令和操作说明
- `report.md`
  - 报告源文件
- `report.pdf`
  - 导出的最终报告
- `src/`
  - 实验源码，例如 `.c`、`.S`、`.ld`
- `assets/`
  - 报告截图、日志截图、图像材料
- `refs/`
  - 题目 PDF 或参考材料

## 跟踪与忽略策略

会被跟踪的内容：

- Markdown 笔记和报告
- 实验源码
- 选定的截图和资源
- 导出的报告文件

会被忽略的内容：

- 大型工程仓库
- 常见编译产物，例如 `*.elf`、`*.bin`、`*.o`
- 生成目录，例如 `build/`、`output/`、`generated-src/`、`target/`
- 本地缓存目录
- 原始课程 handout PDF

具体规则见 [`.gitignore`](/opt/ext1/lzh/CSE5030/.gitignore)。

## 说明

- 这个仓库更像“课程成果档案”，不是完整开发环境快照。
- 实际仿真和编译依赖的大工程保留在本地，不随仓库上传。
