/**************************************************************
  File:      Legolas_v1.ino
  Contents:  This program is designed to provide the functional 
             operation of the ME210 final project.  The demo
             is implemented using the State Driven Programming
             framework. 
  Notes:    Target: Arduino Uno R1 & R2
            Arduino IDE version: 1.0.6 & 1.5.8 BETA

  History:
  when      who  what/why
  ----      ---  -------------------------------------------
  02/23/15  BAA  program created
  03/04/15  BAA  vFinal established

**************************************************************/

/*---------------- Includes ---------------------------------*/
#include <Timers.h>
#include <Servo.h> 

/*---------------- Module Defines ---------------------------*/
//Componwnts
#define ONE_SEC                        1000
#define TIME_INTERVAL                  ONE_SEC
#define RIGHT_PWM                      124
#define LEFT_PWM                       122

//State
#define InitQuery_SM                   0
#define SearchBRB_Init_SM              1
#define BallCollect_SM1                2
#define BallCollect_SM2                3
#define BallCollect_SM3                4
#define ShootSearch_SM                 5
#define Shoot_SM                       6
#define SearchBRB_SM                   7
#define GameOver                       8
//Event
#define NoEvent                        0
#define Bump                           1
#define IR_Tripped                     2
#define InitialSearchTimerExpire       4 //From this point - timers expiring
#define TurningTimeExpire              5
#define RequestingTimeExpire1          6
#define RequestingTimeExpire2          7
#define RequestingTimeExpire3          8
#define ShootSearchTimerExpire         9
#define RightStrafeTimerExpire         10
#define LeftStrafeTimerExpire          11
#define ShootingTimerExpire            12
#define Shot_1_TimerExpire             13 
#define Shot_2_TimerExpire             14
#define Shot_3_TimerExpire             15
#define SearchBRB_SM_TimerExpire       16
#define InitialTimerExpire             17  
#define End                            18

// H bridge L293 pins 1-4
#define mtr_left_DirPin                8  //pin 1
#define mtr_left_EnablePin             6  //pin 2 PWM--All enable pins need to be hooked up to pulsating so that it can vary motor speed
#define mtr_right_DirPin               10 //pin 3
#define mtr_right_EnablePin            11 //pin 4 PWM
#define bumperPin                      A0
#define IR_Pin                         A2
#define Servo_Pin                      9
#define IR_LED                         13

//Fly Wheel Motor Board
#define fly_left_pin                   3
#define fly_right_pin                  5

#define SPEEDSCALER                    2

/*---------------- Global Function Prototypes ---------------*/
int bumperState = 0;  // variable to store the value coming from the sensor
int IR_State = 0;     // variable to store the value coming from the IR sensor
int Servo_Pos = 0;    // variable to store the servo position
unsigned char LegolasEVENTglobal = NoEvent;
Servo Legolas_Servo;  // create servo object to control a servo 
                      // a maximum of eight servo objects can be created 

/*---------------- Module Function Prototypes ---------------*/
unsigned char TestForKey(void);
void RespToKey(void);
boolean TestForBump();
boolean TestForIR();
boolean IR_activated();
boolean IR_deactivated();
boolean bumperPushed();
boolean bumperReleased();
void BumpingResponse (void);
void ServoShoot();
char RightMtrSpeed (char newSpeed);
char LeftMtrSpeed (char newSpeed);
void Flywheels_ON();
void Flywheels_OFF();

//HAVE NOT included any timers, or operating functions
unsigned char LegolasEVENTchecker (void);

/*---------------- Arduino Main Functions -------------------*/
void setup() {  // setup() function required for Arduino
  Serial.begin(9600);
  Serial.println("Starting LegolasPbRAD..");

//Setup pins to output mode
  pinMode(mtr_right_EnablePin, OUTPUT); 
  pinMode(mtr_right_DirPin, OUTPUT); 
  pinMode(mtr_left_EnablePin, OUTPUT); 
  pinMode(mtr_left_DirPin, OUTPUT);
  pinMode(IR_LED, OUTPUT); 
  pinMode(fly_left_pin, OUTPUT); 
  pinMode(fly_right_pin, OUTPUT); 
  RightMtrSpeed(0);
  LeftMtrSpeed(0);
  Legolas_Servo.attach(Servo_Pin);  // attaches the servo on pin 0 to the servo object
  TMRArd_InitTimer(15, 130*TIME_INTERVAL); // 2min10sec - robot will turn off
  TMRArd_InitTimer(13, TIME_INTERVAL);
}

void loop() {  // loop() function required for Arduino
  LegolasEVENTglobal = LegolasEVENTchecker();
  LegolasCASE(LegolasEVENTglobal);
}

/*---------------- Module Functions -------------------------*/

void LegolasCASE (unsigned char LegolasEVENT)
{ 
    static unsigned char CurrentState = InitQuery_SM;
    unsigned char NextState;
    Serial.print("Current state is: ");
    Serial.println(CurrentState, DEC);
    switch (CurrentState)
    {
        case InitQuery_SM  :
            switch(LegolasEVENT)
            {                  
                  case InitialTimerExpire:
                        Serial.println("Yo....");
                        TMRArd_InitTimer(0, TIME_INTERVAL); //InitialSearchTimer
                        CurrentState = SearchBRB_Init_SM;
                        RightMtrSpeed(0);
                        LeftMtrSpeed(0);
                        break;
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default: 
                        break;
            }
            break;
        case SearchBRB_Init_SM  :
           switch(LegolasEVENT)
           {
                  case InitialSearchTimerExpire:
                        //Serial.println("I looked for all the time I was supposed to look");
                        RightMtrSpeed(RIGHT_PWM); 
                        LeftMtrSpeed(LEFT_PWM); 
                        CurrentState = SearchBRB_Init_SM;
                        break;
                  case Bump: 
                        //Serial.println("bumping in searchbrbinit");
                        RightMtrSpeed(-RIGHT_PWM); 
                        LeftMtrSpeed(-LEFT_PWM); 
                        
                        TMRArd_InitTimer(2, 0.2*TIME_INTERVAL); //RequestingTimeExpire1
                        CurrentState = SearchBRB_Init_SM;
                        break;
                  case RequestingTimeExpire1: //Ball Collect Reverse
                       TMRArd_InitTimer(1, 0.9*TIME_INTERVAL); //TurningTimeExpire                       
                       BumpingResponse();
                       CurrentState = SearchBRB_Init_SM;
                       break; 
                  case TurningTimeExpire:
                        RightMtrSpeed(0); 
                        LeftMtrSpeed(0); 
                        CurrentState = SearchBRB_SM;
                        TMRArd_InitTimer(12, TIME_INTERVAL);
                        //Serial.println("I should stop turning about...now");
                        break;
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default: 
                        break;
           }
           break;
       case BallCollect_SM1  :           
           //Serial.println("I am in BallCollect_SM state");
           switch(LegolasEVENT)
           {
                  case RequestingTimeExpire1: //Ball Collect Reverse
                       TMRArd_InitTimer(3, 0.05*TIME_INTERVAL); //RequestingTimeExpire2                       
                       RightMtrSpeed(-0.5*RIGHT_PWM); 
                       LeftMtrSpeed(-0.6*LEFT_PWM); 
                       //Serial.println("Collecting Ball 1_Timer 1");
                       CurrentState = BallCollect_SM1;
                       break; 
                  case RequestingTimeExpire2: //Ball Collect Stop
                       TMRArd_InitTimer(4, TIME_INTERVAL); //RequestingTimeExpire3
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       //Serial.println("Collecting Ball 1_Timer 2");
                       CurrentState = BallCollect_SM1;
                       break; 
                  case RequestingTimeExpire3: //Ball Collect Forward
                       RightMtrSpeed(RIGHT_PWM); 
                       LeftMtrSpeed(LEFT_PWM); 
                       //Serial.println("Collecting Ball 1_Timer 3");
                       CurrentState = BallCollect_SM1;
                       break; 
                  case Bump:
                       TMRArd_InitTimer(2, TIME_INTERVAL); //RequestingTimeExpire1
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       //Serial.println("Collecting Ball 1_Timer 4");
                       CurrentState = BallCollect_SM2;
                       break; 
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default:
                       break;
           }
           break;
     case BallCollect_SM2  :           
           //Serial.println("I am in BallCollect2_SM state");
           switch(LegolasEVENT)
           {
                  case RequestingTimeExpire1: //Ball Collect Reverse
                       TMRArd_InitTimer(3, 0.1*TIME_INTERVAL); //RequestingTimeExpire2                       
                       RightMtrSpeed(-0.5*RIGHT_PWM); 
                       LeftMtrSpeed(-0.6*LEFT_PWM); 
                       //Serial.println("Collecting Ball 2");
                       CurrentState = BallCollect_SM2;
                       break; 
                  case RequestingTimeExpire2: //Ball Collect Stop
                       TMRArd_InitTimer(4, TIME_INTERVAL); //RequestingTimeExpire3
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       CurrentState = BallCollect_SM2;
                       break; 
                  case RequestingTimeExpire3: //Ball Collect Forward
                       RightMtrSpeed(RIGHT_PWM); 
                       LeftMtrSpeed(LEFT_PWM); 
                       CurrentState = BallCollect_SM2;
                       break; 
                  case Bump:
                       TMRArd_InitTimer(2, TIME_INTERVAL); //RequestingTimeExpire1
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       CurrentState = BallCollect_SM3;
                       break; 
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default:
                       break;
           }
           break;
       case BallCollect_SM3  :           
           //Serial.println("I am in BallCollect3_SM state");
           switch(LegolasEVENT)
           {
                   case RequestingTimeExpire1: //Ball Collect Reverse
                       TMRArd_InitTimer(3, 0.1*TIME_INTERVAL); //RequestingTimeExpire2                       
                       RightMtrSpeed(-0.5*RIGHT_PWM); 
                       LeftMtrSpeed(-0.6*LEFT_PWM); 
                       CurrentState = BallCollect_SM3;
                       break; 
                  case RequestingTimeExpire2: //Ball Collect Stop
                       TMRArd_InitTimer(4, TIME_INTERVAL); //RequestingTimeExpire3
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       //Serial.println("Collecting Ball 3");
                       CurrentState = BallCollect_SM3;
                       break; 
                  case RequestingTimeExpire3:
                       TMRArd_InitTimer(5, TIME_INTERVAL); //ShootSearchTimerExpire
                       CurrentState = ShootSearch_SM;
                       break; 
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default:
                       break;
           }
           break;
       case ShootSearch_SM  :
           //Serial.println("I am in ShootSearch_SM state");
           switch(LegolasEVENT)
           { 
                  case ShootSearchTimerExpire:
                       RightMtrSpeed(-0.5*RIGHT_PWM); 
                       LeftMtrSpeed(-0.6*LEFT_PWM); 
                       TMRArd_InitTimer(6, TIME_INTERVAL*5); //RightStrafeTimerExpire
                       CurrentState = ShootSearch_SM;
                       break; 
                  case RightStrafeTimerExpire:
                       RightMtrSpeed(0.5*RIGHT_PWM); 
                       LeftMtrSpeed(0.6*LEFT_PWM); 
                       TMRArd_InitTimer(7, TIME_INTERVAL*2); //LeftStrafeTimerExpire
                       CurrentState = ShootSearch_SM;
                       break; 
                  case LeftStrafeTimerExpire:
                       LeftMtrSpeed(0); 
                       RightMtrSpeed(0); 
                       TMRArd_InitTimer(8, TIME_INTERVAL); //ShootingTimerExpire
                       CurrentState = Shoot_SM;
                       break; 
                  case IR_Tripped:
                       LeftMtrSpeed(0); 
                       RightMtrSpeed(0); 
                       TMRArd_InitTimer(8, TIME_INTERVAL); //ShootingTimerExpire
                       Flywheels_ON();
                       CurrentState = Shoot_SM;
                       break;
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default:
                       break;
           }
           break;
        case Shoot_SM  :
           //Serial.println("I am in Shoot_SM state");
           Flywheels_ON();
           switch(LegolasEVENT)
           {            
                    case ShootingTimerExpire: //Delay for flywheels to speed up
                       TMRArd_InitTimer(9, TIME_INTERVAL); //Shot_1_TimerExpire 
                       CurrentState = Shoot_SM;
                       break;
                    case Shot_1_TimerExpire:
                       ServoShoot();
                       TMRArd_InitTimer(11, 2*TIME_INTERVAL); //Shot_3_TimerExpire <-- code modified to jump to Shot_3_TimerExpire:
                       CurrentState = Shoot_SM;
                       break;
                  case Shot_3_TimerExpire:
                       Flywheels_OFF();
                       TMRArd_InitTimer(12, TIME_INTERVAL); //SearchBRB_SMTimerExpire
                       CurrentState = SearchBRB_SM;
                       break; 
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default: 
                       break;
           }
           break;
        case SearchBRB_SM  :
        //Serial.println("I am in searchBRB state");
           switch(LegolasEVENT)
           {
                  case SearchBRB_SM_TimerExpire:
                       RightMtrSpeed(0.5*RIGHT_PWM); 
                       LeftMtrSpeed(0.6*LEFT_PWM); 
                       CurrentState = SearchBRB_SM;
                       break;
                  case Bump:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0); 
                       TMRArd_InitTimer(2, TIME_INTERVAL); //RequestingTimeExpire1
                       CurrentState = BallCollect_SM1;
                       //Serial.println("Bumped in reg search brb state");
                       break;  
                  case End:
                       RightMtrSpeed(0); 
                       LeftMtrSpeed(0);       
                       CurrentState = GameOver;
                       break;
                  default:
                       break;
           }     
           break;
           case GameOver  :
           Serial.println("Time's up :(");
                  switch(LegolasEVENT)
                  {
                  }
    }    
    return;
}
    
unsigned char LegolasEVENTchecker (void) {
   char LegolasEVENT = NoEvent; 
   
   IR_deactivated();
   bumperReleased();
   
   if (TMRArd_IsTimerExpired(15) == TMRArd_EXPIRED) {
     Serial.println("Game Over.");
     LegolasEVENT = End;
     TMRArd_ClearTimerExpired(15);
     return LegolasEVENT;
   }
   else {
   
  if (TestForIR()){
     LegolasEVENT = IR_Tripped;
   }
  
   if (TestForBump()){
     LegolasEVENT = Bump;
   } 
   
  if (TMRArd_IsTimerExpired(0) == TMRArd_EXPIRED) {
     Serial.println("Search Timer is actually done");
     LegolasEVENT = InitialSearchTimerExpire;
     TMRArd_ClearTimerExpired(0);
   }
   
   if (TMRArd_IsTimerExpired(1) == TMRArd_EXPIRED) {
     LegolasEVENT = TurningTimeExpire;
     TMRArd_ClearTimerExpired(1);
   }
  
   if (TMRArd_IsTimerExpired(2) == TMRArd_EXPIRED) {
     LegolasEVENT = RequestingTimeExpire1;
     TMRArd_ClearTimerExpired(2);
   }
   
   if (TMRArd_IsTimerExpired(3) == TMRArd_EXPIRED) {
     LegolasEVENT = RequestingTimeExpire2;
     TMRArd_ClearTimerExpired(3);
   }
  
   if (TMRArd_IsTimerExpired(4) == TMRArd_EXPIRED) {
     LegolasEVENT = RequestingTimeExpire3;
     TMRArd_ClearTimerExpired(4);
   }
   
   if (TMRArd_IsTimerExpired(5) == TMRArd_EXPIRED) {
     LegolasEVENT = ShootSearchTimerExpire;
     TMRArd_ClearTimerExpired(5);
   }
  
    if (TMRArd_IsTimerExpired(6) == TMRArd_EXPIRED) {
     LegolasEVENT = RightStrafeTimerExpire;
     TMRArd_ClearTimerExpired(6);
   }
   
   if (TMRArd_IsTimerExpired(7) == TMRArd_EXPIRED) {
     LegolasEVENT = LeftStrafeTimerExpire;
     TMRArd_ClearTimerExpired(7);
   }
   
   if (TMRArd_IsTimerExpired(8) == TMRArd_EXPIRED) {
     LegolasEVENT = ShootingTimerExpire;
     TMRArd_ClearTimerExpired(8);
   }
   
    if (TMRArd_IsTimerExpired(9) == TMRArd_EXPIRED) {
     LegolasEVENT = Shot_1_TimerExpire;
     TMRArd_ClearTimerExpired(9);
   }
   
   if (TMRArd_IsTimerExpired(10) == TMRArd_EXPIRED) {
     LegolasEVENT = Shot_2_TimerExpire;
     TMRArd_ClearTimerExpired(10);
   }
   
    if (TMRArd_IsTimerExpired(11) == TMRArd_EXPIRED) {
     LegolasEVENT = Shot_3_TimerExpire;
     TMRArd_ClearTimerExpired(11);
   }
   
   if (TMRArd_IsTimerExpired(12) == TMRArd_EXPIRED) {
     LegolasEVENT = SearchBRB_SM_TimerExpire;
     Serial.println("STOP LOOKING IN BRB");
     TMRArd_ClearTimerExpired(12);
   }
  
   if (TMRArd_IsTimerExpired(13) == TMRArd_EXPIRED) {
     LegolasEVENT = InitialTimerExpire;
     TMRArd_ClearTimerExpired(13);
   }  
   }
   
   Serial.print("Event: ");
   Serial.println(LegolasEVENT, DEC);  
   return LegolasEVENT;
}
boolean TestForBump(){
  if(bumperPushed()) return true;
  else return false;
}

boolean TestForIR() {
  if(IR_activated()) return true;
  else return false;
}

boolean IR_activated() {
  if (digitalRead(IR_Pin) == HIGH && IR_State == 0) {
    IR_State = 1;    // IR is triggered high
    Serial.println("Shoot");
    digitalWrite(IR_LED, HIGH);
    return true;
  } 
  return false;
}

boolean IR_deactivated() {
  if (digitalRead(IR_Pin) == LOW && IR_State == 1) {
    IR_State = 0;    // IR is triggered low
    Serial.println("Lost target.");
    return true;
  }
  return false;
}

boolean bumperPushed() {
  if (digitalRead(bumperPin) == HIGH && bumperState == 0) {
    bumperState = 1;    // bumper is pushed in
    Serial.println("bumper pushed");
    return true;
  } 
  return false;
}

boolean bumperReleased() {
  if (digitalRead(bumperPin) == LOW && bumperState == 1) {
    bumperState = 0;    // bumper has been released
    Serial.println("bumper released");
    return true;
  }
  return false;
}

void BumpingResponse (void) {
       LeftMtrSpeed(-0.5*LEFT_PWM); 
       RightMtrSpeed(0.5*RIGHT_PWM);   
        Serial.println("I hit a bump and imma reverse");
}

char LeftMtrSpeed(char newSpeed) {
  if (newSpeed < 0) {
    digitalWrite(mtr_left_DirPin, HIGH); // set the direction to reverse
  } else {
    digitalWrite(mtr_left_DirPin, LOW); // set the direction to forward
  }
  analogWrite(mtr_left_EnablePin, SPEEDSCALER*abs(newSpeed));
}

char RightMtrSpeed(char newSpeed) { //used for StrafeRightDriving
    if (newSpeed < 0) {
    digitalWrite(mtr_right_DirPin, HIGH); // set the direction to reverse
  } else {
    digitalWrite(mtr_right_DirPin, LOW); // set the direction to forward
  }
  analogWrite(mtr_right_EnablePin, SPEEDSCALER*abs(newSpeed));
} 

void ServoShoot() {
      for(int balls2shoot = 1; balls2shoot < 4; balls2shoot++){
          Serial.println("shooting");
          delay(2000);
          
      for(Servo_Pos = 90; Servo_Pos < 130; Servo_Pos += 1)  // goes from 0 degrees to 180 degrees 
          {                                                 // in steps of 1 degree 
          Legolas_Servo.write(Servo_Pos);                   // tell servo to go to position in variable 'pos' 
          delay(3);                                         // waits 15ms for the servo to reach the position 
          } 


      for(Servo_Pos = 130; Servo_Pos >= 90; Servo_Pos -=1)  // goes from 180 degrees to 0 degrees 
          {                                
          Legolas_Servo.write(Servo_Pos);     // tell servo to go to position in variable 'Servo_Pos' 
          delay(3);                           // waits 15ms for the servo to reach the position 
          } 
      }
}

void Flywheels_ON(){
  analogWrite(fly_left_pin, 170);
  analogWrite(fly_right_pin, 170);
}

void Flywheels_OFF(){
  analogWrite(fly_left_pin, 0);
  analogWrite(fly_right_pin, 0);
}

unsigned char TestForKey(void) {
  unsigned char KeyEventOccurred;
  
  KeyEventOccurred = Serial.available();
  return KeyEventOccurred;
}

void RespToKey(void) {
  unsigned char theKey;
  
  theKey = Serial.read();
  
  Serial.print(theKey);
  Serial.print(", ASCII=");
  Serial.println(theKey,HEX);
}
