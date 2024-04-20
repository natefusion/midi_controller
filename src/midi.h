#ifndef MIDI_H
#define MIDI_H

#include "misc.h"

// Most of this is unused. We could only do so much

typedef enum Status {
  Status_Note_On = 0x90,
  Status_Note_Off = 0x80,
  Status_Instrument_Set = 0xC0,
  Status_Controller_Set = 0xB0,
  Status_Pitch_Bend = 0xE0,
  Status_Pressure = 0xA0,
} Status;

typedef enum Note {
  Note_C = 60,
  Note_CS,
  Note_D,
  Note_Eb,
  Note_E,
  Note_F,
  Note_FS,
  Note_G,
  Note_Ab,
  Note_A,
  Note_Bb,
  Note_B,
} Note;

typedef enum Volume {
  Volume_pppp = 8,
  Volume_ppp = 20,
  Volume_pp = 31,
  Volume_mp = 53,
  Volume_mf = 64,
  Volume_f = 80,
  Volume_ff = 96,
  Volume_fff = 112,
  Volume_ffff = 127
} Volume;

typedef enum Controller {
  Controller_Sound_Bank_Selection_MSB = 0,
  Controller_Modulation_Wheel = 1,
  Controller_Volume = 7,
  Controller_Panoramic = 10,
  Controller_Expression = 11,
  Controller_Sound_Bank_Selection_LSB = 32,
  Controller_Sustain_Pedal = 64,
  Controller_Controllers_Off = 121,
  Controller_Notes_Off = 123,
} Controller;

typedef enum Instrument {
// Piano:
  Acoustic_Grand_Piano,
  Bright_Acoustic_Piano,
  Electric_Grand_Piano,
  Honky_tonk_Piano,
  Electric_Piano_1,
  Electric_Piano_2,
  Harpsichord,
  Clavinet,

// Chromatic Percussion:
  Celesta,
  Glockenspiel,
  Music_Box,
  Vibraphone,
  Marimba,
  Xylophone,
  Tubular_Bells,
  Dulcimer,

// Organ:
  Drawbar_Organ,
  Percussive_Organ,
  Rock_Organ,
  Church_Organ,
  Reed_Organ,
  Accordion,
  Harmonica,
  Tango_Accordion,

// Guitar:
  Acoustic_Guitar_nylon,
  Acoustic_Guitar_steel,
  Electric_Guitar_jazz,
  Electric_Guitar_clean,
  Electric_Guitar_muted,
  Overdriven_Guitar,
  Distortion_Guitar,
  Guitar_harmonics,

// Bass:
  Acoustic_Bass,
  Electric_Bass_finger,
  Electric_Bass_pick,
  Fretless_Bass,
  Slap_Bass_1,
  Slap_Bass_2,
  Synth_Bass_1,
  Synth_Bass_2,

//Strings:
  Violin,
  Viola,
  Cello,
  Contrabass,
  Tremolo_Strings,
  Pizzicato_Strings,
  Orchestral_Harp,
  Timpani,

// Strings (continued):
  String_Ensemble_1,
  String_Ensemble_2,
  Synth_Strings_1,
  Synth_Strings_2,
  Choir_Aahs,
  Voice_Oohs,
  Synth_Voice,
  Orchestra_Hit,

// Brass:
  Trumpet,
  Trombone,
  Tuba,
  Muted_Trumpet,
  French_Horn,
  Brass_Section,
  Synth_Brass_1,
  Synth_Brass_2,

// Reed:
  Soprano_Sax,
  Alto_Sax,
  Tenor_Sax,
  Baritone_Sax,
  Oboe,
  English_Horn,
  Bassoon,
  Clarinet,

// Pipe:
  Piccolo,
  Flute,
  Recorder,
  Pan_Flute,
  Blown_Bottle,
  Shakuhachi,
  Whistle,
  Ocarina,

// Synth Lead:
  Lead_1_square,
  Lead_2_sawtooth,
  Lead_3_calliope,
  Lead_4_chiff,
  Lead_5_charang,
  Lead_6_voice,
  Lead_7_fifths,
  Lead_8_bass_lead,

// Synth Pad:
  Pad_1_new_age,
  Pad_2_warm,
  Pad_3_polysynth,
  Pad_4_choir,
  Pad_5_bowed,
  Pad_6_metallic,
  Pad_7_halo,
  Pad_8_sweep,

// Synth Effects:
  FX_1_rain,
  FX_2_soundtrack,
  FX_3_crystal,
  FX_4_atmosphere,
  FX_5_brightness,
  FX_6_goblins,
  FX_7_echoes,
  FX_8_sci_fi,

// Ethnic:
  Sitar,
  Banjo,
  Shamisen,
  Koto,
  Kalimba,
  Bag_pipe,
  Fiddle,
  Shanai,

// Percussive:
  Tinkle_Bell,
  Agogo,
  Steel_Drums,
  Woodblock,
  Taiko_Drum,
  Melodic_Tom,
  Synth_Drum,

// Sound_effects:
  Reverse_Cymbal,
  Guitar_Fret_Noise,
  Breath_Noise,
  Seashore,
  Bird_Tweet,
  Telephone_Ring,
  Helicopter,
  Applause,
  Gunshot,
} Instrument;

/* Initializes serial port for communication */
void midi_init(void);

char *note_range_tostring(Note note);

void usart_printf(const char* fmt, ...);
void usart_send_char(u8 c);

void midi_send_note_on(Note note, Volume vol);
void midi_send_note_off(Note note);
void midi_send_drum_on(Instrument drum, Volume vol);
void midi_send_drum_off(Instrument drum);

/* value of 0x2000 represents no pitch change
   values of 0x2001-0x3FFF will raise the pitch
   values of 0x0000-0x1FFF will lower the pitch */ 
void midi_set_pitch_bend(u14 value);

void midi_set_pressure(Note note, u7 value); // I would have liked to implement this on the keyboard ...
void midi_set_instrument(Instrument i);
void midi_set_controller(Controller c, u7 value);

#endif
