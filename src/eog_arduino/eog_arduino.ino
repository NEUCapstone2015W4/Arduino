#include <SPI.h>

#define RESOLUTION 16

#if RESOLUTION == 16
#define SCALE_FACTOR 0.000152587890625
#endif

#define BUSY 3
#define RESET 4
#define START_CONVERSION 5
#define CHIP_SELECT 10
#define MISO 12
#define LED 13
#define TOTAL_RAW_BYTES RESOLUTION
#define H_CH 0
#define V_CH 1




// Global variables
float hAverage;
float vAverage;
char inByte;
char outByte;
int calCounter;
boolean firstPass;
int bytesToRead = TOTAL_RAW_BYTES;
byte raw[TOTAL_RAW_BYTES];
signed long parsed[8];
boolean guiInitiated;

float ALPHA = 0.02; // weight factor for averaging stand-by baseline
                    // alpha = 2 / (N+1)
                    // for 0.02 weight, approx N=99 values are significant
                    
float beta = 0.3; //weight value for eye movement check

float blinkalpha = 0.08; //weight value for checking up value

int calIndex = 100; //Initial value for calCounter
                    
float hardTolVert = 0.5; //threshold for vertical detection
float hardTolHor = 1.0; // threshold for horizontal detection

int blinkScalar = 4; // used in determining Blink Threshold = (Average - blinkScalar*hardTolVert)

int n=20; //confirm number of cycles during eye detection

int mainDelay = 25; // millisecond delay at end of main loop, have been using 25 ms
int mainBlinkDelay = 1500; //millisecond delay if outByte = X
int detectDelay = 50; //millisecond delay at beginning of each [sample > avg+thres]
int checkDelay = 50; //millisecond delay for n=[0,10] loops to confirm that eye movement
int outDelay = 1; //millisecond delay after outByte is set
int upBlinkSplitDelay = 15; //millisecond delay in running i,j loops for U and X
                            //primarily responsible for longer up/blink time due to need to execute full X for U
int moveDelay = 750; //delay after !=S

boolean diagMode = false; //if true, will print additional diag info to serial monitor
                          //if true, increase mainDelay to slow down the data



boolean voltageToMotion(float sample, float average, float tolerance, int channel)
{
  // Return if sample voltage is -10.0, assumed to be an error
  if (sample == -10.0 || sample == 10.0)
  {
    return false;
  }
  // If sample voltage is less than estimated average - tolerance,
  // send 'R'/'U' to GUI, if sample voltage is greater than
  // average + tolerance, send 'L'/'D' to GUI, else recalculate
  // estimated average with sample
  float testvar; //typical averaging var for all 4 inputs
  float tempsample; //typical sample var for all 4 inputs
  int i=0; // typ detect loop index
  int j=0; // blink loop index
  float testBlink; //blinking averaging var
  float blinkThres; //threshold or tolerance for blinks


  if (sample < (average - tolerance)) //start of cleaning up delays
  {
    delay(detectDelay);
    if (channel == 0)
    {
      testvar = hAverage;
      for ( i=0; i<n; i++ )
      {
        parseBytesFromADC();
        tempsample = (float)parsed[4]*SCALE_FACTOR*2;
        testvar = (1 - beta) * testvar + beta * tempsample;
        delay(checkDelay);
        if ( testvar < (average - tolerance) )
        {
          outByte = 'R';
          if (diagMode){Serial.println("Assigning R");}
          delay(outDelay); //end of looking to clean up delays
          break;
        }
      }
    }
    else
    {
      testvar = vAverage;
      for ( i=0; i<n; i++ )
      {
        parseBytesFromADC();
        tempsample = (float)parsed[5]*SCALE_FACTOR*2;
        testvar = (1 - blinkalpha) * testvar + blinkalpha * tempsample;
        delay(upBlinkSplitDelay); //loop is shorter than others since there are 2 possible executions here
        if ( (testvar < (average - tolerance)) )
        {
          outByte = 'U';
          if (diagMode){Serial.println("Assigning U");}
          testBlink = (average - tolerance);
          blinkThres = (average - blinkScalar*tolerance);
          for (j=0; j<10; j++)
          {
            parseBytesFromADC();
            tempsample = (float)parsed[5]*SCALE_FACTOR*2;
            testBlink = (1 - beta) * testBlink + beta * tempsample;
            delay(upBlinkSplitDelay);//loop is shorter than others since there are 2 possible executions here
            if ( testBlink < blinkThres )
            {
              outByte = 'X';
              if (diagMode){Serial.println("Assigning X");}
              delay(outDelay);
              return false;
            }
          }
        }
      }
    }
    return false;
  }
  else if (sample > (average + tolerance))
  {
    delay(detectDelay);
    if (channel == 0)
    {
      testvar = hAverage;
      for ( i=0; i<n; i++ )
      {
        parseBytesFromADC();
        tempsample = (float)parsed[4]*SCALE_FACTOR*2;
        testvar = (1 - beta) * testvar + beta * tempsample;
        delay(checkDelay);
        if ( testvar > (average + tolerance) )
        {
          outByte = 'L';
          if (diagMode){Serial.println("Assigning L");}
          delay(outDelay);
          break;
        }
      }
    }
    else
    {
      testvar = vAverage;
      for ( i=0; i<n; i++ )
      {
        parseBytesFromADC();
        tempsample = (float)parsed[5]*SCALE_FACTOR*2;
        testvar = (1 - beta) * testvar + beta * tempsample;
        delay(checkDelay);
        if ( testvar > (average + tolerance) )
        {
          outByte = 'D';
          delay(outDelay);
          break;
        }
      }
    }
    return false;
  }
  else
  {
    average = (1 - ALPHA) * average + ALPHA * sample;
    if (channel == 0)
    {
      hAverage = average;
    }
    else
    {
      vAverage = average;
    }
    return true;
  }
}

void setup() {
  pinMode(BUSY, INPUT);
  pinMode(RESET, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(START_CONVERSION, OUTPUT);
  pinMode(MISO, INPUT);
  
  SPI.begin();

  // Initialization of globals
  firstPass = true;
  calCounter = calIndex;

  digitalWrite(START_CONVERSION, HIGH);
  digitalWrite(CHIP_SELECT, HIGH);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(RESET, LOW);
  
  Serial.begin(57600);
  guiInitiated = false;
}

void loop() {

  if (outByte == 'X'){
    delay(mainBlinkDelay);
  }
  
  parseBytesFromADC(); // run ADC and update samples
  
  float out_v, out_h, vTol, hTol; 
  out_v = (float)parsed[5]*SCALE_FACTOR*2; // assign samples 
  out_h = (float)parsed[4]*SCALE_FACTOR*2;
  
  if (firstPass)
  {
    hAverage = out_h;
    vAverage = out_v;
    firstPass = false;
  }

  if (calCounter > 0)
  {
    vTol = 100;
    hTol = 100;
    hAverage = (1 - ALPHA) * hAverage + ALPHA * out_h;
    vAverage = (1 - ALPHA) * vAverage + ALPHA * out_v;
    if (diagMode){if (calCounter == 1){Serial.print("Ready");}}
    calCounter--;
  }
  else
  {
    vTol = hardTolVert;
    hTol = hardTolHor;
  }

  if (voltageToMotion(out_h, hAverage, hTol, H_CH) &&
      voltageToMotion(out_v, vAverage, vTol, V_CH))
  {
    outByte = 'S';
  }
  
  if (diagMode){
    Serial.println("Vertical Sample: ");
    Serial.println(out_v);
    Serial.println("Horizontal Sample: ");
    Serial.println(out_h);
    Serial.println("Vertical Average: ");
    Serial.println(vAverage);
    Serial.println("Horizontal Average: ");
    Serial.println(hAverage);
        
  }

  if (outByte != 'S'){
    
  Serial.println(outByte);
  delay (moveDelay);
  }
  delay(mainDelay);
}





void parseRawBytes() {
  int i;

  parsed[0] = (raw[0] << 8) + (raw[1] >> 0);
  parsed[1] = (raw[2] << 8) + (raw[3] >> 0);
  parsed[2] = (raw[4] << 8) + (raw[5] >> 0);
  parsed[3] = (raw[6] << 8) + (raw[7] >> 0);
  parsed[4] = (raw[8] << 8) + (raw[9] >> 0);
  parsed[5] = (raw[10] << 8) + (raw[11] >> 0);
  parsed[6] = (raw[12] << 8) + (raw[13] >> 0);
  parsed[7] = (raw[14] << 8) + (raw[15] >> 0);

  for(i=0; i<8; i++) {
    parsed[i] = fixSignBit(parsed[i]);
  }
}

long fixSignBit(long reading) {

  if(reading & 0x8000) { // if reading is < 0 (stored as two's complement)
    return reading | 0xFFFF0000; // set bits 31-16
  }
  else {
    return reading;
  }

}

void parseBytesFromADC()
{
  digitalWrite(START_CONVERSION, LOW);
  delayMicroseconds(10);
  digitalWrite(START_CONVERSION, HIGH);

  while (digitalRead(BUSY) == HIGH) {
    // wait for conversion to complete
  }

  digitalWrite(CHIP_SELECT, LOW);
  
  while (bytesToRead > 0) {
    raw[TOTAL_RAW_BYTES - bytesToRead] = SPI.transfer(0x00);
    bytesToRead--;
  }

  digitalWrite(CHIP_SELECT, HIGH);
  bytesToRead = TOTAL_RAW_BYTES;

  parseRawBytes();
}


//Electrode directions and colors
//Red: right
//Black: left
//Brown: down
//White: up
//Center: green


