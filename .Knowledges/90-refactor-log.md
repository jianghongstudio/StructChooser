# 改造决策日志

> **角色**：按时间记录改造动机、方案、影响面与废弃项；是过程真相源。
> **何时阅读**：了解「为什么现在是这样」；每次实质性改造结束后必须追加。
> **相关源码**：随条目变化
> **相关文档**：[60-known-debt.md](60-known-debt.md)、[README.md](../README.md)、[91-ai-maintenance.md](91-ai-maintenance.md)
> **最后更新**：2026-07-30

## 当前阶段

| 字段 | 值 |
|------|----|
| 阶段 | StructChooser 插件可用 + 知识库框架已搭建 |
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

### 2026-07-30 — 修复启动崩溃：本模块 Register FChooserTableEditorCommands

- **动机**：`RegisterStructChooserTableToolbar` 调 `FChooserTableEditorCommands::Get()` 时 `Instance` 无效（`SharedPointer::IsValid` 断言）。
- **方案**：头文件 `TCommands` 按 DLL 各有静态实例；在 `StructChooserEditorModule::StartupModule` 对本模块再 `Register()`，`Shutdown` 时 `Unregister()`。
- **影响面**：StructChooserEditor 启动
- **文档同步**：`60-known-debt.md`（D1 残留说明）
- **关联债务**：D1 / 跨 DLL TCommands

### 2026-07-30 — 将 main 自建编辑器移植到 Dev_5.7（UE5.7）

- **动机**：主干（UE5.8）已用专用 `FStructChooserTableEditor` 结案 D1；Dev_5.7 需同等方案且适配当前引擎。
- **方案**：从 `origin/main` 检出 TableEditor / Menus / Toolbar / Hidden 类型；补 shim（`IChooserTableViewModel` 等）；Runtime 保留 5.7 `EIteratorStatus` / TRACE / `SetDebugSelectedRow`；修 Nested 删除 API、`GetObjectsWithOuter`、`DeleteRows(uint32)`、Style 查找、Factory `CurrentVersion`。
- **影响面**：StructChooser + StructChooserEditor（Dev_5.7）
- **废弃 / 迁移**：完整重启编辑器；Struct 表走自建编辑器打开
- **文档同步**：`40-editor-tooling.md`、本日志、`60-known-debt.md`
- **关联债务**：D1 在 Dev_5.7 同样结案；新增 D7（5.7 shim 维护）

### 2026-07-30 — 自建 StructChooser 表编辑器（1:1 镜像 + Struct-only Add Row）

- **动机**：引擎 Add Row 无 per-table 钩子；Hidden+右键（Dev_5.7）与 Slate 劫持均不满足「StructChooser 有、官方无、不改引擎」。
- **方案**：镜像引擎 Private 表编辑栈到 `Private/TableEditor/`（`FStructChooserTableEditor` 等）；AssetDefinition 直接打开自建编辑器；Add Row 仅 Struct 三项；独立 `StructChooserTableToolbar`；删除 `StructChooserAddRowPatch`；类型保持 Hidden；Rewind 共用引擎 Track + 保留 TRACE/Debug 消费。
- **影响面**：StructChooserEditor；官方 Chooser 编辑器不变。
- **废弃 / 迁移**：完整重启编辑器；勿再转发 `UChooserTable` AssetDefinition 打开 Struct 表。
- **文档同步**：本日志；`40-editor-tooling.md`；`60-known-debt.md`（D1 结案）
- **关联债务**：D1 结案

### 2026-07-30 — Hidden 隔离 + StructChooser 专用 Add Row 注入【已撤销】

- **动机**：曾尝试劫持引擎 Add Row 菜单。
- **方案**：`StructChooserAddRowPatch` — **已被自建编辑器取代并删除**。
- **关联债务**：D1

### 2026-07-30 — Add Row 恢复 Struct 选项（去掉三行类型 Hidden）【已撤销】

- **动机**：`Meta=(Hidden)` 后引擎 Add Row 跳过 Struct 类型。
- **方案**：曾去掉 Hidden —— **已撤销**（污染官方不可接受；现用自建编辑器）。
- **关联债务**：D1

### 2026-07-30 — 参考 Dev_5.7：Hidden 隔离 + 撤销 Crash Guard（UE5.8 main）

- **动机**：Crash Guard 破坏官方 Nested Edit；需适配 main。
- **方案**：三行类型 Hidden；删除 Crash Guard；右键菜单；Details Result Type。
- **废弃 / 迁移**：Add Row UX 由同日自建编辑器结案（见上）
- **文档同步**：本日志；`40-editor-tooling.md`
- **关联债务**：对齐 Dev_5.7 D1/D6 结论

### 2026-07-27 — Rewind Debugger 接入 TRACE_CHOOSER_EVALUATION（自 Dev_5.7 移植）

- **动机**：`EvaluateStructChooser` 功能正常，但未 emit `ChooserChannel`，Rewind Debugger「Chooser Evaluation」轨道为空。
- **方案**：对齐当前引擎 `UChooserTable::EvaluateChooser`：Stop / Fallback / Continue(`IndicesOut`) 调用 `TRACE_CHOOSER_EVALUATION`；Continue 路径补 `SetDebugSelectedRows`。
- **影响面**：`StructChooserTable.cpp` Runtime 评估；编辑器 Trace / Rewind Debugger
- **废弃 / 迁移**：无；Live Coding / 重编后重新录制即可
- **文档同步**：`11-evaluation.md`、`00-routing.md`、本日志
- **关联债务**：无

### 2026-07-26 — 插件侧 Details 过滤 + 误选纠正（不改引擎）

- **动机**：不改引擎时 Add Row/Details 混排 Object 类型，易配错表。
- **方案**：
  - `FStructChooserRowDetails` 覆盖 `"ChooserRowDetails"` layout，Struct 表 Result `BaseStruct`→`StructChooserBase`
  - `SanitizeInvalidStructResults` 在 PostEdit / PostTransacted 将非法 Object 行重置为 `FStructValueChooser` 并通知
- **影响面**：StructChooser + StructChooserEditor
- **废弃 / 迁移**：无
- **文档同步**：`40`/`60`
- **关联债务**：缓解 D1（Add Row/单元格仍混排）

### 2026-07-26 — 修复误选 Object 结果类型崩溃

- **动机**：单元格选 Asset 等时，同步 Sanitize 改写内存，引擎 Asset 控件仍解引用 → 崩溃。
- **方案**：覆盖 Object 结果控件；在守卫里 `ReplaceInvalidResultAt` 后立即创建 Struct 控件（无需延后一帧）；`PostTransacted` 同步兜底 Add Row。
- **影响面**：StructChooserEditor widgets；StructChooserTable 事务钩子
- **文档同步**：`40`

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
