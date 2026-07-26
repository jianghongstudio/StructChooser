# StructChooser

以结构体实例为主结果的 Chooser 表插件：`UStructChooserTable` 继承引擎 `UChooserTable`，复用 Context / Filter / Output 列；主结果走自建 `EvaluateStructChooser`（`FInstancedStruct`），并支持 Evaluate / Nested Struct Chooser。蓝图请使用 **Evaluate Struct Chooser** 节点，不要用引擎 Object 版 Evaluate Chooser。

> **AI 入口**：先读本 README 的知识索引，再按任务打开 [`.Knowledges/00-routing.md`](.Knowledges/00-routing.md)。改代码后按 [`.Knowledges/91-ai-maintenance.md`](.Knowledges/91-ai-maintenance.md) 同步文档。

## 模块速览

| 模块 | 类型 | 路径 | 职责 |
|------|------|------|------|
| **StructChooser** | Runtime | [`Source/StructChooser/`](Source/StructChooser/) | `UStructChooserTable`、`FStructChooserBase` 行结果、`EvaluateStructChooser*`、FunctionLibrary、自动化测试 |
| **StructChooserEditor** | Editor | [`Source/StructChooserEditor/`](Source/StructChooserEditor/) | 资产工厂 / AssetDefinition、Result 控件、Table Settings Details、Initializer |
| **StructChooserUncooked** | UncookedOnly | [`Source/StructChooserUncooked/`](Source/StructChooserUncooked/) | `UK2Node_EvaluateStructChooser` |

插件依赖（见 [`StructChooser.uplugin`](StructChooser.uplugin)）：引擎 **Chooser**。编辑器侧依赖 `ChooserEditor`（见 Build.cs）以**复用**表编辑器，**不修改**引擎 Chooser 源码。

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
| Runtime：Struct 主结果 + Nested/Evaluate | 已落地 |
| Editor：复用引擎 Chooser 表编辑器 + Result Name UI | 已落地 |
| Add Row / 单元格类型过滤 | 不改引擎做不到藏菜单；Details 已过滤 + PostEdit 误选纠正 |
| BP：`UK2Node_EvaluateStructChooser` | 已落地 |
| 自动化测试 `StructChooser.Evaluate.*` | 已落地 |

当前阶段摘要：可创建 StructChooser 表、填 Struct 行名与详情、嵌套/外链评估、蓝图取结构体。见 [01-architecture.md](.Knowledges/01-architecture.md)。
