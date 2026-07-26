# 扩展开发 Cookbook

> **角色**：落地扩展功能的 Checklist，指向必改文件。
> **何时阅读**：准备新行结果类型、新消费路径或新 Result 控件时。
> **相关源码**：跨 Runtime + Editor + Uncooked
> **相关文档**：[10-asset-model.md](10-asset-model.md)、[40-editor-tooling.md](40-editor-tooling.md)、[91-ai-maintenance.md](91-ai-maintenance.md)
> **最后更新**：2026-07-26

## 新增一种行结果类型

1. 在 `StructChooserTypes.h` 增加 `USTRUCT`，公有继承 `FStructChooserBase`，`Meta=(Category="StructChooser", ...)`。
2. 实现 `ChooseStruct` / `ChooseMultiStruct`（及可选 `GetDebugName`）。
3. Editor：在 `StructChooserEditorWidgets.cpp` 写控件，并 `RegisterWidgetCreator`。
4. 过滤器：现有 filter 已按 `IsChildOf(FStructChooserBase)` 放行，一般无需改；若要排除某类型再收紧 filter。
5. `IsDataValid`：若有额外约束，扩展 `ValidateStructResults`。
6. 文档：更新 [10-asset-model.md](10-asset-model.md)、[02-glossary.md](02-glossary.md)。

## 在蓝图中消费表

1. 放置 **Evaluate Struct Chooser**，指定 `UStructChooserTable`。
2. 连接 `ContextObject`（及后续扩展的 Context 引脚）。
3. 使用 `Result` 结构体引脚（类型随 `OutputStructType`）。
4. 需要多结果时把节点 Mode 设为 AllResults。
5. **不要**用引擎 Evaluate Chooser。

## 在 C++ 中消费表

```cpp
FChooserEvaluationContext Context(/* optional UObject* */);
FInstancedStruct Result;
if (UStructChooserTable::EvaluateStructChooserFirst(Context, Table, Result))
{
    // Result.Get<FMyStruct>() ...
}
```

或 `UStructChooserFunctionLibrary::EvaluateStructChooser`。

## 改 Result 列空名字 UI

1. 只改 `CreateStructValueChooserWidget`（[40-editor-tooling.md](40-editor-tooling.md)）。
2. 保持：空 Name → 明确 Hint；类型名弱化旁标；Value 仍在 Details。

## 常见踩坑

- 在引擎 `ChooserEditor` 里加过滤器挂钩 → **禁止**（约定不改引擎）；菜单混排是 D1 预期。
- Nested 新建了 `UChooserTable` 而非 `UStructChooserTable` → 校验失败 / 评估失败。
- 子表 `OutputStructType` 与父表不一致。

## 待充实

- 为 AnimGraph / ChooserPlayer 类场景增加专用 Initializer 的步骤（若需要）
