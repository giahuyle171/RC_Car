String data_recv;

void serial_update() {
  while (1) {
    if (Serial.available()) {
      int16_t c = Serial.read();
      if (c!='\n' && c!=' ')
        data_recv += (char)c;
      else break;
    }
    else return;
  }
  String id = "";
  int16_t val = 0;
  int i = 0;
  while (i<data_recv.length() && (data_recv[i]<48 || data_recv[i]>57)){
    id += data_recv[i];
    i++;
  }
  for (; i<data_recv.length(); i++){
    val = val*10+(data_recv[i]-48);
  }
  data_recv = "";
  set_data(val, id);
}
void set_data(int16_t val, String id) {
  if (id == "s") {
    radioData[0] = val;
  }
  else if (id == "ss") {
    radioData[1] = val;
  }
}

void telemetryTask() {
  uint32_t time = millis();
  if (time-t_telemetryTask >= 20){
    t_telemetryTask = time;
    // serial_update();
    
    for (int i = 0; i<6; i++) {
      log(radioData[i]);
    }
    Serial.println("");
  }
}