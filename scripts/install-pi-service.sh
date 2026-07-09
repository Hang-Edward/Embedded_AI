#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SERVICE_NAME="embedded-ai.service"
SERVICE_DIR="${HOME}/.config/systemd/user"
SERVICE_FILE="${SERVICE_DIR}/${SERVICE_NAME}"
LOG_DIR="${PROJECT_DIR}/logs"
RUN_SCRIPT="${PROJECT_DIR}/scripts/run-pi-button-assistant.sh"
EXECUTABLE=""
for candidate in \
    "${PROJECT_DIR}/build-pi/hardware/pi_bridge/embedded_ai_pc_bridge"; do
    if [[ -x "${candidate}" ]]; then
        EXECUTABLE="${candidate}"
        break
    fi
done
CONFIG_FILE="${PROJECT_DIR}/config/qwen-vision.ini"
KEY_FILE="${PROJECT_DIR}/config/qwen-vision.key"

mkdir -p "${SERVICE_DIR}" "${LOG_DIR}"

if [[ -z "${EXECUTABLE}" ]]; then
    echo "ERROR: embedded_ai_pc_bridge executable not found." >&2
    echo "Run this first:" >&2
    echo "  cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF" >&2
    echo "  cmake --build build-pi" >&2
    exit 1
fi

if [[ ! -f "${CONFIG_FILE}" ]]; then
    echo "ERROR: missing ${CONFIG_FILE}" >&2
    echo "Copy config/qwen-vision.example.ini to config/qwen-vision.ini and configure it." >&2
    exit 1
fi

if [[ ! -f "${KEY_FILE}" ]]; then
    echo "WARNING: ${KEY_FILE} not found. The service can still start, but Qwen API calls will fail unless api_key_env is set." >&2
fi

cat > "${RUN_SCRIPT}" <<EOF
#!/usr/bin/env bash
set -euo pipefail

cd "${PROJECT_DIR}"
mkdir -p logs captures

LOG_FILE="${LOG_DIR}/embedded-ai.log"

find_serial_port() {
    if [[ -n "\${EMBEDDED_AI_PORT:-}" ]]; then
        if [[ -e "\${EMBEDDED_AI_PORT}" ]]; then
            echo "\${EMBEDDED_AI_PORT}"
            return 0
        fi
        echo "Configured EMBEDDED_AI_PORT does not exist: \${EMBEDDED_AI_PORT}" >> "\${LOG_FILE}"
    fi

    local by_id
    by_id="\$(find /dev/serial/by-id -maxdepth 1 -type l 2>/dev/null | grep -Ei 'stlink|st-link|stmicro|nucleo' | head -n 1 || true)"
    if [[ -n "\${by_id}" && -e "\${by_id}" ]]; then
        readlink -f "\${by_id}"
        return 0
    fi

    local acm
    acm="\$(find /dev -maxdepth 1 -name 'ttyACM*' 2>/dev/null | sort | head -n 1 || true)"
    if [[ -n "\${acm}" && -e "\${acm}" ]]; then
        echo "\${acm}"
        return 0
    fi

    return 1
}

echo "==================================================" >> "\${LOG_FILE}"
echo "Embedded AI service starting at \$(date --iso-8601=seconds)" >> "\${LOG_FILE}"
echo "Project: ${PROJECT_DIR}" >> "\${LOG_FILE}"

if ! PORT="\$(find_serial_port)"; then
    echo "ERROR: no NUCLEO serial port found. Checked EMBEDDED_AI_PORT, /dev/serial/by-id, and /dev/ttyACM*." >> "\${LOG_FILE}"
    exit 1
fi

echo "Port: \${PORT}" >> "\${LOG_FILE}"

exec "${EXECUTABLE}" "\${PORT}" --qwen >> "\${LOG_FILE}" 2>&1
EOF

chmod +x "${RUN_SCRIPT}"

cat > "${SERVICE_FILE}" <<EOF
[Unit]
Description=Embedded AI Reality Bridge Button Assistant
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=${PROJECT_DIR}
ExecStart=${RUN_SCRIPT}
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
EOF

systemctl --user daemon-reload
systemctl --user enable --now "${SERVICE_NAME}"

if command -v loginctl >/dev/null 2>&1; then
    loginctl enable-linger "${USER}" >/dev/null 2>&1 || true
fi

echo "Installed and started ${SERVICE_NAME}."
echo "Status:"
systemctl --user --no-pager status "${SERVICE_NAME}" || true
echo
echo "View live log:"
echo "  tail -f ${LOG_DIR}/embedded-ai.log"
echo
echo "Stop service:"
echo "  systemctl --user stop ${SERVICE_NAME}"
