#include <Arduino.h>
#include <Controller.h>

Controller controller("RobotAP", "James000") ;

void setup () {
  Serial.begin(115200) ;
  controller.configureL298N(9, 7, 6, 10, 5, 4) ;
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs (1200);
  controller.enableStatusLED(LED_BUILTIN);

  controller.beginAP(true) ;
  // Connect to WiFi " RobotAP " then open :
  // http ://10.0.0.2
}
void loop() {
  controller.update ();
}