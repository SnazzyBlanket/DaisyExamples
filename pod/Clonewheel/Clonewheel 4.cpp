#include "daisysp.h"
#include "daisy_pod.h"

#include <stdio.h>
#include <string.h>
using namespace daisysp;
using namespace daisy;


DaisyPod        pod;
MidiUsbHandler midi;
Tone            flt;                // for deemphasis
DelayLine<float, 100>  delay;       // for chorus/vibrato
Oscillator      lfo;                //      "       ""

//#include "BlinkUtil.cpp"


// FUNCTION DECLARATIONS
void HardwareInit();
void TonewheelInit();
void TonePremphInit();
void ToneFreqInit();
void ChorusInit();
void TonewheelRun();
float KeyDrawbarRun();
void KeyToDrawbars(int); 
float ChorusRun(float);
float PercussionRun();
float DemphasisRun(float);
float MySoftClip(float);
void ReadControls();
void HandleMidi(MidiEvent m);
void SetNoteOn(int);
void SetNoteOff(int);
//void BlinkInit();
//void BlinkStart();
//void BlinkStop();

// GLOBAL DATA - most emulate organ hardware 
#define POLYPH 32                   // N-way polyphonic 
#define ATTENUATE 0.25f              // to avoid clipping
#define COMP_FCTR 0.03f             // function of notes and drawbars on
#define PERC_FAST 0.9998f
#define PERC_SLOW 0.99994f
#define PERC_MAX 16.0f
#define CV_MAX 24
#define PERC_2ND 3
#define PERC_3RD 4
#define REGISTS 8
#define PI  3.1412926
float samplerate;
double tone[12 * 10];	            // 9+ octaves, using 1 to 109
double tone_premph[12 * 10];        // used for key click prephesis 
float premph_freqL = 300;
float premph_freqH = 3000;          // about 10 dB for now
float norm_coef, b2_coef, a2_coef;  // for deemphesis filter
double frq_coef[12 + 1];            // use 1 to 12, not 0 to 11
double sin_osc[12 + 1];             // oscillator state variables
double cos_osc[12 + 1];
int notes_on = 0;                   // how many notes playing
int play_list[POLYPH];              // MIDI notes currently playing
float dbar_bus[9];                  // drawbar signals from tones
float dbar_set[9];                  // drawbar settings (registrations)
float dbars_on;                     // sum of all drawbar settings
float dbar_out;
int transpose = 0;
float cv_lfo;
float cv_depth = 0;
int perc_dbar = PERC_2ND;           // to start
float perc_decay = PERC_FAST;       // to start
float perc_env = 0;                 // decay envelope - from 1.0
float perc_ampl = 0;                // to start
int db_regists[REGISTS][9] = 
   {{8,0, 0,8,0,8, 0,0,0},          // Procol Harum
    {8,8, 8,8,0,0, 0,0,0},          // Green Onions
    {8,8, 8,0,0,0, 0,0,8},          // Gospel
    {8,8, 5,3,2,4, 5,8,8},          // Blues
    {8,0, 8,0,0,0, 0,0,8},          // Reggae
    {8,8, 8,8,0,8, 8,0,6},          // Whiter Shade of Pale
    {8,8, 8,8,8,8, 8,8,8},          // Full
    {0,0, 8,0,0,0, 0,0,0}};         // last is single sine for test
float drive = 0.25;
float volume = 0.25;
float compress = COMP_FCTR; 
bool note_trig = false;

void AudioCallback(AudioHandle::InterleavingInputBuffer 
 in,AudioHandle::InterleavingOutputBuffer 
 out, size_t size)  // SAMPLE LOOP
{
//BlinkStart();                        // mark beginning of callback
// At sample rate:
for(size_t i = 0; i < size; i += 2)
{
    float sig;
    TonewheelRun();
    sig = KeyDrawbarRun();
    sig = ChorusRun(sig);
    sig += PercussionRun();
    sig = DemphasisRun(sig);
    sig = MySoftClip(sig * drive * ATTENUATE);
    out[i + 1] = out[i] = sig * volume;
}
// At callback rate:
ReadControls();
//midi.Listen(); 
//while(midi.HasEvents())
//{
  //  HandleMidi(midi.PopEvent());
//}
//BlinkStop();                         // mark end of callback
}

int main(void)                  // MAIN -- INITIALIZE AND RUN

{

    HardwareInit();
    TonewheelInit();
    ChorusInit();
    pod.StartAdc();
    pod.StartAudio(AudioCallback);
   
   // for(;;) {}
     while(1) {}
}

void HardwareInit()             // INITIALIZE POD AND SEED HARDWARE 
{
    pod.Init();
    System::Delay(250);
   // midi.Init(MidiUsbHandler::INPUT_MODE_UART1, MidiUsbHandler::OUTPUT_MODE_NONE);
    samplerate = pod.AudioSampleRate();
    flt.Init(samplerate);
  //  BlinkInit();
  //  midi.StartReceive();
}

void TonewheelInit()            // SET UP BASE OSCILLATORS AND TONEWHEELS
{
    TonePremphInit();
    ToneFreqInit();
    // Set base octave SV oscillators initial states
    for (int i = 1; i <= 12; i++)	    // for each semitone in an 12
    {
        sin_osc[i] = 1.0;               // set starting SV 
        cos_osc[i] = 0.0;
    }
    // Reset notes playing list
    for (int i = 0; i < POLYPH; i++)
    {
        play_list[i] = 2;               // 0 means not playing
    }
}

void TonePremphInit()           // PRECALC PREEMPHASIS VALUES FOR KEY CLICK
{
    float freq = 32.69;                                // tone #1 frequency
    for (int i = 1; i <= 96; i++)
    {
        // preload an array with preemphasis values for efficiency
        tone_premph[i] = sqrt(1 + (freq/premph_freqL)*(freq/premph_freqL)) /
            sqrt(1 + (freq/premph_freqH)*(freq/premph_freqH));  // shelving HP
        freq *= 1.0595;                                // semitone ratio steps
    }
    for (int i = 97; i <= 108; i++)
    {
        tone_premph[i] = tone_premph[i - 12];           // top octave foldover
    }
// Shelving treble cut/boost (from Seven Woods Audio) 
    b2_coef = exp(-PI * premph_freqH / samplerate);     // for deemphasis filter
    a2_coef = exp(-PI * premph_freqL / samplerate);
    norm_coef = ((1.01 - a2_coef) / (1.01 - b2_coef));
}

void ToneFreqInit()             // CALC BASE FREQUENCIES FROM HAMMOND GEAR RATIOS 
{
    frq_coef[1] = 2*PI * 40 * 85/104 / samplerate;     // C  = 32.692 Hz
    frq_coef[2] = 2*PI * 40 * 71/82 / samplerate;      // C# = 34.634 Hz
    frq_coef[3] = 2*PI * 40 * 67/73 / samplerate;      // D  = 36.712 Hz
    frq_coef[4] = 2*PI * 40 * 105/108 / samplerate;    // D# = 38.889 Hz
    frq_coef[5] = 2*PI * 40 * 103/100 / samplerate;    // E  = 41.200 Hz
    frq_coef[6] = 2*PI * 40 * 84/77 / samplerate;      // F  = 43.636 Hz
    frq_coef[7] = 2*PI * 40 * 74/64 / samplerate;      // F# = 46.250 Hz
    frq_coef[8] = 2*PI * 40 * 98/80 / samplerate;      // G  = 49.000 Hz
    frq_coef[9] = 2*PI * 40 * 96/74 / samplerate;      // G# = 51.892 Hz
    frq_coef[10] = 2*PI * 40 * 88/64 / samplerate;     // A  = 55.000 Hz
    frq_coef[11] = 2*PI * 40 * 67/46 / samplerate;     // A# = 58.261 Hz
    frq_coef[12] = 2*PI * 40 * 108/70 / samplerate;    // B  = 61.714 Hz
}

void TonewheelRun()             // GENERATE ALL 108 SIMULTANIOUS TONES 
{
    for (int i = 1; i <= 12; i++)	    // for each semitone in an 12
    {
        // run 12 base octave 2nd order SV oscillators 
        sin_osc[i] = sin_osc[i] + frq_coef[i] * cos_osc[i];
        cos_osc[i] = cos_osc[i] - frq_coef[i] * sin_osc[i];
        tone [i] = cos_osc[i];                  // first 12
        // run 7 octave generators on each, using cos(2x)=1-2*sin(x)^2
        tone[i + 12] = 1 - 2 * tone[i] * tone[i];
        tone[i + 2*12] = 1 - 2 * tone[i + 12] * tone[i + 12];
        tone[i + 3*12] = 1 - 2 * tone[i + 2*12] * tone[i + 2*12]; 
        tone[i + 4*12] = 1 - 2 * tone[i + 3*12] * tone[i + 3*12]; 
        tone[i + 5*12] = 1 - 2 * tone[i + 4*12] * tone[i + 4*12]; 
        tone[i + 6*12] = 1 - 2 * tone[i + 5*12] * tone[i + 5*12]; 
        tone[i + 7*12] = 1 - 2 * tone[i + 6*12] * tone[i + 6*12];
        tone[i + 8*12] = tone[i + 7*12];        // last 12 is "foldback"
    }
}

float KeyDrawbarRun()           // APPLY ALL ON-KEYS TO THE DRAWBARS 
{
    for (int i = 0; i < 9; i++)
    {
        dbar_bus[i] = 0;                           // reset drawar signals
    }
    // proccess all of the notes currently playing
    for (int i = 0; i < POLYPH; i++)
    {
        int note = play_list[i];
        if (note != 0)
        {
            note += transpose;
            KeyToDrawbars(note - 23);              // Midi note # to Hammond tone #
        }
    }
    // generate the output signal from the drawbars
    float out = 0;
    dbars_on = 0;
    for (int i = 0; i < 9; i++)
    {
        // apply the current drawbar settings when summing them
        out += dbar_bus[i] * dbar_set[i];          // sum the drawar signals
        dbars_on += dbar_set[i];                   // sum the settings for compression 
    }
    return out;
}

void KeyToDrawbars(int key)     // APPLY ONE KEY HARMONICS TO THE DRAWBARS
{
    
    if (key > 12)                                            // only if not too low
    {
        dbar_bus[0] += tone[key - 12] * tone_premph[key - 12]; // Sub Fundamental
    }
    dbar_bus[1] += tone[key + 7] * tone_premph[key + 7];     // Sub Third 
    dbar_bus[2] += tone[key + 0] * tone_premph[key + 0];     // Fundamental  
    dbar_bus[3] += tone[key + 12] * tone_premph[key + 12];   // Second Harm
    dbar_bus[4] += tone[key + 19] * tone_premph[key + 19];   // Third 
    dbar_bus[5] += tone[key + 24] * tone_premph[key + 24];   // Fourth 
    dbar_bus[6] += tone[key + 28] * tone_premph[key + 28];   // Fifth
    dbar_bus[7] += tone[key + 31] * tone_premph[key + 31];   // Sixth
    dbar_bus[8] += tone[key + 36] * tone_premph[key + 36];   // Eighth
}

void ChorusInit()               // INITIALIZE THE C/V DELAY AND LFO 
{
    delay.Init();
    lfo.Init(samplerate);
    lfo.SetWaveform(lfo.WAVE_SIN);
    lfo.SetFreq(7.0);
    lfo.SetAmp(0.5);
}

float ChorusRun(float in)       // SIMPLE HAMMOND CHORUS - 9HZ, 1MS DELAY 
{
    delay.Write(in);
    cv_lfo = lfo.Process();
    float ptr =  (0.5 + cv_lfo) * cv_depth + 1; 
    delay.SetDelay(ptr);  
    float out = delay.Read() + in;              // chorus, not doing vibrato
    return out;
}

float PercussionRun()           // GENERATES SEPARATE HARMONIC PERCUSSION
{
    if (note_trig)
    {
        perc_env = 1;                   // retrigger percussion
        note_trig = false;
    }
    perc_env *= perc_decay;             // update envelope 
    return dbar_bus[perc_dbar] * perc_ampl * perc_env;     
}

float DemphasisRun(float in)    // MATCHES PREEMPHASIS APPIED TO TONE GENERATOR
{
    static float prev_in, prev_out; // Z-1 for shelving filter
    float comp;                 // compression factor - function of notes and drawbars
    comp = exp2(-compress * float (notes_on) * dbars_on);
    // Shelving treble cut/boost (from Seven Woods Audio) 
    float out = (-in + prev_in * b2_coef + prev_out * a2_coef);
    prev_in = in;
    prev_out = out;
    return out * norm_coef * comp;
}

float MySoftClip(float in)      // 1/X DISTORTION FROM FV-1
{
    float out = in;
    if ( in > 1.0)
    {
        out = 2.0 - 1.0 / in;
    }
    if ( in < -1.0)
    {
        out = -2.0 - 1.0 / in;
    }

    return out / 2.0;
}

void  ReadControls()            // READ ENCODER, POTS and SETS LEDS
{
    pod.ProcessAnalogControls();
    pod.ProcessDigitalControls();
    static int regist, perc_butn, clik_butn;
    // encoder selects amoung common drawbar registrations
    regist += pod.encoder.Increment();
    regist = (regist % REGISTS + REGISTS) % REGISTS;    // no negative mod
    for (int i = 0; i < 9; i++)
    {
        float dbar = db_regists[regist][i] / 8.0f;      // Hammond 0-8 to 0-1
        dbar_set[i] = dbar * dbar;                      // make nonlinear
    }
    // encoder button scrolls octave shifts
    if (pod.encoder.RisingEdge())
    {
         transpose -= 12;           // 1 octave down
        if (transpose == -24)
        {
            transpose = 12;         // and 1 up
        }
    }
    // button 1 selects amoung percussion options
    if(pod.button1.RisingEdge())
    {
        switch((++perc_butn) % 4)
        {
            case 0:
                perc_dbar = PERC_2ND;
                perc_decay = PERC_FAST;
                break;
            case 1:
                perc_dbar = PERC_2ND;
                perc_decay = PERC_SLOW;
                break;
            case 2:
                perc_dbar = PERC_3RD;
                perc_decay = PERC_FAST;
                break;
            case 3:
                perc_dbar = PERC_3RD;
                perc_decay = PERC_SLOW;
//                perc_butn = -1;
                break;
            default: break;
        }
     }
    if(pod.button2.RisingEdge())
    {
        switch((++clik_butn) % 4)
        {
            case 0:
                premph_freqL = 300;     // +20dB preemhesis
                premph_freqH = 3000;
                break;
            case 1:
                premph_freqL = 800;
                premph_freqH = 3000;
                break;
            case 2:
                premph_freqL = 3000;
                premph_freqH = 3000;
                break;
            case 3:
                premph_freqL = 3000;
                premph_freqH = 800;
//                clik_butn = -1;
                break;
            default: break;
        }
        TonePremphInit();               // recalculate preemphesis
    }
    // pots adjust percussion and chorus depth
    float knob_val = pod.knob1.Process();
    perc_ampl = PERC_MAX * knob_val;
    float led_on = (perc_env > 0.15) * knob_val;
    pod.led1.Set(led_on * (perc_dbar == PERC_2ND), led_on * (perc_dbar == PERC_3RD), 0);

    knob_val = pod.knob2.Process();
    cv_depth = CV_MAX * knob_val;
    if (pod.button2.Pressed())
    {
        led_on = (clik_butn % 4) * 0.2;
    pod.led2.Set(led_on, 0, 0);
    }
    else
    {
        led_on = (cv_lfo > 0.40) * (notes_on > 0) * knob_val * 0.6;
        pod.led2.Set(0, 0, led_on);
    }
    pod.UpdateLeds();
}

void HandleMidi(MidiEvent event) // PROCESS MIDI MESSAGES (CALLBACK RATE)
{
    switch(event.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = event.AsNoteOn();
            // This is to avoid Max/MSP Note outs for now.
            if (event.data[1] != 0)
            {
                p = event.AsNoteOn();
                SetNoteOn(12);///was p.note);
            }
        }
        break;
        case NoteOff:
        {
        NoteOnEvent p = event.AsNoteOn();
            if (event.data[1] != 0)
            {
                p = event.AsNoteOn();
                SetNoteOff(p.note);

            }
        }
        break;
        case ControlChange:
        {
            ControlChangeEvent p = event.AsControlChange();
            float val = (float)p.value/127.0;
            switch(p.control_number)
            {
                case 17:
                    // CC 1 
                    volume = val * val;
                    break;
                case 19:
                    // CC 2 
                    drive = val * val;
                    break;
                case 20:
                    // CC 3 
                    compress = COMP_FCTR * val * 2.0;
                    break;
                case 21:
                    // CC 4 
                    drive = val * val;
                    break;
                default: break;
            }
            break;
        }        
        default: break;
    }
}

void SetNoteOn(int midi_note)   // PUTS NOTE IN PLAY LIST
{
    // make sure it is currently off - helps with overrun issue
    for (int i = 0; i < POLYPH; i++)
        {
            if (play_list[i] == midi_note)     // does entry match?
            {
                play_list[i] = 0;              // turn it off 
                notes_on--;                    // decrement count
            }
        } 
    // now turn it on once
    for ( int i = 0; i < POLYPH; i++)
    {
        if (play_list[i] == 0)                  // is this entry open?
        {
            play_list[i] = midi_note;           // add it
            notes_on++;                         // increment count
            if (notes_on == 2)
            {
                note_trig = true;               // trigger for percussion
            }
            break;
        }
    }
}

void SetNoteOff(int midi_note)  // DELETES NOTE FROM PLAY LIST
{
    for (int i = 0; i < POLYPH; i++)
    {
        if (play_list[i] == midi_note)     // does entry match?
        {
            play_list[i] = 0;              // turn it off 
            notes_on--;                      // decrement count
        }
    }
}

