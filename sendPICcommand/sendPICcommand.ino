/*
 *  This sketch checks an empty eeprom
 */
 
void setup() {

  // initialize serial communications at 4800 bps:
  Serial.begin(4800);
  Serial.println("Starting...");
  
  // check empty eeprom
  checkEmptyEeprom();
  delay(3000);
  
  // checkEmptyEeprom() passed
  Serial.println("Passed!");

}

void loop() {
  // put your main code here, to run repeatedly:

}

///////////////////////////////////////////////////////
void checkEmptyEeprom() {

  uint8_t answer = 0;

  // checks if PIC is reading an empty EEPROM for 5 seconds
  answer = sendPICcommand("99 000", "500: 255|", 5000) || sendPICcommand("99 000", "1000: 255|", 5000);
  if (answer == 0)
  {
    // waits for an answer from the module
    while (answer == 0) {   // sendPICcommand every two seconds and wait for the answer
	  answer = sendPICcommand("99 000", "500: 255|", 2000) || sendPICcommand("99 000", "1000: 255|", 2000);
    }
  }

}

/////////////////////////////////////////////////////////////////
int8_t sendPICcommand(char* PICcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  //while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  while (Serial.available()) { //Cleans the input buffer
    Serial.read();
  }

  Serial.println(PICcommand);    // Send the PIC command


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (Serial.available() != 0) {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial.read();
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}
