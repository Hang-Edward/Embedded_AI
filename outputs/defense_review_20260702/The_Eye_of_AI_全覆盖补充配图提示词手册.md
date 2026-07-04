# The Eye of AI 全覆盖补充配图提示词手册

适用文件：

- 结题报告 `Final_Report_final.docx`
- 答辩演示 `ai_reality_bridge_english.pptx`
- 已有配图 Figure A-G 的扩展补充

目标：让结题报告的每个主要部分、答辩 PPT 的每一页，至少拥有一张与本页论点直接相关的图片。新增图片不重复堆砌“系统架构”，而是分别承担叙事、解释、证据、课程要求映射和视觉收束等功能。

---

## 一、统一生成规范

### 1. 公共风格前缀

将下面一段放在每条提示词开头，可保持整套图片风格统一：

```text
Create a publication-quality academic figure for The Eye of AI, a functional embedded AI prototype that gives screen-bound AI perception and feedback channels into physical reality. The project is a reproducible prototype, not a finished smart-glasses product. Use a disciplined technical visual language: clean geometry, strong alignment, balanced whitespace, thin connectors, concise English labels, restrained deep navy, cyan, gray-white and small warm-gold accents. The figure must be suitable for a graduation report and a technical defense presentation. Prioritize structure, evidence and readability over decoration.
```

### 2. 公共负向提示词

```text
No marketing poster, no social-media infographic, no cyberpunk neon overload, no fantasy scene, no decorative giant AI brain, no cartoon characters, no excessive icons, no glossy commercial advertisement, no long paragraphs, no text overflow, no misspelled title, no watermark, no chaotic arrows, no random circuitry, no low-quality SVG appearance.
```

### 3. 文字与后期原则

- 图内只放英文短标签，建议每个标签不超过 3-4 个词。
- 若模型生成文字不稳定，优先保证结构正确，再由 Canva、PowerPoint 或 Word 补标签。
- 报告图优先生成 `4:3` 或 `3:2`；PPT 主视觉优先生成 `16:9`。
- 同一概念在报告和 PPT 中复用时，尽量只裁切，不重新生成另一种美术风格。
- 真实硬件照片、桌面软件截图和生成式概念图应交替使用，避免整份材料只有一种图片形态。

---

## 二、已有 Figure A-G 的职责

| 编号 | 已有图片主题 | 最适合承担的论点 |
|---|---|---|
| Figure A | AI 演化时间线 | IDE Completion -> Web Chatbot -> Desktop Agent -> Eye of AI |
| Figure B | AI 与现实世界桥梁 | 项目核心意义，不只是 API wrapper |
| Figure C | 原型机与成品眼镜边界 | 当前是功能原型，不是工业产品 |
| Figure D | 系统闭环架构 | 感知、推理、控制、反馈与再次触发 |
| Figure E | 单次交互与 fallback | 语音、拍照、识别、回答及失败退化 |
| Figure F | Prototype boundary | 当前有效范围与尚未完成的产品化能力 |
| Figure G | Future roadmap | Now -> Next -> Future |

---

## 三、结题报告逐页覆盖建议

以下页码依据当前报告渲染结果整理。排版调整后页码可能变化，应以“章节内容”而不是绝对页码作为最终插入依据。

| 当前页 | 内容 | 推荐图片 | 状态 |
|---|---|---|---|
| 1 | 封面 | Figure H：AI 获得感知通道的封面主视觉 | 新增 |
| 2 | 目录 | Figure I：报告四阶段导航图 | 新增 |
| 3 | 1.1 背景与动机 | Figure A + Figure B | 已有 |
| 4 | Related Work / Core Innovations | Figure J：三项核心创新 | 新增 |
| 5 | 1.2 System Objectives | Figure K：能力目标地图 | 新增 |
| 6 | 2.1 Key Issues | Figure L：问题-方案-证据矩阵 | 新增 |
| 7 | 2.2 Duty Assignments | Figure M：团队责任泳道 | 新增 |
| 8 | 3.1 Runtime Environment | Figure N：软硬件运行栈 | 新增 |
| 9 | 3.2 General Design | Figure D + Figure O：三节点部署拓扑 | 已有 + 新增 |
| 10 | 3.3.1 OOP 起始 | Figure P：C++ 继承与多态结构 | 新增 |
| 11 | OOP / Random File Processing | Figure P + Figure Q | 新增 |
| 12 | UI and Interaction Design | Figure R：Qt 控制中心功能地图 | 新增 |
| 13 | Hardware I/O | Figure S：硬件信号拓扑 | 新增 |
| 14 | NUCLEO Pin Annotation | Figure T：针脚与信号注释图 | 新增，优先结合实拍图后期标注 |
| 15 | 4 Programming Progress | Figure U：开发里程碑时间线 | 新增 |
| 16 | 5.1 Function Testing | Figure V：测试覆盖与故障注入矩阵 | 新增 |
| 17 | 5.2 System Testing | Figure W：端到端验证故事板 | 新增 |
| 18 | 集成验证与界面证据 | Figure W + 真实硬件/软件截图 | 新增 + 实拍 |
| 19 | 5.3 Limitations | Figure F | 已有 |
| 20 | 6 Personal Summary / Future | Figure G | 已有 |
| 21 | Team Reflections | Figure M 或 Figure X | 新增 |
| 22 | References | Figure Y：研究依据版图 | 新增，可小尺寸使用 |
| 23 | Appendix / Course Checklist | Figure X：课程要求追踪矩阵 | 新增 |

---

## 四、PPT 18 页逐页覆盖建议

| PPT 页 | 页面主题 | 推荐图片 |
|---|---|---|
| 1 | Cover | Figure H，作为全幅或右侧主视觉 |
| 2 | Contents | Figure I，作为四章节导航图 |
| 3 | Background section divider | Figure A，弱化文字，强化演化方向 |
| 4 | Background & Motivation | Figure A + Figure B，二选一为主图 |
| 5 | System Goals | Figure K |
| 6 | Hardware Composition | Figure N 或 Figure S |
| 7 | Architecture section divider | Figure D 的简化裁切版 |
| 8 | Five-Layer Architecture | Figure D 或 Figure N |
| 9 | Layer Code Mapping | Figure Z：代码目录与层映射 |
| 10 | Closed-Loop Data Flow | Figure D |
| 11 | Complete Interaction Flow | Figure E |
| 12 | C++ OOP Design | Figure P |
| 13 | File I/O & Qt UI | Figure Q + Figure R 的左右双图 |
| 14 | Testing section divider | Figure V 的简化裁切版 |
| 15 | Testing Matrix & Exception Handling | Figure V + Figure W |
| 16 | Conclusion section divider | Figure G |
| 17 | Course Requirements & Team Mapping | Figure X + Figure M |
| 18 | Thank You / Outlook | Figure AA：AI 之眼收束主视觉，或 Figure G |

---

## 五、新增图片提示词 Figure H-AA

## Figure H：AI 获得“眼睛”的封面主视觉

### Figure Brief

- 用途：报告封面、PPT 第 1 页、章节收束页。
- 核心信息：AI 不再只存在于屏幕，而是通过摄像头、麦克风和嵌入式反馈感知现实。
- 构图：左侧数字空间，中央抽象“Eye of AI”感知节点，右侧真实桌面环境；用清晰光路或数据通道连接。
- 注意：这是一张概念型学术主视觉，不是系统框图，也不是智能眼镜广告。

### 最终提示词

```text
Create a refined academic cover illustration for a project titled The Eye of AI. Show screen-bound artificial intelligence on the left as a restrained digital workspace containing code, chat and file abstractions. In the center, place an elegant perception gateway shaped by a camera aperture and a subtle eye motif, labeled only “The Eye of AI”. On the right, show a realistic but clean physical study environment with paper, a desk object, a human hand, and embedded feedback devices. Connect the center gateway to camera vision, microphone voice input, and small LCD / LED feedback channels. The visual message must be that AI gains a bridge to perceive and respond to physical reality. Use a wide cinematic academic composition, deep navy and cyan palette, gray-white physical objects, small warm-gold highlights, soft controlled depth, no commercial product advertising. Keep labels minimal: “Digital AI”, “Perception Bridge”, “Physical Reality”. Leave clean negative space for a report title or PPT title. Aspect ratio 16:9.
```

### 负向提示词

```text
No smart-glasses advertisement, no giant human eye, no cyberpunk city, no humanoid robot, no neon poster, no long slogan, no excessive glow, no cluttered desk, no distorted hardware, no text paragraph.
```

---

## Figure I：报告与答辩四阶段导航图

### Figure Brief

- 用途：报告目录页、PPT 第 2 页。
- 核心信息：Background -> Architecture -> Validation -> Conclusion。
- 构图：四段连续路径，每段使用不同但统一的视觉符号。

### 最终提示词

```text
Create a publication-quality four-stage navigation figure for The Eye of AI technical report. Arrange four connected stages from left to right: “Background”, “Architecture”, “Validation”, “Conclusion”. Background should contain a subtle screen-bound AI symbol and a bridge toward reality. Architecture should show compact layers for perception, reasoning, Raspberry Pi and NUCLEO. Validation should show a checklist, camera frame and fault-status indicator. Conclusion should show a path from current prototype to future wearable integration. Use one continuous directional ribbon with four restrained panels, consistent geometry, concise labels, deep navy and cyan with small gold checkpoints, generous whitespace, suitable for a table-of-contents page. Aspect ratio 16:9.
```

### 负向提示词

```text
No colorful corporate process infographic, no oversized numbers, no stock-photo collage, no decorative arrows, no long text, no cartoon style.
```

---

## Figure J：三项核心创新结构图

### Figure Brief

- 用途：报告 1.1.2 Core Innovations、PPT 动机页补图。
- 三项创新：hybrid prototype、replaceable C++ services、hardware-triggered interaction。
- 构图：中心问题 + 三个等权创新模块 + 底部共同结果。

### 最终提示词

```text
Create a professional academic innovation map for The Eye of AI. Place the central challenge “Screen-Bound AI” at the top or center. Connect it to three equal innovation pillars: “Hybrid Prototype” showing Raspberry Pi + NUCLEO + Qt; “Replaceable C++ Services” showing abstract interfaces with real and mock implementations; “Hardware-Triggered Interaction” showing a physical three-key input and local LCD / LED feedback. Converge the three pillars into one bottom outcome labeled “Reproducible Reality Bridge”. Use a triangular or three-column structure, disciplined alignment, low-saturation navy, cyan and gray-white, small gold emphasis on the shared outcome, thin arrows and concise labels. The figure should communicate modest but concrete engineering innovation, not exaggerated scientific breakthrough. Aspect ratio 4:3 or 16:9.
```

### 负向提示词

```text
No trophy icons, no marketing claims, no “revolutionary” text, no glowing brain, no crowded benefit cards, no long descriptions.
```

---

## Figure K：系统目标与能力地图

### Figure Brief

- 用途：报告 1.2、PPT 第 5 页。
- 能力：image, voice, vision reasoning, hardware trigger, local feedback, Qt supervision, audit log。
- 构图：中心闭环，外围七项能力，不用普通“七个圆圈”营销图。

### 最终提示词

```text
Create a publication-style capability map for The Eye of AI system objectives. Use a structured central loop labeled “Perceive -> Understand -> Respond”. Around the loop, arrange seven compact capability modules with clear connectors: “Image Capture”, “Voice Input”, “Qwen Perception”, “DeepSeek Reasoning”, “Hardware Trigger”, “Local Feedback”, “Qt Supervision”, and “Audit Log”. Group image, voice and trigger as inputs; Qwen and DeepSeek as intelligence; LCD, LED, Qt and audit log as outputs and evidence. Show the system as one coherent closed loop rather than disconnected features. Use clean geometric modules, restrained navy / cyan / gray-white colors, small gold accents for course evidence, concise English labels only, strong alignment and balanced whitespace. Aspect ratio 16:9.
```

### 负向提示词

```text
No radial marketing wheel, no colorful app icons, no benefit poster, no giant central logo, no long feature sentences, no decorative gradients.
```

---

## Figure L：关键问题-工程方案-代码证据矩阵

### Figure Brief

- 用途：报告 2.1、PPT 测试或工程设计说明。
- 核心信息：每个关键问题都有对应工程方案和可核验代码证据。
- 构图：三列矩阵，六行主题，强调 traceability。

### 最终提示词

```text
Create a professional academic traceability matrix titled “Engineering Issues and Evidence” for The Eye of AI. Use three vertical columns: “Key Issue”, “Engineering Solution”, “Code Evidence”. Use six aligned rows for image capture, speech input, hardware trigger, AI analysis, history storage, and Qt display. Represent each row with a concise technical symbol and short labels, then connect the issue to its solution and evidence using thin horizontal arrows. Include brief evidence labels such as OpenCvCameraService, QwenAsrService, QwenVisionService, AuditLogStore, SerialPort, and ChatPage, but no file paths or long descriptions. Use a clean grid, white or pale-gray cells, deep navy headers, cyan solution column, small gold markers for verified evidence, publication-quality typography. Aspect ratio 16:9.
```

### 负向提示词

```text
No spreadsheet screenshot, no tiny unreadable code, no long file paths, no decorative dashboard, no red-green traffic-light overload, no dense paragraph cells.
```

---

## Figure M：五人团队责任泳道图

### Figure Brief

- 用途：报告 2.2、6.1，PPT 第 17 页。
- 核心信息：五人职责相互独立但汇入同一个演示闭环。
- 构图：五条泳道汇入 Integration & Defense。

### 最终提示词

```text
Create a clean academic team responsibility swimlane diagram for a five-member C++ project. Use five horizontal lanes labeled “Linux & Deployment”, “AI Services”, “NUCLEO Firmware”, “Qt Control Center”, and “Testing & Documentation”. In each lane, show three concise milestones or deliverables with small technical symbols, not portraits. Align the lanes along a shared development timeline and converge them into a final integration node labeled “Integrated Prototype & Defense”. Add thin dependency connectors only where necessary: AI services to Qt, NUCLEO firmware to Raspberry Pi workflow, testing across all lanes. Use restrained navy, cyan, gray-white and small gold completion markers, precise grid alignment, professional report style. Aspect ratio 16:9.
```

### 负向提示词

```text
No cartoon team members, no corporate org chart, no portraits, no motivational poster, no crowded task lists, no long sentences.
```

---

## Figure N：软硬件运行环境分层栈

### Figure Brief

- 用途：报告 3.1、PPT 第 6 或第 8 页。
- 层次：physical devices -> OS/runtime -> C++ libraries -> AI services -> user interfaces。
- 构图：垂直分层栈，左右标出 Windows / Raspberry Pi / NUCLEO 的边界。

### 最终提示词

```text
Create a publication-quality runtime environment stack for The Eye of AI. Build a vertically layered diagram with three deployment columns: “Windows PC”, “Raspberry Pi 5”, and “NUCLEO-F446RE”. At the bottom show physical devices: Logitech C270, LCD, status LED, three-key keypad. Above them show operating environments: Windows, Raspberry Pi OS / Linux, STM32 firmware. Above that show C++ technologies: Qt 6 Widgets, OpenCV, libcurl, serial communication, systemd. At the top show cloud services “Qwen ASR / VL” and “DeepSeek Reasoning”, connected only to the Raspberry Pi / control workflow. Use clear layer boundaries, thin cross-column connectors, concise labels, restrained academic palette, no decorative hardware render clutter. Aspect ratio 16:9.
```

### 负向提示词

```text
No cloud-computing marketing diagram, no random logos, no exaggerated 3D chips, no product advertisements, no long version numbers, no text overflow.
```

---

## Figure O：三节点部署与通信拓扑

### Figure Brief

- 用途：报告 3.2，PPT 架构说明。
- 三节点：Windows supervision、Raspberry Pi workflow、NUCLEO hardware control。
- 通道：SSH, USB serial, USB camera/audio, GPIO/SPI。

### 最终提示词

```text
Create a professional deployment topology diagram for The Eye of AI using three main nodes. Left or top: “Windows Control Center” for supervision, history and diagnostics. Center: “Raspberry Pi Workflow Node” for camera capture, audio recording, API calls, orchestration and local display. Right or bottom: “NUCLEO Hardware Node” for physical trigger, keypad events and low-level control. Connect Windows to Raspberry Pi with “SSH / LAN”; connect Raspberry Pi to NUCLEO with “USB Serial”; connect Logitech C270 to Raspberry Pi with “USB Video + Audio”; connect LCD and status light to Raspberry Pi with “SPI / GPIO”; connect keypad to NUCLEO with “GPIO”. Clearly distinguish data flow, control flow and status feedback using three line styles and a compact legend. Use precise academic network-diagram geometry, restrained colors and minimal symbols. Aspect ratio 16:9.
```

### 负向提示词

```text
No internet cloud map, no router advertisement, no messy cable spaghetti, no photorealistic desk scene, no excessive protocol labels, no arrow crossings.
```

---

## Figure P：C++ 继承、多态与依赖反转结构图

### Figure Brief

- 用途：报告 3.3.1、PPT 第 12 页。
- 必须证明：abstract interfaces、real/mock implementations、App 依赖抽象、动态替换。
- 构图：简化 UML，重点展示两层以上继承和多态调用。

### 最终提示词

```text
Create a publication-quality simplified UML architecture figure demonstrating C++ inheritance and polymorphism in The Eye of AI. Place “App / WorkflowManager” at the top depending on abstract interfaces, not concrete classes. In the middle, show three abstract base classes with pure virtual operations: “CameraService: capture()”, “AiVisionService: analyze()”, and “AudioRecorder: record()”. Below them, show derived implementations: OpenCvCameraService under CameraService; QwenVisionService and MockAiVisionService under AiVisionService; ShellAudioRecorder under AudioRecorder. Use standard UML inheritance arrows and dependency arrows with a small legend. Add one concise callout: “Runtime Substitution”. Keep the diagram rigorous, aligned and readable, with deep navy interface headers, cyan implementation blocks, gray-white background, small gold emphasis on polymorphic substitution. Aspect ratio 16:9.
```

### 负向提示词

```text
No pseudo-code paragraphs, no inaccurate UML arrows, no decorative class icons, no giant code snippets, no crowded method lists, no marketing infographic style.
```

---

## Figure Q：二进制随机文件读写机制图

### Figure Brief

- 用途：报告 3.3.2、PPT 第 13 页。
- 核心信息：append、calculate offset、seek、read、update、write back。
- 构图：上方操作流程，下方二进制记录块和指针定位示意。

### 最终提示词

```text
Create a professional academic figure explaining binary random-access file processing in AuditLogStore. Use two coordinated panels. Panel A shows the operation flow: “Append Record -> Calculate Offset -> seekg / seekp -> Read Target -> Update Status -> Write Back”. Panel B shows a binary file named “audit-log.dat” divided into fixed-size record blocks with IDs, task type, risk level, image path, summary and status. Highlight one selected record and show the file pointer jumping directly to its byte offset without rewriting the entire file. Include concise labels “Fixed-Size Record”, “Random Access”, and “In-Place Update”. Use a clean technical diagram, aligned blocks, thin arrows, restrained navy / cyan / gray-white palette and one gold highlight for the selected record. Aspect ratio 16:9.
```

### 负向提示词

```text
No database server illustration, no spreadsheet, no fake source code wall, no long binary strings, no complex memory addresses, no decorative folder icons.
```

---

## Figure R：Qt 控制中心功能地图

### Figure Brief

- 用途：报告 3.3.3、PPT 第 13 页。
- 页面：Live Chat, History, Diagnostics, Camera, Raw Log, Settings。
- 构图：中心 Qt shell，六个页面围绕，右侧标注对应数据源。

### 最终提示词

```text
Create a publication-quality functional map of the Qt 6 Windows Control Center for The Eye of AI. In the center, show a simplified desktop application shell labeled “Qt Control Center”. Around it, arrange six page modules: “Live Chat”, “History”, “Diagnostics”, “Camera View”, “Raw Log”, and “Settings”. Connect each page to its main data source: AI conversation, audit-log.dat, SSH / serial status, latest camera frame, runtime log, and saved configuration. Show BasePage as a shared UI foundation beneath the pages, indicating page inheritance without turning the figure into a full UML diagram. Use a calm academic interface abstraction, not a literal screenshot; deep navy shell, cyan page accents, gray-white labels, thin connectors, balanced spacing. Aspect ratio 16:9.
```

### 负向提示词

```text
No fake glossy app screenshot, no Apple advertisement, no neon glassmorphism poster, no tiny unreadable UI text, no random charts, no long menu labels.
```

---

## Figure S：硬件输入输出信号拓扑

### Figure Brief

- 用途：报告 3.3.4、PPT 第 6 页。
- 核心信息：哪些设备接 Raspberry Pi，哪些接 NUCLEO，信号如何流动。
- 构图：双控制器中心 + 输入输出左右分区。

### 最终提示词

```text
Create a professional academic hardware signal topology for The Eye of AI prototype. Use two central controller blocks: Raspberry Pi 5 and NUCLEO-F446RE. Place input devices on the left: Logitech C270 camera and microphone, NUCLEO blue button, and three-key keypad with K-A, K-B and K-C. Place output devices on the right: 1.8-inch LCD, red-yellow-green status light, Qt desktop display, and audit log. Clearly show which devices connect to Raspberry Pi and which connect to NUCLEO, with concise interface labels such as USB, USB Serial, GPIO and SPI. Use different thin line styles for media data, control events and feedback status. Keep wiring readable and reproducible, with clean orthogonal routing, restrained academic colors and no cable crossings. Aspect ratio 16:9.
```

### 负向提示词

```text
No breadboard confusion, no physically impossible cables, no random pin numbers, no decorative electronics poster, no tangled wires, no photorealistic clutter.
```

---

## Figure T：NUCLEO 针脚与三键键盘注释图

### Figure Brief

- 用途：报告 Figure 8 附近、附录装配说明。
- 最佳方案：上传真实 NUCLEO 俯视照片作为参考，让模型只做清晰标注；若无参考图，不要让模型虚构精确板级布局。
- 内容：3V3、GND、A0/A1/A2 或项目实际使用脚位，以及 K-A/K-B/K-C 对应关系。

### 最终提示词（配合真实照片编辑）

```text
Edit the supplied top-view photograph of the real NUCLEO-F446RE prototype into a publication-quality hardware annotation figure. Preserve the exact board geometry and all existing connectors. Add clean callout lines and concise labels only for the pins actually used by the three-key keypad and serial connection. Group the annotations into “Power”, “Key Inputs”, and “USB Serial”. Add a small inset showing the keypad pins K-A, K-B, K-C, VCC and GND mapped to the corresponding NUCLEO connections. Use thin cyan callout lines, navy label boxes, small gold highlights for trigger K-B, and a gray-white technical background. Do not invent hidden pins or alter the board. Keep all labels outside the board where possible. Aspect ratio 4:3.
```

### 负向提示词

```text
Do not redesign the PCB, do not invent pin numbers, do not move connectors, no cable clutter, no decorative glow, no illegible tiny labels, no long wiring instructions inside the image.
```

---

## Figure U：四阶段开发里程碑时间线

### Figure Brief

- 用途：报告第 4 章 Programming Progress。
- 四阶段：hardware bring-up、multimodal services、Qt control center、integration & testing。
- 构图：时间线 + 每阶段产物 + 一条风险/修复轨迹。

### 最终提示词

```text
Create a publication-quality development timeline for The Eye of AI. Show four sequential engineering stages: “Hardware Bring-Up”, “Multimodal Services”, “Qt Control Center”, and “Integration & Testing”. Under each stage, show two concise deliverables: serial and button verification; camera / audio / Qwen integration; chat / history / diagnostics UI; auto-start / fallback / defense rehearsal. Add a thin secondary track beneath the timeline showing representative problems and resolutions: serial detection, OpenCV configuration, API authentication, SSH service recovery. Use milestone diamonds, short labels, balanced spacing, restrained navy and cyan palette, gold completion markers, no calendar dates unless needed. Aspect ratio 16:9.
```

### 负向提示词

```text
No Gantt chart with tiny dates, no project-management dashboard, no decorative people, no long task lists, no colorful corporate timeline.
```

---

## Figure V：测试覆盖与故障注入矩阵

### Figure Brief

- 用途：报告 5.1、PPT 第 15 页。
- 行：camera, microphone/ASR, serial/keypad, API/network, LCD/LED, Qt/history。
- 列：normal, disconnected, timeout, invalid response, recovery。

### 最终提示词

```text
Create a professional academic test coverage matrix for The Eye of AI. Use rows for “Camera”, “Microphone / ASR”, “Serial / Keypad”, “API / Network”, “LCD / LED”, and “Qt / History”. Use columns for “Normal”, “Disconnected”, “Timeout”, “Invalid Response”, and “Recovery”. Fill cells with restrained symbols for passed test, fallback used, warning, or not applicable, accompanied by a compact legend. Highlight the end-to-end recovery path rather than presenting only pass marks. Add a narrow right-side conclusion column showing “Expected Behavior”. Use a clean matrix, readable labels, low-saturation blue and gray cells, cyan pass markers, warm gold fallback markers and limited muted red for faults. Aspect ratio 16:9.
```

### 负向提示词

```text
No fake numerical benchmark, no oversized green checkmarks, no marketing scorecard, no tiny unreadable cells, no random percentages, no long paragraphs.
```

---

## Figure W：端到端演示验证故事板

### Figure Brief

- 用途：报告 5.2、PPT 第 15 页。
- 核心信息：一次真实操作从按键到本地和桌面输出的完整证据链。
- 构图：六帧序列，适合将生成图替换为真实截图或实拍照片。

### 最终提示词

```text
Create a six-panel academic validation storyboard for one successful The Eye of AI interaction. Arrange panels from left to right or top to bottom: “1 Trigger”, “2 Record Voice”, “3 Capture Scene”, “4 Qwen Perception”, “5 DeepSeek Answer”, “6 LCD + Qt Feedback”. Use consistent framing and a restrained technical style. The trigger panel shows a physical three-key keypad; the recording panel shows a microphone waveform and five-second indicator; the capture panel shows a camera frame; the perception panel shows compact scene and speech tokens; the answer panel shows structured reasoning output; the feedback panel shows synchronized LCD, status light and desktop interface. Include a slim evidence strip with status transitions “Ready -> Processing -> Complete”. Make the storyboard caption-friendly and suitable for later replacement with real prototype screenshots. Aspect ratio 16:9.
```

### 负向提示词

```text
No comic-book panels, no speech bubbles, no fictional humanoid AI, no cinematic drama, no invented performance metrics, no long captions inside panels.
```

---

## Figure X：课程要求-代码-演示证据追踪图

### Figure Brief

- 用途：报告 Appendix、PPT 第 17 页。
- 课程要求：2000+ C++ lines、UI、OOP、two-level inheritance、polymorphism、random file I/O。
- 构图：左要求，中实现，右证据，形成答辩闭环。

### 最终提示词

```text
Create a publication-quality course-requirement traceability figure for The Eye of AI C++ project. Use three aligned columns: “Course Requirement”, “Implementation”, and “Defense Evidence”. Include rows for “2000+ C++ Lines”, “Qt GUI”, “Object-Oriented Design”, “Two-Level Inheritance”, “Polymorphism”, and “Random File I/O”. Map them to concise implementation labels such as Qt Widgets pages, abstract service interfaces, BasePage hierarchy, real / mock service substitution, and AuditLogStore seek-based update. In the evidence column, use short labels for code structure, runtime screenshot, history page and binary log demonstration. Use thin mapping lines, a disciplined matrix layout, dark navy headers, cyan implementation blocks, small gold evidence markers, high readability and no inflated claims. Aspect ratio 16:9.
```

### 负向提示词

```text
No certificate design, no grading badge, no arbitrary line-count number beyond the stated requirement, no tiny code screenshots, no long explanations, no celebratory poster.
```

---

## Figure Y：研究依据与技术选择版图

### Figure Brief

- 用途：报告 1.1.1 或 References 页。
- 三类依据：smart-glasses interaction、edge intelligence、vision-language models。
- 构图：三支研究脉络汇聚到 prototype design decisions。

### 最终提示词

```text
Create a professional academic literature-to-design map for The Eye of AI. Use three research streams on the left: “Smart-Glasses Interaction”, “Edge Intelligence”, and “Vision-Language Models”. Under each stream, show two short concerns: touchless multimodal interaction; latency, privacy and resource constraints; visual reasoning capability and compute cost. Connect the streams to design decisions on the right: “Desktop Prototype First”, “Cloud-Assisted Inference”, “Replaceable Service Interfaces”, and “Explicit Prototype Boundary”. Use a converging evidence-map layout with thin arrows, restrained academic colors, minimal document symbols and concise labels. The figure should show that technical choices are grounded in literature rather than selected arbitrarily. Aspect ratio 16:9 or 4:3.
```

### 负向提示词

```text
No fake citation numbers, no book-cover collage, no academic seal, no invented author names, no long bibliography text, no colorful mind map.
```

---

## Figure Z：代码目录与五层架构映射图

### Figure Brief

- 用途：PPT 第 9 页、报告 3.2 或附录。
- 核心信息：目录不是随意堆文件，而是与架构层对应。
- 构图：左侧简化目录树，右侧五层架构，中间映射线。

### 最终提示词

```text
Create a publication-quality code-structure mapping figure for The Eye of AI. Use a simplified repository tree on the left with major folders only: software/windows_control_center, hardware/pi_bridge, firmware/nucleo_pingpong, config, scripts, docs, and tests. On the right, show five architecture layers: Presentation, Application, Service, Communication, and Hardware. Draw clean mapping lines from folders and representative C++ classes to their corresponding layers. Include a small annotation “Headers / Sources Separated” and show .h / .cpp as paired document symbols. Use precise alignment, restrained navy and cyan palette, gray-white background, small gold markers for course evidence, concise English labels and no actual long file paths. Aspect ratio 16:9.
```

### 负向提示词

```text
No IDE screenshot, no full repository listing, no tiny filenames, no fake source code, no tangled mapping lines, no software marketing diagram.
```

---

## Figure AA：答辩结尾“从原型到具身 AI”收束图

### Figure Brief

- 用途：PPT 第 18 页、报告总结页。
- 核心信息：项目完成的是现实桥梁的第一步，未来是可穿戴、独立网络、端侧推理与更丰富反馈。
- 构图：当前原型在左下，桥梁式路径通向右上未来形态；中间保留 Thank You 文本空间。

### 最终提示词

```text
Create an elegant academic closing illustration for The Eye of AI defense. Show the current modular prototype at the lower left as a compact abstraction of Raspberry Pi, NUCLEO, camera, keypad, LCD and status light. A clear ascending pathway moves through “Richer Feedback”, “Onboard Networking”, “Edge Reasoning”, and “Wearable Integration”, ending in a restrained future embodied-AI symbol at the upper right. The future symbol should suggest perception and interaction without looking like a finished commercial smart-glasses advertisement. Preserve large clean negative space in the center for “Thank You” and one closing sentence. Use deep navy, cyan, gray-white and subtle gold milestones, disciplined academic composition, soft depth, no hype. Aspect ratio 16:9.
```

### 负向提示词

```text
No product launch poster, no luxury glasses advertisement, no humanoid robot, no sci-fi city, no giant slogan, no rainbow gradient, no excessive glow.
```

---

## 六、推荐生成优先级

### 第一批：直接补齐课程答辩证据

1. Figure P：C++ 继承与多态
2. Figure Q：随机文件读写
3. Figure X：课程要求追踪
4. Figure V：测试覆盖矩阵
5. Figure Z：代码目录映射

### 第二批：补齐系统工程解释

1. Figure N：运行环境栈
2. Figure O：部署通信拓扑
3. Figure S：硬件信号拓扑
4. Figure R：Qt 功能地图
5. Figure U：开发时间线

### 第三批：提升叙事和视觉完成度

1. Figure H：封面主视觉
2. Figure I：目录导航
3. Figure J：创新结构
4. Figure K：能力地图
5. Figure W：验证故事板
6. Figure AA：答辩收束图

---

## 七、最终检查清单

生成每张图后逐项检查：

- 图的主论点能否在 3 秒内看懂；
- 是否与相邻页面文字直接对应，而不是仅仅“看起来像 AI”；
- 图内是否只有短标签，没有长段落；
- 是否存在拼写错误、乱码、重复模块或错误箭头；
- 是否把当前项目正确描述为 functional prototype；
- 是否避免把 Qwen 与 DeepSeek 的职责混淆；
- 是否正确区分 Raspberry Pi 与 NUCLEO 的作用；
- 是否避免宣称尚未验证的性能数据；
- 是否与 Figure A-G 使用同一套深蓝、青色、灰白与少量金色语义；
- 放入 Word 或 PPT 后，缩小到实际尺寸仍能看清主要标签。

本手册与已有 Figure A-G 配合后，可覆盖当前报告全部主要页面和 PPT 18 页，且图片类型包含概念主视觉、架构图、UML、流程图、矩阵、时间线、拓扑图、故事板和证据追踪图，避免整套材料只出现重复的“方框加箭头”。
