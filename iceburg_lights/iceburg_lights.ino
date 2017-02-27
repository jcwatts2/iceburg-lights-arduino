#include <Adafruit_NeoPixel.h>
#include <pt.h>
#include "StateIndicator.h"

#define NUM_OF_PIXELS 24
#define PROXIMITY_FACET_NUMBER 6
#define IDLE_DELAY 60
#define NUM_OF_FACETS 6


static struct pt idleDraw, proximityDraw, touchedDraw, correspondingDraw;//, stateChanged;

struct Color {
  int red;
  int green;
  int blue;
};

class Facet;

class State {
  
  private:
      State* touchedState;
      State* idleState;
      State* proximityState;
      State* correspondingTouchState;
  
  public: 
    virtual void handleDraw(Facet* facet, StateIndicator stateToDraw) {
      Serial.print("State CLASS handle draw\n");
    }
    virtual void handleChangeTo(Facet* facet) {}
    virtual int getStateIndicator() {}
    
    void setOtherStates(State* touched, State* idle, State* proximity, State* correspondingTouch) {              
       touchedState = touched;
       idleState = idle;
       proximityState = proximity;
       correspondingTouchState = correspondingTouch;
    }
    
    State* handleChange(Facet* facet, StateIndicator toState) {
           
       State* finalState = this;
              
       switch(toState) {
                    
             case PROXIMITY:
                finalState = proximityState;
                break;
                 
             case CORRESPONDING_TOUCH:
                finalState = correspondingTouchState;
                break;
                
             case IDLE:
                finalState = idleState;  
                break;
                
             case TOUCHED:
                finalState = touchedState;
                break;                
       }
       finalState->handleChangeTo(facet);
       return finalState;
    }
};

class Facet {
  Color facetColor;
  Adafruit_NeoPixel* ring;
  State* state;
  int pinNumber;
  int fade;
  int fadeCount;

  public:
      Facet(State* initState, int pin, int red, int green, int blue);
      void handleDraw(StateIndicator stateToDraw);
      void handleChanged(int changeIndicator);
      void init();
      Color* getColor();
      Adafruit_NeoPixel* getRing();
      void showAll(float red, float green, float blue);
      void showAll(Color* color);
      
      void setFadeIn() {
        fade = 1;
      }
      void setFadeOut() {
        fade = 0;
      }
      int getFadeIn() {
        return fade;
      }
      int getFadeCount() {
        return fadeCount;
      }
      void setFadeCount(int f) {
         fadeCount = f;
      }
};

class Idle : public State {
  
    public:
        
        void handleDraw(Facet* facet, StateIndicator stateToDraw) {
             
             if (IDLE == stateToDraw) {
                 Color* color = facet->getColor();
             
                 for (uint16_t i = 0; i < facet->getRing()->numPixels(); i++) {
                    facet->getRing()->setPixelColor(i, color->red, color->green, color->blue); 
                 }
                 facet->getRing()->show();
             }
        }
};

class Proximity : public State {
  
    int offset = 1;
    int swipeLength = 12;
   
    public:
      Proximity() {
      }
      
      void handleChangeTo(Facet* facet) {
          facet->setFadeCount(0);
          facet->setFadeOut();
      }
      
      void handleDraw(Facet* facet, StateIndicator stateToDraw) {
          
          if (PROXIMITY != stateToDraw) {
              return;
          }
          
          Adafruit_NeoPixel* ring = facet->getRing();
          Color* color = facet->getColor(); 
         
          if (facet->getFadeCount() >= 256) {
            facet->setFadeOut();
            facet->setFadeCount(255);
            
          } else if (facet->getFadeCount() <= 0) {
            facet->setFadeIn();
            facet->setFadeCount(2);
          
          } else if (facet->getFadeIn()) {
             facet->setFadeCount(facet->getFadeCount() + 1);
             
          } else {
             facet->setFadeCount(facet->getFadeCount() - 2);
          }
          
          float k = facet->getFadeCount() / 256.0;
          
          facet->showAll((k * color->red), (k * color->green), (k * color->blue));
      }
};

class Touched : public State {
       
     public:
         
         void handleChangeTo(Facet* facet) {
             for (uint16_t i = 0; i < 5; i++) {
                 facet->showAll(facet->getColor());
                 delay(100);
                 facet->showAll(0, 0, 0);
                 delay(100);
             }
             facet->showAll(facet->getColor());
         }
    
         void handleDraw(Facet* facet, StateIndicator stateToDraw) {
                      
             if (TOUCHED == stateToDraw) {
                 facet->showAll(facet->getColor());
             }
         }
};

class CorrespondingTouch : public State {
  
   private:
      Color touchedColor = {250, 215, 0}; 
  
   public:
         void handleChangeTo(Facet* facet) {
             for (uint16_t i = 0; i < 5; i++) {
                 facet->showAll(&touchedColor);
                 delay(100);
                 facet->showAll(0, 0, 0);
                 delay(100);
             }
             facet->showAll(&touchedColor);
         }
    
         void handleDraw(Facet* facet, StateIndicator stateToDraw) {
                      
             if (CORRESPONDING_TOUCH == stateToDraw) {
                 facet->showAll(&touchedColor);
             }
         }
};

Facet::Facet(State* initState, int pin, int red, int green, int blue) {
      ring = new Adafruit_NeoPixel(NUM_OF_PIXELS, pin, NEO_GRB + NEO_KHZ800);
      pinNumber = pin;
      facetColor = {red, green, blue};
      state = initState;
}
    
void Facet::handleDraw(StateIndicator stateToDraw) {
    state->handleDraw(this, stateToDraw);
}

void Facet::handleChanged(int changeIndicator) {
  
Serial.print("change indicator ");
Serial.println(changeIndicator);
    state = state->handleChange(this, (StateIndicator)changeIndicator);
}
    
void Facet::init() {
    pinMode(pinNumber, OUTPUT);
    ring->begin();
    ring->clear();
    ring->show();
}
    
Color* Facet::getColor() {
    return &facetColor;
}
    
Adafruit_NeoPixel* Facet::getRing() {
    return ring;
}

void Facet::showAll(float red, float green, float blue) {
  
    for (uint16_t i = 0; i < ring->numPixels(); i++) {
        ring->setPixelColor(i, red, green, blue); 
    }
    ring->show();
}

void Facet::showAll(Color* color) {
   showAll(color->red, color->green, color->blue); 
}

Facet* facets[6];

void handleDraw(StateIndicator stateToDraw) {
  
    for (int i = 0; i < 6; i++) {
        facets[i]->handleDraw(stateToDraw);
    }
}

static int drawIdle(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(IDLE);
    }
    PT_END(pt);
}

static int drawProximity(struct pt *pt, int interval) {
  
    static unsigned long timestamp = 0;
 
    PT_BEGIN(pt);
 
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval);
        timestamp = millis(); // take a new timestamp
        handleDraw(PROXIMITY);
    }
    PT_END(pt);
}

static int drawTouch(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(TOUCHED);
    }
    PT_END(pt);
}

static int drawCorresponding(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(CORRESPONDING_TOUCH);
    }
    PT_END(pt);
}

void handleChange(int number, int state) {
    if (number > NUM_OF_FACETS) {
      return;
    }
    
    if (state < 0 || state > 3) {
       return;
    }
    
    if (number == NUM_OF_FACETS) { //proximity facet indicator
       for (int i = 0; i < NUM_OF_FACETS; i++) {
         facets[i]->handleChanged(state);
       }       
    } else {
       facets[number]->handleChanged(state); 
    }
}

void fireworks() {
  
}

void setup() {
  PT_INIT(&idleDraw);
  PT_INIT(&proximityDraw);
  PT_INIT(&touchedDraw);
  PT_INIT(&correspondingDraw);

  Serial.begin(9600); //open serial port for incoming commands
  
  Idle* idleState = new Idle();
  Proximity* proximityState = new Proximity();
  Touched* touchedState = new Touched();
  CorrespondingTouch* cpTouchState = new CorrespondingTouch();

  idleState->setOtherStates(touchedState, idleState, proximityState, cpTouchState);             
  proximityState->setOtherStates(touchedState, idleState, proximityState, cpTouchState);
  touchedState->setOtherStates(touchedState, idleState, proximityState, cpTouchState);
  cpTouchState->setOtherStates(touchedState, idleState, proximityState, cpTouchState);
   
  facets[0] = new Facet(idleState, 7, 0, 0, 255);//blue 0,0,255: KEEP
  facets[1] = new Facet(idleState, 6, 0, 60, 255);//made up color 0,60,255
  facets[2] = new Facet(idleState, 5, 0, 97, 255);//made up: 0,97,255: KEEP
  facets[3] = new Facet(idleState, 4, 0, 127, 255);//don't know name-bluish aqua 0,127,255: KEEP
  facets[4] = new Facet(idleState, 3, 40, 191, 255);//made up 40,191,255
  facets[5] = new Facet(idleState, 2, 30, 144, 255);//dodger blue 30,144,255: KEEP
 
  for (int i = 0; i < 6; i++) {
      facets[i]->init();
  }
  
  Serial.setTimeout(50);
}

void loop() {

    while (Serial.available() > 0) {
        int number = Serial.parseInt();
        int state = Serial.parseInt();
        
        Serial.print(number);
        Serial.print(":");
        Serial.println(state);
        
        if (state == THE_MAIN_EVENT) {
            
            fireworks();
            
        } else {
            handleChange(number, state);
        }
    }

    drawIdle(&idleDraw, 200);
    drawProximity(&proximityDraw, 10);
    drawTouch(&touchedDraw, 900);
    drawCorresponding(&correspondingDraw, 900);

}
