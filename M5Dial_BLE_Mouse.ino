#include <M5Dial.h>
#include <BleMouse.h>

BleMouse bleMouse("M5Dial BLE Mouse");

// ---- LCD theme (readable mono look) ----
bool lcdTheme = true; // toggle LCD theming
uint16_t LCD_BG, LCD_FG, LCD_GHOST;

void lcdInitColors() {
  // light warm gray background, high-contrast ink, soft secondary
  LCD_BG    = M5Dial.Display.color565(236, 238, 230);
  LCD_FG    = M5Dial.Display.color565(12, 16, 12);
  LCD_GHOST = M5Dial.Display.color565(128, 136, 128);
}

void lcdFillBackground() {
  if (!lcdTheme) { M5Dial.Display.clear(BLACK); return; }
  // solid background only (no border, no dithering) for readability on round screen
  M5Dial.Display.fillScreen(LCD_BG);
}

// draw a thick rounded segment block
static inline void lcdSeg(int x, int y, int w, int h, bool on) {
  M5Dial.Display.fillRoundRect(x, y, w, h, 2, on ? LCD_FG : LCD_GHOST);
}

// 7-seg digit renderer (x,y top-left; s scale; d digit)
void lcdDraw7SegDigit(int x, int y, int s, int d, bool ghost = false) {
  bool segs[10][7] = {
    {1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},
    {1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}
  };
  int a = 3 * s; // segment thickness
  int b = s;     // inner margin
  auto draw = [&](int id, bool on) {
    bool enable = on;
    if (!on && ghost) { enable = true; }
    switch (id) {
      case 0: lcdSeg(x + b,       y,         4*s, a, enable); break; // top
      case 1: lcdSeg(x + 4*s + b, y + b,     a,   4*s, enable); break; // top-right
      case 2: lcdSeg(x + 4*s + b, y + 5*s+b, a,   4*s, enable); break; // bottom-right
      case 3: lcdSeg(x + b,       y + 9*s,   4*s, a, enable); break; // bottom
      case 4: lcdSeg(x,           y + 5*s+b, a,   4*s, enable); break; // bottom-left
      case 5: lcdSeg(x,           y + b,     a,   4*s, enable); break; // top-left
      case 6: lcdSeg(x + b,       y + 4*s,   4*s, a, enable); break; // middle
    }
  };
  for (int i = 0; i < 7; ++i) draw(i, segs[d][i]);
}

void lcdDrawColon(int x, int y, int s) {
  M5Dial.Display.fillCircle(x, y + 3*s, s, LCD_FG);
  M5Dial.Display.fillCircle(x, y + 7*s, s, LCD_FG);
}

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

// 回転状態管理
struct RotationState {
  int rotation = 0;               // 画面回転 (0-3: 0°,90°,180°,270°)
  bool setupMode = false;         // 設定モード中
  uint32_t buttonPressStart = 0;  // ボタン押下開始時刻
  bool longPressProcessed = false; // 長押し処理済みフラグ
  
  int getRotationDegrees() {
    return rotation * 90;
  }
  
  void reset() {
    setupMode = false;
    buttonPressStart = 0;
    longPressProcessed = false;
  }
};

RotationState rotationState;

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

// 設定モード用UI表示
void drawRotationSetup() {
  lcdFillBackground();
  M5Dial.Display.setTextSize(2);
  
  // タイトル
  M5Dial.Display.setCursor(45, 20);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : YELLOW);
  M5Dial.Display.println("ROTATION");
  M5Dial.Display.setCursor(65, 45);
  M5Dial.Display.println("SETUP");
  
  // 現在の回転角度表示
  int rotationDegrees = rotationState.getRotationDegrees();
  M5Dial.Display.setTextSize(4);
  M5Dial.Display.setCursor(70, 78);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : WHITE, lcdTheme ? LCD_BG : BLACK);
  M5Dial.Display.printf("%d", rotationDegrees);
  
  // 回転段階表示
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.setCursor(20, 110);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_GHOST : GREEN);
  M5Dial.Display.printf("Step: %d/4", rotationState.rotation + 1);
  
  // 回転パターン表示
  M5Dial.Display.setCursor(20, 125);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_GHOST : CYAN);
  const char* rotationNames[] = {"Normal", "Right", "Upside", "Left"};
  M5Dial.Display.printf("Mode: %s", rotationNames[rotationState.rotation]);
  
  // 操作ガイド
  M5Dial.Display.setCursor(15, 155);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : ORANGE);
  M5Dial.Display.println("Turn: 0→90→180→270");
  M5Dial.Display.setCursor(15, 170);
  M5Dial.Display.println("Hold: Save & Exit");
  M5Dial.Display.setCursor(15, 185);
  M5Dial.Display.setTextColor(lcdTheme ? LCD_GHOST : ORANGE);
  M5Dial.Display.println("Simple & Clean");
  
  // ビジュアル回転インジケーター
  int centerX = 200, centerY = 100;
  int radius = 25;
  float rad = rotationState.getRotationDegrees() * PI / 180.0f;
  int endX = centerX + radius * cos(rad - PI/2);
  int endY = centerY + radius * sin(rad - PI/2);
  
  uint16_t ring = lcdTheme ? LCD_GHOST : WHITE;
  uint16_t needle = lcdTheme ? LCD_FG : RED;
  M5Dial.Display.drawCircle(centerX, centerY, radius, ring);
  M5Dial.Display.drawLine(centerX, centerY, endX, endY, needle);
  M5Dial.Display.fillCircle(endX, endY, 3, needle);
}

// シンプル90度回転システム - 座標変換なし

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true); // エンコーダー有効化
  
  // シンプル画面回転
  M5Dial.Display.setRotation(rotationState.rotation);
  
  M5Dial.Display.setBrightness(40);
  lcdInitColors();
  lcdFillBackground();
  M5Dial.Display.setCursor(40, 60);
  if (lcdTheme) M5Dial.Display.setTextColor(LCD_FG, LCD_BG);
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
  
  // エンコーダーボタン長押し検出（設定モード切り替え）
  if (M5Dial.BtnA.isPressed()) {
    if (rotationState.buttonPressStart == 0) {
      rotationState.buttonPressStart = millis();
      Serial.println("Button press started");
    } else {
      uint32_t pressDuration = millis() - rotationState.buttonPressStart;
      Serial.printf("Button pressed for %dms\n", pressDuration);
      
      if (pressDuration > 1500 && !rotationState.setupMode && !rotationState.longPressProcessed) {
        // 1.5秒長押しで設定モード開始
        rotationState.setupMode = true;
        rotationState.longPressProcessed = true; // 処理済みフラグ
        encoderState.init(readEncoder()); // エンコーダー初期化
        drawRotationSetup();
        Serial.println("Entered rotation setup mode");
      } else if (pressDuration > 1000 && rotationState.setupMode && !rotationState.longPressProcessed) {
        // 設定モード中の1秒長押し → 設定保存して終了
        rotationState.setupMode = false;
        rotationState.longPressProcessed = true; // 処理済みフラグ
        // 現在の回転設定を適用
        M5Dial.Display.setRotation(rotationState.rotation);
        Serial.printf("Rotation saved: %d×90 = %d\n", 
                     rotationState.rotation, rotationState.getRotationDegrees());
        
        // 保存完了メッセージを表示
        lcdFillBackground();
        M5Dial.Display.setTextSize(2);
        M5Dial.Display.setCursor(20, 60);
        M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : GREEN);
        M5Dial.Display.println("Settings Saved!");
        delay(2000);
        
        // 接続状態画面に戻る
        if (bleMouse.isConnected()) {
          lcdFillBackground();
          M5Dial.Display.setTextSize(2);
          M5Dial.Display.setCursor(40, 50);
          M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : GREEN);
          M5Dial.Display.println("Connected!");
          
          M5Dial.Display.setTextSize(1);
          M5Dial.Display.setCursor(20, 90);
          M5Dial.Display.setTextColor(lcdTheme ? LCD_GHOST : YELLOW);
          M5Dial.Display.println("Long press button");
          M5Dial.Display.setCursor(30, 105);
          M5Dial.Display.println("for rotation setup");
        } else {
          lcdFillBackground();
          M5Dial.Display.setCursor(40, 60);
          if (lcdTheme) M5Dial.Display.setTextColor(LCD_FG);
          M5Dial.Display.println("Waiting...");
        }
      }
    }
  } else if (rotationState.buttonPressStart > 0) {
    // ボタンが離された
    uint32_t pressDuration = millis() - rotationState.buttonPressStart;
    Serial.printf("Button released after %dms\n", pressDuration);
    
    // 360度モードでは短押しによるモード切り替えは不要
    rotationState.buttonPressStart = 0;
    rotationState.longPressProcessed = false; // フラグリセット
  }
  
  // 設定モード処理
  if (rotationState.setupMode) {
    long encNow = readEncoder();
    long delta = encNow - encoderState.lastCount;
    
    if (abs(delta) >= 4) { // エンコーダー感度調整
      // シンプルな回転切り替え：0→1→2→3→0
      if (delta > 0) {
        rotationState.rotation = (rotationState.rotation + 1) % 4; // 時計回り
      } else {
        rotationState.rotation = (rotationState.rotation + 3) % 4; // 反時計回り（+3は-1と同じ効果）
      }
      
      // 画面回転をリアルタイム更新
      M5Dial.Display.setRotation(rotationState.rotation);
      
      Serial.printf("Rotation: %d×90 = %d (delta=%ld)\n", 
                   rotationState.rotation, rotationState.getRotationDegrees(), delta);
      
      encoderState.lastCount = encNow;
      drawRotationSetup(); // UI更新
    }
    return; // 設定モード中は通常処理をスキップ
  }
  
  // 接続状態確認のみ
  static bool wasConnected = false;
  bool currentlyConnected = bleMouse.isConnected();
  
  if (currentlyConnected != wasConnected) {
    Serial.print("Connection changed: ");
    Serial.println(currentlyConnected ? "CONNECTED" : "DISCONNECTED");
    wasConnected = currentlyConnected;
    
    if (currentlyConnected) {
      lcdFillBackground();
      M5Dial.Display.setTextSize(2);
      M5Dial.Display.setCursor(40, 50);
      M5Dial.Display.setTextColor(lcdTheme ? LCD_FG : GREEN);
      M5Dial.Display.println("Connected!");
      
      M5Dial.Display.setTextSize(1);
      M5Dial.Display.setCursor(20, 90);
      M5Dial.Display.setTextColor(lcdTheme ? LCD_GHOST : YELLOW);
      M5Dial.Display.println("Long press button");
      M5Dial.Display.setCursor(30, 105);
      M5Dial.Display.println("for rotation setup");
      
      Serial.println("Basic connection test successful");
    } else {
      lcdFillBackground();
      M5Dial.Display.setCursor(40, 60);
      if (lcdTheme) M5Dial.Display.setTextColor(LCD_FG);
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
