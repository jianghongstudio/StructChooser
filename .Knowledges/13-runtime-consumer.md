# 运行时 / 蓝图消费

> **角色**：说明如何从游戏与蓝图取得 StructChooser 主结果。
> **何时阅读**：接 BP、改 Library、改 K2 Expand 时。
> **相关源码**：`StructChooserFunctionLibrary.*`、`EvaluateStructChooserNode.*`
> **相关文档**：[11-evaluation.md](11-evaluation.md)、[60-known-debt.md](60-known-debt.md)
> **最后更新**：2026-07-26

## C++ / Blueprint Library

`UStructChooserFunctionLibrary`：

| 函数 | 说明 |
|------|------|
| `EvaluateStructChooser(ContextObject, Table)` | 返回 `FInstancedStruct`（First） |
| `EvaluateStructChooserMulti(...)` | `TArray<FInstancedStruct>` |
| `EvaluateStructChooserWithContext` / `MultiWithContext` | 已有 `FChooserEvaluationContext` |
| `EvaluateStructChooserTyped`（CustomThunk） | 按引脚结构体类型拷贝到 OutResult；供 K2 |
| `EvaluateStructChooserTypedMulti` | 数组版 Typed |
| `EvaluateStructChooserTypedWithContext` | Context + Typed |

Typed 路径：结果 ScriptStruct 须 `IsChildOf` 目标属性结构体，否则 Clear。

## 蓝图节点 UK2Node_EvaluateStructChooser

- 菜单分类：`Struct Chooser`
- 属性：`Chooser`（`UStructChooserTable`）、`Mode`（FirstResult / AllResults）
- 引脚：Exec、`ContextObject`、`Result`（`PC_Struct`，类型=`OutputStructType`；All 时为 Array）
- 订阅：`OnOutputStructTypeChanged`、`OnContextClassChanged` → Reconstruct
- Expand：调用 Typed / TypedMulti Library 函数，并把 `ChooserTable` 默认设为节点上的资产

**禁止**：对 StructChooser 资产使用引擎 `Evaluate Chooser` / `EvaluateChooser2` 指望拿到结构体主结果。

## Context 参数

当前 K2 节点仅提供单个 `ContextObject` 便捷入口（构造 `FChooserEvaluationContext`）。多参数 / 结构体 Context 输入可走 Library 的 `WithContext` 变体，或后续扩展节点（见债务）。

## 待充实

- 完整对齐引擎 EvaluateChooser2 的多 Object/Struct Context 引脚生成
