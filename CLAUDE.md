# M5Dial BLE Mouse - Claude Code Project

## プロジェクト概要

M5DialをBluetooth Low Energy（BLE）マウスとして使用するためのArduinoプロジェクト。タッチ操作によるマウス制御を実現。

## 技術スタック

- **ハードウェア**: M5Dial (ESP32-S3)
- **開発環境**: Arduino IDE
- **ライブラリ**: 
  - M5Dial
  - ESP32-NimBLE-Mouse
- **言語**: C++ (Arduino)

## 実装済み機能

- ✅ タッチドラッグ → マウスカーソル移動
- ✅ タップ → 左クリック
- ✅ ダブルタップ → 右クリック
- ✅ 安定したBLE接続
- ✅ 設定可能な感度調整
- ✅ エンコーダー回転 → 縦スクロール（安全実装済み）

## アーキテクチャ

```
M5Dial_BLE_Mouse.ino
├── TouchState struct      # タッチ状態管理
├── EncoderState struct    # エンコーダー状態管理
├── 設定定数              # 感度、タイミング調整
├── setup()               # 初期化処理
└── loop()                # メインループ
    ├── タッチ処理
    ├── BLE通信
    └── エンコーダー処理（無効化中）
```

## 重要な設定値

- `MOUSE_GAIN`: 2.5f（マウス感度）
- `TAP_MAX_MS`: 220ms（タップ判定時間）
- `DOUBLE_TAP_MS`: 250ms（ダブルタップ間隔）

## 解決済み問題

1. **エンコーダークラッシュ**: `M5Dial.Encoder.read()`でメモリ違反
   - ✅ `readEncoder()`関数で異常値チェック実装
   - ✅ 安全な初期化処理
   - ✅ クラッシュ耐性確認済み

## 開発メモ

- BLE接続は安定
- タッチ処理はpress/releaseではなくclick()を使用
- delay(5ms)で高FPS化済み
- Macのトラックパッドとの競合回避済み

## テスト環境

- M5Dial (ESP32-S3搭載)
- macOS (MacBook Air)
- Arduino IDE 2.x
- esp32 by Espressif Systems (M5Dial用)