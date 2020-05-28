// ---------------------------------------------------------------------------
// Robot controlado remotamente por acelerometro:  - v1.0 - 15/11/2019
//
// AUTOR:
// Creado por Angel Villanueva - @avilmaru
//
// LINKS:
// Blog: http://www.mecatronicalab.es
//
//
// HISTORICO:
// 15/11/2019 v1.0 - Release inicial.
//
// ---------------------------------------------------------------------------

#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

const int R_LED_PIN = 22;
const int G_LED_PIN = 23;
const int B_LED_PIN = 24;

long tiempo_prev = 0;
float pitch_prev = 0;
float roll_prev = 0;

// Sensibilidad del giroscopio   ±2000 dps -> 70
// Sensibilidad del acelerómetro ±4g -> 0.122
const float A_S = 0.122;
const float G_S = 70.0;

void setup() {
  
  //Serial.begin(9600);
  //while (!Serial);

  setColor("RED");

 if (!IMU.begin()) {
    //Serial.println("Failed to initialize IMU!");
    while (1);
  }

  
  // begin ble initialization
  if (!BLE.begin()) {
    //Serial.println("starting BLE failed!");
    while (1);
  }

  //Serial.println("BLE Central - gesture control");

}

void loop() {
  
     connectToPeripheral();
}


void connectToPeripheral(){

  BLEDevice peripheral;

  do
  {
     // start scanning for peripherals
    BLE.scanForUuid(deviceServiceUuid);
    peripheral = BLE.available();
    
  } while (!peripheral);

  
  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    //Serial.print("Found  ");
    //Serial.print(peripheral.address());
    //Serial.print(" '");
    //Serial.print(peripheral.localName());
    //Serial.print("' ");
    //Serial.print(peripheral.advertisedServiceUuid());
    //Serial.println();
  
    // stop scanning
    BLE.stopScan();
  
    controlPeripheral(peripheral);
   
  }
  
}

void controlPeripheral(BLEDevice peripheral) {

  
  // connect to the peripheral
  //Serial.println("Connecting ...");

  if (peripheral.connect()) {
    //Serial.println("Connected");
    setColor("BLUE");
  } else {
    //Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  //Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    //Serial.println("Attributes discovered");
  } else {
    //Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic gestureCharacteristic = peripheral.characteristic(deviceServiceCharacteristicUuid);
    
  if (!gestureCharacteristic) {
    //Serial.println("Peripheral does not have gesture characteristic!");
    peripheral.disconnect();
    return;
  } else if (!gestureCharacteristic.canWrite()) {
    //Serial.println("Peripheral does not have a writable gesture characteristic!");
    peripheral.disconnect();
    return;
  }

  
  while (peripheral.connected()) {

    String command = calculoAngulos();

    //Serial.print("comando: ");
    //Serial.println(command);
      
    if (command != "")
    {
      setColor("GREEN");
      int n = command.length(); 
      // declaring character array 
      char char_array[n + 1]; 
      // copying the contents of the string to char array 
      strcpy(char_array, command.c_str()); 
      gestureCharacteristic.writeValue((const char*)char_array);
 
      delay(1);
    }
  
  }

  //Serial.println("Peripheral disconnected!");

}
  
String calculoAngulosConFiltroComplementario()
{
    float ax, ay, az;
    float gx, gy, gz;
    float pitch,_pitch;
    float roll,_roll;
    float dt;
    String _command = "";
    
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);
    }
    
    if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(gx, gy, gz);
    }
    
    // Ratios
    ax = ax/A_S;
    ay = ay/A_S;
    az = az/A_S;

    gx = gx/G_S;
    gy = gy/G_S;
    gz = gz/G_S;

    dt = (millis() - tiempo_prev) / 1000.0;
    tiempo_prev = millis();
  
    // Calcular los ángulos con el acelerometro
    _pitch = atan2(ax, sqrt(ay*ay + az*az));
    _roll =  atan2(ay, sqrt(ax*ax + az*az));
    
    // Convertimos de radianes a grados
    _pitch *= (180.0 / PI);
    _roll  *= (180.0 / PI);  
     
    // Aplicar el filtro complementario
    pitch = 0.98*(pitch_prev + gx*dt) + (0.02*_pitch);
    roll =  0.98*(roll_prev + gy*dt) + (0.02*_roll);

   //Serial.print(F("Rotacion en X:  "));
   //Serial.print(pitch);
   //Serial.print(F("\t Rotacion en Y: "));
   //Serial.println(roll);
   
    pitch_prev = pitch;
    roll_prev = roll;

    _command =  String(pitch, DEC) + "," + String(roll, DEC);
    
    return _command;
    
}

 
String calculoAngulos()
{
    float ax, ay, az;
    float gx, gy, gz;
    float pitch,roll;
    String _command = "";
    
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);
    }
    
    // Calcular los ángulos con el acelerometro
    pitch = atan2(ax, sqrt(ay*ay + az*az));
    roll =  atan2(ay, sqrt(ax*ax + az*az));
    
    // Convertimos de radianes a grados
    pitch *= (180.0 / PI);
    roll  *= (180.0 / PI);  
     
   //Serial.print(F("Rotacion en X:  "));
   //Serial.print(pitch);
   //Serial.print(F("\t Rotacion en Y: "));
   //Serial.println(roll);
   
    _command =  String(pitch, DEC) + "," + String(roll, DEC);
    
    return _command;
    
}


void setColor(String color)
{

  if (color == "RED")
  {
    digitalWrite(R_LED_PIN, LOW);  // High values -> lower brightness
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(B_LED_PIN, HIGH);
    
  }else if (color == "GREEN")
  {
    digitalWrite(R_LED_PIN, HIGH);  // High values -> lower brightness
    digitalWrite(G_LED_PIN, LOW);
    digitalWrite(B_LED_PIN, HIGH);
    
  }else if (color == "BLUE")
  {
    digitalWrite(R_LED_PIN, HIGH);  // High values -> lower brightness
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(B_LED_PIN, LOW);
    
  }else if (color == "BLACK")
  {
    digitalWrite(R_LED_PIN, HIGH);  // High values -> lower brightness
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(B_LED_PIN, HIGH);
  }        

}
