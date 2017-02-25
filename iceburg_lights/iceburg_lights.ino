#include <Adafruit_NeoPixel.h>
#include <pt.h>
#include "StateIndicator.h"

#define NUM_OF_PIXELS 24
#define PROXIMITY_FACET_NUMBER 6
#define IDLE_DELAY 60
#define NUM_OF_FACETS 6


static struct pt idleDraw, proximityDraw, touchedDraw, correspondingDraw;

struct Color {
  int red;
  int green;
  int blue;
};

class Facet;

class State {
  public: 
    virtual void handleDraw(Facet* facet, StateIndicator stateToDraw) {
      Serial.print("State CLASS handle draw\n");
    }
    virtual State* handleChange(Facet* facet, int changeIndicator) {}
    virtual int getStateIndicator() {}
};

class Facet {
  Color facetColor;
  Adafruit_NeoPixel* ring;
  State* state;
  int pinNumber;
  int idleLead;

  public:
    Facet(int pin, int red, int green, int blue);
    void handleDraw(StateIndicator stateToDraw);
    void handleChanged(int changeIndicator);
    void init();
    void resetLead();
    int getLead();
    void setLead(int lead);
    Color* getColor();
    Adafruit_NeoPixel* getRing();
};


class Idle : public State {
};

class Proximity : public State {
  
    int offset = 1;
    int swipeLength = 8;
   
    public:
      Proximity() {
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
};

class CorrespondingTouch : public State {
};

Facet::Facet(int pin, int red, int green, int blue) {
      ring = new Adafruit_NeoPixel(NUM_OF_PIXELS, pin, NEO_GRB + NEO_KHZ800);
      pinNumber = pin;
      facetColor = {red, green, blue};
      state = new Proximity();
      idleLead = -1;
}
    
void Facet::handleDraw(StateIndicator stateToDraw) {
        state->handleDraw(this, stateToDraw);
}

void Facet::handleChanged(int changeIndicator) {
        state = state->handleChange(this, changeIndicator);
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


Facet facets[6] = {
  Facet(7, 0, 0, 255),//blue 0,0,255: KEEP
  Facet(6, 0, 60, 255),//made up color 0,60,255
  Facet(5, 0, 97, 255),//made up: 0,97,255: KEEP
  Facet(4, 0, 127, 255),//don't know name-bluish aqua 0,127,255: KEEP
  Facet(3, 40, 191, 255),//made up 40,191,255
  Facet(2, 30, 144, 255)//dodger blue 30,144,255: KEEP
};

void handleDraw(StateIndicator stateToDraw) {
  
    for (int i = 0; i < 6; i++) {
        facets[i].handleDraw(stateToDraw);
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

void setup() {
  PT_INIT(&idleDraw);
  PT_INIT(&proximityDraw);
  PT_INIT(&touchedDraw);
  PT_INIT(&correspondingDraw);

  Serial.begin(9600); //open serial port for incoming commands
  
  Serial.println("SETUP");
  
  for (int i = 0; i < 6; i++) {
      facets[i].init();
  }
  
  Serial.setTimeout(50);
}

int once = 0;

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

    //drawIdle(&idleDraw, 900);
    drawProximity(&proximityDraw, 60);

    //drawTouch(&touchedDraw, 900);
    //drawCorresponding(&correspondingDraw, 900);
}


/*
struct Proximity {
  uint16_t head;
};

struct Facet {

};

TimedAction idle = TimeAction(100, idle);
TimedAction proximityAction = TimedAction(60, proximitySwirl);
TimedAction touchedAction = TimedAction(, touched);
TimedAction correspondingTouchAction = TimedAction(, correspondingTouch);

void runDraw(struct pt *pt, int interval, state s) {
  
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  
  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();

    for (int i = 0; i < states
  }
  PT_END(pt);
}

void correspondingTouch() {
  
}

void touched() {
    
}


void idleSwirl() {
  
}




uint16_t colorWheel(ring, uint32_t onC, uint32_t offC, uint16_t swLength, uint16_t head, uint16_t offset) {
 
   for (int16_t i = 0; i < offset; i++) {
        ring.setPixelColor(((head + i) % strip.numPixels()), onC);
        ring.setPixelColor(((head - (swLength - i)) % strip.numPixels()), offC);
   } 
   strip.show();
   return ((head + offset) % strip.numPixels());
}
*/
