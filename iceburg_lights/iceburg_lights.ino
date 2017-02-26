#include <Adafruit_NeoPixel.h>
#include <pt.h>
#include "StateIndicator.h"

#define NUM_OF_PIXELS 24
#define PROXIMITY_FACET_NUMBER 6
#define IDLE_DELAY 60
#define NUM_OF_FACETS 6


static struct pt idleDraw, proximityDraw, touchedDraw, correspondingDraw, stateChanged;

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
 Serial.println("Moving to proximity");
                finalState = proximityState;
                break;
                 
             case CORRESPONDING_TOUCH:
 Serial.println("Moving to corresponding");
                finalState = correspondingTouchState;
                break;
                
             case IDLE:
 Serial.println("Moving to idle");
                finalState = idleState;  
                break;
                
             case TOUCHED:
 Serial.println("Moving to touch");
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
  int idleLead;

  public:
      Facet(State* initState, int pin, int red, int green, int blue);
      void handleDraw(StateIndicator stateToDraw);
      void handleChanged(int changeIndicator);
      void init();
      void resetLead();
      int getLead();
      void setLead(int lead);
      Color* getColor();
      Adafruit_NeoPixel* getRing();
      void showAll(int red, int green, int blue);
      void showAll(Color* color);
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
          facet->setLead(-1);
      }
      
      void handleDraw(Facet* facet, StateIndicator stateToDraw) {
          
          if (PROXIMITY != stateToDraw) {
              return;
          }
          
          Adafruit_NeoPixel* ring = facet->getRing();
          Color* color = facet->getColor(); 
          int lead = facet->getLead();
        
          if (lead == -1) {
              ring->clear();
   
              for (uint16_t i = 0; i < swipeLength; i++) {
                 ring->setPixelColor(i, color->red, color->green, color->blue); 
              }
              lead = swipeLength;
              
          } else {
          
              for (int16_t i = 0; i < offset; i++) {
                  ring->setPixelColor(((lead + i) % ring->numPixels()), color->red, color->green, color->blue);
                  ring->setPixelColor(((ring->numPixels() + (lead - (swipeLength - i))) % ring->numPixels()), 0, 0, 0);
              } 
          
              lead = ((lead + offset) % ring->numPixels());
          }
          facet->setLead(lead);
          ring->show();
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
};

Facet::Facet(State* initState, int pin, int red, int green, int blue) {
      ring = new Adafruit_NeoPixel(NUM_OF_PIXELS, pin, NEO_GRB + NEO_KHZ800);
      pinNumber = pin;
      facetColor = {red, green, blue};
      state = initState;
      idleLead = -1;
}
    
void Facet::handleDraw(StateIndicator stateToDraw) {
    state->handleDraw(this, stateToDraw);
}

void Facet::handleChanged(int changeIndicator) {
Serial.println("Handle Changed");
    state = state->handleChange(this, (StateIndicator)changeIndicator);
}
    
void Facet::init() {
    pinMode(pinNumber, OUTPUT);
    ring->begin();
    ring->clear();
    ring->show();
}
    
void Facet::resetLead() {
    idleLead = -1;
}
    
int Facet::getLead() {
    return idleLead;
}
    
void Facet::setLead(int lead) {
    idleLead = lead;
}
    
Color* Facet::getColor() {
    return &facetColor;
}
    
Adafruit_NeoPixel* Facet::getRing() {
    return ring;
}

void Facet::showAll(int red, int green, int blue) {
  
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
  
}

static int changeState(struct pt *pt, int interval) {
     
    static int to = 0;
  
    static unsigned long timestamp = 0;
  
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        
        if (to == 0) {
 Serial.println("To proximity");
            for (int i = 0; i < 6; i++) {
                facets[i]->handleChanged(PROXIMITY);
            }
            to = 1;
        } else if (to == 1) {
           facets[4]->handleChanged(TOUCHED);
           to = 0;
        }
    }
    PT_END(pt);
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

    //if (once == 0) {  
       //handleDraw(PROXIMITY);
       //  facets[0].handleDraw(PROXIMITY);
      //   facets[1].handleDraw(PROXIMITY);
    //}
    //once++;
    //while (Serial.available() > 0) {
    //    static int number = Serial.parseInt();
    //    static int state = Serial.parseInt();
    //    handleChange(number, state);
    //}

    drawIdle(&idleDraw, 200);
    drawProximity(&proximityDraw, 60);
    drawTouch(&touchedDraw, 900);
    drawCorresponding(&correspondingDraw, 900);
    
    changeState(&stateChanged, 12000);
    
}
