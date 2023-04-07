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

  while(digitalRead(sensor1_pin) == LOW){
    stepper.run();
    //counter = stepper.currentPosition();
    //Serial.println(counter);

    // Auf Fehler prüfen
    if(stepper.currentPosition() > (end_pos * 1.25) || stepper.currentPosition() < (end_pos * -1.25)){
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