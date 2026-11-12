template <typename T>
void print(T s) {
  Serial.print(s);
  Serial.print(" ");
}
void calibrate(unsigned long duration_ms) {
  digitalWrite(PC13, 0);
  for (int i = 0; i < 9; i++) {
    sensor_min[i] = 4095; // (Dùng 4095 nếu dùng STM32/ESP32 ADC 12-bit)
    sensor_max[i] = 0;
  }

  Serial.println("BAT DAU CALIB");
  unsigned long startTime = millis();

  while (millis() - startTime < duration_ms) {
    for (int i = 0; i < 9; i++) {
      uint16_t val = analogRead(SENSOR_PINS[i]);
      if (val < sensor_min[i]) sensor_min[i] = val;
      if (val > sensor_max[i]) sensor_max[i] = val;
    }
  }

  Serial.println("CALIB HOAN TAT! Nguong Threshold tu dong cua tung mat:");
  for (int i = 0; i < 9; i++) {
    sensor_threshold[i] = (sensor_min[i] + sensor_max[i])*0.5f;
  }
  digitalWrite(PC13, 1);
}

void readSensors(void) {
  for (int i = 0; i < 9; i++) {
    sensor_raw[i] = analogRead(SENSOR_PINS[i]);

    // So sánh với ngưỡng riêng đã tính được lúc Calib
    if (sensor_raw[i] > sensor_threshold[i]) {
      sensor_digital[i] = 1; // Vạch đen
    } else {
      sensor_digital[i] = 0; // Nền trắng
    }
  }
}

int32_t calculatePosition(void) {
  int32_t weighted_sum = 0;
  int32_t total_sum = 0;
  static int32_t last_direction = 0;

  for (int i = 0; i < 9; i++) {
    weighted_sum += (int32_t)sensor_digital[i] * WEIGHTS[i];
    total_sum    += (int32_t)sensor_digital[i];
  }

  if (total_sum == 0) {
    // return (last_direction < 0) ? -6000 : 6000; 
    return (last_direction ); 
  }

  int32_t position = weighted_sum / total_sum;
  last_direction = position;
  return position;
}
int32_t computePID(int32_t error) {
  integral += error;
  integral = constrain(integral, -10000, 10000);
  int32_t derivative = error - last_error;
  int32_t output = (Kp * error) + (Ki * integral) + (Kd * derivative);
  last_error = error;
  return output;
}
void runPIDControl(void) {

  int32_t error = calculatePosition();
  int32_t pid_output = computePID(error);

  int left_speed  = BASE_SPEED + pid_output;
  int right_speed = BASE_SPEED - pid_output;
  motorL.run(left_speed);
  motorR.run(right_speed);
}
void controlTask() {
  uint32_t time = millis();
  if (time-t_controlTask >= 20) {
    t_controlTask = time;
    readSensors();
    uint16_t sensor_pattern = 0;
    for (int i = 0; i < 9; i++) {
      if (sensor_digital[i] == 1) {
        sensor_pattern |= (1 << i);
      }
    }
    int32_t total_sum = 0;
    for (int i = 0; i < 9; i++) {
      total_sum += (int32_t)sensor_digital[i];
    }

    if (run == 0) {
      reset();
      return;
    }

    if (total_sum >= 8) {
      reset();
      return;
    }
    switch (sensor_pattern) {
      case 0b100000000:
        motorL.run(R);
        motorR.run(-R);
        break;

      case 0b000000001:
        motorL.run(-R);
        motorR.run(R);
        break;

      case 0b100000001:
        motorL.run(R);
        motorR.run(-R);
        break;
      case 0b01000010:
        motorL.run(R);
        motorR.run(-R);
        break;


      default:
        runPIDControl();
        break;
    }
 

  }
}
void reset() {
  run = 0;
  motorL.run(0);
  motorR.run(0);
  last_error = 0;
  integral = 0;
}
void handleButton() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long currentMillis = millis();

  if (reading == LOW) { 
    if (!isButtonDown) {
      isButtonDown = true;
      pressStartTime = currentMillis;
      longPressHandled = false;
    } else {
      if (!longPressHandled && (currentMillis - pressStartTime >= LONG_PRESS_TIME)) {  
              
        calibrate(10000); 
        longPressHandled = true;
      }
    }
  } else { 
    if (isButtonDown) {
      unsigned long pressDuration = currentMillis - pressStartTime;
      
      if (pressDuration >= DEBOUNCE_DELAY && !longPressHandled) {
        run = !run;
        blinkLED(500, 150);
      }

      isButtonDown = false;
    }
  }
}
void blinkLED(unsigned long duration_ms, unsigned long interval_ms) {
  unsigned long startBlinkTime = millis();
  bool ledState = false;

  while (millis() - startBlinkTime < duration_ms) {
    ledState = !ledState;
    if (ledState) {
      digitalWrite(PC13, LOW);
    } else {
      digitalWrite(PC13, HIGH);
    }
    delay(interval_ms);
  }
  
  digitalWrite(PC13, HIGH); 
}

void checkObstacle(void) {
  int reading = digitalRead(OBSTACLE_BTN_PIN);

  // Kiểm tra chống dội (Debounce)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Nếu trạng thái nút thực sự thay đổi (Nhấn xuống = LOW do dùng PULLUP)
    if (reading == LOW) {
      // Thay đổi giá trị biến trạng thái
      avoidObstacleManeuver();
    }
  }
  lastButtonState = reading;
}

void moveForwardDuration(int speed, unsigned long duration) {
  motorL.run(speed-40);
  motorR.run(speed+25);
  delay(duration);
  motorL.run(0);
  motorR.run(0);
  delay(100); // Dừng ngắn ổn định xe
}

void turnRight90(void) {
  // Bánh trái tiến, bánh phải lùi để quay tại chỗ sang phải
  motorL.run(R+5);
  motorR.run(-R);
  delay(400); // <-- CẦN HIỆU CHỈNH: Thời gian quay đủ 90 độ
  motorL.run(0);
  motorR.run(0);
  delay(100);
}

void turnLeft90(void) {
  // Bánh trái lùi, bánh phải tiến để quay tại chỗ sang trái
  motorL.run(-R);
  motorR.run(R+30);
  delay(400); // <-- CẦN HIỆU CHỈNH: Thời gian quay đủ 90 độ
  motorL.run(0);
  motorR.run(0);
  delay(100);
}

void avoidObstacleManeuver(void) {
  Serial.println("Bat dau chu trinh di chuyen theo mau...");

  // Dừng xe ngắn trước khi bắt đầu
  motorL.run(0);
  motorR.run(0);
  delay(50);

  // 1. Quay phải 90 độ
  turnRight90();

  // Tiến một đoạn
  moveForwardDuration(100, 500);

  // 2. Quay trái 90 độ
  turnLeft90();

  // Tiến tiếp một đoạn
  moveForwardDuration(100, 900);

  // // 3. Quay phải 90 độ
  // turnLeft90();

  // // Tiến tiếp một đoạn
  // moveForwardDuration(90, 500);

  // // 4. Quay phải 90 độ lần cuối
  // turnRight90();

}