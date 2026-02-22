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
// Route speed (slowed down)
constexpr int8_t ROLL_SPEED_LEFT = 35;   // Left motor speed for route (reduced from 45)
constexpr int8_t ROLL_SPEED_RIGHT = 33;  // Right motor speed for route (reduced from 43)
constexpr float MS_PER_METER = 3200.0f;  // Calibrated timing (increased for slower speed)

// Manual roll speed (slower, more precise)
constexpr int8_t MANUAL_SPEED_LEFT = 25;   // Left motor speed for manual (slower)
constexpr int8_t MANUAL_SPEED_RIGHT = 23;  // Right motor speed for manual (reduced - was leaning left)
constexpr float MANUAL_MS_PER_METER = 4200.0f;  // Slower = more time per meter

// Rotate configuration
constexpr int8_t ROTATE_SPEED = 35;      // Motor speed for route rotation (reduced from 45)
constexpr float MS_PER_DEGREE = 16.0f;   // Time per degree of rotation (increased from 12)

// Manual rotate speed (slower)
constexpr int8_t MANUAL_ROTATE_SPEED = 30;      // Motor speed for manual rotation buttons
constexpr float MANUAL_MS_PER_DEGREE = 18.0f;   // Slower rotation timing

// Abort flag for route
volatile bool routeAborted = false;

// Stop all motors immediately
void stopMotors() {
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  analogWrite(9, 0);
  analogWrite(10, 0);
  Serial.println("[STOP] Motors stopped.");
}

// Abort the current route
void abortRoute() {
  routeAborted = true;
  stopMotors();
  Serial.println("[ABORT] Route aborted!");
}

// Delay that checks for abort and processes controller updates
// Returns true if aborted, false if delay completed normally
bool delayWithAbortCheck(uint16_t ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    controller.update();  // Process incoming requests (including abort)
    if (routeAborted) {
      stopMotors();
      return true;  // Aborted
    }
    delay(10);  // Small delay between checks
  }
  return false;  // Completed normally
}

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
  
  // Roll for calculated time (with abort check)
  if (delayWithAbortCheck(rollTimeMs)) {
    return;  // Aborted
  }
  
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

// Manual roll function (slower speed, accepts signed distance)
void rollManualSigned(int distanceCm) {
  if (distanceCm == 0) return;
  
  Serial.print("[ROLL] Manual roll ");
  Serial.print(distanceCm);
  Serial.println("cm...");
  
  uint16_t rollTimeMs = (uint16_t)(abs(distanceCm) / 100.0f * MANUAL_MS_PER_METER);
  
  // Set direction based on sign
  if (distanceCm > 0) {
    // Forward
    digitalWrite(7, HIGH);  // IN1
    digitalWrite(6, LOW);   // IN2
    digitalWrite(5, HIGH);  // IN3
    digitalWrite(4, LOW);   // IN4
  } else {
    // Backward
    digitalWrite(7, LOW);   // IN1
    digitalWrite(6, HIGH);  // IN2
    digitalWrite(5, LOW);   // IN3
    digitalWrite(4, HIGH);  // IN4
  }
  
  // Set slower speed for manual control
  int pwmLeft = map(MANUAL_SPEED_LEFT, 0, 100, 0, 255);
  int pwmRight = map(MANUAL_SPEED_RIGHT, 0, 100, 0, 255);
  analogWrite(9, pwmLeft);
  analogWrite(10, pwmRight);
  
  delay(rollTimeMs);
  
  // Stop (brake)
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  analogWrite(9, 0);
  analogWrite(10, 0);
  
  Serial.println("[ROLL] Done.");
}

// Roll button wrappers (8 buttons)
void rollFwd5()  { rollManualSigned(5); }
void rollFwd15() { rollManualSigned(15); }
void rollFwd25() { rollManualSigned(25); }
void rollFwd50() { rollManualSigned(50); }
void rollBwd5()  { rollManualSigned(-5); }
void rollBwd15() { rollManualSigned(-15); }
void rollBwd25() { rollManualSigned(-25); }
void rollBwd50() { rollManualSigned(-50); }

/**
 * Rotate the robot in place by the given degrees.
 * Positive degrees → turn right
 * Negative degrees → turn left
 */
void rotate(int16_t degrees) {
  if (degrees == 0) return;
  
  Serial.print("[ROTATE] Rotating ");
  Serial.print(degrees);
  Serial.println(" degrees...");
  
  // Calculate time based on absolute degrees
  uint16_t rotateTimeMs = (uint16_t)(abs(degrees) * MS_PER_DEGREE);
  
  // Set speed
  int pwm = map(ROTATE_SPEED, 0, 100, 0, 255);
  
  if (degrees > 0) {
    // Turn right: left forward, right backward
    digitalWrite(7, HIGH);  // IN1 - left forward
    digitalWrite(6, LOW);   // IN2
    digitalWrite(5, LOW);   // IN3 - right backward
    digitalWrite(4, HIGH);  // IN4
  } else {
    // Turn left: left backward, right forward
    digitalWrite(7, LOW);   // IN1 - left backward
    digitalWrite(6, HIGH);  // IN2
    digitalWrite(5, HIGH);  // IN3 - right forward
    digitalWrite(4, LOW);   // IN4
  }
  
  // Apply speed to both motors
  analogWrite(9, pwm);   // ENA - left motor
  analogWrite(10, pwm);  // ENB - right motor
  
  // Rotate for calculated time (with abort check)
  if (delayWithAbortCheck(rotateTimeMs)) {
    return;  // Aborted
  }
  
  // Stop both motors (brake)
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  analogWrite(9, 0);
  analogWrite(10, 0);
  
  Serial.println("[ROTATE] Done.");
}

// Manual rotate (slower speed for button control)
void rotateManual(int16_t degrees) {
  if (degrees == 0) return;
  
  Serial.print("[ROTATE] Manual rotating ");
  Serial.print(degrees);
  Serial.println(" degrees...");
  
  // Calculate time based on absolute degrees (slower)
  uint16_t rotateTimeMs = (uint16_t)(abs(degrees) * MANUAL_MS_PER_DEGREE);
  
  // Set slower speed
  int pwm = map(MANUAL_ROTATE_SPEED, 0, 100, 0, 255);
  
  if (degrees > 0) {
    // Turn right: left forward, right backward
    digitalWrite(7, HIGH);  // IN1 - left forward
    digitalWrite(6, LOW);   // IN2
    digitalWrite(5, LOW);   // IN3 - right backward
    digitalWrite(4, HIGH);  // IN4
  } else {
    // Turn left: left backward, right forward
    digitalWrite(7, LOW);   // IN1 - left backward
    digitalWrite(6, HIGH);  // IN2
    digitalWrite(5, HIGH);  // IN3 - right forward
    digitalWrite(4, LOW);   // IN4
  }
  
  // Apply speed to both motors
  analogWrite(9, pwm);   // ENA - left motor
  analogWrite(10, pwm);  // ENB - right motor
  
  // Rotate for calculated time
  delay(rotateTimeMs);
  
  // Stop both motors (brake)
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  analogWrite(9, 0);
  analogWrite(10, 0);
  
  Serial.println("[ROTATE] Done.");
}

// Rotate button wrappers (8 buttons) - use slower manual speed
void rotateR90() { rotateManual(90); }
void rotateR45() { rotateManual(45); }
void rotateR15() { rotateManual(15); }
void rotateR5()  { rotateManual(5); }
void rotateL90() { rotateManual(-90); }
void rotateL45() { rotateManual(-45); }
void rotateL15() { rotateManual(-15); }
void rotateL5()  { rotateManual(-5); }

// ============ ROUTE ============
void executeRoute() {
    routeAborted = false;  // Reset abort flag
    Serial.println("[ROUTE] Starting route...");

    // Step 1: Rotate 45 degrees right
    rotate(10);
    if (routeAborted) { stopMotors(); return; }
    if (delayWithAbortCheck(400)) return;  // Brief pause between movements

    // Step 2: Roll maybe a lil more
    rollForward(0.25f);
    // if (routeAborted) { stopMotors(); return; }

    // <50
    rotate(38);
    if (routeAborted) { stopMotors(); return; }
    if (delayWithAbortCheck(400)) return;

    rollForward(0.17f);
    // if (routeAborted) { stopMotors(); return; }

    rotate(3);
    if (routeAborted) { stopMotors(); return; }
    if (delayWithAbortCheck(400)) return;

    rollForward(0.27f);
    // if (routeAborted) { stopMotors(); return; }

    // <60
    rotate(-50);
    if (routeAborted) { stopMotors(); return; }
    if (delayWithAbortCheck(400)) return;

    rollForward(0.25f);
    if (routeAborted) { stopMotors(); return; }

    rotate(-20);
    if (routeAborted) { stopMotors(); return; }
    if (delayWithAbortCheck(400)) return;

    rollForward(0.25f);
    if (routeAborted) { stopMotors(); return; }

    Serial.println("[ROUTE] Route complete.");
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
  controller.registerButton("Preset 2", presetTwo);
  controller.registerButton("Preset 3", presetThree);
  controller.registerButton("Preset 4", presetFour);
  controller.registerButton("Preset 5", presetFive);
  
  // Rotation buttons (8 buttons)
  controller.registerButton("R +90", rotateR90);
  controller.registerButton("R +45", rotateR45);
  controller.registerButton("R +15", rotateR15);
  controller.registerButton("R +5", rotateR5);
  controller.registerButton("L -5", rotateL5);
  controller.registerButton("L -15", rotateL15);
  controller.registerButton("L -45", rotateL45);
  controller.registerButton("L -90", rotateL90);
  
  // Roll buttons (8 buttons)
  controller.registerButton("Fwd 5", rollFwd5);
  controller.registerButton("Fwd 15", rollFwd15);
  controller.registerButton("Fwd 25", rollFwd25);
  controller.registerButton("Fwd 50", rollFwd50);
  controller.registerButton("Bwd 5", rollBwd5);
  controller.registerButton("Bwd 15", rollBwd15);
  controller.registerButton("Bwd 25", rollBwd25);
  controller.registerButton("Bwd 50", rollBwd50);

  // Route
  controller.registerButton("Run Route", executeRoute);
  controller.registerButton("ABORT", abortRoute);


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