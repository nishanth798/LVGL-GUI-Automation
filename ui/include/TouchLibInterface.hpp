#ifdef SENSCAP_EN

#pragma once

#include "stdint.h"

class TP_Point {
public:
  TP_Point(void) {};
  ~TP_Point() {};

  TP_Point(uint8_t id, uint16_t x, uint16_t y, uint16_t size, uint8_t pressure, uint8_t state)
      : id(id), x(x), y(y), size(size), pressure(pressure), state(state) {}

  bool operator==(TP_Point point) {
    return ((point.x == x) && (point.y == y) && (point.size == size) &&
            (point.pressure == pressure) && (point.state == state));
  }

  bool operator!=(TP_Point point) {
    return ((point.x != x) || (point.y != y) || (point.size != size) ||
            (point.pressure != pressure) || (point.state != state));
  }

  uint8_t id;
  uint16_t x;
  uint16_t y;
  uint8_t size;
  uint8_t pressure;
  uint8_t state;
};

class TouchLibInterface {
public:
  TouchLibInterface() {}

  virtual ~TouchLibInterface() {}

  virtual bool init() = 0;

  virtual void deinit() = 0;

  virtual bool enableSleep() = 0;

  virtual bool read() = 0;

  virtual uint8_t getPointNum() = 0;

  virtual TP_Point getPoint(uint8_t n) = 0;

  // virtual void setRotation(uint8_t r) = 0;

  virtual uint8_t getRotation() = 0;
};

class TouchLibGesture {
public:
  TouchLibGesture() {}

  virtual ~TouchLibGesture() {}

  virtual bool isEnableGesture() = 0;

  virtual bool enableGesture() = 0;

  virtual bool disableGesture() = 0;

  virtual uint8_t getGesture(void) = 0;
};

#endif // SENSCAP_EN