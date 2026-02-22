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
  digitalWrite(M3_IN1, LOW);
  digitalWrite(M3_IN2, HIGH);
  analogWrite(M3_EN, pwm);

  if (degree >= 90) {
    delay(1000); //we will need more time 
  } else {
    float actRPM = (float)rpm*(float)m3Speed / 100.0f;
    float delayTime = ((float)degree / 360.0f) / (actRPM / 60.0f) * 1000.0f; 
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
  motorThreeRelease();
}

void presetTwo(){
  currentDegree = 45;
  motorThreePullback();
  motorThreeRelease();
}

void presetThree(){
  currentDegree = 30;
  motorThreePullback();
  motorThreeRelease();
}

void presetFour(){
  currentDegree = 20;
  motorThreePullback();
  motorThreeRelease();
}

void presetFive(){
  currentDegree = 10;
  motorThreePullback();
  motorThreeRelease();
}

// Roll forward configuration
// Calibrated: 0.5m → 0.46m, 0.25m → 0.20m, adjusted factor upward
constexpr int8_t ROLL_SPEED_LEFT = 45;   // Left motor speed (reduced to correct right curve)
constexpr int8_t ROLL_SPEED_RIGHT = 50;  // Right motor speed
constexpr float MS_PER_METER = 2475.0f;  // Calibrated timing (increased from 1880)

void rollForward(float distanceMeters) {
  Serial.print("[ROLL] Rolling forward ");
  Serial.print(distanceMeters);
  Serial.println("m...");
  
  // Calculate time based on calibrated speed
  uint16_t rollTimeMs = (uint16_t)(distanceMeters * MS_PER_METER);
  
  // Directly control motors via L298N pins
  // Left motor: ENA=9, IN1=7, IN2=6
  // Right motor: ENB=10, IN3=5, IN4=4
  
  // Set direction: forward
  digitalWrite(7, HIGH);  // IN1
  digitalWrite(6, LOW);   // IN2
  digitalWrite(5, HIGH);  // IN3
  digitalWrite(4, LOW);   // IN4
  
  // Set speed (with compensation for curve)
  int pwmLeft = map(ROLL_SPEED_LEFT, 0, 100, 0, 255);
  int pwmRight = map(ROLL_SPEED_RIGHT, 0, 100, 0, 255);
  analogWrite(9, pwmLeft);   // ENA - left motor
  analogWrite(10, pwmRight); // ENB - right motor
  
  // Roll for calculated time
  delay(rollTimeMs);
  
  // Stop both motors (brake)
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  analogWrite(9, 0);
  analogWrite(10, 0);
  
  Serial.println("[ROLL] Done.");
}

// Wrapper functions for UI buttons
void rollQuarterMeter() { rollForward(0.25f); }
void rollHalfMeter()    { rollForward(0.50f); }
void roll75cm()         { rollForward(0.75f); }
void rollOneMeter()     { rollForward(1.00f); }

void setup () {
  Serial.begin(9600) ;
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  MotorThreeInit();

  //Shooting presents
  controller.registerButton("Preset 1", presetOne);
  controller.registerButton("Preset 2", presetTwo);
  controller.registerButton("Preset 3", presetThree);
  controller.registerButton("Preset 4", presetFour);
  controller.registerButton("Preset 5", presetFive);
  
  // Movement
  controller.registerButton("Roll 0.25m", rollQuarterMeter);
  controller.registerButton("Roll 0.5m", rollHalfMeter);
  controller.registerButton("Roll 0.75m", roll75cm);
  controller.registerButton("Roll 1m", rollOneMeter);


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