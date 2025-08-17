# M5Dial BLE Mouse

M5DialをBluetooth Low Energy（BLE）マウスとして使用するためのArduinoスケッチです。

## 機能

- **ドラッグ操作** → マウスカーソル移動
- **タップ** → 左クリック
- **ダブルタップ** → 右クリック
- **エンコーダー回転** → 縦スクロール（実装予定）

## 必要なライブラリ

- [M5Dial](https://github.com/m5stack/M5Dial) 
- [ESP32-NimBLE-Mouse](https://github.com/wakwak-koba/ESP32-NimBLE-Mouse)

## 動作環境

- M5Dial
- Arduino IDE
- ESP32 Board Package

## 使用方法

1. Arduino IDEでこのスケッチを開く
2. M5Dialに書き込み
3. デバイス（Mac/Windows/Linux）のBluetooth設定で「M5Dial BLE Mouse」をペアリング
4. マウスとして動作開始

## 設定パラメータ

```cpp
static constexpr float MOUSE_GAIN = 2.5f;      // マウス感度
static constexpr int DEADZONE = 1;             // デッドゾーン
static constexpr int TAP_MAX_MOVE = 6;         // タップ判定移動量
static constexpr int TAP_MAX_MS = 220;         // タップ判定時間
static constexpr uint32_t DOUBLE_TAP_MS = 250; // ダブルタップ判定時間
```

## トラブルシューティング

### 接続できない場合
- デバイスのBluetooth設定から一度削除してペアリングし直す
- M5Dialを再起動する

### 動作が不安定な場合
- シリアルモニタ（115200bps）でデバッグ情報を確認

## 開発

### ビルド
```bash
# Arduino IDEまたはPlatformIOでコンパイル
```

### 開発状況
- [x] 基本マウス移動
- [x] 左クリック（タップ）
- [x] 右クリック（ダブルタップ）
- [ ] エンコーダースクロール（調整中）
- [ ] 感度設定UI

## ライセンス

MIT License

## 貢献

Issue、Pull Requestを歓迎します。