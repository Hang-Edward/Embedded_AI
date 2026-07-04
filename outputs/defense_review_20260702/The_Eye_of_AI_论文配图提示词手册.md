# The Eye of AI 论文配图提示词手册

适用对象：

- 结题报告《The Eye of AI》修改与补图
- 答辩 PPT《The Eye of AI》修改与补图
- 供外部高质量生图模型直接使用

使用建议：

1. 优先使用支持复杂构图、版式控制、英文短标签、学术图风格的高质量模型。
2. 每张图尽量先生成 `16:9` 或 `4:3` 横版主图，再按页面需要裁切。
3. 图内文字一律使用**简洁英文短标签**，长解释放在报告或 PPT 的 caption / bullet 中。
4. 若模型对文字不稳定，可先生成“结构正确、文字极少”的版本，再用后期工具补英文标签。

---

## 一、全局统一风格总提示词

下面这段可以作为所有图片的“公共前缀提示词”：

### 公共正向提示词

```text
Create a publication-quality academic figure for a thesis and defense presentation.
Style: professional paper figure, structured layout, restrained low-saturation palette, deep blue + cyan + gray-white with small gold accents, clear reading path, precise alignment, balanced spacing, caption-friendly composition, clean geometric blocks, thin arrows, short formal English labels only, subtle depth but no poster style, high information density with order.
The figure should look suitable for an academic PDF, a graduation thesis, and a technical defense slide.
Avoid clutter. Avoid decorative marketing aesthetics. Avoid cartoon style. Avoid oversized icons. Avoid glowing commercial poster effects. Avoid long paragraphs inside the figure.
```

### 公共负向提示词

```text
No marketing poster, no social-media infographic style, no childish illustration, no bright oversaturated colors, no exaggerated glow, no chaotic layout, no text overflow, no long paragraphs, no dense decorative icons, no low-quality SVG look, no random background objects, no watermark, no misspelled large titles.
```

---

## 二、核心必做图片（7 张）

这 7 张是当前两份意见文档里最核心、最值得优先生成的图。

---

## Figure A：AI 演化时间线总览图

### 用途

- 结题报告：标题页后、摘要后、1.1 背景与动机开头
- PPT：封面后、Background & Motivation 页

### 想表达的核心意思

把项目的根叙事讲清楚：  
AI 从 IDE 补全工具，进化到 web chatbot，再进化到 desktop agent，但它始终被困在屏幕里；The Eye of AI 为它搭建了通往现实世界的感知桥梁。

### 推荐版式

- 横向时间线
- 从左到右四阶段
- 最后一阶段视觉权重最大

### 必须出现的视觉元素

- Stage 1: IDE Completion
- Stage 2: Web Chatbot
- Stage 3: Desktop Agent
- Stage 4: Eye of AI
- 最后一阶段出现 camera, microphone, physical scene, Raspberry Pi / embedded feedback abstraction
- 一条 secondary flow 表示 perception -> reasoning -> feedback

### 图内文字建议

- `IDE Completion`
- `Web Chatbot`
- `Desktop Agent`
- `Eye of AI`
- `Perception`
- `Reasoning`
- `Feedback`
- `Physical Reality`

### 完整正向提示词

```text
Create a publication-quality academic timeline figure about the evolution of AI toward The Eye of AI.
Show four stages from left to right: IDE Completion, Web Chatbot, Desktop Agent, Eye of AI.
The purpose is to explain that AI evolved from code assistance to conversational systems to desktop agents, but remained screen-bound until The Eye of AI connected it to physical reality.
Layout: horizontal four-stage timeline, strong left-to-right reading path, each stage represented by a clean structured module with very short English labels.
The first three stages should feel progressively more capable but still confined to the digital screen world.
The final stage, Eye of AI, should visually connect a camera, microphone, physical scene, and embedded feedback devices to an AI reasoning core, emphasizing the bridge from screen-bound intelligence to the real world.
Add a subtle lower-level flow showing perception -> reasoning -> feedback.
Style: professional thesis figure, clean geometry, restrained deep blue and cyan palette, gray-white background structure, small gold highlights, thin arrows, strong spacing discipline, minimal but meaningful iconography, elegant academic composition, not a poster.
Use concise English labels only. No long text paragraphs inside the figure.
```

### 负向提示词

```text
No marketing poster, no cyberpunk style, no giant glowing brain, no sci-fi city background, no long sentences, no decorative badge icons, no neon overload, no crowded timeline, no cartoon people, no random laptop ads.
```

### 建议比例

- 首选：`16:9`
- 备选：`4:3`

---

## Figure B：AI 与现实世界桥梁意义图

### 用途

- 结题报告：1.1 / 1.2 / UI 相关说明页
- PPT：封面页、动机页、系统意义页

### 想表达的核心意思

项目不是单纯调用 API，而是在“AI 系统”和“现实物理世界”之间建立桥梁。

### 推荐版式

- 左右对照 + 中间桥梁
- 左边是 digital AI world
- 右边是 physical world
- 中间是 Eye of AI bridge

### 必须出现的视觉元素

- 左侧：code window, chat window, file operations, browser abstraction
- 右侧：desk scene, paper, object, person view, voice, hardware feedback
- 中间：bridge / gateway / perception interface

### 图内文字建议

- `Digital AI Space`
- `Physical Reality`
- `The Eye of AI`
- `Vision`
- `Voice`
- `Actionable Feedback`

### 完整正向提示词

```text
Create a professional academic concept figure showing the core significance of The Eye of AI as a bridge between digital AI systems and the physical world.
Composition: left side represents screen-bound AI capabilities such as code assistance, web chat, file operations, and desktop agents; right side represents physical reality including a desk scene, paper documents, real objects, human voice, and hardware feedback devices.
In the center, place a clean structured bridge module labeled The Eye of AI, connecting the two worlds through vision, voice, and actionable feedback.
The image should clearly communicate that the project is not merely another API application, but a prototype bridge enabling AI to perceive and interact with reality.
Style: publication-quality academic illustration, symmetric or near-symmetric composition, low-saturation blue and cyan palette, subtle gold accent for the bridge, strong information hierarchy, simple geometry, thin connectors, clean labels, elegant and serious.
Only short English labels. No long explanatory text inside the image.
```

### 负向提示词

```text
No fantasy bridge in landscape, no dramatic sci-fi portal, no poster slogan, no flashy 3D gaming scene, no cartoon face, no text-heavy infographic blocks, no messy collage.
```

### 建议比例

- 首选：`16:9`

---

## Figure C：原型机 vs 成品智能眼镜 边界澄清图

### 用途

- 结题报告：硬件设计节、Figure 4 附近、章节小结
- PPT：硬件组成页、prototype clarification 页

### 想表达的核心意思

我们做的是**功能原型验证**，不是工业成品智能眼镜。

### 推荐版式

- 左右双栏对照
- 左侧：current prototype
- 右侧：future product vision

### 必须出现的视觉元素

- 左侧：Raspberry Pi, NUCLEO, webcam, LCD, buttons, wires, LED feedback
- 右侧：sleek smart glasses concept
- 中间或底部：shared capability logic

### 图内文字建议

- `Current Prototype`
- `Validated Functions`
- `Future Smart-Glasses Form`
- `Same Interaction Logic`
- `Not Final Product`

### 完整正向提示词

```text
Create a publication-style comparison figure that clearly distinguishes the current The Eye of AI prototype from a future smart-glasses product form.
Layout: two-column comparison.
Left column shows the current reproducible functional prototype with Raspberry Pi, NUCLEO, webcam, LCD screen, buttons, LED feedback, wiring, and modular hardware arrangement.
Right column shows a clean conceptual future smart-glasses form factor, more integrated and wearable.
The central message is that the current work validates smart-glasses-like multimodal interaction functions, but does not claim a finished industrial wearable product.
Add a subtle shared capability layer or connector indicating that both sides share the same interaction logic: perception, reasoning, and feedback.
Style: professional thesis comparison figure, clean structured blocks, restrained colors, deep blue / cyan / gray-white, subtle gold emphasis for validated functions, neat labels, high clarity, no marketing glamour.
Only concise English labels.
```

### 负向提示词

```text
No product advertisement look, no Apple commercial aesthetic, no fashion poster, no exaggerated futuristic holograms, no cluttered circuit mess, no long paragraphs.
```

### 建议比例

- 首选：`16:9`
- 备选：`3:2`

---

## Figure D：系统闭环架构总图

### 用途

- 结题报告：1.2、3.1、3.2
- PPT：系统目标页、总体设计页、架构页

### 想表达的核心意思

把整个系统闭环讲清楚：  
输入感知 -> Qwen 感知层 -> DeepSeek 推理层 -> Raspberry Pi / NUCLEO / 输出反馈 -> 用户再次触发。

### 推荐版式

- 标准系统框图
- 上层软件 / 中层设备协调 / 下层硬件反馈
- 左到右主流向，局部回环

### 必须出现的视觉元素

- Camera input
- Microphone / voice input
- Qwen perception layer
- DeepSeek reasoning layer
- Raspberry Pi control node
- NUCLEO hardware node
- LCD / LED / button / keypad / feedback devices
- Audit log / history / desktop control center
- User trigger loop

### 图内文字建议

- `Camera`
- `Voice`
- `Qwen Perception`
- `DeepSeek Reasoning`
- `Raspberry Pi`
- `NUCLEO`
- `LCD`
- `LED`
- `Control Center`
- `Audit Log`
- `User Trigger`

### 完整正向提示词

```text
Create a professional academic system block diagram for The Eye of AI prototype.
Purpose: show the full closed-loop architecture from multimodal sensing to reasoning to embedded feedback and user re-trigger.
Structure: left-to-right main flow with a clear system boundary and layered grouping.
Include camera input and voice input on the left.
Then a perception layer labeled Qwen Perception for scene understanding and speech understanding.
Then a reasoning layer labeled DeepSeek Reasoning for final answer composition and decision making.
Then a Raspberry Pi control node coordinating runtime logic and hardware communication.
Then a NUCLEO node controlling physical trigger and feedback devices.
Output devices should include LCD, LED status light, keypad or button trigger, and local feedback outputs.
Also show a desktop control center and audit log / conversation history as a supervision surface.
Add a feedback arrow showing that the user can trigger another cycle after receiving the response.
Style: publication-quality architecture figure, strong alignment, grouped layers, thin arrows, dashed boundaries where necessary, restrained deep blue / cyan / gray-white palette with a few gold highlights, concise English labels only, serious academic tone.
```

### 负向提示词

```text
No glossy product poster, no random 3D render clutter, no giant decorative AI eye, no excessive icons, no rainbow colors, no dense paragraph labels, no chaotic arrow crossings.
```

### 建议比例

- 首选：`16:9`

---

## Figure E：单次工作流 + fallback 流程图

### 用途

- 结题报告：2.1、3.3.2、附录 quick reference
- PPT：Demo flow、Appendix

### 想表达的核心意思

展示一次真实交互流程，以及语音失败时系统如何自动退化到“直接看图描述”。

### 推荐版式

- 主流程纵向或横向展开
- 关键 fallback 分支单独强调

### 必须出现的视觉元素

- User trigger
- 5-second voice recording
- ASR by Qwen
- If ASR success -> reasoning
- If ASR fail or empty -> direct scene analysis
- Image capture
- Qwen visual understanding
- DeepSeek final answer
- LCD / desktop / LED feedback

### 图内文字建议

- `Trigger`
- `Record Voice`
- `Qwen ASR`
- `ASR Failed`
- `Capture Scene`
- `Qwen Vision`
- `DeepSeek Answer`
- `Fallback`

### 完整正向提示词

```text
Create a publication-style workflow figure for a single interaction cycle in The Eye of AI prototype, including fallback behavior.
Purpose: explain how the system handles a normal user interaction and how it degrades gracefully when speech recognition fails.
Layout: clean workflow diagram with a primary path and one emphasized fallback branch.
Main path: user trigger -> 5-second voice recording -> Qwen ASR -> scene capture -> Qwen vision understanding -> DeepSeek final reasoning and answer -> LCD / desktop feedback -> ready for next trigger.
Fallback branch: if ASR is empty or fails, the system skips text understanding and performs direct scene analysis before DeepSeek generates the final response.
Use thin arrows, consistent boxes, and one clearly marked fallback path.
Style: publication-quality academic flowchart, low-saturation blue / cyan palette, small warm-color emphasis on the fallback branch, strong readability, short English labels only, no clutter.
```

### 负向提示词

```text
No business process poster, no emoji icons, no bright warning stickers everywhere, no long explanations inside boxes, no tangled flow arrows.
```

### 建议比例

- 首选：`16:9`
- 备选：`4:3`

---

## Figure F：prototype boundary / limitations map

### 用途

- 结题报告：5.3 Limitations
- PPT：Limitations / Reflection 页

### 想表达的核心意思

把“限制”讲成合理的 prototype boundary，而不是项目缺陷堆砌。

### 推荐版式

- 中心是 current validated scope
- 外围是 not yet included / future expansion

### 必须出现的视觉元素

- Center: current validated prototype
- Around it: no industrial miniaturization, no standalone wearable power system, no fully independent mobile deployment, no mass-production shell
- Future rings: wearable integration, onboard networking, miniaturized optics, integrated energy management

### 图内文字建议

- `Validated Prototype Scope`
- `Not Yet Integrated`
- `Future Expansion`
- `Wearable Integration`
- `Power Miniaturization`
- `Standalone Deployment`

### 完整正向提示词

```text
Create a professional academic boundary map figure for The Eye of AI prototype.
Purpose: explain current project limitations as a reasonable prototype boundary rather than as isolated weaknesses.
Composition: center-focused scope map with a clearly defined validated prototype region in the middle and outer regions showing what is intentionally not yet integrated.
The center should represent the validated scope: multimodal sensing, reasoning, desktop supervision, embedded trigger, and local feedback.
Outer zones should represent future but currently absent capabilities: industrial miniaturization, standalone wearable power, fully independent networking, product-grade shell integration, lightweight optics integration, and mass-produced ergonomic packaging.
The figure should visually communicate that the current work is a functional prototype bridge, not a finished consumer smart-glasses product.
Style: publication-quality academic scope diagram, calm low-saturation palette, clear region boundaries, thin lines, concise English labels, strong hierarchy, no dramatic illustration.
```

### 负向提示词

```text
No failure-warning poster, no giant red cross marks, no chaotic radial labels, no comic style, no corporate infographic clichés.
```

### 建议比例

- 首选：`4:3`
- 备选：`1:1`

---

## Figure G：未来工作路线图

### 用途

- 结题报告：第 4 章或 6.2 节
- PPT：Conclusion / Outlook

### 想表达的核心意思

把项目从当前原型，自然推到后续可扩展方向，形成答辩收束。

### 推荐版式

- 三阶段 roadmap
- Now -> Next -> Future

### 必须出现的视觉元素

- Current prototype
- Better embedded feedback
- richer scene reasoning
- wearable integration
- independent networking
- product miniaturization
- embodied AI future direction

### 图内文字建议

- `Now`
- `Next`
- `Future`
- `Prototype Bridge`
- `Richer Feedback`
- `Wearable Integration`
- `Autonomous Deployment`
- `Embodied AI`

### 完整正向提示词

```text
Create a publication-quality future roadmap figure for The Eye of AI project.
Purpose: show how the current prototype can evolve into a richer embodied AI system in future work.
Structure: a clean three-stage roadmap from left to right or from near to far: Now, Next, Future.
Now should show the current prototype bridge with multimodal sensing, reasoning, LCD and embedded feedback.
Next should show improved interaction quality such as richer feedback, stronger scene reasoning, and better integration of local hardware modules.
Future should show wearable integration, onboard networking, miniaturization, more independent deployment, and a clearer embodied AI direction.
The roadmap should feel technically credible and academically aspirational, not like a startup pitch deck.
Style: professional thesis roadmap figure, restrained colors, clean blocks or stepping layers, thin arrows, subtle depth, concise English labels, strong hierarchy.
```

### 负向提示词

```text
No startup growth chart cliché, no flashy rocket icons, no corporate timeline poster, no motivational slogan graphics, no exaggerated sci-fi world.
```

### 建议比例

- 首选：`16:9`

---

## 三、建议额外补充图片（至少再做 5 张）

下面这些图不是原来 7 张核心图，但非常适合把两份文档变得更丰富、不单调。

---

## Figure H：桌面 Agent 控制中心在整个系统中的角色图

### 用途

- 结题报告：UI 说明节
- PPT：Desktop control center / supervision page

### 核心意思

把 Windows 控制中心从“调试工具”抬升成“监督界面 / explanatory surface / operator console”。

### 完整正向提示词

```text
Create a professional academic UI-role figure showing the Windows desktop control center as the supervision and explanation surface of The Eye of AI system.
The figure should not look like a product screenshot poster.
Instead, show a structured relationship diagram: physical sensing and hardware loop on one side, AI perception and reasoning in the center, and the desktop control center on the other side acting as monitoring, conversation history, diagnostics, and demonstration surface.
Emphasize that the desktop UI is part of the prototype validation and system observability, not merely a debug window.
Style: publication-quality system-role diagram, clean grouped blocks, restrained deep blue palette, concise English labels such as Monitoring, Diagnostics, Conversation, History, Hardware Status, Supervision Surface.
```

### 建议比例

- `16:9`

---

## Figure I：演示证据三联图

### 用途

- 结题报告：测试结果页
- PPT：Demo evidence / validation

### 核心意思

用一张整洁的三联图展示：

- 左：硬件原型
- 中：现实场景采集
- 右：桌面 Agent 回复与状态反馈

### 完整正向提示词

```text
Create a publication-quality triptych evidence figure for The Eye of AI prototype demonstration.
Panel (a): hardware prototype setup with Raspberry Pi, NUCLEO, display, trigger controls, and feedback modules.
Panel (b): a real physical scene being captured by the camera, such as desk objects, paper, or a simple question sheet.
Panel (c): desktop agent response and system feedback surface, showing that perception and reasoning results are visible and auditable.
The goal is to make the demo evidence look organized, credible, and paper-ready.
Use three aligned panels with small labels (a), (b), (c), and concise English captions inside or below each panel.
Style: academic multi-panel figure, clean spacing, restrained palette, subtle annotation arrows if necessary, not a marketing collage.
```

### 建议比例

- `16:9`

---

## Figure J：硬件信号分组 / pin mapping 解释图

### 用途

- 结题报告：接线图附近
- PPT：硬件实现细节页

### 核心意思

不是简单照片，而是把接线逻辑讲清楚：  
谁负责供电、谁负责信号、谁负责触发、谁负责显示。

### 完整正向提示词

```text
Create a professional academic hardware signal grouping diagram for The Eye of AI prototype.
Purpose: explain the logic of the hardware wiring and pin mapping without becoming a raw engineering schematic.
Show Raspberry Pi, NUCLEO, LCD, keypad trigger, LED indicator, and camera-related modules with grouped signal categories such as power, trigger, display, status feedback, and control communication.
Use color-coded but restrained signal classes and clearly separated grouped arrows.
The figure should help a reader understand why the wiring is organized this way, not just where each wire goes.
Style: publication-quality hardware explanation figure, layered but clean, thin connectors, calm color discipline, short English labels, precise grouping boxes, suitable for a thesis.
```

### 建议比例

- `4:3`

---

## Figure K：验证矩阵 / test coverage 图

### 用途

- 结题报告：5.1 / 5.2
- PPT：Testing & validation

### 核心意思

把测试项从“表格 pass/pass”升级成更有工程味的验证矩阵。

### 完整正向提示词

```text
Create a publication-style validation matrix figure for The Eye of AI prototype.
Rows represent test dimensions such as camera capture, voice trigger, ASR, vision understanding, reasoning response, LCD feedback, hardware trigger, desktop supervision, history logging, and fallback behavior.
Columns represent validation categories such as function correctness, response visibility, closed-loop integrity, fault tolerance, and demonstration readiness.
Use a clean matrix layout with restrained semantic colors to indicate covered and validated items.
The goal is to make the system validation look structured and engineering-oriented.
Style: professional thesis matrix figure, neat grid, concise English labels, low-saturation colors, no decorative clutter.
```

### 建议比例

- `4:3`

---

## Figure L：AI 之眼场景化概念图

### 用途

- 结题报告：摘要前后、结尾升华
- PPT：封底、愿景页

### 核心意思

一张更有感染力但仍然学术克制的概念图，展示“AI 透过原型机看见现实”的意象。

### 完整正向提示词

```text
Create a high-end academic concept illustration for The Eye of AI.
The image should symbolize an AI system perceiving the physical world through a prototype sensory bridge.
Show a real desk or learning environment, a camera-based perception path, and a subtle abstract AI cognition core connected to embedded feedback outputs.
The figure should feel like a conceptual thesis illustration rather than a commercial poster.
It should communicate: AI is no longer trapped inside software interfaces; it now has a bridge to reality.
Use disciplined composition, elegant dark blue and cyan palette, subtle gold highlights, realistic physical context, and minimal short English annotations if needed.
No exaggerated cyberpunk style.
```

### 建议比例

- `16:9`

---

## 四、如果你要直接丢给别的 AI，推荐这样发

你可以把每张图按下面格式喂给别的模型：

```text
任务：生成论文配图
图名：Figure D – Closed-loop Architecture
用途：用于结题报告第 3 章和答辩PPT架构页
风格要求：
[粘贴“公共正向提示词”]

本图专属提示词：
[粘贴该图“完整正向提示词”]

负向约束：
[粘贴“公共负向提示词” + 该图专属负向提示词]

输出要求：
- 横版 16:9
- 图内只保留简洁英文短标签
- 不要长段落
- 适合论文PDF插图
```

---

## 五、优先级建议

如果你时间不够，先生成：

1. Figure D：系统闭环架构总图
2. Figure A：AI 演化时间线
3. Figure B：桥梁意义图
4. Figure E：单次流程 + fallback
5. Figure C：原型机 vs 成品眼镜

如果你想让 PPT 更惊艳，再补：

6. Figure L：AI 之眼场景化概念图
7. Figure G：未来路线图
8. Figure I：演示证据三联图

---

## 六、备注

- 如果外部模型生成文字能力差，就让它先只生成“结构正确、文字极少”的版本。
- 对于架构图、流程图、矩阵图，最重要的是结构、分组、箭头和留白，不是花哨。
- 这套提示词已经统一围绕项目的核心意义：**AI 经过长期发展已经能写代码、聊天、操作桌面，但仍被困在电脑里；The Eye of AI 为 AI 搭起了通向现实世界的桥梁。**
