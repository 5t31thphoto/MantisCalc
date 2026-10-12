// Host-side mock of the subset of M5Unified/M5GFX used by MantisCalculator.
// Lets src/main.cpp compile and run under g++ for logic + layout tests.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>

inline unsigned long& mock_ms() { static unsigned long t = 0; return t; }
inline unsigned long millis() { return mock_ms(); }
inline void delay(unsigned long ms) { mock_ms() += ms; }

struct DrawnText { int x, y, size; std::string s; };
inline std::vector<DrawnText>& mock_texts() { static std::vector<DrawnText> v; return v; }

class MockGFX {
 public:
  int cx = 0, cy = 0, ts = 1;
  void fillScreen(uint16_t) {}
  void fillRect(int, int, int, int, uint16_t) {}
  void fillRoundRect(int, int, int, int, int, uint16_t) {}
  void drawFastHLine(int, int, int, uint16_t) {}
  void drawFastVLine(int, int, int, uint16_t) {}
  void setTextSize(int s) { ts = s; }
  void setTextColor(uint16_t) {}
  void setTextColor(uint16_t, uint16_t) {}
  void setCursor(int x, int y) { cx = x; cy = y; }
  void setRotation(int) {}
  void setTextWrap(bool) {}
  int textWidth(const char* s) { return 6 * ts * (int)strlen(s); }
  void print(const char* s) { mock_texts().push_back({cx, cy, ts, s}); cx += textWidth(s); }
  void printf(const char* f, ...) { char b[256]; va_list a; va_start(a, f); vsnprintf(b, sizeof b, f, a); va_end(a); print(b); }
  void pushImage(int, int, int, int, const uint16_t*) {}
  int width() { return 320; }
  int height() { return 240; }
};
class M5GFX : public MockGFX {};
class M5Canvas : public MockGFX {
 public:
  explicit M5Canvas(M5GFX*) {}
  void setColorDepth(int) {}
  void setPsram(bool) {}
  void* createSprite(int, int) { static char buf[1]; return buf; }
  void pushSprite(int, int) {}
};
namespace m5 {
struct touch_detail_t {
  int x = 0, y = 0; bool pressed = false;
  bool wasPressed() const { return pressed; }
  bool isPressed() const { return pressed; }
};
}
struct MockTouch { m5::touch_detail_t d; m5::touch_detail_t getDetail(int = 0) { return d; } };
struct MockBtn { bool p = false; bool wasPressed() const { return p; } };
struct MockSpeaker { int beeps = 0; void tone(float, uint32_t = 0) { beeps++; } void setVolume(int) {} };
struct MockPower { int vib = 0; void setVibration(uint8_t l) { vib = l; } };
struct MockCfg {};
struct MockM5 {
  M5GFX Display; MockTouch Touch; MockBtn BtnA, BtnB, BtnC; MockSpeaker Speaker; MockPower Power;
  MockCfg config() { return MockCfg(); }
  void begin(const MockCfg&) {}
  void update() {}
};
inline MockM5 M5;
