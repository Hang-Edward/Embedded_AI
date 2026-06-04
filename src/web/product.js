const productState = {
  describe: {
    status: "画面理解完成 / 场景描述",
    result: "当前画面是学习桌面：有题目纸、书本、水杯和手机。水杯靠近电子设备，建议先移到安全位置。",
    json: {
      mode: "scene_description",
      scene: "学习桌面",
      objects: ["题目纸", "书本", "水杯", "手机"],
      risk: "low",
      answer: "桌面上有学习材料和电子设备，水杯需要远离手机。"
    },
    oled: "场景: 学习桌面",
    serial: "PROJECTOR:SCENE_SUMMARY;LED:BLUE;BUZZER:OFF;VIB:OFF"
  },
  solve: {
    status: "拍题完成 / 解题提示",
    result: "识别到一张数学题纸。AI 不直接给空泛答案，而是返回步骤化提示：先列出已知条件，再写公式，最后代入计算。",
    json: {
      mode: "problem_solving",
      input: "题目纸图像",
      risk: "none",
      answer_style: "step_by_step_hint",
      answer: "先圈出关键词，再建立方程，最后检查单位和结果。"
    },
    oled: "解题: 步骤提示已投影",
    serial: "PROJECTOR:SHOW_STEPS;LED:GREEN;BUZZER:OFF;VIB:OFF"
  },
  voice: {
    status: "语音输入完成 / 多模态问答",
    result: "用户语音问题：这道题怎么解？系统把麦克风文本和摄像头画面一起发送给千问 Omni，返回简短讲解并通过投影展示。",
    json: {
      mode: "voice_question",
      speech: "这道题怎么解？",
      visual_context: "题目纸和草稿区域",
      output: ["text", "speaker"],
      answer: "我会先读题，再给你分步骤提示。"
    },
    oled: "语音: 正在讲解",
    serial: "PROJECTOR:VOICE_ANSWER;SPEAKER:ON;LED:CYAN;BUZZER:OFF"
  },
  risk: {
    status: "识别完成 / 高风险",
    result: "画面中检测到疑似热源和杂乱线缆。建议立即检查电源区域，并通过蜂鸣器和震动提醒用户。",
    json: {
      mode: "risk_detection",
      scene: "实验桌风险场景",
      objects: ["热源", "线缆", "电子模块"],
      risk: "high",
      short_hint: "疑似热源靠近线缆"
    },
    oled: "警告: 检查电源区域",
    serial: "PROJECTOR:WARNING;LED:RED;BUZZER:ON;VIB:ON"
  }
};

const productElements = {
  scanSceneBtn: document.getElementById("scanSceneBtn"),
  solveProblemBtn: document.getElementById("solveProblemBtn"),
  voiceAskBtn: document.getElementById("voiceAskBtn"),
  riskSceneBtn: document.getElementById("riskSceneBtn"),
  aiStatus: document.getElementById("aiStatus"),
  aiResult: document.getElementById("aiResult"),
  jsonResult: document.getElementById("jsonResult"),
  oledText: document.getElementById("oledText"),
  ledStatus: document.getElementById("ledStatus"),
  buzzerStatus: document.getElementById("buzzerStatus"),
  vibrationStatus: document.getElementById("vibrationStatus"),
  serialBridge: document.getElementById("serialBridge"),
  prototypeState: document.getElementById("prototypeState"),
  cameraView: document.getElementById("cameraView")
};

function renderProductScene(type) {
  const scene = productState[type];
  const isRisk = type === "risk";

  productElements.aiStatus.textContent = scene.status;
  productElements.aiResult.textContent = scene.result;
  productElements.jsonResult.textContent = JSON.stringify(scene.json, null, 2);
  productElements.oledText.textContent = scene.oled;
  productElements.serialBridge.textContent =
    `[cpp] OpenCV captured scene.jpg\n` +
    `[cpp] microphone question buffered\n` +
    `[qwen] multimodal result parsed\n` +
    `[serial] TX -> ${scene.serial}`;
  productElements.prototypeState.textContent = isRisk ? "Alert / 已触发风险反馈" : "Active / 已完成多模态处理";

  productElements.ledStatus.className = isRisk ? "feedback-led red" : "feedback-led blue";
  productElements.buzzerStatus.className = isRisk ? "buzzer active" : "buzzer";
  productElements.vibrationStatus.className = isRisk ? "vibration active" : "vibration";
  productElements.cameraView.className = isRisk ? "camera-view risk-mode" : "camera-view";
}

productElements.scanSceneBtn.addEventListener("click", () => renderProductScene("describe"));
productElements.solveProblemBtn.addEventListener("click", () => renderProductScene("solve"));
productElements.voiceAskBtn.addEventListener("click", () => renderProductScene("voice"));
productElements.riskSceneBtn.addEventListener("click", () => renderProductScene("risk"));
