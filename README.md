# M5Dial BLE Mouse

M5DialをBluetooth Low Energy（BLE）マウスとして使用するためのArduinoスケッチです。タッチスクリーンとエンコーダーを使った直感的なマウス操作を実現します。

## ✨ 機能

### 完全実装済み ✅
- **タッチ移動**: 指の移動でマウスカーソル移動（感度調整済み）
- **タップ**: 左クリック、**ダブルタップ**: 右クリック
- **長押しドラッグ**: 画面長押しでドラッグモード（左ボタン押下維持）。ドラッグ中は画面色を反転
- **エンコーダー回転**: 縦スクロール（安全実装）
- **安定したBLE接続**: 高速レスポンス／クラッシュ対策

### 今後の機能（計画中） 🛠️
- 🎛️ **設定 UI** - タッチスクリーンでの感度調整
- 🌵 **接続成功音** - Bluetooth接続時のサウンド
- 👁️ **目玉 UI** - 移動方向を示す可愛いインターフェース
- ✨ **軌跡表示** - ドラッグ軌跡のグラフィック表示

## 必要なライブラリ

- [M5Dial](https://github.com/m5stack/M5Dial) 
- [ESP32-NimBLE-Mouse](https://github.com/wakwak-koba/ESP32-NimBLE-Mouse)

## 動作環境

- **ハードウェア**: M5Dial (ESP32-S3搭載)
- **開発環境**: Arduino IDE 2.x 推奨
- **ボードパッケージ**: esp32 by Espressif Systems
- **テスト環境**: macOS (Windows/Linuxでも動作予定)

## 使用方法

1. Arduino IDEでこのスケッチを開く
2. M5Dialに書き込み
3. デバイス（Mac/Windows/Linux）のBluetooth設定で「M5Dial BLE Mouse」をペアリング
4. マウスとして動作開始

### 操作一覧（デフォルト）
- 指でスワイプ: カーソル移動
- シングルタップ: 左クリック
- ダブルタップ: 右クリック
- 指を長押し（約0.25秒）→そのまま移動: ドラッグ（左ボタン押下維持）
  - ドラッグ中: 画面色が反転（視認性重視）
- エンコーダー回転: 縦スクロール
- 本体ボタンAを長押し: 画面回転設定モード（0/90/180/270°）
  - 画面上の表記（英語・中央寄せ）: 上部に「ROTATION SETUP」、中央に角度の数字のみ
  - 設定保存後: 中央に「Saved」を短時間表示し、画面を一瞬反転

#### 画面表示メッセージ（英語・中央寄せ）
- 接続時: 「Connected!」／ヒント「Setup: long press」
- 未接続: 「Waiting...」
- 保存時: 「Saved」（短時間、反転表示）

## ⚙️ 設定パラメータ

コード内で調整可能なパラメータ:

```cpp
// 基本操作設定
static constexpr float MOUSE_GAIN = 2.5f;      // マウス感度
static constexpr int DEADZONE = 1;             // 微小移動無視闾値
static constexpr int TAP_MAX_MOVE = 6;         // タップ判定移動量[px]
static constexpr int TAP_MAX_MS = 220;         // タップ最大時間[ms]
static constexpr uint32_t DOUBLE_TAP_MS = 250; // ダブルタップ間隔[ms]

// スクロール設定
static constexpr float SCROLL_PER_CLICK = 1.0f; // エンコーダースクロール感度
static constexpr int SCROLL_DEAD = 0;          // スクロールデッドゾーン

// ドラッグ設定
static constexpr uint32_t DRAG_HOLD_MS = 250;   // ドラッグ開始の長押し時間[ms]
static constexpr int DRAG_MAX_PREMOVE = 6;      // 開始前に許容する移動量[px]
```

## トラブルシューティング

### 接続できない場合
- デバイスのBluetooth設定から一度削除してペアリングし直す
- M5Dialを再起動する

### 動作が不安定な場合
- シリアルモニタ（115200bps）でデバッグ情報を確認
  - DRAG_START/DRAG_END ログで誤検出有無を確認
  - 誤検出が多い場合は `DRAG_HOLD_MS` を増やす or `DRAG_MAX_PREMOVE` を下げる

## 開発

### ビルド
```bash
# Arduino IDEでコンパイル
```

### 📈 開発状況

**Phase 1: 基本機能** ✅ **完了**
- [x] 基本マウス移動
- [x] 左クリック（タップ）
- [x] 右クリック（ダブルタップ）
- [x] エンコーダー縦スクロール
- [x] 安定性・エラー耐性

**Phase 2: 使いやすさ向上** 🛠️
- [ ] 設定UI実装
- [ ] 接続状況表示改善

**Phase 3: 遊び心機能** 🎮
- [ ] 接続成功音（簡単・効果的）
- [ ] 目玉UI（移動方向表示）
- [ ] 軌跡表示（ドラッグ軌跡）
- [ ] 統計表示（クリック数など）

## ライセンス

MIT License
