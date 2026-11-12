template <typename T>
void log(T s) {
  Serial.print(s);
  Serial.print(" ");
}
void controlTask() {
  uint32_t time = millis();
  if (time-t_controlTask >= 20) {
    t_controlTask = time;
    int joy1 = analogRead(A5);
    int joy2 = analogRead(A3);
    // int val1 = analogRead(A0);
    // int val2 = analogRead(A1);
    int switch1 = digitalRead(6);
    int switch2 = digitalRead(5);
    radioData[0] = joy1;
    radioData[1] = joy2;
    // radioData[2] = val1;
    // radioData[3] = val2;
    radioData[4] = switch1;
    radioData[5] = switch2;
    radioData.write(&radioData.data, sizeof(radioData.data));

  }
}

