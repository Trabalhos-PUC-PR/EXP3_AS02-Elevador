#include <Adafruit_NeoPixel.h>

// pinos in e out
#define in A0
#define callIn A1
#define button 2
#define callButton 3
#define LED_PIN   13
#define LED_COUNT 10

// "elevador" em si
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// estados e atributos do elevador
#define elevatorEmergencyLED 4
#define elevatorOpenLED 5
#define elevatorOperatingLED 6
int currentFloor = 0;
int queuedFloor = currentFloor;

#define elevatorOffline strip.Color(100, 100, 100)
#define elevatorOpenDoor strip.Color(0, 0, 255)
#define elavatorOperating strip.Color(255, 255, 0)
#define elevatorEmergency strip.Color(255, 0, 0)
uint32_t elevatorState = elevatorOpenDoor;

// se está no estado de emergência
bool emergencyState = false;
// se está ligado ou desligado
bool elevatorOnline = true;
// se o elevador está se movendo
bool elevatorMoving = false;
// se ele está fazendo uma parada rapida
bool elevatorOnHold = false;
bool elevatorPreparing = false;

bool newCall = false;
// se eu for implementar esse onhold, VAI SER BEM DEPOIS!
int callOnHold[10];
int floorCalling = 0;
bool floorCallUp = false;
bool floorCallDown = false;

bool movingUp = false;
bool movingDown = false;

// ms pro elevador ir de andar para andar
int elevatingTime = 500;
// ms pra porta fechar (o professor pediu, tenho q implementar de algm forma)
#define staticClosingTime 500 
int doorClosingTime = staticClosingTime;
int alignDelay = 1500;
unsigned long currentTime = 0;
unsigned long startTime = 0;

unsigned long currentStop = 0;
unsigned long startStop = 0;


void setup()
{
  Serial.begin(9600);
  pinMode(in, INPUT);
  pinMode(callIn, INPUT);
  pinMode(button, INPUT);
  pinMode(callButton, INPUT);

  pinMode(elevatorEmergencyLED, OUTPUT);
  pinMode(elevatorOpenLED, OUTPUT);
  pinMode(elevatorOperatingLED, OUTPUT);

  strip.begin();
  strip.show();
  strip.setBrightness(50);

  attachInterrupt(digitalPinToInterrupt(button), changeLed, RISING);
  attachInterrupt(digitalPinToInterrupt(callButton), floorCall, RISING);
  
  digitalWrite(elevatorOpenLED, HIGH);
}

void loop()
{
  refreshDisplay();
  printAll();

  if(elevatorOnline){
    if(emergencyState){
      MAYDAY();
    }else{
      if(newCall && currentFloor == floorCalling){
        onHold();
      }else{
        if(newCall && currentFloor != floorCalling){
          moveTo(floorCalling);
        }else{
          if(queuedFloor != currentFloor){
            moveTo(queuedFloor);
          }else{
            if(elevatorMoving){
              onStandBy();
            }
          }
        }
      }
    }
  }
}
  
void refreshDisplay(){
  strip.clear();
  if(elevatorOnline){
    if(newCall){
      strip.setPixelColor(floorCalling, strip.Color(255, 100, 255)); 
    }
    strip.setPixelColor(queuedFloor, strip.Color(255, 0, 255)); 
    strip.setPixelColor(currentFloor, elevatorState); 
  }
  strip.show();
}

void printAll(){
  // template pra print pq é chato digitar
  /*
    Serial.print("");
    Serial.println();
  */
  //Serial.println("-");
}

void changeLed(){
  if(!emergencyState && !elevatorMoving && elevatorOnline && !elevatorPreparing){
    switch(analogRead(in)){
      // 9° até o T
      case(1018): queuedFloor = 9; break;
      case(974):  queuedFloor = 8; break;
      case(930):  queuedFloor = 7; break;
      case(890):  queuedFloor = 6; break;
      case(853):  queuedFloor = 5; break;
      case(818):  queuedFloor = 4; break;
      case(787):  queuedFloor = 3; break;
      case(758):  queuedFloor = 2; break;
      case(731):  queuedFloor = 1; break;
      case(706):  queuedFloor = 0; break;
    }
  }
  switch(analogRead(in)){
    // diminui o delay p/ fechar a porta (exagerados 5s)
      case(682):
      if(queuedFloor != currentFloor){
        doorClosingTime = 0;
      }
      break;

      // emergencia
      case(660):
      emergencyState = true;
      break;

      // ON | OFF
      case(639):
      case(620):
      switchPower();
      break;
  }
}

void floorCall(){
  switch(analogRead(callIn)){
   // 9° andar - Térreo | desce sobe, nessa ordem
   case 998: makeCall(9, true); break;
   case 974: makeCall(9,false); break; 
   case 952: makeCall(8, true); break;
   case 930: makeCall(8,false); break; 
   case 909: makeCall(7, true); break;
   case 890: makeCall(7,false); break; 
   case 871: makeCall(6, true); break;
   case 853: makeCall(6,false); break; 
   case 835: makeCall(5, true); break;
   case 818: makeCall(5,false); break;
   case 802: makeCall(4, true); break;
   case 787: makeCall(4,false); break; 
   case 772: makeCall(3, true); break;
   case 758: makeCall(3,false); break; 
   case 744: makeCall(2, true); break;
   case 731: makeCall(2,false); break; 
   case 718: makeCall(1, true); break;
   case 706: makeCall(1,false); break; 
   case 694: makeCall(0, true); break; 
   case 682: makeCall(0,false); break;
  }
}

void makeCall(int floor, bool goingUp){

  if(!newCall){ // colocar no else a lógica pra lista das novas paradas
    if(!elevatorMoving && !elevatorPreparing){
      queuedFloor = floor;
    }else{
      
      if(movingUp == goingUp){
        if(floor > currentFloor && floor != queuedFloor){
          newCall = true;
          floorCalling = floor;
        }
      }
      if(movingDown == !goingUp){
        if(floor < currentFloor && floor != queuedFloor){
          newCall = true;
          floorCalling = floor;
        }
      }
      
    }
  }
}

void moveTo(int destination){
  if(!elevatorMoving){
    if(startTime == 0){
      Serial.print("fechando");
      elevatorPreparing = true;
      startTime = millis();
    }
    currentTime = millis();
  }
  if (elevatorMoving || currentTime - startTime > doorClosingTime)
  {
    if(!elevatorMoving || startTime == 0){
      startTime = millis();
    }
    elevatorMoving = true;
    digitalWrite(elevatorOpenLED, LOW);
    digitalWrite(elevatorOperatingLED, HIGH);
    elevatorState = elavatorOperating;
    doorClosingTime = staticClosingTime;
    
    if(currentFloor > destination){ 
      movingDown  = true;
    }else{
      movingUp = true;
    }
/*
    Serial.print("startTime:..");
    Serial.println(startTime);
    Serial.print("currentTime:");
    Serial.println(currentTime);
    Serial.print("elapsed:....");
    Serial.println(currentTime - startTime);
*/
    currentTime = millis();

    if(currentTime - startTime > elevatingTime){
      startTime = 0;
      if(currentFloor > destination){
        currentFloor--;
        movingDown = true;
      }else{
        currentFloor++;
        movingUp = true;
      }
    }
  }
}

void onStandBy(){
  Serial.println("ALINHANDO...");
  delay(alignDelay);
  startTime = 0;
  elevatorState = elevatorOpenDoor;
  digitalWrite(elevatorOpenLED, HIGH);
  digitalWrite(elevatorOperatingLED, LOW);
  doorClosingTime = staticClosingTime;
  elevatorPreparing = false;
  elevatorMoving = false;
  movingUp = false;
  movingDown = false;
}

void onHold(){
  
  if(startStop == 0){
    Serial.println("ALINHANDO...");
    delay(alignDelay);
    startStop = millis();
  }
  
  elevatorOnHold = true;
  elevatorMoving = false;
  elevatorState = elevatorOpenDoor;
  digitalWrite(elevatorOpenLED, HIGH);
  digitalWrite(elevatorOperatingLED, LOW);

  currentStop = millis();
  
  if (currentStop - startStop > doorClosingTime){
    startStop = 0;
    elevatorOnHold = false;
    elevatorMoving = true;
    elevatorState = elavatorOperating;
    digitalWrite(elevatorOpenLED, LOW);
    digitalWrite(elevatorOperatingLED, HIGH);
	newCall = false;
  }
}		

void switchPower(){
  if(elevatorOnline){
    elevatorOnline = false;
    emergencyState = false;
    elevatorMoving = false;
    digitalWrite(elevatorOpenLED, LOW);
    digitalWrite(elevatorOperatingLED, LOW);
    digitalWrite(elevatorEmergencyLED, LOW);
    elevatorState = elevatorOffline;
  }else{
    elevatorOnline = true;
    elevatorState = elevatorOpenDoor;
    digitalWrite(elevatorOpenLED, HIGH);
  }
}

void MAYDAY(){
  Serial.println("EMERGENCIA! REINICIE O ELEVADOR PARA CONCERTAR O PROBLEMA!");
  digitalWrite(elevatorOperatingLED, LOW);
  digitalWrite(elevatorOpenLED, LOW);

  digitalWrite(elevatorEmergencyLED, HIGH);
  strip.setPixelColor(currentFloor, elevatorEmergency);
  strip.show();
  delay(250);
  strip.setPixelColor(currentFloor, strip.Color(0, 0, 0));
  strip.show();
  digitalWrite(elevatorEmergencyLED, LOW);
  delay(250);
}
