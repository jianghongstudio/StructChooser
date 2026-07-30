# StructChooser

以结构体实例为主结果的 Chooser 表插件：`UStructChooserTable` 继承引擎 `UChooserTable`，复用 Context / Filter / Output 列；主结果走自建 `EvaluateStructChooser`（`FInstancedStruct`），并支持 Evaluate / Nested Struct Chooser。蓝图请使用 **Evaluate Struct Chooser** 节点，不要用引擎 Object 版 Evaluate Chooser。

> **AI 入口**：先读本 README 的知识索引，再按任务打开 [`.Knowledges/00-routing.md`](.Knowledges/00-routing.md)。改代码后按 [`.Knowledges/91-ai-maintenance.md`](.Knowledges/91-ai-maintenance.md) 同步文档。

## 模块速览

| 模块 | 类型 | 路径 | 职责 |
|------|------|------|------|
| **StructChooser** | Runtime | [`Source/StructChooser/`](Source/StructChooser/) | `UStructChooserTable`、`FStructChooserBase` 行结果、`EvaluateStructChooser*`、FunctionLibrary、自动化测试 |
| **StructChooserEditor** | Editor | [`Source/StructChooserEditor/`](Source/StructChooserEditor/) | 自建表编辑器（`Private/TableEditor/`）、工厂 / AssetDefinition、Result 控件、Toolbar |
| **StructChooserUncooked** | UncookedOnly | [`Source/StructChooserUncooked/`](Source/StructChooserUncooked/) | `UK2Node_EvaluateStructChooser` |

插件依赖（见 [`StructChooser.uplugin`](StructChooser.uplugin)）：引擎 **Chooser**。编辑器侧依赖 `ChooserEditor`（列控件 / 命令 / Style），**自建** StructChooser 表编辑器，**不修改**引擎 Chooser 源码。

## 主数据流（摘要）

```
UStructChooserTable (OutputStructType)
        → 引擎列 Filter / SetOutputs（Context）
        → ResultsStructs 命中 FStructChooserBase
        → ChooseMultiStruct → FInstancedStruct
        → Evaluate / Nested 再入 EvaluateStructChooser
        → FunctionLibrary / UK2Node_EvaluateStructChooser
```

## 知识文档索引

面向 AI / 协作者的系列文档，位于 [`.Knowledges/`](.Knowledges/)。每篇只承担一类问题；**不要**把全部细节塞进 README。

| 文档 | 何时打开 | 一句话职责 |
|------|----------|------------|
| [00-routing.md](.Knowledges/00-routing.md) | 不确定该读哪篇时 | 按任务 / 症状路由到具体文档 |
| [01-architecture.md](.Knowledges/01-architecture.md) | 需要总览或划模块边界时 | 三模块边界与主数据流 |
| [02-glossary.md](.Knowledges/02-glossary.md) | 遇到陌生术语时 | 领域术语权威定义 |
| [10-asset-model.md](.Knowledges/10-asset-model.md) | 改表资产 / 行结果类型时 | `UStructChooserTable` 与 Result 模型 |
| [11-evaluation.md](.Knowledges/11-evaluation.md) | 改评估循环 / Nested / Evaluate 时 | `EvaluateStructChooser` 权威 |
| [13-runtime-consumer.md](.Knowledges/13-runtime-consumer.md) | 改 BP / Library / K2 节点时 | 消费 API 与节点 Expand |
| [40-editor-tooling.md](.Knowledges/40-editor-tooling.md) | 改工厂 / Result UI 时 | Editor（约定不改引擎） |
| [50-extension-cookbook.md](.Knowledges/50-extension-cookbook.md) | 要落地扩展功能时 | Checklist |
| [60-known-debt.md](.Knowledges/60-known-debt.md) | 评估改造风险 / 踩坑时 | 已知坑与技术债 |
| [90-refactor-log.md](.Knowledges/90-refactor-log.md) | 了解改造历史 / 记决策时 | 改造决策日志 |
| [91-ai-maintenance.md](.Knowledges/91-ai-maintenance.md) | 改完代码要同步文档时 | AI 协作与知识库维护规则 |

## 改造状态

| 项 | 状态 |
|----|------|
| 知识文档框架 | 已搭建（骨架 + 路由 + 基线） |
| Runtime：Struct 主结果 + Nested/Evaluate | 已落地（UE5.7 `EIteratorStatus` 适配） |
| Editor：自建 `FStructChooserTableEditor` + Struct-only Add Row | 已落地（自 main 移植到 Dev_5.7） |
| 官方 Chooser 隔离 | 行类型 `Hidden`；不覆盖引擎 Object creator |
| BP：`UK2Node_EvaluateStructChooser` | 已落地 |
| 自动化测试 `StructChooser.Evaluate.*` | 已落地 |
| 目标引擎 | **UE 5.7.4**（`Dev_5.7`）；main 为 5.8 方案源 |

当前阶段摘要：StructChooser 表由自建编辑器打开；Add Row 仅 Struct 三项；官方 ObjectResult 菜单无 StructChooser。见 [40-editor-tooling.md](.Knowledges/40-editor-tooling.md)。
