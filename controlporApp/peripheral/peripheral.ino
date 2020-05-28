// ---------------------------------------------------------------------------
// Mini Robot 4x4 controlado remotamente por App y acelerómetro via BLE:  - v1.0 - 14/03/2020
//
// AUTOR:
// Creado por Angel Villanueva - @avilmaru
//
// LINKS:
// Blog: http://www.mecatronicalab.es
//
//
// HISTORICO:
// 14/03/2020 v1.0 - Release inicial.
//
// ---------------------------------------------------------------------------

#include <ArduinoBLE.h>
#include <MKRMotorCarrier.h>

const char* deviceServiceUuid = "0000FFE0-0000-1000-8000-00805F9B34FB";
const char* deviceServiceCharacteristicUuid = "0000FFE1-0000-1000-8000-00805F9B34FB";

bool automaticMode = false;

const int PIN_TRIGGER = A5;
const int PIN_ECHO = A6;
const int MIN_DISTANCIA = 25;

String command = "";

//Variable to store the battery voltage
static int batteryVoltage;
//Variable to change the motor speed and direction
static int dutyX = 0;
static int dutyY = 0;
static int duty1 = 0;
static int duty2 = 0;

int servopos = 90;
int direccionServo = 1;

// BLE gesture Service
BLEService movementService(deviceServiceUuid); 

// BLE gesture Switch Characteristic 
BLEStringCharacteristic movementCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite,512);


void setup() {
  
  //Serial.begin(9600);
  //while (!Serial);

  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  
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
  servo2.setAngle(servopos);
  delay(500);
   
 
  // begin ble initialization
  if (!BLE.begin()) {
    //Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("MKR Mini Robot 4x4");
  BLE.setAdvertisedService(movementService);

  // add the characteristic to the service
  movementService.addCharacteristic(movementCharacteristic);

  // add service
  BLE.addService(movementService);

  // set the initial value for the characeristic:
  movementCharacteristic.writeValue("");

  // start advertising
  BLE.advertise();

  //Serial.println("MKR RC Robot Peripheral");
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
      if (movementCharacteristic.written()) {
        
         command = movementCharacteristic.value();
         //Serial.print(F("commmand value:  "));
         //Serial.println(command);
         sendInstruction(command);
         
       }else{
        
        if (automaticMode == true)
          modoAutomatico();
         
       }

      //Keep active the communication MKR1010 & MKRMotorCarrier
      //Ping the samd11
      controller.ping();
      //wait
      delay(1);

  
    }

    // when the central disconnects, print it out:
    //Serial.print(F("Disconnected from central: "));
    //Serial.println(central.address());
  }

  //Keep active the communication MKR1010 & MKRMotorCarrier
  //Ping the samd11
  controller.ping();
  //wait
  delay(1);

}

void sendInstruction(String str) {

  if (str.length() == 0)
    return;
    
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
    if (str == "F")    //  Forward
    {
        duty1 = 100;
        duty2 = 100;
    }
    else if (str == "B") // Backward 
    {
        duty1 = -100;
        duty2 = -100;
    }
    else if (str == "L") // Turn Left  
    {
        duty1 = -50;
        duty2 = 50;
    }
    else if (str == "R") // Turn Right 
    {
        duty1 = 50;
        duty2 = -50;
    }
    else if (str == "S") // Stop 
    {
        duty1 = 0;
        duty2 = 0;
      
    }
    else if (str == "A") // Automatic Mode 
    {
        automaticMode = true;
        duty1 = 0;
        duty2 = 0;
    }
    else if (str == "M") // Manual Mode 
    {
        servo2.setAngle(90);
        delay(50);
        automaticMode = false;
        duty1 = 0;
        duty2 = 0;
    }
    else
    {
        duty1 = 0;
        duty2 = 0;
    }


    //Serial.print(F("Mode: "));
    //Serial.print(automaticMode);
    //Serial.print(F("valor en duty1: "));
    //Serial.print(duty1);
    //Serial.print(F("\t valor en duty2: "));
    //Serial.println(duty2);

    if (automaticMode == true){
      modoAutomatico();
    }else{
      M1.setDuty(duty1);
      M3.setDuty(duty1);
      M2.setDuty(duty2); 
      M4.setDuty(duty2); 
    }
       
    
  }
 

}


long measureDistance() {
  
  // The PING is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIGGER, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH);
  
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  long cm = duration / 29 / 2;  // microsecondsToCentimeter

  return cm;

}



void modoAutomatico()
{
    unsigned int distancia_medida = 0;

    distancia_medida = measureDistance();

  
    while ((distancia_medida < MIN_DISTANCIA) && (distancia_medida !=0)) 
    {

      M1.setDuty(-50);
      M3.setDuty(-50);
      M2.setDuty(50); 
      M4.setDuty(50);
       
      delay(80);
     
      distancia_medida = measureDistance();
     
    }

    M1.setDuty(100);
    M3.setDuty(100);
    M2.setDuty(100); 
    M4.setDuty(100);  
    
    moverServo();
    delay(30);
     
}

void moverServo()
{

    if (servopos == 40)
      direccionServo = 1;
    else if (servopos == 140)
      direccionServo = 2;
    
    if (direccionServo == 1)
      servopos+=10;
    else if (direccionServo == 2)
      servopos-=10;
    
    servo2.setAngle(servopos);
    delay(15);
    
}
