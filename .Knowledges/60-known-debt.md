# 已知坑与技术债

> **角色**：记录已识别的风险与债务；解决后迁移到 [90-refactor-log.md](90-refactor-log.md)。
> **何时阅读**：规划改造、排查诡异行为、评估「能不能动这块」时。
> **相关源码**：全插件
> **相关文档**：[90-refactor-log.md](90-refactor-log.md)、[01-architecture.md](01-architecture.md)
> **最后更新**：2026-07-30（D1：自建编辑器结案）

## 基线快照

- **日期**：2026-07-26
- **状态**：插件功能可用；下列条目来自实现审阅，非完整审计。

## 债务清单

### D1 — Add Row 菜单无 per-table 钩子（已结案：自建编辑器）

- **位置**：曾依赖引擎 `SChooserCreateRowButton::MakeCreateResultMenu`
- **结案（2026-07-30）**：`UStructChooserTable` 改走 `FStructChooserTableEditor`；自有 Add Row 仅 Struct 三项；三行类型 `Meta=(Hidden)` 隔离官方。
- **残留**：自建编辑器与引擎在极少数辅助功能上可能仍有细微差异；升级引擎时需 diff 镜像源。
- **约束**：不改引擎 Chooser 源码。

### D2 — 父类 ObjectResult 占位语义易误导（已缓解）

- **位置**：`ApplyStructChooserDefaults`；Details 曾暴露 `ResultType` / `OutputObjectType`
- **问题**：资产上曾显示 Result Type = Object Of Type；新手可能改回 Class/NoPrimary 破坏 UI。
- **缓解**：`FStructChooserDetails` 隐藏 `ResultType` 与 `OutputObjectType`（2026-07-26）；运行时仍内部占位。
- **残留**：属性仍存在于序列化数据中，仅 UI 隐藏。

### D3 — K2 节点 Context 能力弱于引擎 EvaluateChooser2

- **位置**：`UK2Node_EvaluateStructChooser`
- **问题**：仅 `ContextObject`；多参数 / 结构体 Context 需手写 Library WithContext。
- **影响**：复杂 Chooser 参数表在 BP 侧体验不如引擎节点。

### D4 — 引擎 Object 评估路径静默失败

- **位置**：`FStructChooserBase::ChooseObject` → nullptr
- **问题**：误用引擎 Evaluate Chooser 不会崩溃，但永远无主结果，易误判「表配错了」。
- **建议方向**：Editor 校验 / 节点 Pin 类型限制拒绝 `UStructChooserTable`；或日志警告。

## 已缓解（保留痕迹）

### D0 — Result 空名字 Hint 使用结构体类型名

- **原问题**：Hint 显示 “Chooser Player Settings” 像已填名字。
- **缓解**：Hint 改为 `Enter name...`，类型名右侧弱化显示（2026-07-26）。
- **文档**：[40-editor-tooling.md](40-editor-tooling.md)
