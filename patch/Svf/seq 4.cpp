//  ============================================================
//
//  Program: ArdCore Sequencer
//#define NUM_WAVEFORMS 4
uint8_t waveforms[NUM_WAVEFORMS] = {
                                    Oscillator::WAVE_SAW,//WAVE_POLYBLEP_SAW                                                                                     
                                    Oscillator::WAVE_POLYBLEP_TRI,//WAVE_POLYBLEP_TRI                                                                           
                                    Oscillator::WAVE_SIN,
                                    Oscillator::WAVE_SQUARE};
pot_rotary.Init(hw.knob[hw.KNOB_1], 0, 4, Parameter::LINEAR);
waveform = DSY_CLAMP(pot_rotary.Process(), 0, NUM_WAVEFORMS)
osc_trem.Init(samplerate); // Initializes the oscillator
osc_trem.SetWaveform(waveforms[waveform]); // This sets the waveform to whatever the pot selected.
//  Description: A basic 8 step sequencer, where the input
//               is done using the two ArdCore knobs
//
//  I/O Usage:
//    Knob 1: Pitch to be recorded at the current step
//    Knob 2: Current step selection
//    Analog In 1: unused
//    Analog In 2: unused
//    Digital Out 1: Trigger on step output
//    Digital Out 2: unused
//    Clock In: External clock input
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  10 Dec 2010
//  Modified: 13 Feb 2011 - rework to use knobs only
//
//  ============================================================

//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // the standard trigger time
bool LOW = false;
bool HIGH = true;
//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
long digTime[2] = {0, 0};      // the times of the last HIGH

//  recording and playback variables
int seqValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int maxSeqValue = 8;
int currSeqPlay = 0;

//  ==================== start of setup() ======================

/*void setup() {
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  attachInterrupt(0, isr, RISING);
}*/ 

//  ==================== start of loop() =======================

void loop()
{
  // on clock tick, move forward and play.
  if (clkState == HIGH) {
    clkState = LOW;
    
    currSeqPlay = (++currSeqPlay % maxSeqValue);
    dacOutput(seqValue[currSeqPlay]);
    
    digState[0] = HIGH;
    digTime[0] = millis();
    digitalWrite(digPin[0], HIGH);
  }

  // record the current knob position values
  int tempPos = analogRead(1) >> 7;
  int tempVal = quantNote(analogRead(0));
  seqValue[tempPos] = tempVal;    
}

//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(long v)
{
  // feed this routine a value between 0 and 255 and teh DAC
  // output will send it out.
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
}

//  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
int quantNote(int v)
{
  // feed this routine the input from one of the analog inputs
  // and it will return the value in a 0-64 range.
  return (v >> 4) << 2;
}

//  ===================== end of program =======================
