#include <M5Dial.h>
#include <BleMouse.h>

BleMouse bleMouse("M5Dial BLE Mouse");

// タッチ状態管理
struct TouchState {
  bool touching = false;
  int lastX = 0, lastY = 0;
  int startX = 0, startY = 0;
  uint32_t touchStartMs = 0;
  uint32_t lastTapEndMs = 0;
  bool lastTapValid = false;
  
  void reset() {
    touching = false;
    lastTapValid = false;
  }
};

TouchState touchState;

// エンコーダー状態管理
struct EncoderState {
  long lastCount = 0;
  void init(long initialCount) {
    lastCount = initialCount;
  }
};

EncoderState encoderState;

// 設定
static constexpr float MOUSE_GAIN = 2.5f;  // 感度アップ
static constexpr int DEADZONE = 1;
static constexpr int TAP_MAX_MOVE = 6;
static constexpr int TAP_MAX_MS = 220;
static constexpr uint32_t DOUBLE_TAP_MS = 250;
static constexpr float SCROLL_PER_CLICK = 1.0f;
static constexpr int SCROLL_DEAD = 0;  // スクロールデッドゾーン

// 安全なエンコーダー読み取り関数
long readEncoder() {
  long result = M5Dial.Encoder.read();
  
  // 異常値をチェック（ハードウェアエラー対策）
  if (result < -1000000L || result > 1000000L) {
    result = encoderState.lastCount;
  }
  
  return result;
}

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true); // エンコーダー有効化
  M5Dial.Display.setBrightness(40);
  M5Dial.Display.clear(BLACK);
  M5Dial.Display.setCursor(40, 60);
  M5Dial.Display.println("Minimal BLE Mouse");
  
  Serial.begin(115200);
  Serial.println("Starting minimal BLE Mouse...");
  
  bleMouse.begin();
  Serial.println("BLE Mouse started");
  
  // エンコーダー安全初期化
  delay(100);
  encoderState.init(readEncoder());
  Serial.println("Encoder initialized safely");
}

void loop() {
  M5Dial.update();
  
  // 接続状態確認のみ
  static bool wasConnected = false;
  bool currentlyConnected = bleMouse.isConnected();
  
  if (currentlyConnected != wasConnected) {
    Serial.print("Connection changed: ");
    Serial.println(currentlyConnected ? "CONNECTED" : "DISCONNECTED");
    wasConnected = currentlyConnected;
    
    if (currentlyConnected) {
      M5Dial.Display.clear(BLACK);
      M5Dial.Display.setCursor(40, 60);
      M5Dial.Display.println("Connected!");
      Serial.println("Basic connection test successful");
    } else {
      M5Dial.Display.clear(BLACK);
      M5Dial.Display.setCursor(40, 60);
      M5Dial.Display.println("Waiting...");
    }
  }
  
  // 接続時の処理
  if (currentlyConnected) {
    // 基本的なタッチ処理テスト
    auto t = M5Dial.Touch.getDetail();
    
    // 参考コードと同じ方式のタッチ処理
    if (t.isPressed()) {
      int x = (int)t.x, y = (int)t.y;
      
      if (!touchState.touching) {
        // タッチ開始
        touchState.touching = true;
        touchState.startX = touchState.lastX = x;
        touchState.startY = touchState.lastY = y;
        touchState.touchStartMs = millis();
        Serial.printf("Touch start at (%d,%d)\n", x, y);
      }
      
      // 相対移動計算
      int dx = x - touchState.lastX;
      int dy = y - touchState.lastY;
      
      if (abs(dx) > DEADZONE || abs(dy) > DEADZONE) {
        // マウス移動（クリックなし）
        bleMouse.move((int)(dx * MOUSE_GAIN), (int)(dy * MOUSE_GAIN));
        Serial.printf("Move: %d,%d\n", dx, dy);
        touchState.lastX = x;
        touchState.lastY = y;
      }
    } else {
      // タッチ終了
      if (touchState.touching) {
        uint32_t dur = millis() - touchState.touchStartMs;
        int move = abs(touchState.lastX - touchState.startX) + abs(touchState.lastY - touchState.startY);
        bool isTap = (dur <= TAP_MAX_MS && move <= TAP_MAX_MOVE);
        
        Serial.printf("Touch end: dur=%dms, move=%dpx, isTap=%s\n", dur, move, isTap ? "YES" : "NO");
        
        if (isTap) {
          uint32_t now = millis();
          if (touchState.lastTapValid && (now - touchState.lastTapEndMs) <= DOUBLE_TAP_MS) {
            // ダブルタップ → 右クリック
            Serial.printf("Attempting RIGHT CLICK... connected=%s\n", bleMouse.isConnected() ? "YES" : "NO");
            bleMouse.press(MOUSE_RIGHT);
            delay(50);
            bleMouse.release(MOUSE_RIGHT);
            Serial.println(">>> RIGHT CLICK!");
            touchState.lastTapValid = false;
          } else {
            // シングルタップ → 左クリック
            Serial.printf("Attempting LEFT CLICK... connected=%s\n", bleMouse.isConnected() ? "YES" : "NO");
            bleMouse.press(MOUSE_LEFT);
            delay(50);
            bleMouse.release(MOUSE_LEFT);
            Serial.println(">>> LEFT CLICK!");
            touchState.lastTapValid = true;
            touchState.lastTapEndMs = now;
          }
        } else {
          touchState.lastTapValid = false;
        }
        
        touchState.touching = false;
      }
    }
    
    // エンコーダーによる縦スクロール（安全実装）
    long encNow = readEncoder();
    long delta = encNow - encoderState.lastCount;
    if (abs(delta) > SCROLL_DEAD) {
      // 符号反転で自然なスクロール方向
      int scroll = (int)(-delta * SCROLL_PER_CLICK);
      bleMouse.move(0, 0, scroll, 0);
      Serial.printf("Scroll: delta=%ld, scroll=%d\n", delta, scroll);
      encoderState.lastCount = encNow;
    }
  }
  
  delay(5); // 高FPS化
}