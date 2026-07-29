# 编辑器工具与引擎挂钩

> **角色**：Editor 模块全景；约定**不改**引擎 Chooser 源码。
> **何时阅读**：改工厂、Result UI、Add Row、打开编辑器方式时。
> **相关源码**：`Source/StructChooserEditor/`（含 `Private/TableEditor/`）
> **相关文档**：[10-asset-model.md](10-asset-model.md)、[60-known-debt.md](60-known-debt.md)
> **最后更新**：2026-07-30（自建 1:1 表编辑器；Struct-only Add Row；Hidden 隔离官方）

## 打开编辑器

`UAssetDefinition_StructChooserTable::OpenAssets` → `FStructChooserTableEditor::CreateEditor`（**不再**转发引擎 `UChooserTable` 编辑器）。

自建栈在 `Private/TableEditor/`，由引擎 ChooserEditor Private 镜像改名而来。

## 创建资产

| 入口 | 类型 |
|------|------|
| Content Browser → Struct Chooser Table | `UStructChooserTableFactory`：弹 StructViewer 选 `OutputStructType` |

创建后：`ApplyStructChooserDefaults`、默认一行 `FStructValueChooser`。不挂引擎 Create Chooser Table 对话框（已删除 `FStructChooserInitializer`）。

## Result 单元格控件

在 `StructChooserEditorModule::StartupModule` 注册（**仅** StructChooser 行类型）：

| 行类型 | 控件要点 |
|--------|----------|
| `FStructValueChooser` | 左：`SEditableTextBox`（`Name`）；右：弱化结构体类型名 |
| `FEvaluateStructChooser` | `SObjectPropertyEntryBox`，仅 `UStructChooserTable` |
| `FNestedStructChooser` | 嵌入表 + `IChooserTableWidgetInterface::OpenObject` 下钻 |

**禁止**覆盖引擎 Nested/Asset/Evaluate Object 结果 creator。

## 结果类型菜单

三行类型 `Meta=(Hidden)` → 官方引擎 Add Row **无** Struct。

自建编辑器 `SStructChooserCreateRowButton` 使用 `MakeStructChooserCreateRowMenu`，**仅**：

- Struct / Evaluate Struct Chooser / Nested Struct Chooser（含 Fallback 子菜单）

| UI | 官方 `UChooserTable` | `UStructChooserTable` |
|----|----------------------|------------------------|
| 编辑器 | 引擎 `FChooserTableEditor` | `FStructChooserTableEditor` |
| Add Row | 无 STRUCTCHOOSER（Hidden） | 仅上述三项 |
| 工具栏 | `ChooserTableToolbar` | `StructChooserTableToolbar`（Table Settings / AutoPopulate / Debug Target） |
| Rewind 轨道 | 引擎已注册 | **共用**引擎 Track（不重注册）；依赖 `TRACE_CHOOSER_EVALUATION` + Debug 状态消费 |

## Rewind Debugger

- Runtime：`StructChooserTable.cpp` 发 `TRACE_CHOOSER_EVALUATION`
- 轨道：引擎 `FChoosersTrackCreator` / `FRewindDebuggerChooser`
- 自建 ViewModel/Table 镜像引擎 Debug 高亮（`GetDebugSelectedRows` 等）与 Debug Target 菜单

## 模块启动

1. `LoadModuleChecked(ChooserEditor)`
2. `RegisterStructChooserWidgets()`
3. `RegisterStructChooserEditorMenus()`（自建 ContextMenu）
4. `RegisterStructChooserTableToolbar()`
5. `FStructChooserTableEditor::RegisterWidgets()`（Row/Column Details layouts）
6. `FStructChooserDetails` for `UStructChooserTable`

## 待充实

- Nested 树中与引擎 Object Nested 混用边界（StructChooser 仅嵌套 Struct 表）
