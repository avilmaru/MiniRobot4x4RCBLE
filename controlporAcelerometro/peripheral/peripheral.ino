// ---------------------------------------------------------------------------
// Mini Robot 4x4 controlado remotamente por acelerometro:  - v1.0 - 15/04/2020
//
// AUTOR:
// Creado por Angel Villanueva - @avilmaru
//
// LINKS:
// Blog: http://www.mecatronicalab.es
//
//
// HISTORICO:
// 15/04/2020 v1.0 - Release inicial.
//
// ---------------------------------------------------------------------------

#include <ArduinoBLE.h>
#include <Arduino_MKRRGB.h>
#include <MKRMotorCarrier.h>

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

String command = "";

//Variable to store the battery voltage
static int batteryVoltage;
//Variable to change the motor speed and direction
static int dutyX = 0;
static int dutyY = 0;
static int duty1 = 0;
static int duty2 = 0;

// BLE gesture Service
BLEService gestureService(deviceServiceUuid); 

// BLE gesture Switch Characteristic 
BLEStringCharacteristic gestureCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite,512);


void setup() {
  
  //Serial.begin(9600);
  //while (!//Serial);

  //Establishing the communication with the motor shield
  if (controller.begin()) 
    {
      //Serial.print("MKR Motor Shield connected, firmware version ");
      //Serial.println(controller.getFWVersion());
    } 
  else 
    {
      //Serial.println("Couldn't connect! Is the red led blinking? You may need to update the firmware with FWUpdater sketch");
      while (1);
    }

  // Reboot the motor controller; brings every value back to default
  //Serial.println("Reboot the motor controller");
  controller.reboot();
  delay(500);
  servo2.setAngle(90);
  delay(500);


  // initialize the display
  MATRIX.begin();
  // set the brightness, supported values are 0 - 255
  MATRIX.brightness(120);
  // configure the text scroll speed
  MATRIX.textScrollSpeed(50);
  MATRIX.clear();
  MATRIX.endDraw();


  // begin ble initialization
  if (!BLE.begin()) {
    //Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Mini Robot 4x4 peripheral");
  BLE.setAdvertisedService(gestureService);

  // add the characteristic to the service
  gestureService.addCharacteristic(gestureCharacteristic);

  // add service
  BLE.addService(gestureService);

  // set the initial value for the characeristic:
  gestureCharacteristic.writeValue("");

  // start advertising
  BLE.advertise();

  //Serial.println("Mini Robot 4x4 peripheral");
}

void loop() {
  
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    
    //Serial.print("Connected to central: ");
    // print the central's MAC address:
    //Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      
      // if the remote device wrote to the characteristic,
      if (gestureCharacteristic.written()) {
         command = gestureCharacteristic.value();
         //Serial.print(F("commmand value:  "));
         //Serial.println(command);
         sendInstruction(command);
       }
      
    }

    // when the central disconnects, print it out:
    //Serial.print(F("Disconnected from central: "));
    //Serial.println(central.address());
  }
}

void sendInstruction(String str) {


  if (str.length() == 0)
    return;
    
  // split 
  int pos = str.indexOf(",");
  int strLength = str.length();

  dutyX = str.substring(0,pos).toInt(); 
  dutyY = str.substring(pos+1,strLength).toInt(); 
 
  //Serial.print(F("valor en X:  "));
  //Serial.print(dutyX);
  //Serial.print(F("\t valor en Y: "));
  //Serial.println(dutyY);

  //Take the battery status
  float batteryVoltage = (float)battery.getConverted();
  
  //Reset to the default values if the battery level is lower than 7V
  if (batteryVoltage < 7) 
  {
    //Serial.println(" ");
    //Serial.println("WARNING: LOW BATTERY");
    //Serial.println("ALL SYSTEMS DOWN");
    M1.setDuty(0);
    M2.setDuty(0);
    M3.setDuty(0);
    M4.setDuty(0);

    while (batteryVoltage < 7) 
    {
      batteryVoltage = (float)battery.getConverted();
    }
  }
  else
  {
    if ((dutyY >= 30) && (dutyX > -20) && (dutyX < 20)) // adelante
    {
        duty1 = 80;
        duty2 = 80;
    }
    else if ((dutyY <= -30) && (dutyX > -20) && (dutyX < 20)) // atrás 
    {
        duty1 = -80;
        duty2 = -80;
    }
    else if ((dutyX >= 20) && (dutyY > -30) && (dutyY < 30)) // izquierda 
    {
        duty1 = -40;
        duty2 = 40;
    }
    else if ((dutyX <= -20) && (dutyY > -30) && (dutyY < 30)) // derecha 
    {
        duty1 = 40;
        duty2 = -40;
    }
    else if ((dutyY >= 30) && (dutyX >= 20)) // adelante + izquierda 
    {
        duty1 = 40;
        duty2 = 85;
    }
    else if ((dutyY >= 30) && (dutyX <= -20)) // adelante + derecha 
    {
        duty1 = 85;
        duty2 = 40;    
    }
    else if ((dutyY <= -30) && (dutyX >= 20)) // atras + izquierda 
    {
        duty1 = -50;
        duty2 = -75;
    }
    else if ((dutyY <= -30) && (dutyX <= -20)) // atras + derecha 
    {
        duty1 = -75;
        duty2 = -50;        
    }
    else
    {
        duty1 = 0;
        duty2 = 0;
    }


    //Serial.print(F("valor en duty1: "));
    //Serial.print(duty1);
    //Serial.print(F("\t valor en duty2: "));
    //Serial.println(duty2);
  
    M1.setDuty(duty1);
    M3.setDuty(duty1); 
    M2.setDuty(duty2);
    M4.setDuty(duty2); 
       
       
    //Keep active the communication MKR1010 & MKRMotorCarrier
    //Ping the samd11
    controller.ping();
    //wait
    delay(1);
  
  }


}
