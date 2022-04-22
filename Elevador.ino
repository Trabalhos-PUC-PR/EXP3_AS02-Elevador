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
uint32_t elevatorState = strip.Color(255, 0, 0);
// se está no estado de emergência
bool emergencyState = false;
// se está ligado ou desligado
bool elevatorOnline = true;
// se o elevador está se movendo
bool elevatorMoving = false;

bool newCall = false;
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

  // checks do estado do elevador,
  // dependendo do estado, ele pode ou não pode
  // realizar certas ações
  // coloquei eles no loop em vez de um while pra ter acesso
  // aos interrupts em qualquer momento
  if(elevatorOnline){
    if(emergencyState){
      MAYDAY();
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


  Serial.println("-");
}

void changeLed(){
  if(!emergencyState && !elevatorMoving && elevatorOnline){
    switch(analogRead(in)){
      // Do Térreo ao 9° andar
      case(787):queuedFloor = 0; break;
      case(127):queuedFloor = 1; break;
      case(64): queuedFloor = 2; break;
      case(43):	queuedFloor = 3; break;
      case(32): queuedFloor = 4; break;
      case(26): queuedFloor = 5; break;
      case(21): queuedFloor = 6; break;
      case(18): queuedFloor = 7; break;
      case(16): queuedFloor = 8; break;
      case(14): queuedFloor = 9; break;

      // diminui o delay p/ fechar a porta (exagerados 5s)
      case(13):
      if(queuedFloor != currentFloor){
        doorClosingTime = 0;
      }
      break;

      // emergencia
      case(12): emergencyState = true; break;

      // ON (só vou usar esse botão)
      case(11):
      case(10):
      switchPower();
      break;
    }
  }else{
    
    if(analogRead(in) == 11){
      switchPower();
    }
    
    if(analogRead(in) == 12 && elevatorOnline){
      emergencyState = true;	
    }
    
  }
}

void floorCall(){
  switch(analogRead(callIn)){
   // 9° andar - Térreo | desce sobe, nessa ordem
   case  15: makeCall(9,false); break; 
   case  16: makeCall(9, true); break;
   case  17: makeCall(8,false); break; 
   case  18: makeCall(8, true); break;
   case  19: makeCall(7,false); break; 
   case  21: makeCall(7, true); break;
   case  22: makeCall(6,false); break; 
   case  24: makeCall(6, true); break;
   case  26: makeCall(5,false); break;
   case  28: makeCall(5, true); break;
   case  31: makeCall(4,false); break; 
   case  34: makeCall(4, true); break;
   case  38: makeCall(3,false); break; 
   case  44: makeCall(3, true); break;
   case  51: makeCall(2,false); break; 
   case  61: makeCall(2, true); break;
   case  77: makeCall(1,false); break; 
   case 102: makeCall(1, true); break;
   case 152: makeCall(0,false); break;
   case 293: makeCall(0, true); break; 
  }
}

void makeCall(int floor, bool goingUp){
  if(elevatorMoving){
    if(movingUp == goingUp){
      if(floor > queuedFloor){
        newCall = !newCall;
      }
    }
  }else{
    queuedFloor = floor;
  }
}

void moveTo(int destination){
  if(!elevatorMoving){
    if(startTime == 0){
      Serial.print("fechando");
      startTime = millis();
    }
    // get the current "time" 
    // (actually the number of milliseconds since the program started)
    currentTime = millis();
  }
  //test whether the period has elapsed
  if (elevatorMoving || currentTime - startTime > doorClosingTime)
  {
    if(!elevatorMoving || startTime == 0){
      startTime = millis();
    }
    elevatorMoving = true;
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, HIGH);

    Serial.print("startTime:..");
    Serial.println(startTime);
    Serial.print("currentTime:");
    Serial.println(currentTime);
    Serial.print("elapsed:....");
    Serial.println(currentTime - startTime);

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
  digitalWrite(elevatorOpen, HIGH);
  digitalWrite(elevatorOperating, LOW);
  doorClosingTime = staticClosingTime;
  elevatorMoving = false;
  movingUp = false;
  movingDown = false;
}

void switchPower(){
  if(elevatorOnline){
    elevatorOnline = false;
    emergencyState = false;
    elevatorMoving = false;
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, LOW);
    digitalWrite(elevatorEmergency, LOW);
  }else{
    elevatorOnline = true;
    digitalWrite(elevatorOpen, HIGH);
  }
}

void MAYDAY(){
  Serial.println("EMERGENCIA! REINICIE O ELEVADOR PARA CONCERTAR O PROBLEMA!");
  digitalWrite(elevatorOperating, LOW);
  digitalWrite(elevatorOpen, LOW);

  digitalWrite(elevatorEmergency, HIGH);
  delay(250);
  digitalWrite(elevatorEmergency, LOW);
  delay(250);
}
