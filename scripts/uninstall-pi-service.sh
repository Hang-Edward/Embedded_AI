#!/usr/bin/env bash
set -euo pipefail

SERVICE_NAME="embedded-ai.service"
SERVICE_FILE="${HOME}/.config/systemd/user/${SERVICE_NAME}"

systemctl --user stop "${SERVICE_NAME}" >/dev/null 2>&1 || true
systemctl --user disable "${SERVICE_NAME}" >/dev/null 2>&1 || true

if [[ -f "${SERVICE_FILE}" ]]; then
    rm -f "${SERVICE_FILE}"
fi

systemctl --user daemon-reload

echo "Uninstalled ${SERVICE_NAME}."
