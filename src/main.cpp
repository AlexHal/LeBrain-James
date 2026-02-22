#include <Arduino.h>
#include <Controller.h>

Controller controller("LeBrain-James", "James000");

//Thrower ie motor 3
constexpr uint8_t M3_EN  = 3;
constexpr uint8_t M3_IN1 = 2;
constexpr uint8_t M3_IN2 = 8;
constexpr uint8_t M3_MIN_PWM = 90;

int8_t m3Speed  = 90; 
int8_t currentDegree = 0;
int8_t rpm = 200;

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
  int pwm = map(constrain((int)m3Speed, 0, 100), 0, 100, 0, 255); // change speed maybe?
  if (pwm > 0 && pwm < M3_MIN_PWM) pwm = M3_MIN_PWM;

  if (degree >= 90) {
    digitalWrite(M3_IN1, LOW);
    digitalWrite(M3_IN2, HIGH);
    analogWrite(M3_EN, pwm);
    delay(1000); //we will need more time 
  } else {
    digitalWrite(M3_IN1, LOW);
    digitalWrite(M3_IN2, HIGH);
    analogWrite(M3_EN, pwm);
    int delayTime = (60 / (rpm * m3Speed / 100 ))*(currentDegree / 360); 
    // delay based on time
    delay(delayTime);

    //brake till ready to shoot
    digitalWrite(M3_IN1, HIGH);
    digitalWrite(M3_IN2, HIGH);
    
  }

}

void motorThreeRelease(){
  //realease motor
  digitalWrite(M3_IN1, LOW);
  digitalWrite(M3_IN2, LOW);
  currentDegree = 0;
}


void presetOne(){
  currentDegree = 90;
  motorThreePullback();
  motorThreePullback();
}

void presetTwo(){
  currentDegree = 45;
  motorThreePullback();
  motorThreePullback();
}

void presetThree(){
  currentDegree = 30;
  motorThreePullback();
  motorThreePullback();
}

void presetFour(){
  currentDegree = 20;
  motorThreePullback();
  motorThreePullback();
}

void presetFive(){
  currentDegree = 10;
  motorThreePullback();
  motorThreePullback();
}

void setup () {
  Serial.begin(9600) ;
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  MotorThreeInit();

  //Shooting presents
  controller.registerButton("Preset 1", presetOne);
  controller.registerButton("Preset 4", presetTwo);
  controller.registerButton("Preset 3", presetThree);
  controller.registerButton("Preset 4", presetFour);
  controller.registerButton("Preset 5", presetFive);


  controller.registerSlider("Pullback Degree", 0, 90, 45, onDegree);
  controller.registerButton("PullBack", motorThreePullback);
  controller.registerButton("Release",  motorThreeRelease);


  controller.beginAP(true) ;
  // Connect to WiFi " RobotAP " then open :
  // http ://10.0.0.2
}

void loop() {
  controller.update ();
}