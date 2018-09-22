/*******************************************************************************
 * Copyright (c) 2018 Tim Hornikel. All rights reserved.
 * Project  : VAN-ESK Buzzer
 * Compiler : Electron, v0.8.0-rc.8
 * Verion   : 1.2.0
 * Date     : 21/09/2018
*******************************************************************************/

// Set your 3rd-party SIM APN here
// https://docs.particle.io/reference/firmware/electron/#setcredentials-
STARTUP(cellular_credentials_set("internet", "", "", NULL));

// Declaration of Version
String FSWVersion = "SWx";

#include "clickButton.h";

// the Button
const int buttonPin1 = 3;
ClickButton button1(buttonPin1, HIGH, CLICKBTN_PULLUP);

// Button results
int function = 0;

// Declaration of digital IOs
int redLED = D0;
int greenLED = D1;
int blueLED = D2;
int onBoardLED = D7;
//int button = D3;
int switchMode = D4;

// Declaration of state variables
int iValButton = 0; // actual state of the button
int iNumberMails = 4; // total number of prepared e-mails to sent randomly

// Declaration of time variables
unsigned long lastSync = millis();
unsigned long ONE_DAY_MILLIS (24 * 60 * 60 * 1000);
unsigned long lastNotification = millis();
unsigned long threshold (15 * 60 * 1000); // 15 minutes
unsigned long lastCheck = millis();
unsigned long thresholdCheck (5 * 1000); // 5 seconds

/* This function is called once at start up ----------------------------------*/
void setup() {

  pinMode(D3, INPUT_PULLDOWN);

  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  button1.debounceTime   = 50;   // Debounce timer in ms
  button1.multiclickTime = 500;  // Time limit for multi clicks
  button1.longClickTime  = 1000; // time until "held-down clicks" register


    // Declaration of IOs
    pinMode(blueLED, OUTPUT);                // Output of notification LED
    pinMode(redLED, OUTPUT);
    pinMode(greenLED, OUTPUT);
    pinMode(onBoardLED, OUTPUT);
    //pinMode(button, INPUT_PULLDOWN);            // Input of button signal
    pinMode(switchMode, INPUT_PULLDOWN);

    // Resetting the notification time
    lastNotification = lastNotification + threshold;

    // Declaration of particle function to turn the notification threshold on and off from the cloud.
    Particle.function("Threshold",notificationThresholdToggle);
    Particle.function("SoC",SoC);
    //Particle.variable("SoC", SoC);
    Particle.variable("FW Version", FSWVersion);
    Particle.variable("NumberMails", iNumberMails);

    // Activation of serial interface for debugging
    Serial.begin(9600);  // open serial over TX and RX pins
    Serial.println("Setup complete");

    // KeepAlive signal for 3rd party SIM-card
    Particle.keepAlive(30);

}


/* Main function that loops forever ------------------------------------------*/
void loop() {

    if (digitalRead(switchMode) == false) {

      Serial.println("System sleep");
      System.sleep(switchMode, RISING, 0, SLEEP_NETWORK_STANDBY);

    } else {

      checkButton();
      checkStatus();
      updateTime();

    }

}

/*******************************************************************************
 * Function Name  : random_seed_from_cloud
 * Description    : recieves a new random seed from the cloud
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
 void random_seed_from_cloud(unsigned seed) {
   srand(seed);
 }

/*******************************************************************************
 * Function Name  : updateTime
 * Description    : Request time synchronization from the Particle Cloud once a day
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void updateTime() {

    if (millis() - lastSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Particle Cloud
    Particle.syncTime();
    Serial.println("Time updated");
    lastSync = millis();
    }

}

/*******************************************************************************
 * Function Name  : CheckStatus
 * Description    : Checks if Particle cloud is connected
 * Input          : None
 * Output         : Blue flashing LED every 5 seconds
 * Return         : None
 *******************************************************************************/
void checkStatus() {

  if ((Particle.connected() == true) && (millis() - lastCheck > thresholdCheck)) {

    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(blueLED, HIGH);
    delay(100);
    digitalWrite(blueLED, LOW);
    lastCheck = millis();

  }

}

/*******************************************************************************
 * Function Name  : flashLED
 * Description    : Funcion for flashing the notification LED
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void flashLED(int iTimes) {

    for (int i=0; i < iTimes; i++) {

      digitalWrite(onBoardLED, HIGH);
      digitalWrite(redLED, HIGH);
      delay(150);
      digitalWrite(onBoardLED, LOW);
      digitalWrite(redLED, LOW);
      delay(100);

      }

}

/*******************************************************************************
 * Function Name  : lightLED
 * Description    : Funcion for lighting the notification LED
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void lightLED(int iSeconds) {

    int iTime = iSeconds * 1000;

    digitalWrite(onBoardLED, HIGH);
    digitalWrite(greenLED, HIGH);
    delay(iTime);
    digitalWrite(onBoardLED, LOW);
    digitalWrite(greenLED, LOW);

}

/*******************************************************************************
 * Function Name  : sendMail
 * Description    : Function for sending e-mail to newsgroup
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void sendMail() {

    // read the input pin
    //iValButton = digitalRead(button);
    String MailGun = "M";
    String Random = "";

    // check if threshold for timeout has been reached
    if ((millis() - lastNotification > threshold)) {

      // random number between 1 and 10
      int r = random(1, 1 + iNumberMails);
      Random =  String(r);
      MailGun = String(MailGun + Random);

      // send e-mail to mailgroup and reset threshold for timeout
      //Particle.publish(MailGun, PRIVATE);
      bool success;
      success = Particle.publish(MailGun, PRIVATE);

      if (!success) {

        // get here if event publish did not work
        Serial.println("Mail NOT sent");
        //lastNotification = millis();
        flashLED(3);

      } else if (success) {

        Serial.println("Mail sent");
        lastNotification = millis();
        lightLED(2);

      }

    } else {

      Serial.println("Threshold active");
      Particle.publish("Threshold active", PRIVATE);
      flashLED(5);

    }

}

/*******************************************************************************
 * Function Name  : checkButton
 * Description    : Function for checing the state of the button
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void checkButton() {

  // Update button state
  button1.Update();

  // Save click codes in LEDfunction, as click codes are reset at next Update()
  if(button1.clicks != 0) function = button1.clicks;

  if(function == 1) {

    Serial.println("SINGLE click");
    //Particle.publish("SINGLE click", PRIVATE);
    sendMail();

  } else if(function == 2) {

    Serial.println("DOUBLE click");
    //Particle.publish("DOUBLE click", PRIVATE);

  } else if(function == 3) {

    Serial.println("TRIPLE click");
    //Particle.publish("TRIPLE click", PRIVATE);

  } else if(function == -1) {

    Serial.println("SINGLE LONG click");
    //Particle.publish("SINGLE LONG click", PRIVATE);

  } else if(function == -2) {

    Serial.println("DOUBLE LONG click");
    //Particle.publish("DOUBLE LONG click", PRIVATE);

  } else if(function == -3) {

    Serial.println("TRIPLE LONG click");
    //Particle.publish("TRIPLE LONG click", PRIVATE);

  }

  function = 0;
  delay(5);


}

/*******************************************************************************
 * Function Name  : checkButton
 * Description    : Function for remotely setting the threshold
 * Input          : None
 * Output         : None
 * Return         : Value of the state (1 / 0 for activated / deactivated) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int notificationThresholdToggle(String thresholdState) {

    if (thresholdState == "get") {

        int intThreshold = int(threshold)/(60*1000);                           // Deactivation of threshold
        Serial.println("Threshold deactivated");
        return intThreshold;

    } else if (thresholdState == "on") {

        threshold = 15 * 60 * 1000;                // Threshold set to 15 minutes
        int intThreshold = int(threshold)/(60*1000);
        Serial.println("Threshold activated");
        return intThreshold;

    } else if (thresholdState == "off") {

        threshold = 0;                             // Deactivation of threshold
        Serial.println("Threshold deactivated");
        return 0;
    }

    else {

        return -1;

    }

}

/*******************************************************************************
 * Function Name  : sSOC
 * Description    : Function for remotely checking the state of charge
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
float SoC(String command) {

    FuelGauge fuel;
    float Battery;

    if (command == "get") {

        Battery = fuel.getSoC();

        return Battery;

      } else {

        return 0.0;

    }

}
