# 编辑器工具与引擎挂钩

> **角色**：Editor 模块全景；约定**不改**引擎 Chooser 源码。
> **何时阅读**：改工厂、Result UI、Add Row、打开编辑器方式时。
> **相关源码**：`Source/StructChooserEditor/`
> **相关文档**：[10-asset-model.md](10-asset-model.md)、[60-known-debt.md](60-known-debt.md)
> **最后更新**：2026-07-26（行 Details Result 过滤 + PostEdit 误选纠正）

## 打开编辑器

`UAssetDefinition_StructChooserTable::OpenAssets` **不**直接链接 `FChooserTableEditor::CreateEditor`（该符号未导出）。改为：

```
UAssetDefinitionRegistry → UChooserTable 的 AssetDefinition → OpenAssets
```

从而复用引擎完整 Chooser 表编辑器（Result 列、列工厂、Nested 树、Debug Target）。

## 创建资产

| 入口 | 类型 |
|------|------|
| Content Browser → Struct Chooser Table | `UStructChooserTableFactory`：弹 StructViewer 选 `OutputStructType` |
| 引擎 Chooser 创建对话框（可选） | `FStructChooserInitializer`：`OverrideClass` → `UStructChooserTable` |

创建后：`ApplyStructChooserDefaults`、默认一行 `FStructValueChooser`。

## Result 单元格控件

在 `StructChooserEditorModule::StartupModule` 注册：

| 行类型 | 控件要点 |
|--------|----------|
| `FStructValueChooser` | 左：`SEditableTextBox`（`Name`，空时 Hint=`Enter name...`）；右：弱化结构体类型名 |
| `FEvaluateStructChooser` | `SObjectPropertyEntryBox`，仅 `UStructChooserTable`，按 `OutputStructType` 过滤 |
| `FNestedStructChooser` | 新建/选择嵌入 `UStructChooserTable`，同步类型与 `RootChooser` |

结构体字段值仍在 Details 中编辑（选中行）。

## 结果类型菜单（不改引擎）

| UI | 行为 |
|----|------|
| Add Row / 单元格类型下拉 | 仍可能混入引擎 Object 类型（无扩展点，见 D1） |
| 行 Details Result 下拉 | `FStructChooserRowDetails` 将 `BaseStruct` 改为 `StructChooserBase`，只显示 Struct 系类型 |
| 误选 Object 行 | 单元格控件创建时立刻 `ReplaceInvalidResultAt` 并画出 Struct 控件；`PostTransacted` 同步兜底 |
| 点 Asset 等崩溃 | 已防：不在引擎 Asset 控件存活时改内存；守卫里先改类型再建 Struct UI |

约定：Add Row 请只选 StructChooser 分类；误选会被纠正。`IsDataValid` 仍为保存校验防线。

## Table Settings Details

`FStructChooserDetails`（注册在 `UStructChooserTable`）：

- 隐藏父类占位属性 `ResultType`、`OutputObjectType`（UI 只保留 `Output Struct Type`）
- 与引擎 `FChooserDetails` 一样隐藏 Category=`Hidden` 的 Results/Columns 数组

## 模块启动

`FStructChooserEditorModule::StartupModule`：

1. `LoadModuleChecked(ChooserEditor)`（确保引擎先注册 `ChooserRowDetails` layout）
2. `RegisterStructChooserWidgets()`
3. `RegisterCustomClassLayout(StructChooserTable)` → `FStructChooserDetails`
4. `RegisterCustomClassLayout("ChooserRowDetails")` → `FStructChooserRowDetails`（覆盖引擎同名 layout）

`UChooserRowDetails` 未从 ChooserEditor 导出：用反射读写 `Chooser` / `Properties`，**不改**引擎源码。

## 待充实

- Nested 树中嵌入 Struct 表与引擎 Nested Chooser 混用时的 UX 边界说明
