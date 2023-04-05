// reference run in debug_mode
void blind_rev_dev(){
  stepper.setCurrentPosition(0);
  Serial.println("DEV reference run");
}

// normal reference run
bool blind_rev(){
  Serial.println("Start reference run!");
  //unsigned long counter = 0;
  
  digitalWrite(en_pin, LOW);                        // enable motor

  stepper.setMaxSpeed(reference_speed);             // set movment speed
  stepper.setAcceleration(movement_accel);          // set acceleration
  
  stepper.move(end_pos * -1.3);                     // slightly more so that I can catch the error

  while(digitalRead(sensor1_pin) == LOW){
    stepper.run();
    //counter = stepper.currentPosition();
    //Serial.println(counter);

    // check if something gone wrong during homeing
    if(stepper.currentPosition() > (end_pos * 1.25) || stepper.currentPosition() < (end_pos * -1.25)){
      stepper.stop();
      Serial.println("Reference run failed!");
      digitalWrite(en_pin, HIGH);
      return false;
    }
  }

  digitalWrite(en_pin, HIGH);                       // disable motor
  
  stepper.setCurrentPosition(0);
  Serial.println("Reference run complete!");

  return true;
}