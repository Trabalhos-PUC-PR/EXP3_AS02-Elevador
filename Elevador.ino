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
#define elevatorEmergency 4
#define elevatorOpen 5
#define elevatorOperating 6
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

  pinMode(elevatorEmergency, OUTPUT);
  pinMode(elevatorOpen, OUTPUT);
  pinMode(elevatorOperating, OUTPUT);

  strip.begin();
  strip.show();
  strip.setBrightness(50);

  attachInterrupt(digitalPinToInterrupt(button), changeLed, RISING);
  attachInterrupt(digitalPinToInterrupt(callButton), floorCall, RISING);
  
  digitalWrite(elevatorOpen, HIGH);
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
  strip.setPixelColor(currentFloor, elevatorState); 
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
  if(!emergencyState && !elevatorMoving && elevatorOnline){
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

      // diminui o delay p/ fechar a porta (exagerados 5s)
      case(682):
      if(queuedFloor != currentFloor){
        //doorClosingTime /= 4;
        doorClosingTime = 0;
      }
      break;

      // emergencia
      case(660):
      emergencyState = true;
      break;

      // ON (só vou usar esse botão)
      case(639):
      case(620):
      switchPower();
      break;
    }
  }else{
    // caso o elevador esteja DESLIGADO, 
    // ele só pode executar uma ação (ligar)
    if(analogRead(in) == 639 || analogRead(in) == 620){
      switchPower();
    }
	// para ativar o estado de emergencia durante o movimento
    if(analogRead(in) == 660 && elevatorOnline){
      emergencyState = true;	
    }
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
    if(!elevatorMoving){
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
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, HIGH);
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
  digitalWrite(elevatorOpen, HIGH);
  digitalWrite(elevatorOperating, LOW);
  doorClosingTime = staticClosingTime;
  elevatorMoving = false;
  movingUp = false;
  movingDown = false;
}

void onHold(){
  Serial.println("ONHOLD!");
  elevatorOnHold = true;
  elevatorMoving = false;
  elevatorState = elevatorOpenDoor;
  digitalWrite(elevatorOpen, HIGH);
  digitalWrite(elevatorOperating, LOW);

  if(startStop == 0){
    startStop = millis();
  }
  currentStop = millis();
  
  if (currentStop - startStop > doorClosingTime){
    startStop = 0;
    elevatorOnHold = false;
    elevatorMoving = true;
    elevatorState = elevatorOperating;
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, HIGH);
	newCall = false;
  }
}		

void switchPower(){
  if(elevatorOnline){
    elevatorOnline = false;
    emergencyState = false;
    elevatorMoving = false;
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, LOW);
    digitalWrite(elevatorEmergency, LOW);
    elevatorState = elevatorOffline;
  }else{
    elevatorOnline = true;
    elevatorState = elevatorOpenDoor;
    digitalWrite(elevatorOpen, HIGH);
  }
}

void MAYDAY(){
  Serial.println("EMERGENCIA! REINICIE O ELEVADOR PARA CONCERTAR O PROBLEMA!");
  digitalWrite(elevatorOperating, LOW);
  digitalWrite(elevatorOpen, LOW);

  digitalWrite(elevatorEmergency, HIGH);
  strip.setPixelColor(currentFloor, elevatorEmergency);
  strip.show();
  delay(250);
  strip.clear();
  strip.show();
  digitalWrite(elevatorEmergency, LOW);
  delay(250);
}
