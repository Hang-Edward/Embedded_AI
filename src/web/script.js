const state = {
  temperature: 36.8,
  humidity: 42,
  vibration: 0.18,
  current: 0.72,
  approvedPlan: null,
  emergency: false
};

const elements = {
  tempValue: document.getElementById("tempValue"),
  humidityValue: document.getElementById("humidityValue"),
  vibrationValue: document.getElementById("vibrationValue"),
  currentValue: document.getElementById("currentValue"),
  tempMeter: document.getElementById("tempMeter"),
  humidityMeter: document.getElementById("humidityMeter"),
  vibrationMeter: document.getElementById("vibrationMeter"),
  currentMeter: document.getElementById("currentMeter"),
  refreshTime: document.getElementById("refreshTime"),
  commandInput: document.getElementById("commandInput"),
  parseBtn: document.getElementById("parseBtn"),
  approveBtn: document.getElementById("approveBtn"),
  runDemoBtn: document.getElementById("runDemoBtn"),
  emergencyBtn: document.getElementById("emergencyBtn"),
  planList: document.getElementById("planList"),
  riskBadge: document.getElementById("riskBadge"),
  serialOutput: document.getElementById("serialOutput"),
  insightText: document.getElementById("insightText"),
  deviceState: document.getElementById("deviceState")
};

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function updateTelemetry(next = {}) {
  state.temperature = clamp(next.temperature ?? state.temperature + (Math.random() - 0.48) * 1.6, 20, 80);
  state.humidity = clamp(next.humidity ?? state.humidity + (Math.random() - 0.5) * 4.5, 20, 90);
  state.vibration = clamp(next.vibration ?? state.vibration + (Math.random() - 0.46) * 0.12, 0, 2);
  state.current = clamp(next.current ?? state.current + (Math.random() - 0.48) * 0.18, 0, 5);

  elements.tempValue.textContent = `${state.temperature.toFixed(1)}°C`;
  elements.humidityValue.textContent = `${Math.round(state.humidity)}%`;
  elements.vibrationValue.textContent = `${state.vibration.toFixed(2)}g`;
  elements.currentValue.textContent = `${state.current.toFixed(2)}A`;

  elements.tempMeter.value = state.temperature;
  elements.humidityMeter.value = state.humidity;
  elements.vibrationMeter.value = state.vibration;
  elements.currentMeter.value = state.current;
  elements.refreshTime.textContent = `Refreshed: ${new Date().toLocaleTimeString("zh-CN", { hour12: false })}`;
}

function inferPlan(text) {
  const normalized = text.toLowerCase();
  const actions = [];
  let risk = "safe";
  let riskText = "低风险";

  if (text.includes("温度") || normalized.includes("temperature")) {
    actions.push("读取 temperature 传感器，并设置 45°C 预警阈值。");
    actions.push("当温度超过阈值时，发送 FAN:ON 并记录散热事件。");
  }

  if (text.includes("湿度") || normalized.includes("humidity")) {
    actions.push("读取 humidity 传感器，并在湿度过高时触发 DEHUMIDIFIER:ON。");
  }

  if (text.includes("震动") || text.includes("振动") || normalized.includes("vibration")) {
    actions.push("检测 vibration 数据，超过 1.2g 时降低电机转速。");
    risk = "warn";
    riskText = "中风险";
  }

  if (text.includes("电机") || normalized.includes("motor")) {
    actions.push("监测 motor current，超过 2.5A 时发送 MOTOR:SPEED=45。");
    risk = "warn";
    riskText = "中风险";
  }

  if (text.includes("停机") || text.includes("立刻") || normalized.includes("stop")) {
    actions.push("检测连续异常次数，达到 3 次后进入人工确认停机流程。");
    risk = "dangerous";
    riskText = "高风险";
  }

  if (actions.length === 0) {
    actions.push("建立 5 秒采样周期，持续读取温度、湿度、震动和电机电流。");
    actions.push("生成异常摘要，但不执行任何硬件动作。");
  }

  actions.push("C++ SafetyGate 检查设备上限、动作权限和人工确认要求。");
  actions.push("执行后写入 audit.log，保留时间戳、传感器帧和串口命令。");

  return { actions, risk, riskText };
}

function renderPlan(plan) {
  elements.planList.innerHTML = "";
  plan.actions.forEach((item) => {
    const li = document.createElement("li");
    li.textContent = item;
    elements.planList.appendChild(li);
  });

  elements.riskBadge.textContent = plan.riskText;
  elements.riskBadge.className = `badge ${plan.risk}`;
  elements.approveBtn.disabled = false;
  state.approvedPlan = plan;
}

function buildSerialCommands(plan) {
  const commands = [
    "[cpp] SensorFrame frame = serial.readFrame();",
    "[cpp] SafetyGate::validate(plan, frame);"
  ];

  if (plan.actions.some((item) => item.includes("FAN:ON"))) {
    commands.push("[serial] TX -> FAN:ON");
  }
  if (plan.actions.some((item) => item.includes("DEHUMIDIFIER:ON"))) {
    commands.push("[serial] TX -> DEHUMIDIFIER:ON");
  }
  if (plan.actions.some((item) => item.includes("MOTOR:SPEED"))) {
    commands.push("[serial] TX -> MOTOR:SPEED=45");
  }
  if (plan.risk === "dangerous") {
    commands.push("[cpp] high risk action detected, require manual confirmation before STOP");
  }

  commands.push("[audit] append audit.log with checksum and timestamp");
  return commands.join("\n");
}

function approvePlan() {
  if (!state.approvedPlan) return;

  elements.serialOutput.textContent = buildSerialCommands(state.approvedPlan);
  elements.insightText.textContent =
    "诊断结论：AI 已经把自然语言转换为结构化控制计划，但最终执行权在 C++层。" +
    "这种架构解决了嵌入式行业里的核心痛点：AI 能理解复杂意图，却不能直接、不受控地操作真实设备。" +
    "C++提供实时性、确定性、安全边界和可审计日志，因此系统比普通聊天式 AI 更可信。";
}

function runScenario() {
  updateTelemetry({
    temperature: 48.6,
    humidity: 73,
    vibration: 1.36,
    current: 2.84
  });

  elements.commandInput.value = "温度超过 45°C 时打开风扇，湿度超过 70% 时启动除湿，震动超过 1.2g 时降低电机转速。";
  const plan = inferPlan(elements.commandInput.value);
  renderPlan(plan);
  approvePlan();
}

function emergencyStop() {
  state.emergency = true;
  elements.deviceState.textContent = "设备保护模式：等待人工复位";
  elements.serialOutput.textContent =
    "[interrupt] EMERGENCY_STOP received\n" +
    "[serial] TX -> MOTOR:STOP\n" +
    "[serial] TX -> FAN:ON\n" +
    "[audit] emergency stop recorded";
  elements.insightText.textContent =
    "紧急停机模拟：高风险动作没有交给 AI自由发挥，而是由 C++中断处理和状态机直接接管。" +
    "这正是嵌入式系统必须强调的安全边界。";
}

document.querySelectorAll("[data-prompt]").forEach((button) => {
  button.addEventListener("click", () => {
    elements.commandInput.value = button.dataset.prompt;
  });
});

elements.parseBtn.addEventListener("click", () => {
  renderPlan(inferPlan(elements.commandInput.value));
  elements.serialOutput.textContent = "[cpp] plan parsed, waiting for user approval...";
});

elements.approveBtn.addEventListener("click", approvePlan);
elements.runDemoBtn.addEventListener("click", runScenario);
elements.emergencyBtn.addEventListener("click", emergencyStop);

updateTelemetry();
setInterval(() => {
  if (!state.emergency) updateTelemetry();
}, 2400);
