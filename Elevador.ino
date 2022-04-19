#include <Adafruit_NeoPixel.h>

// pinos in e out
#define in A0
#define button 2
#define LED_PIN   13
#define LED_COUNT 8

// "elevador" em si
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// estados do elevador
#define elevatorEmergency 3
#define elevatorOpen 4
#define elevatorOperating 5
int currentFloor = 7;
int queuedFloor = currentFloor;
uint32_t elevatorState = strip.Color(255, 0, 0);
// se está no estado de emergência
bool emergencyState = false;
// se está ligado ou desligado
bool elevatorOnline = true;
// se o elevador está se movendo
bool elevatorMoving = false;

// ms pro elevador ir de andar para andar
int elevatingTime = 500;
// ms pra porta fechar (o professor pediu, tenho q implementar de algm forma)
int doorClosingTime = 5000;

void setup()
{
  Serial.begin(9600);
  pinMode(in, INPUT);
  pinMode(button, INPUT);
  
  pinMode(elevatorEmergency, OUTPUT);
  pinMode(elevatorOpen, OUTPUT);
  pinMode(elevatorOperating, OUTPUT);
  
  strip.begin();
  strip.show();
  strip.setBrightness(50);
  
  attachInterrupt(digitalPinToInterrupt(button), changeLed, RISING);

  digitalWrite(elevatorOpen, HIGH);
}

void loop()
{
  refreshDisplay();
  printAll();
}

void refreshDisplay(){
  strip.clear();
  strip.setPixelColor(currentFloor, elevatorState); 
  strip.show();
}

void printAll(){/*
  Serial.print("b: ");
  Serial.println(digitalRead(button));
  Serial.print("R: ");
  Serial.println(analogRead(in));//
  Serial.print("state: ");
  Serial.println(elevatorOnline);//
  Serial.print("emer: ");
  Serial.println(emergencyState);
  Serial.print("onli: ");
  Serial.println(elevatorOnline);	
  Serial.print("move: ");
  Serial.println(elevatorMoving);*/
}

void moveTo(int destination){
  delay(doorClosingTime);
  elevatorMoving = true;
  digitalWrite(elevatorOpen, LOW);
  digitalWrite(elevatorOperating, HIGH);
  while(destination != currentFloor){
    delay(elevatingTime);
    if(currentFloor > destination){
      currentFloor--;
    }else{
      currentFloor++;
    }
    refreshDisplay();
  }
  digitalWrite(elevatorOpen, HIGH);
  digitalWrite(elevatorOperating, LOW);
  elevatorMoving = false;
}

void changeLed(){
  Serial.print("check: ");
  Serial.println(!emergencyState && !elevatorMoving && elevatorOnline);
  if(!emergencyState && !elevatorMoving && elevatorOnline){
    switch(analogRead(in)){
      case(39): //andares, do 0 até o 7
        moveTo(0);
        //elevatorState = strip.Color(255, 0, 0);
      break;
      case(43):
        moveTo(1);
        //elevatorState = strip.Color(0, 255, 0);
      break;
      case(50):
        moveTo(2);
        //elevatorState = strip.Color(0, 0, 255);
      break;
      case(58):
        moveTo(3);
        //elevatorState = strip.Color(255, 0, 255);
      break;
      case(69):
        moveTo(4);
        //elevatorState = strip.Color(255, 255, 0);
      break;
      case(87):
        moveTo(5);
        //elevatorState = strip.Color(255, 255, 255);
      break;
      case(115):
        moveTo(6);
        //elevatorState = strip.Color(0, 255, 255);
      break;
      case(172):
        moveTo(7);
        //elevatorState = strip.Color(255, 100, 0);
      break;
      case(336):// nem deus sabe oqq isso faz
      break;
      case(35):	// fecha a porta ???????????????????
      break;
      case(31):	// emergencia
      if(!emergencyState){
          emergencyState = !emergencyState;
          MAYDAY();
      }
      break;
      case(29):	// ON (só vou usar esse botão)
      	elevatorOnline = false;
        digitalWrite(elevatorOpen, LOW);
        digitalWrite(elevatorOperating, LOW);
        digitalWrite(elevatorEmergency, LOW);
      break;
      case(27):	// OFF
      break;
    }
  }else{
    switch(analogRead(in)){
        case(29): // desliga o elevador
            elevatorOnline = !elevatorOnline;
            emergencyState = false;
            elevatorMoving = false;
            if(elevatorOnline){
                digitalWrite(elevatorOpen, HIGH);
            }
        break;
        case(31): // liga o modo de emergencia
            if(!emergencyState && elevatorOnline){
            	emergencyState = true;
            	MAYDAY();
            }
        break;
    }
  }
}

void MAYDAY(){
  digitalWrite(elevatorOperating, LOW);
  digitalWrite(elevatorOpen, LOW);
  while(emergencyState){
  	digitalWrite(elevatorEmergency, HIGH);
    delay(250);
  	digitalWrite(elevatorEmergency, LOW);
    delay(250);
  }
}
