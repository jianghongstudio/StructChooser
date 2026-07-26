# 领域术语表

> **角色**：术语权威定义；其他文档引用本表，避免同词多义。
> **何时阅读**：遇到陌生类型名 / 注释中的概念时。
> **相关源码**：`StructChooserTable.h`、`StructChooserTypes.h`、`StructChooserFunctionLibrary.h`
> **相关文档**：[01-architecture.md](01-architecture.md)、[10-asset-model.md](10-asset-model.md)
> **最后更新**：2026-07-26

## 核心术语

| 术语 | 含义 |
|------|------|
| **StructChooserTable** | 资产 `UStructChooserTable`，`UChooserTable` 子类；主结果为结构体 |
| **OutputStructType** | 表级 `UScriptStruct*`，约束行 Value 类型与子表兼容性 |
| **FStructChooserBase** | 行结果基类（继承 `FObjectChooserBase` 以便进 `ResultsStructs`），提供 `ChooseStruct` / `ChooseMultiStruct` |
| **FStructValueChooser** | 具体结构体值行；含显示名 `Name` + 载荷 `Value`（`FInstancedStruct`） |
| **Name（行显示名）** | `FStructValueChooser::Name`，仅编辑器展示/调试友好名，不参与评估匹配 |
| **Value** | 行内真正返回的 `FInstancedStruct` |
| **FEvaluateStructChooser** | 引用**另一份** `UStructChooserTable` 资产，选中行时再评估 |
| **FNestedStructChooser** | 引用**同资产内嵌入**的 `UStructChooserTable`（Outer/NestedObjects） |
| **EvaluateStructChooser** | 本插件评估入口；过滤列 → SetOutputs → `ChooseMultiStruct` |
| **ResultsStructs / CookedResults** | 父类字段：编辑器源数据 / Cook 后运行时数据 |
| **ResultTypeFilter** | （已撤回）曾拟用引擎挂钩过滤菜单；现约定不改引擎，见 D1 |
| **Evaluate Struct Chooser（节点）** | `UK2Node_EvaluateStructChooser`；按 `OutputStructType` 出 Struct 引脚 |

## 易混淆

| 易混对 | 区分 |
|--------|------|
| StructChooser vs 引擎 NoPrimaryResult | 后者无主结果、禁止 Nested；前者有主结果结构体且可嵌套 |
| `EvaluateStructChooser` vs 引擎 `EvaluateChooser` | 前者返回结构体；后者只认 `UObject*`，对本表行无用 |
| `FEvaluateStructChooser` vs 引擎 `FEvaluateChooser` | 分别指向 Struct / Object 子表；菜单过滤后 Struct 表不应出现后者 |
| `Name` vs `OutputStructType` | Name 是行标签；OutputStructType 是表级结构体类型 |
| `FStructChooserBase` vs `FObjectChooserBase` | 前者加 Struct API；仍继承后者以便放进引擎结果数组 |
| 打开编辑器用的 ObjectResult 占位 vs 真正主结果 | 父类 `ResultType=ObjectResult` 只为 UI；语义以 Struct 评估为准 |

## 待充实

- 多结果（AllResults）与 Output 列数组模式的交叉说明
