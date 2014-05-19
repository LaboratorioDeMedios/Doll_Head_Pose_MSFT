#pragma once

#include "ofxMidi.h"
#include "FaceData.h"
#include "FaceAnalizer.h"

#define MIDI_GENERATOR_MODE_PENTATONIC 0
#define MIDI_GENERATOR_MODE_SCREAM 1
#define MIDI_GENERATOR_PENTATONIC_SCALE_MAX 6
#define MIDI_GENERATOR_MOUTH_OPEN_POINTS_THRESHOLD 13
#define MIDI_MAX_CHANNELS 8

class MidiGenerator {

public:
	MidiGenerator();

	void generate(int mode, FaceData face, int channel);

	void testNoteOn(int channel, int note, int velocity);

	ofxMidiOut midiOut;

private:	 	
	
	void playScream(FaceData face, int channel);


	// pentatonic	
	void playPentatonic(FaceData face, int channel);

	int getSelectedNotePentatonic(float rot[3]);	
	int getSelectedOctavePentatonic(float rot[3]);

	int noteBeingPlayed[MIDI_MAX_CHANNELS];
	bool isPlayingNote[MIDI_MAX_CHANNELS];
	
	int selectedNote, selectedOctave;
	int baseNotesPentatonic [5];

	FaceAnalizer fa;
};