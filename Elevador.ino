#include <Adafruit_NeoPixel.h>

// pinos in e out
#define in A0
#define button 2
#define LED_PIN   13
#define LED_COUNT 8

// "elevador" em si
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// estados e atributos do elevador
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
#define staticClosingTime 5000 
int doorClosingTime = staticClosingTime;
unsigned long currentTime = 0;
unsigned long startTime = 0;


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
        onStandBy();
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
  
  /*
  Serial.print("current: ");
  Serial.println(currentFloor);
  
  Serial.print("queued:  ");
  Serial.println(queuedFloor);
  
  Serial.print("Online:  ");
  Serial.println(elevatorOnline);
  */
  
  Serial.println("-");
}

void changeLed(){
  if(!emergencyState && !elevatorMoving && elevatorOnline){
    switch(analogRead(in)){
      // Cada andar 
      case(39): 
      queuedFloor = 0;
      break;
      case(43):
      queuedFloor = 1;
      break;
      case(50):
      queuedFloor = 2;
      break;
      case(58):
      queuedFloor = 3;
      break;
      case(69):
      queuedFloor = 4;
      break;
      case(87):
      queuedFloor = 5;
      break;
      case(115):
      queuedFloor = 6;
      break;
      case(172):
      queuedFloor = 7;
      break;
      
      // nem deus sabe oqq isso faz (botão extrema esquerda)
      case(336):
      break;
      
      // diminui o delay p/ fechar a porta (exagerados 5s)
      case(35):
      if(queuedFloor != currentFloor){
      	//doorClosingTime /= 4;
        doorClosingTime = 0;
      }
      break;
      
      // emergencia
      case(31):
      emergencyState = true;
      break;
      
      // ON (só vou usar esse botão)
      case(29):
      switchPower();
      break;
      
      // OFF (pra nn falar que eu nn fiz nada)
      case(27):
      switchPower();
      break;
    }
  }else{
    // caso o elevador esteja DESLIGADO, 
    // ele só pode executar uma ação (ligar)
    // especificamente o ON pra ligar YOO
    if(analogRead(in) == 29){
      switchPower();
    }
  }
}

void moveTo(int destination){
  
  /*
  Serial.print("startTime:..");
  Serial.println(startTime);
  Serial.print("currentTime:");
  Serial.println(currentTime);
  Serial.print("elapsed:....");
  Serial.println(currentTime - startTime);
  */
  
  if(!elevatorMoving){
    //IMPLEMENTAR UMA FUNC PRA FACILIDAR MINHA VIDA COM
    // MILLIS, TERMINO ISSO DPS
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
    elevatorMoving = true;
    digitalWrite(elevatorOpen, LOW);
    digitalWrite(elevatorOperating, HIGH);
    Serial.print("SUBINDO!");

    delay(elevatingTime);
    if(currentFloor > destination){
      currentFloor--;
    }else{
      currentFloor++;
    }
  }
}

void onStandBy(){
  startTime = 0;
  digitalWrite(elevatorOpen, HIGH);
  digitalWrite(elevatorOperating, LOW);
  doorClosingTime = staticClosingTime;
  elevatorMoving = false;
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
  digitalWrite(elevatorOperating, LOW);
  digitalWrite(elevatorOpen, LOW);

  digitalWrite(elevatorEmergency, HIGH);
  delay(250);
  digitalWrite(elevatorEmergency, LOW);
  delay(250);
}
