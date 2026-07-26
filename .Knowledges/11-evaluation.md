# 评估循环与 Nested / Evaluate

> **角色**：`EvaluateStructChooser` 行为权威说明。
> **何时阅读**：改过滤顺序、输出列时机、嵌套传播或调试选中行时。
> **相关源码**：`Source/StructChooser/Private/StructChooserTable.cpp`、`StructChooserTypes.cpp`
> **相关文档**：[10-asset-model.md](10-asset-model.md)、[13-runtime-consumer.md](13-runtime-consumer.md)
> **最后更新**：2026-07-26（UE5.7：`EIteratorStatus` 无 Failed）

## 入口 API

| API | 行为 |
|-----|------|
| `EvaluateStructChooser(Context, Table, Callback)` | 多结果迭代；Callback 收 `const FInstancedStruct&` |
| `EvaluateStructChooserFirst(...)` | 取第一个成功结果后 Stop |
| `EvaluateStructChooserAll(...)` | 收集全部 Continue 结果 |

对照引擎 `UChooserTable::EvaluateChooser`，本路径在命中行后调用 `FStructChooserBase::ChooseMultiStruct`，而不是 `ChooseMulti(UObject*)`。

## `EIteratorStatus`（UE5.7）

引擎枚举仅为 `{ Continue, ContinueWithOutputs, Stop }`（**无** `Failed`）。本插件约定：

| 返回值 | 含义 |
|--------|------|
| `Continue` | 未命中 / 无有效结果 |
| `ContinueWithOutputs` | 命中且产生结果（含 Multi 下 Callback 原返回 `Continue` 时的提升） |
| `Stop` | 命中且终止迭代 |

`ChooseMultiStruct` 在 Callback 返回 `Continue` 时提升为 `ContinueWithOutputs`，以便 Multi 模式仍被父表计为成功、避免误走 Fallback。

## 评估步骤（摘要）

1. 空表 → `Continue`。
2. `VALIDATE_CHOOSER_CONTEXT`；Editor 下 `UpdateDebugging`。
3. 结果数组：`CookedResults`，或 Editor 未 Cook 时用 `ResultsStructs`。
4. 禁用行跳过；逐列 `Filter`（含 ScratchArea / Cost 排序，对齐引擎）。
5. 对每个候选行：先 `SetOutputs`（Context），再 `ChooseMultiStruct`。
6. Callback / 行返回 `Stop` → 结束；全部为 `Continue` → 尝试 `FallbackResult`（Struct 版）并写 Fallback 列输出。
7. 函数返回：有命中 → `ContinueWithOutputs`，否则 → `Continue`。

## 行级传播

| 行类型 | ChooseMultiStruct |
|--------|-------------------|
| `FStructValueChooser` | Callback(`Value`)；Value 无效则 `Continue`；Callback=`Continue` 时提升为 `ContinueWithOutputs` |
| `FEvaluateStructChooser` | `EvaluateStructChooser(Context, Chooser, Callback)` |
| `FNestedStructChooser` | 同上，指向嵌入表 |

子表 `OutputStructType` 不一致时，校验阶段应失败；运行时若绕过校验则可能拷贝失败（Library Typed 路径会 Clear）。

## 与引擎 Object 评估的关系

对同一张 `UStructChooserTable` 调用父类 `EvaluateChooser`：行上 `ChooseObject` 为 `nullptr` → 得不到主结果（自动化测试覆盖此断言）。Context Output 列是否仍被写入取决于是否走过 Struct 评估路径的 SetOutputs；**不要**依赖 Object 路径消费本表。

## 调试

- Editor：`SetDebugSelectedRow` 在 Stop / Fallback 时设置。
- `GetDebugName`：`FStructValueChooser` 优先 `Name`，否则结构体类型名。

## 自动化测试

- `StructChooser.Evaluate.NestedAndDirect`
- `StructChooser.Evaluate.EvaluateStructChooserRef`

路径：`Source/StructChooser/Private/StructChooserTests.cpp`。

## 待充实

- Multi 模式下 Output 列多输出数组与引擎对齐细节
