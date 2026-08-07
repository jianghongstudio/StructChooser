# 表资产与行结果模型

> **角色**：说明 `UStructChooserTable` 字段约定与三种行结果类型。
> **何时阅读**：改表结构、行类型、校验规则时。
> **相关源码**：`Source/StructChooser/Public/StructChooserTable.h`、`StructChooserTypes.h`、对应 `.cpp`
> **相关文档**：[11-evaluation.md](11-evaluation.md)、[02-glossary.md](02-glossary.md)
> **最后更新**：2026-07-26

## UStructChooserTable

继承 `UChooserTable`（再上为 `UChooserSignature`）。

| 字段 / 行为 | 说明 |
|-------------|------|
| `OutputStructType` | 本表主结果结构体类型；子表必须一致 |
| `ApplyStructChooserDefaults()` | 强制父类 `ResultType = ObjectResult`，`OutputObjectType` 占位为 `UObject`，以便引擎编辑器显示 Result 列 |
| `OnOutputStructTypeChanged` | Editor 多播；K2 节点订阅以重建 Result 引脚 |
| `IsDataValid` | 要求已设 `OutputStructType`；行/Fallback 必须为 StructChooser 结果类型，且类型匹配 |

沿用父类：

- `ContextData`、`ColumnsStructs`（Filter / Output 列）
- `ResultsStructs`（Editor）、`CookedResults`（Cooked）、`FallbackResult`
- `RootChooser`、`NestedObjects` / Nested 外层机制

## 行结果类型（均派生于 FStructChooserBase → FObjectChooserBase）

| 类型 | Meta Category | 职责 |
|------|---------------|------|
| `FStructValueChooser` | StructChooser | `Name`（显示）+ `Value`（`FInstancedStruct` 返回值） |
| `FEvaluateStructChooser` | StructChooser | `Chooser` → 外部资产 **或** 同资产嵌入子表（编辑器 Select Existing） |
| `FNestedStructChooser` | StructChooser | `Chooser` → 同包内嵌 `UStructChooserTable` |

`FStructChooserBase` 自身 `Meta=(Hidden)`，不出现在菜单。`ChooseObject` 默认返回 `nullptr`，避免被引擎 Object 评估误用成假资产。

## 校验规则（IsDataValid）

1. `OutputStructType` 非空。
2. `FStructValueChooser::Value` 的 ScriptStruct（若有效）必须等于本表 `OutputStructType`。
3. Evaluate / Nested 指向的子表（取其 Root）`OutputStructType` 必须等于本表。
4. 若结果是 `FObjectChooserBase` 但不是 `FStructChooserBase` → 报错（禁止混用引擎 Asset 行）。

## 创建约定

- 工厂 / Initializer：设 `OutputStructType`，并默认加一行 `FStructValueChooser`（`Value` 初始化为该类型）。
- Nested 新建嵌入表：拷贝父 Root 的 `OutputStructType`，`RootChooser` 指向 Root，并 `AddNestedObject`。

## 待充实

- Cook 路径上 `PopulateCookedData` 与 Struct 行的完整调用链摘录
