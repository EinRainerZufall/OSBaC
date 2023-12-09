// DEV Homing
void blind_rev_dev(){
  stepper.setCurrentPosition(0);
  Serial.println("DEV reference run");
}


// Normales Homing
bool blind_rev(){
  Serial.println("Start reference run!");
  //unsigned long counter = 0;
  
  digitalWrite(en_pin, LOW);                        // Motor an

  stepper.setMaxSpeed(reference_speed);             // set movment speed
  stepper.setAcceleration(movement_accel);          // set acceleration
  
  stepper.move(end_pos * -1.3);                     // bissle mehr als die gesamtlänge für mögliche Fehler

  while(analogRead(sensor1_pin) > sensor1_A_Wert){
    stepper.run();
    //counter = stepper.currentPosition();
    //Serial.println(counter);

    // Auf Fehler prüfen
    if(stepper.currentPosition() > (settings.getULong64("end_pos") * 1.25) || stepper.currentPosition() < (settings.getULong64("end_pos") * -1.25)){
      stepper.stop();
      Serial.println("Reference run failed!");
      digitalWrite(en_pin, HIGH);                   // Motor aus
      return false;
    }
  }

  digitalWrite(en_pin, HIGH);                       // Motor aus
  
  stepper.setCurrentPosition(0);
  Serial.println("Reference run complete!");

  return true;
}


// Motor anhalten und Speichern der IST Position
bool motor_stop(){
  stepper.stop();
  if(stepper.isRunning()) return false;
  cur_pos = map_uint64_t(stepper.currentPosition(), 0, settings.getULong64("end_pos"), 100, 0);
  return true;
} 


// Motor bewegung prüfen
bool motor_move_check(int pos){
  if(pos < 0 || pos > 100) return false;
  uint64_t steps = 0;

  steps = map_uint64_t(pos, 100, 0, 0, settings.getULong64("end_pos"));

  stepper.moveTo(steps);

  return true;
}