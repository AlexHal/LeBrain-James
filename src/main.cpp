#include <Arduino.h>
#include <Controller.h>

Controller controller("LeBrain-James", "James000");

//Thrower ie motor 3
constexpr uint8_t M3_EN  = 3;
constexpr uint8_t M3_IN1 = 2;
constexpr uint8_t M3_IN2 = 8;
constexpr uint8_t M3_MIN_PWM = 90;

bool   m3Enabled = false;
int8_t m3Speed   = 0; 



//Setup motor 3
void setMotorThree(int8_t spd) {
    if (!m3Enabled || spd == 0) {
        digitalWrite(M3_IN1, HIGH);
        digitalWrite(M3_IN2, HIGH);
        analogWrite(M3_EN, 0);  
        return;
    }
    if (spd > 0) { digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW); }
    else         { digitalWrite(M3_IN1, LOW);  digitalWrite(M3_IN2, HIGH); spd = -spd; }

    int pwm = map(constrain((int)spd, 0, 100), 0, 100, 0, 255);
    if (pwm > 0 && pwm < M3_MIN_PWM) pwm = M3_MIN_PWM;
    analogWrite(M3_EN, pwm);
}

//Togle M3
void toggleM3() {
    m3Enabled = !m3Enabled;
    Serial.print("[M3] "); Serial.println(m3Enabled ? "ENABLED" : "DISABLED");
    setMotorThree(m3Speed);  

void onDrive(int8_t left, int8_t right) {
    m3Speed = (int8_t)(((int)left + (int)right) / 2);
    setMotorThree(m3Speed);
}




void setup () {
  Serial.begin(9600) ;
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  controller.registerDriveCallback(onDrive);
  controller.registerButton("M3", toggleM3);  // appears on the web UI



  controller.beginAP(true) ;
  // Connect to WiFi " RobotAP " then open :
  // http ://10.0.0.2
}
void loop() {
  controller.update ();
}