# Repository Guidelines

## Project Structure & Module Organization
- `M5Dial_BLE_Mouse.ino`: Main sketch. Handles touch input, encoder scroll, rotation UI, and BLE mouse (HID) actions.
- `.vscode/`: Editor settings and build hints. Optional but recommended.
- `README.md`: Usage and device notes. Keep in sync with code changes.
- Add new modules as `.h/.cpp` only if logic grows; otherwise keep the sketch cohesive and simple.

## Build, Test, and Development Commands
- Arduino IDE: Open `.ino` → select your board (e.g., M5Dial/ESP32-S3) and port → Upload.
- Arduino CLI (examples):
  - List boards/ports: `arduino-cli board list`
  - Compile: `arduino-cli compile --fqbn <fqbn> .`
  - Upload: `arduino-cli upload -p <port> --fqbn <fqbn> .`
  - Serial monitor: `arduino-cli monitor -p <port> -c baudrate=115200`
- Typical cores/libs: ESP32 core and M5Stack + BleMouse libraries installed via Library Manager or CLI.

## Arduino CLI ワークフロー
- セットアップ（初回のみ）:
  - 追加URL登録: `arduino-cli config init && arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  - ESP32コア: `arduino-cli core update-index && arduino-cli core install esp32:esp32`
  - ライブラリ: `arduino-cli lib install "M5Dial"`、`arduino-cli lib install "ESP32-NimBLE-Mouse"`（見つからない場合はGitHubから手動導入）
- FQBN/ポートの決定:
  - ポート確認: `arduino-cli board list`
  - 本プロジェクト推奨 FQBN: `m5stack:esp32:m5dial`（M5Stack ボードパッケージ使用）。
  - 既定の接続ポート（例）: `/dev/cu.usbmodem31101`。
- 便利な環境変数例:
  - `export FQBN=m5stack:esp32:m5dial`
  - `export PORT=/dev/cu.usbmodem31101`
- ビルド/書き込み/モニタ:
  - コンパイル: `arduino-cli compile --fqbn "$FQBN" .`
  - アップロード: `arduino-cli upload -p "$PORT" --fqbn "$FQBN" .`
  - シリアル: `arduino-cli monitor -p "$PORT" -c baudrate=115200`

## Coding Style & Naming Conventions
- Language: Arduino C++ (ESP32).
- Indentation: 2 spaces; no tabs; wrap ~100 cols.
- Naming: `PascalCase` for types (e.g., `TouchState`), `lowerCamelCase` for variables/functions, `UPPER_SNAKE_CASE` for constants.
- Comments/logs: concise; keep `Serial` messages actionable. Match existing tone (JP/EN both OK).

## Testing Guidelines
- No unit test harness; rely on device testing.
- Smoke test: pair BLE, verify cursor move on drag, left/right tap behavior, encoder scroll, rotation setup (long‑press), and on‑screen status.
- Capture edge cases (debounce, dead‑zones, rotation) and note exact steps in the PR.

## Commit & Pull Request Guidelines
- Commits: imperative, concise, reference issues when relevant. Examples: `feat: implement 90° rotation`, `fix: stabilize encoder reads`.
- Branches: `feature/<topic>`, `fix/<topic>`.
- PRs must include: purpose/summary, what changed and why, board/FQBN and port used, test steps and results, screenshots/video for UI, and any regressions considered.

## コミュニケーションと言語
- 既定のコミュニケーション言語は日本語です（英語も可）。
- Issue/PR/レビューコメントは簡潔な日本語で。必要に応じて英語併記OK。
- 変更点の要約・再現手順・`Serial` ログは箇条書きで短く明確に。
- コードの識別子や API 名は英語、UI/表示文言は現状の混在（JP/EN）を維持。

## Security & Configuration Tips
- BLE HID sends input to the host. Test on a non‑critical machine first.
- Keep display brightness reasonable and retain the short loop delay to avoid excessive power/CPU usage.
