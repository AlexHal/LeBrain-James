#include <Arduino.h>
#include <Controller.h>

Controller controller("LeBrain-James", "James000");

//Thrower ie motor 3
constexpr uint8_t M3_EN  = 11;
constexpr uint8_t M3_IN1 = 12;
constexpr uint8_t M3_IN2 = 8;
constexpr uint8_t M3_MIN_PWM = 90;

bool   m3Enabled = false;
int8_t m3Speed  = 90; 



//Setup motor 3
void setMotorThree(int8_t spd) {
    Serial.print("[M3] setMotorThree called: enabled=");
    Serial.print(m3Enabled);
    Serial.print(" spd=");
    Serial.println(spd);

    String in1, in2;


    if (!m3Enabled) {
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

//Togle M3
void toggleM3() {
    m3Enabled = !m3Enabled;
    Serial.print("[M3] "); Serial.println(m3Enabled ? "ENABLED" : "DISABLED");
    setMotorThree(m3Speed);  
}




void setup () {
  Serial.begin(9600) ;
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  //controller.registerDriveCallback(onDrive);
  controller.registerButton("M3", toggleM3);  // appears on the web UI



  controller.beginAP(true) ;
  // Connect to WiFi " RobotAP " then open :
  // http ://10.0.0.2
}

void loop() {
  controller.update ();
}