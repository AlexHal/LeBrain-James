#include <Arduino.h>
#include <Controller.h>

volatile bool playSound = false;


Controller controller("LeBrain-James", "James000");

//Thrower ie motor 3
constexpr uint8_t M3_EN  = 3;
constexpr uint8_t M3_IN1 = 2;
constexpr uint8_t M3_IN2 = 8;
constexpr uint8_t M3_MIN_PWM = 90;
constexpr uint8_t DAC_PIN = DAC; //TODO: set on board

int8_t m3Speed  = 100; 

int currentDegree = 0;
int rpm = 150; // Maybe uint8_t ?  //adjust with new bat pack

void onDegree(int deg) {
    currentDegree = deg;
    Serial.print("[M3] Degree = "); Serial.println(deg);
}



void MotorThreeInit(){
  pinMode(2, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(3, OUTPUT);
}


//Setup motor 3
void setMotorThree(int8_t spd) {
    Serial.print("[M3] setMotorThree called: enabled=");
    //Serial.print(m3Enabled);
    Serial.print(" spd=");
    Serial.println(spd);

    String in1, in2;


    if (spd == 0) {
      Serial.println("[M3] → BRAKE");
      digitalWrite(M3_IN1, HIGH);
      digitalWrite(M3_IN2, HIGH);
      in1 = "HIGH";
      in2 = "HIGH";
      return;
    } else if (spd > 0 ) {
      digitalWrite(M3_IN1, HIGH);
      digitalWrite(M3_IN2, LOW);
      in1 = "HIGH";
      in2 = "LOW";
    } else {
      digitalWrite(M3_IN1, LOW);
      digitalWrite(M3_IN2, HIGH);
      in1 = "LOW";
      in2 = "HIGH";

      spd = -spd;
    }
    
        

    int pwm = map(constrain((int)spd, 0, 100), 0, 100, 0, 255);
    if (pwm > 0 && pwm < M3_MIN_PWM) pwm = M3_MIN_PWM;

    Serial.print("[M3] → IN1=");
    Serial.print(in1);
    Serial.print(" IN2=");
    Serial.print(in2);
    Serial.print(" pwd=");
    Serial.print(pwm);
    
    analogWrite(M3_EN, pwm);    
}

/**
 * input is from 0 to 90?
 */
void motorThreePullback(){
    int degree = currentDegree;
    int pwm = map(constrain((int)m3Speed, 0, 100), 0, 100, 0, 255);
    if (pwm > 0 && pwm < M3_MIN_PWM) pwm = M3_MIN_PWM;

    float actRPM    = (float)rpm * (float)m3Speed / 100.0f;
    float delayTime = ((float)degree / 360.0f) / (actRPM / 60.0f) * 1000.0f;

    Serial.println("[M3] --- motorThreePullback ---");
    Serial.print("[M3] degree=");      Serial.println(degree);
    Serial.print("[M3] m3Speed=");     Serial.println(m3Speed);
    Serial.print("[M3] rpm=");         Serial.println(rpm);
    Serial.print("[M3] pwm=");         Serial.println(pwm);
    Serial.print("[M3] actRPM=");      Serial.println(actRPM);
    Serial.print("[M3] delayTime=");   Serial.print(delayTime); Serial.println("ms");

    digitalWrite(M3_IN1, HIGH);
    digitalWrite(M3_IN2, LOW);
    analogWrite(M3_EN, pwm);
    Serial.println("[M3] Motor running...");

    delay((unsigned long)delayTime);

    digitalWrite(M3_IN1, HIGH);
    digitalWrite(M3_IN2, HIGH);
    Serial.println("[M3] Brake applied");
}

void motorThreeRelease(){
  //realease motor
  digitalWrite(M3_IN1, LOW);
  digitalWrite(M3_IN2, LOW);
  int pwm = map(constrain((int)m3Speed, 0, 100), 0, 100, 0, 255);
    if (pwm > 0 && pwm < M3_MIN_PWM) pwm = M3_MIN_PWM;
  analogWrite(M3_EN, pwm);
  currentDegree = 0;

  controller.triggerSound();
}


void presetOne(){
  currentDegree = 210;
  motorThreePullback();
  motorThreeRelease();
}

void presetTwo(){
  currentDegree = 20;
  motorThreePullback();
  motorThreeRelease();
}



extern "C" char* sbrk(int incr);

int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}


void display_freeram(){
  Serial.print(F("- SRAM left: "));
  Serial.println(freeRam());
}


void setup () {
  Serial.begin(9600);
  Serial.println("hi");





  //motor
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  MotorThreeInit();

  //Shooting presents
  controller.registerButton("Shoot", presetOne);
  controller.registerButton("Nudge", presetTwo);



  controller.registerSlider("Pullback Degree", 0, 250, 90, onDegree);
  controller.registerButton("PullBack", motorThreePullback);
  controller.registerButton("Release",  motorThreeRelease);


  controller.beginAP(true) ;
  // Connect to WiFi " RobotAP " then open :
  // http ://10.0.0.2
}

void loop() {
  controller.update ();
  //display_freeram();
}