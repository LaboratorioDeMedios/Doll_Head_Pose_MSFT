#include "MidiGenerator.h"


void MidiGenerator::testNoteOn(int channel, int note, int velocity) {
	cout << "sending note on: " << channel << ", " << note << ", " << velocity << endl;
	midiOut.sendNoteOn(channel, 55,  127);  // channel, note, velocity 
}


void MidiGenerator::generate(int mode, FaceData face, int channel) {

	switch (mode) {

	case MIDI_GENERATOR_MODE_PENTATONIC:
		playPentatonic(face, channel);
		break;

	case MIDI_GENERATOR_MODE_SCREAM:
		playScream(face, channel);		
		break;

	}
}

void MidiGenerator::playScream(FaceData face, int channel) {
	#define NUMBER_CONTROL_CHANGES 5 // todo store this better
	pair <int, int> controlChanges[NUMBER_CONTROL_CHANGES]; // channel, value

	controlChanges[0].first = 10; // control changes are mapped onto knobs or x/y controls in ableton live
	controlChanges[0].second = ofMap(face.faceRot[0], 0, 25, 0, 127);

	controlChanges[1].first = 11;
	controlChanges[1].second = ofMap(face.faceRot[1], 20, -20, 127, 0);

	controlChanges[2].first = 12;
	controlChanges[2].second = ofMap(face.faceRot[2], 40, -35, 0, 127);

	controlChanges[3].first = 13;
	controlChanges[3].second = ofMap(face.faceRot[2] * face.faceRot[1], -800, 800, 0, 127);

	controlChanges[4].first = 14;	

	float d = fa.distFacePoints(face, 18, 80);

	controlChanges[4].second = ofMap(d, 0, 30, 0, 127);

	for (int i = 0; i < NUMBER_CONTROL_CHANGES; i++) {
		midiOut.sendControlChange(channel, controlChanges[i].first, controlChanges[i].second);
	}

	if (fa.mouthOpen(face)) { // mouth open
		midiOut.sendNoteOn(channel, 55,  127);  // channel, note, velocity 		
	} else { // mouth closed 		
		midiOut.sendNoteOn(channel, 55,  0);  // channel, note, velocity  
	}	
}

void MidiGenerator::playPentatonic(FaceData face, int channel) {
	// we output a note in the pentatoinc scale according to some head rotation
	// to be defined
	// A minor (Am) pentatonic scale is: A C D (Eg) E G, where Eg is the "blues" note. We can trigger it with eyes closed or something (if only)	
	/*
	calculate the selected note and octave
	set noteToPlay = the corresponding note
	if not playing and mouth closed do nothing
	if playing and mouth closed send note off
	if mouth open
	if not playing 
	send note on
	playing = true

	if playing this note do nothing

	if playing other note 
	if eye closed // to do
	glissando
	set note being played as new note					
	if eye open
	note off 
	note on new note
	*/

	selectedNote = getSelectedNotePentatonic(face.faceRot); 
	selectedOctave = getSelectedOctavePentatonic(face.faceRot);

	int selectedNotePenta = -1;

	if (selectedNote >= 0 && selectedNote <= 4) {			
		selectedNotePenta = baseNotesPentatonic[selectedNote]; // from 0..4 to pentatonic
	}


	int correspondingOctave = MIDI_GENERATOR_PENTATONIC_SCALE_MAX - selectedOctave;

	int noteToPlay = selectedNotePenta + correspondingOctave * 12;

	if (!fa.mouthOpen(face)) {
		if (isPlayingNote) {				
			midiOut.sendNoteOn(channel, noteBeingPlayed[channel],  0); // note off						
			isPlayingNote[channel] = false;
		} 

	} else { // mouth is open
		if (!isPlayingNote) {
			midiOut.sendNoteOn(channel, noteToPlay,  127);			
			isPlayingNote[channel] = true;
		} else { // is playing note

			if (noteBeingPlayed[channel] != noteToPlay) {
				// to do: glissando option
				midiOut.sendNoteOn(channel, noteBeingPlayed[channel], 0); // note off
				midiOut.sendNoteOn(channel, noteToPlay, 127); // note on				
				noteBeingPlayed[channel] = noteToPlay;					
			}
		}
	}
}

MidiGenerator::MidiGenerator() {
	midiOut.listPorts(); // via instance	
	midiOut.openPort(1);

	// set up midi notes for the minor pentatonic 
	baseNotesPentatonic[0] = 9; //  {9, 12, 14, 16, 19} = pentatonic Am
	baseNotesPentatonic[1] = baseNotesPentatonic[0] + 3;
	baseNotesPentatonic[2] = baseNotesPentatonic[1] + 2;
	baseNotesPentatonic[3] = baseNotesPentatonic[2] + 2;
	baseNotesPentatonic[4] = baseNotesPentatonic[3] + 3;
	
}

int MidiGenerator::getSelectedNotePentatonic(float rot[3]) {
	float r = rot[2];
	if (r > 20) return 0;
	if (r > 10) return 1;
	if (r > -10) return 2;
	if (r > -20) return 3;
	return 4;
}

int MidiGenerator::getSelectedOctavePentatonic(float rot[3]) {
	float r = rot[0];	
	if (r > 35) return 0;
	if (r > 25) return 1;
	if (r > 0) return 2;
	if (r > -15) return 3;
	return 4;;
}