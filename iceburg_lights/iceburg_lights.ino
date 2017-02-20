
#include <Utility.h>
#include <TimedAction.h>

#define NUM_OF_PIXELS 24
#define PROXIMITY_FACET_NUMBER 6
#define IDLE_DELAY 60


TimedAction idle = TimeAction(100, idle);
TimedAction proximityAction = TimedAction(60, proximitySwirl);
TimedAction touchedAction = TimedAction(, touched);
TimedAction correspondingTouchAction = TimedAction(, correspondingTouch);


void loop() {
 
  while (Serial.available()) {
    
     int number = Serial.parseInt();
     int state = Serial.parseInt();
    
     updateState(number, state); 
  }
 
}

void correspondingTouch() {
  
}

void touched() {
  
}


void idleSwirl() {
  
}


void updateState() {
  
  
}


uint16_t colorWheel(ring, uint32_t onC, uint32_t offC, uint16_t swLength, uint16_t head, uint16_t offset, uint8_wait) {
 
   for (int16_t i = 0; i < offset; i++) {
        ring.setPixelColor(((head + i) % strip.numPixels()), onC);
        ring.setPixelColor(((head - (swLength - i)) % strip.numPixels()), offC);
   } 
   strip.show();
   return ((head + offset) % strip.numPixels());
}
