template <typename T>
void log(T s) {
  Serial.print(s);
  Serial.print(" ");
}
int l_s = 325;
int m_s = 505;
int h_s = 685;

int max_s = 200;

int l_a = 410;
int m_a = 505;
int h_a = 620;
int max_a = 90;
void controlTask() {
  uint32_t time = millis();
  if (time-t_controlTask >= 20) {
    t_controlTask = time;
    radioData.update();
    int s = radioData[0];
    int a = radioData[1];
    if (radioData[4] == 1) {
      max_s = 300;
      if (radioData[5] == 1)
        max_s = 450;
    }
    else max_s = 200;
    if (l_s < s && s < h_s) {
      if (s < m_s-5) {
        RPM = (float)(m_s-s)*max_s/ (float)(m_s-l_s);
      }
      else if (s > m_s + 5) {
        RPM = - (float)(s-m_s)*max_s/ (float)(h_s - m_s);
      }
      else RPM = 0;
    }
    if (RPM >max_s) RPM = max_s;
    else if (RPM <-max_s) RPM = -max_s;

    if (l_a < a && a < h_a) {
      if (a < m_a-5) {
        STEERING = (float)(m_a-a)*max_a/ (float)(m_a-l_a);
      }
      else if (a > m_a + 5) {
        STEERING = - (float)(a-m_a)*max_a/ (float)(h_a - m_a);
      }
      else STEERING = 0;
    }
    servo.write(90+STEERING);
    esc.writeMicroseconds(1500+RPM);
  }
}

