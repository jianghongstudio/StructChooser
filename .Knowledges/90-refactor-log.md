# 改造决策日志

> **角色**：按时间记录改造动机、方案、影响面与废弃项；是过程真相源。
> **何时阅读**：了解「为什么现在是这样」；每次实质性改造结束后必须追加。
> **相关源码**：随条目变化
> **相关文档**：[60-known-debt.md](60-known-debt.md)、[README.md](../README.md)、[91-ai-maintenance.md](91-ai-maintenance.md)
> **最后更新**：2026-07-26

## 当前阶段

| 字段 | 值 |
|------|----|
| 阶段 | StructChooser 适配 UE5.7.4 可编译 + 功能可用 |
| 基线日期 | 2026-07-26 |
| 技术债索引 | [60-known-debt.md](60-known-debt.md) |

同步更新 [README.md](../README.md)「改造状态」表。

## 日志格式（后续条目请遵守）

```markdown
### YYYY-MM-DD — 简短标题

- **动机**：为什么改
- **方案**：做了什么（要点，可链 PR/提交）
- **影响面**：模块 / 资产 / 兼容性
- **废弃 / 迁移**：旧 API、旧术语、需用户操作
- **文档同步**：更新了哪些 `.Knowledges` / README
- **关联债务**：D1…（若关闭或缓解）
```

## 条目

### 2026-07-26 — 适配 UE5.7.4 Chooser API（不改引擎）

- **动机**：插件按 UE5.8 Chooser API 编写，在当前引擎 5.7.4 下无法编译。
- **方案**：
  - `EIteratorStatus::Failed` → `Continue` / 成功用 `ContinueWithOutputs`；Callback 的 `Continue` 提升为 `ContinueWithOutputs` 以保 Multi/Fallback 语义
  - `FStructChooserInitializer`：去掉 `OverrideClass` / `InitializeSignature`，改为 `Initialize(UChooserTable*)`，并 `Meta=(Hidden)`（引擎工厂无 OverrideClass）
  - EditorWidgets：不 include Private `ChooserEditorStyle.h`，改 `FSlateStyleRegistry`；Nested Widget 对齐五参数 `FChooserWidgetCreator`；去掉 `IChooserTableWidgetInterface`
  - Factory：去掉不存在的 `UChooserTable::CurrentVersion`
- **影响面**：StructChooser / StructChooserEditor Runtime+Editor；行为对齐原 Failed 语义
- **废弃 / 迁移**：勿从引擎「Chooser Table」创建对话框选 Struct（已 Hidden）；请用 Content Browser → Struct Chooser Table
- **文档同步**：`11`/`40`/`60`/本条目；README 改造状态
- **关联债务**：新增 D5（引擎创建对话框无法 OverrideClass）

### 2026-07-26 — 插件侧 Details 过滤 + 误选纠正（不改引擎）

- **动机**：不改引擎时 Add Row/Details 混排 Object 类型，易配错表。
- **方案**：
  - `FStructChooserRowDetails` 覆盖 `"ChooserRowDetails"` layout，Struct 表 Result `BaseStruct`→`StructChooserBase`
  - `SanitizeInvalidStructResults` 在 PostEdit / PostTransacted 将非法 Object 行重置为 `FStructValueChooser` 并通知
- **影响面**：StructChooser + StructChooserEditor
- **废弃 / 迁移**：无
- **文档同步**：`40`/`60`
- **关联债务**：缓解 D1（Add Row/单元格仍混排）

### 2026-07-26 — 撤回引擎 ChooserEditor ResultTypeFilter

- **动机**：项目约定「不改引擎」；此前为过滤 Add Row/Details 菜单改了 `ChooserEditor`，违反约定。
- **方案**：引擎侧 `git revert`；插件去掉 `RegisterResultTypeFilter` 调用；菜单混排记为 D1。
- **影响面**：UnrealEngine ChooserEditor；StructChooserEditor 模块启动
- **文档同步**：`40`/`60`
- **关联债务**：D1 重写为不改引擎约束下的菜单混排

### 2026-07-26 — Table Settings 隐藏 ResultType / ResultClass

- **动机**：StructChooser 只使用 `OutputStructType`；Details 上 Object Result Type/Class 易误导。
- **方案**：`FStructChooserDetails` 隐藏 `ResultType`、`OutputObjectType`，并隐藏 Hidden 分类默认属性。
- **影响面**：StructChooserEditor Details
- **废弃 / 迁移**：无；字段仍序列化占位
- **文档同步**：`40`/`60`
- **关联债务**：缓解 D2

### 2026-07-26 — 搭建 AnimTable 风格知识库

- **动机**：与 `UGame/Plugins/AnimTable` 对齐 AI/协作文档结构，降低后续改造上下文成本。
- **方案**：新增 `README.md` + `.Knowledges/`（`00`/`01`/`02`/`10`/`11`/`13`/`40`/`50`/`60`/`90`/`91`）。
- **影响面**：仅文档
- **废弃 / 迁移**：无
- **文档同步**：本条目；README 索引
- **关联债务**：—

### 2026-07-26 — Result 列空名字显示优化

- **动机**：空 `Name` 时 Hint 使用结构体类型名，易被当成已填写内容。
- **方案**：`CreateStructValueChooserWidget` 改为 Hint=`Enter name...` + 右侧弱化类型名。
- **影响面**：StructChooserEditor UI
- **废弃 / 迁移**：无
- **文档同步**：`40`/`60`（缓解 D0）
- **关联债务**：缓解 D0

### 2026-07-26 — Add Row / 类型下拉过滤（已撤回）

- **动机**：Struct 表菜单混入引擎 Asset / Evaluate Chooser / Proxy 等项。
- **方案**：曾改引擎 `RegisterResultTypeFilter`；因违反「不改引擎」已 revert（见上条）。
- **影响面**：无（已撤回）
- **关联债务**：D1

### 2026-07-26 — 初版 StructChooser 插件（UChooserTable 子类方案）

- **动机**：NoPrimaryResult 无法 Nested/Evaluate；需要结构体主结果且可嵌套，且不整体改引擎主结果模型。
- **方案**：
  - `UStructChooserTable` + `FStructChooserBase` 系行类型 + `EvaluateStructChooser`
  - Editor 复用引擎表编辑器（AssetDefinition 转发）
  - `UK2Node_EvaluateStructChooser` + FunctionLibrary
  - 自动化测试 Nested / Evaluate 引用
- **影响面**：新插件；引擎 Chooser 仅后续挂钩
- **废弃 / 迁移**：无旧资产
- **文档同步**：知识库初建时补录
- **关联债务**：D2/D3/D4 基线
