# 问题路由（AI 第二入口）

> **角色**：按任务 / 症状把读者导向正确知识文档，避免通读全库。
> **何时阅读**：不确定该打开哪篇 `.Knowledges` 文档时；接到改造任务后的第二步（第一步是 README 索引）。
> **相关源码**：全插件（本文件不绑定单一路径）
> **相关文档**：[README.md](../README.md)、[91-ai-maintenance.md](91-ai-maintenance.md)
> **最后更新**：2026-07-26

## 使用方式

1. 在下方「按任务」或「按症状」表中找到最接近的一行。
2. 打开对应文档；需要术语时再开 [02-glossary.md](02-glossary.md)。
3. 改代码后按 [91-ai-maintenance.md](91-ai-maintenance.md) 回写文档。

## 按任务路由

| 任务 | 优先打开 | 可能还需要 |
|------|----------|------------|
| 理解插件整体 / 划模块边界 | [01-architecture.md](01-architecture.md) | [02-glossary.md](02-glossary.md) |
| 查术语含义 | [02-glossary.md](02-glossary.md) | — |
| 改 `UStructChooserTable` / Result 行类型 | [10-asset-model.md](10-asset-model.md) | [11-evaluation.md](11-evaluation.md) |
| 改评估循环 / Nested / Evaluate 传播 | [11-evaluation.md](11-evaluation.md) | [10-asset-model.md](10-asset-model.md) |
| 改 FunctionLibrary / K2 节点 | [13-runtime-consumer.md](13-runtime-consumer.md) | [11-evaluation.md](11-evaluation.md) |
| 改工厂 / Result 单元格 / Details | [40-editor-tooling.md](40-editor-tooling.md) | 约定不改引擎 |
| 落地扩展 Checklist | [50-extension-cookbook.md](50-extension-cookbook.md) | 对应领域文档 |
| 评估改造风险、已知坑 | [60-known-debt.md](60-known-debt.md) | [90-refactor-log.md](90-refactor-log.md) |
| 记录或查阅改造决策 | [90-refactor-log.md](90-refactor-log.md) | [60-known-debt.md](60-known-debt.md) |
| 同步知识库 / 协作约定 | [91-ai-maintenance.md](91-ai-maintenance.md) | [README.md](../README.md) |

## 按症状路由

| 症状 / 现象 | 优先打开 | 备注 |
|-------------|----------|------|
| 引擎 Evaluate Chooser 拿不到结构体 | [13-runtime-consumer.md](13-runtime-consumer.md)、[60-known-debt.md](60-known-debt.md) | 必须用 Evaluate Struct Chooser；`ChooseObject` 恒空 |
| Nested / Evaluate 子表结果不对 | [11-evaluation.md](11-evaluation.md)、[10-asset-model.md](10-asset-model.md) | 核对 `OutputStructType` 是否一致 |
| Add Row / 单元格仍出现 Asset 等 | [40-editor-tooling.md](40-editor-tooling.md)、[60-known-debt.md](60-known-debt.md) | D1 残留；误选会被 PostEdit 纠正；Details Result 已过滤 |
| Result 空名字看起来像已填类型名 | [40-editor-tooling.md](40-editor-tooling.md) | Hint 应为 `Enter name...`，类型名在右侧弱化 |
| Cook / 运行时没有行结果 | [11-evaluation.md](11-evaluation.md)、[10-asset-model.md](10-asset-model.md) | Editor 用 `ResultsStructs`；Cooked 用 `CookedResults` |
| 校验报非 StructChooser 行类型 | [10-asset-model.md](10-asset-model.md) | 行必须是 `FStructChooserBase` 派生 |
| Shipping 拉不到 Uncooked 模块 | [01-architecture.md](01-architecture.md) | `StructChooserUncooked` 仅 UncookedOnly |

## 模块 → 文档速查

| 源码模块 | 默认文档 |
|----------|----------|
| `Source/StructChooser/` | [10-asset-model.md](10-asset-model.md)、[11-evaluation.md](11-evaluation.md)、[13-runtime-consumer.md](13-runtime-consumer.md) |
| `Source/StructChooserEditor/` | [40-editor-tooling.md](40-editor-tooling.md) |
| `Source/StructChooserUncooked/` | [13-runtime-consumer.md](13-runtime-consumer.md) |
