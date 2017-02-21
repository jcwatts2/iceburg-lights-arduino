#include <Adafruit_NeoPixel.h>
#include <pt.h>

#define NUM_OF_PIXELS 24
#define PROXIMITY_FACET_NUMBER 6
#define IDLE_DELAY 60
#define NUM_OF_FACETS 6

enum state {
    idle,
    proximity,
    touched,
    correspondingTouch
};

static struct pt idleDraw, proximityDraw, touchedDraw, correspondingDraw;

class Facet;

struct Color {
  int red;
  int green;
  int blue;
};

class State {
  public: 
    virtual void handleDraw(Adafruit_NeoPixel ring, Color color, int stateToDraw) {}
    virtual State handleChange(Facet* facet, int changeIndicator) {}
    virtual int getStateIndicator() {}
};

class Idle : public State {
};

class Proximity : public State {
    int lead;
    int offset = 1;
    int swipeLength = 6;
   
    public:
      Proximity() {
          lead = -1;
      }
      
      void handleDraw(Adafruit_NeoPixel ring, Color color, int stateToDraw) {

          if (stateToDraw != proximity) {
            return;
          }
        
          if (lead == -1) {
            //reset ring
          }
        
          for (int16_t i = 0; i < offset; i++) {
              ring.setPixelColor(((lead + i) % ring.numPixels()), color.red, color.green, color.blue);
              ring.setPixelColor(((lead - (swipeLength - i)) % ring.numPixels()), 255, 255, 255);
          } 
          ring.show();
          lead = ((lead + offset) % ring.numPixels());       
      }
};

class Touched : public State {
};

class CorrespondingTouch : public State {
};

class Facet {
  Color color;
  Adafruit_NeoPixel ring;
  State state;
  int pin;

  public:
    Facet(int pin, Color color) {
      ring = Adafruit_NeoPixel(NUM_OF_PIXELS, pin, NEO_GRB + NEO_KHZ800);
      pin = pin;
      color = color;
      state = Idle();
    }
   
    int getPin() {
      return pin;  
    }
    
    Adafruit_NeoPixel getRing() {
      return ring;
    }

    Color getColor() {
      return color;  
    }
    
    void handleDraw(int stateToDraw) {
        state.handleDraw(ring, color, stateToDraw);
    }

    void handleChanged(int changeIndicator) {
        state = state.handleChange(this, changeIndicator);
    }
};

Facet facets[NUM_OF_FACETS] = {
  Facet(7,  {0, 255, 255}),//blue 0,0,255: KEEP
  Facet(6, {0, 255, 255}),//made up color 0,60,255
  Facet(5, {0, 255, 255}),//made up: 0,97,255: KEEP
  Facet(4, {0, 255, 255}),//don't know name-bluish aqua 0,127,255: KEEP
  Facet(3, {0, 255, 255}),//made up 40,191,255
  Facet(2, {0, 255, 255})//dodger blue 30,144,255: KEEP
};

void handleDraw(state stateToDraw) {
    for (int i = 0; i < NUM_OF_FACETS; i++) {
        facets[i].handleDraw(stateToDraw);
    }
}

static int drawIdle(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(idle);
    }
    PT_END(pt);
}

static int drawProximity(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(proximity);
    }
    PT_END(pt);
}

static int drawTouch(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(touched);
    }
    PT_END(pt);
}

static int drawCorresponding(struct pt *pt, int interval) {
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(1) {
        PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
        timestamp = millis(); // take a new timestamp
        handleDraw(correspondingTouch);
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
  
}

void loop() {
    while (Serial.available() > 0) {
        static int number = Serial.parseInt();
        static int state = Serial.parseInt();
        handleChange(number, state);
    }

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
