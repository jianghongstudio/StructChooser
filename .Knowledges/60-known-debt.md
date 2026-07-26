# 已知坑与技术债

> **角色**：记录已识别的风险与债务；解决后迁移到 [90-refactor-log.md](90-refactor-log.md)。
> **何时阅读**：规划改造、排查诡异行为、评估「能不能动这块」时。
> **相关源码**：全插件
> **相关文档**：[90-refactor-log.md](90-refactor-log.md)、[01-architecture.md](01-architecture.md)
> **最后更新**：2026-07-26（D1：Details 过滤 + PostEdit 纠正已缓解）

## 基线快照

- **日期**：2026-07-26
- **状态**：插件功能可用；下列条目来自实现审阅，非完整审计。

## 债务清单

### D1 — Add Row / 单元格类型下拉仍混排（不改引擎；已部分缓解）

- **位置**：转发打开 `UChooserTable` 编辑器；`SChooserCreateRowButton` / 单元格 `CreateWidget`
- **问题**：不改 `ChooserEditor` 时无法过滤 Add Row / 单元格类型列表。
- **缓解**：
  - 行 Details Result：`FStructChooserRowDetails` 将 `BaseStruct` → `StructChooserBase`
  - 误选：单元格守卫立刻改回 Struct 并画 Struct UI；`PostTransacted` 同步兜底；`IsDataValid` 仍校验
- **残留**：Add Row / 单元格下拉仍可能看到 Asset 等项（点选不崩，会被纠正）。
- **建议方向（完整清菜单）**：自建 StructChooser 表编辑器。
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
