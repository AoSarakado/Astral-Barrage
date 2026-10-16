# Agent 角色身份牌

本目录是项目 Agent 角色定义的版本化来源。Git 提交保存本目录当时的内容，因此提交中的 `Agent-Card` 路径可定位到对应版本的身份牌。角色 ID 表示本次工作的职责，不代表个人身份或 Git 账户。

| 角色 ID | 身份牌 | 主要职责 |
| --- | --- | --- |
| `coordinator` | [coordinator.md](coordinator.md) | 分析目标、拆分任务、维护进度与交付边界 |
| `engineer` | [engineer.md](engineer.md) | 实现 C++/CMake 改动并验证运行结果 |
| `reviewer` | [reviewer.md](reviewer.md) | 独立检查方案、代码、测试证据与风险 |

开始任务时，在 `Task.md` 记录实际参与的角色 ID 和职责。一个 Agent 可以在不同任务中承担不同角色，但同一项输出必须标明负责角色；不要用角色 ID 冒充 Git 作者身份。新增角色时，先新增唯一 ID 的身份牌并更新本索引。
