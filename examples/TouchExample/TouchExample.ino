#include "TSC2004.h"

TSC2004 ts;

void setup() {
  Serial.begin(9600);

  ts.begin();
}

void loop() {
  if (!ts.touched()) {
    return;
  }

  const TS_Point p = ts.getPoint();

  Serial.print("(");
  Serial.print(p.x);
  Serial.print(", ");
  Serial.print(p.y);
  Serial.print(", ");
  Serial.print(p.z);
  Serial.println(")");
}
