#include <fstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;

const unordered_map<string, int> NOTES = {
	{"C-0", 0x00}, {"C#0", 0x00}, {"D-0", 0x00}, {"D#0", 0x00}, {"E-0", 0x00}, {"F-0", 0x00}, {"F#0", 0x00}, {"G-0", 0x00}, {"G#0", 0x00}, {"A-0", 0x00}, {"A#0", 0x01}, {"B-0", 0x02},
	{"C-1", 0x03}, {"C#1", 0x04}, {"D-1", 0x05}, {"D#1", 0x06}, {"E-1", 0x07}, {"F-1", 0x08}, {"F#1", 0x09}, {"G-1", 0x0A}, {"G#1", 0x0B}, {"A-1", 0x0C}, {"A#1", 0x0D}, {"B-1", 0x0E},
	{"C-2", 0x0F}, {"C#2", 0x10}, {"D-2", 0x11}, {"D#2", 0x12}, {"E-2", 0x13}, {"F-2", 0x14}, {"F#2", 0x15}, {"G-2", 0x16}, {"G#2", 0x17}, {"A-2", 0x18}, {"A#2", 0x19}, {"B-2", 0x1A},
	{"C-3", 0x1B}, {"C#3", 0x1C}, {"D-3", 0x1D}, {"D#3", 0x1E}, {"E-3", 0x1F}, {"F-3", 0x20}, {"F#3", 0x21}, {"G-3", 0x22}, {"G#3", 0x23}, {"A-3", 0x24}, {"A#3", 0x25}, {"B-3", 0x26},
	{"C-4", 0x27}, {"C#4", 0x28}, {"D-4", 0x29}, {"D#4", 0x2A}, {"E-4", 0x2B}, {"F-4", 0x2C}, {"F#4", 0x2D}, {"G-4", 0x2E}, {"G#4", 0x2F}, {"A-4", 0x30}, {"A#4", 0x31}, {"B-4", 0x32},
	{"C-5", 0x33}, {"C#5", 0x34}, {"D-5", 0x35}, {"D#5", 0x36}, {"E-5", 0x37}, {"F-5", 0x38}, {"F#5", 0x39}, {"G-5", 0x3A}, {"G#5", 0x3B}, {"A-5", 0x3C}, {"A#5", 0x3D}, {"B-5", 0x3E},
	{"C-6", 0x3F}, {"C#6", 0x40}, {"D-6", 0x41}, {"D#6", 0x42}, {"E-6", 0x43}, {"F-6", 0x44}, {"F#6", 0x45}, {"G-6", 0x46}, {"G#6", 0x47}, {"A-6", 0x48}, {"A#6", 0x49}, {"B-6", 0x4A},
	{"C-7", 0x4B}, {"C#7", 0x4C}, {"D-7", 0x4D}, {"D#7", 0x4E}, {"E-7", 0x4F}, {"F-7", 0x50}, {"F#7", 0x51}, {"G-7", 0x52}, {"G#7", 0x53}, {"A-7", 0x54}, {"A#7", 0x55}, {"B-7", 0x56},
	{"...", -1},  {"---", -2}, {"===", -3}
};

//for some reason, famitracker flips the period bits so 0-# represents an F in the register.
const unordered_map<string, int> NOISE_NOTES = {
	{"0-#", 0x0F}, {"1-#", 0x0E}, {"2-#", 0x0D}, {"3-#", 0x0C}, {"4-#", 0x0B}, {"5-#", 0x0A}, {"6-#", 0x09}, {"7-#", 0x08},
	{"8-#", 0x07}, {"9-#", 0x06}, {"A-#", 0x05}, {"B-#", 0x04}, {"C-#", 0x03}, {"D-#", 0x02}, {"E-#", 0x01}, {"F-#", 0x00},
	{"...", -1},  {"---", -2}, {"===", -3}
};

const unordered_map<string, int> DPCM_NOTES = {
	{"C-", 0}, {"C#", 1}, {"D-", 2}, {"D#", 3}, {"E-", 4}, {"F-", 5}, {"F#", 6}, {"G-", 7}, {"G#", 8}, {"A-", 9}, {"A#", 10}, {"B-0", 11},
	{"..", -1},  {"--", -2}, {"==", -3}
};

enum VOLUME_LEVELS {
	Zero = 0x57, One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Eleven, Twelve, Thirteen, Fourteen, Fifteen //Fifteen == 0x66, if Zero == 0x57
};

const unordered_map<string, int> EFFECTS = {
	{"0", 0xE5}, {"1", 0xE6}, {"2", 0xE7}, {"3", 0xE8}, {"4", 0xE9}, {"7", 0xEA}, {"A", 0xEB}, {"B", 0xEC}, {"C", 0xED}, {"D", 0xEE}, {"E", 0xEF}, {"F", 0xF0}, {"G", 0xF1},
	{"H", 0xF2}, {"I", 0xF3}, {"J", 0xF5}, {"P", 0xF6}, {"Q", 0xF7}, {"R", 0xF8}, {"S", 0xF9}, {"V", 0xFA}, {"W", 0xFB}, {"X", 0xFC}, {"Y", 0xFD}, {"Z", 0xFE}
};

//vibrato table: http://famitracker.com/wiki/index.php?title=4xy
const int VIBRATO_DEPTH[16] = { 1.0, 1.5, 2.5, 4.0, 5.0, 7.0, 10.0, 12.0, 14.0, 17.0, 22.0, 30.0, 44.0, 64.0, 96.0, 128.0 };
const double OLD_VIBRATO_DEPTH[16] = { 1.0, 1.0, 2.0, 3.0, 4.0, 7.0, 8.0, 15.0, 16.0, 31.0, 32.0, 63.0, 64.0, 127.0, 128.0, 255.0 };

//https://wiki.nesdev.com/w/index.php/APU_Noise
const int NOISE_PERIOD[16] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

const int MAX_DPCM = 255;
const int MAX_INSTRUMENTS = 255;
const int MAX_CHANNELS = 5;
const int MAX_FRAMES = (pow(2, 16)-1)/2 / MAX_CHANNELS; //we are using 16 bytes for offsetting frames. since we are grabbing byte data, we must *2 in avr, which gives us half the maximum number of offsets
const int MAX_ROWS = 255;

struct Macro {
	uint8_t loop = -1;
	uint8_t release = -1;
	uint8_t mode = -1;
	vector<uint8_t> values;
};

struct DPCMSample {
	int id = -1;
	int length = -1;
	vector<int> samples;
};

struct Instrument {
	Macro* volume = NULL;
	Macro* arpeggio = NULL;
	Macro* pitch = NULL;
	Macro* hiPitch = NULL;
	Macro* duty = NULL;
};

struct DPCMKey {
	int octave = -1;
	int note = -1;
	int dpcmID = -1;
	int pitch = -1;
	int loop = -1;
	int unknown = -1;
	int dCounter = -1;
};

struct Frame {
	int channel[MAX_CHANNELS];
};

struct Channel {
	string note = "...";
	string instrument = "..";
	string volume = ".";
	string fx1 = "...";
	string fx2 = "...";
	string fx3 = "...";
	string fx4 = "...";
};

struct Row {
	vector<Channel*> channels;
};

struct Pattern {
	vector<Row*> rows;
};

int calculateTicksPerRow(int tempo, int speed);
void processRows(ofstream& output, vector<Row*>* rows, int channel, int speed, int numOfRows, int* prevVolume, int* volume, map<int, Instrument*>* instruments, int* prevInstrument, int* instrument, map<int, vector<DPCMKey*>*>* dpcmKeys);
void processEffect(ofstream& output, string fx, int* volume, int* prevVolume);
void calculateDelay(ofstream& output, int* delay);
int findMacroIndex(map<int, Macro*>* macros, Macro* macroTarget);
void processMacros(ofstream& output, map<int, Macro*>* macros, int macroType);
void generateNoteTable(ofstream& output);
void generateNoisePeriodTable(ofstream& output);
void generateVibratoTable(ofstream& output);
void generatePulseVolumeTable(ofstream& output);
void generateTNDVolumeTable(ofstream& output);

int main() {
	string fileName = "Corridors of Time.txt";
	ifstream file(fileName); //some .txt files won't read properly without, ios::binary
	ofstream output(fileName.substr(0, fileName.size() - 4) + "_OUTPUT.txt", std::ofstream::out | std::ofstream::trunc);

	if (file.fail()) {
		cout << "Error: Could not open the text file" << endl;
		exit(1);
	}

	generateNoteTable(output);
	generateNoisePeriodTable(output);
	generateVibratoTable(output);
	generatePulseVolumeTable(output);
	generateTNDVolumeTable(output);

	//READ AND STORE MACRO DATA
	map<int, Macro*> volumeMacros;
	map<int, Macro*> arpeggioMacros;
	map<int, Macro*> pitchMacros;
	map<int, Macro*> hiPitchMacros;
	map<int, Macro*> dutyMacros;

	string line;
	while (getline(file, line)) {
		if (line == "# Macros") {
			break;
		}
	}
	while (getline(file, line)) {
		istringstream ss(line);
		string data;
		ss >> data;
		if (data != "MACRO") {
			break;
		}
		else {
			ss >> data;
		}

		map<int, Macro*>* type = NULL;
		switch (stoi(data)) {
		case 0:
			type = &volumeMacros;
			break;
		case 1:
			type = &arpeggioMacros;
			break;
		case 2:
			type = &pitchMacros;
			break;
		case 3:
			type = &hiPitchMacros;
			break;
		case 4:
			type = &dutyMacros;
			break;
		}
		int id, loop, release, mode;
		vector<uint8_t> values;
		ss >> id >> loop >> release >> mode >> data;
		while (ss >> data) {
			values.push_back(stoi(data));
		}

		Macro* macro = new Macro;
		macro->loop = loop;
		macro->release = release;
		macro->mode= mode;
		macro->values = values;
		type->insert({ id, macro });
	}

	//int z = 0;
	//for (auto x : volumeMacros) {
	//	cout << z++ << ". ";
	//	cout << x.first << " " << x.second->values.back() << endl;
	//}

	//READ AND STORE DPCM DATA
	vector<DPCMSample*> dpcmSamples;

	while (getline(file, line)) {
		if (line == "# DPCM samples") {
			break;
		}
	}
	while (getline(file, line)) {
		istringstream ss(line);
		string data;
		ss >> data;
		if (data != "DPCMDEF") {
			if (data != "") {
				cout << "ERROR: Expected DPCMDEF but recieved " << data << "." << endl;
			}
			break;
		}

		int dpcmID, dpcmLength;
		vector<int> samples;
		ss >> dpcmID >> dpcmLength;
		for (int i = 0; i < dpcmLength; i++) {
			if (i % 32 == 0) {
				getline(file, line);
				ss.str(line);
				ss.clear();
				ss >> data >> data;					//get rid of "DPCM : "
			}
			ss >> data;
			samples.push_back(stoi(data, NULL, 16));
		}

		DPCMSample* sample = new DPCMSample;
		sample->id = dpcmID;
		sample->length = dpcmLength;
		sample->samples = samples;
		if (dpcmSamples.size() > MAX_DPCM) {
			cout << "ERROR: More than " << MAX_DPCM << " DPCM samples detected." << endl;
			exit(1);
		}
		else {
			dpcmSamples.push_back(sample);
		}
	}

	//READ AND STORE INSTRUMENT DATA
	map<int, Instrument*> instruments;
	map<int, vector<DPCMKey*>*> dpcmKeys;

	while (getline(file, line)) {
		if (line == "# Instruments") {
			break;
		}
	}
	while (getline(file, line)) {
		istringstream ss(line);
		string data;
		ss >> data;
		if (data != "INST2A03" && data != "KEYDPCM") {
			if (data != "") {
				cout << "ERROR: Non-INST2A03 instrument found." << endl;
				exit(1);
			}
			break;
		}

		if (data == "INST2A03") {
			int id, volumeID, arpeggioID, pitchID, hiPitchID, dutyID;
			ss >> id >> volumeID >> arpeggioID >> pitchID >> hiPitchID >> dutyID;
			Instrument* instrument = new Instrument;
			if (volumeMacros.count(volumeID) != 0) {
				instrument->volume = volumeMacros.at(volumeID);
			}
			if (arpeggioMacros.count(arpeggioID) != 0) {
				instrument->arpeggio = arpeggioMacros.at(arpeggioID);
			}
			if (pitchMacros.count(pitchID) != 0) {
				instrument->pitch = pitchMacros.at(pitchID);
			}
			if (hiPitchMacros.count(hiPitchID) != 0) {
				instrument->hiPitch = hiPitchMacros.at(hiPitchID);
			}
			if (dutyMacros.count(dutyID) != 0) {
				instrument->duty = dutyMacros.at(dutyID);
			}

			if (instruments.size() > MAX_INSTRUMENTS) {
				cout << "ERROR: More than " << MAX_INSTRUMENTS << " instruments detected. Tip: Delete any unused instruments in Edit > Clean up." << endl;
				exit(1);
			}
			else {
				instruments.insert({ id, instrument });
			}
		}
		else {
			int instID, octave, note, dpcmID, pitch, loop, unknown, dCounter;
			ss >> instID >> octave >> note >> dpcmID >> pitch >> loop >> unknown >> dCounter;
			DPCMKey* key = new DPCMKey;
			key->octave = octave;
			key->note = note;
			key->dpcmID = dpcmID;
			key->pitch = pitch;
			key->loop = loop;
			key->unknown = unknown;
			key->dCounter = dCounter;
			if (dpcmKeys.find(instID) == dpcmKeys.end()) {
				dpcmKeys.insert({ instID, new vector<DPCMKey*> });
			}
			dpcmKeys.at(instID)->push_back(key);
		}
	}

	//z = 0;
	//for (auto x : instruments) {
	//	cout << z++ << ". " << x.first;
	//	if (x.second->volume != NULL) {
	//		cout << " " << x.second->volume->values.back();
	//	}
	//	cout << endl;
	//}

	//TRACK DATA
	int tempo;
	int speed; //# of ticks per row in FTM
	int songNumber = -1;

	while (getline(file, line)) {
		if (line == "# Tracks") {
			break;
		}
	}
	while (getline(file, line)) {
		istringstream ss(line);
		string data;
		ss >> data;
		if (data == "TRACK") {
			songNumber++;

			//TIMING DATA
			int numOfRows;
			ss >> numOfRows;
			if (numOfRows > MAX_ROWS) {
				//This shouldn't ever be a problem because famitracker only allows up to 255 rows anyway.
				cout << "WARNING: More than " << MAX_ROWS << " rows per pattern. Rows above 255 will be ignored." << endl;
			}
			ss >> speed >> tempo >> data;
			speed = calculateTicksPerRow(tempo, speed);
			
			//COLUMN DATA
			getline(file, line);
			ss.str(line);
			ss.clear();
			ss >> data >> data;
			for (int i = 0; i < MAX_CHANNELS; i++) {
				ss >> data;
			}
			if (!ss.eof()) {
				cout << "WARNING: More than " << MAX_CHANNELS << " channels detected. Extra channels will be ignored." << endl;
			}

			//FRAME DATA
			vector<Frame*> frames;
			streampos prevLine = file.tellg();

			while (getline(file, line)) {
				ss.str(line);
				ss.clear();
				ss >> data;
				if (data == "ORDER") {
					int pattern;
					ss >> data >> data;
					
					Frame* frame = new Frame;
					for (int i = 0; i < MAX_CHANNELS; i++) {
						ss >> hex >> pattern;
						frame->channel[i] = pattern;
					}
					frames.push_back(frame);
				}
				else if (data == "PATTERN") {
					file.seekg(prevLine);
					break;
				}
				else {
					prevLine = file.tellg();
				}
				if (frames.size() > MAX_FRAMES) {
					cout << "Warning: More than " << MAX_FRAMES << " frames detected for song " << songNumber << ". Extra frames will be ignored." << endl;
				}
			}

			//z = 0;
			//for (auto x : frames) {
			//	cout << z++ << ". " << x->channel[4] << endl;
			//}

			//PATTERN DATA
			unordered_map<int, Pattern*> patterns;

			while (getline(file, line)) {
				Pattern* pattern = new Pattern;
				ss.str(line);
				ss.clear();
				ss >> data;
				if (data == "PATTERN") {
					int patternID;
					ss >> hex >> patternID;
					while (getline(file, line)) {
						ss.str(line);
						ss.clear();
						ss >> data;
						if (data == "ROW") {
							Row* row = new Row;

							ss >> data >> data;
							for (int maxChannels = MAX_CHANNELS; maxChannels > 0; maxChannels--) {
								Channel* channel = new Channel;
								string note, instrument, volume, fx1, fx2, fx3, fx4;
								ss >> note >> instrument >> volume >> fx1;

								ss >> data;
								if (data != ":") {
									fx2 = data;
									channel->fx2 = fx2;
									ss >> data;
									if (data != ":") {
										fx3 = data;
										channel->fx3 = fx3;
										ss >> data;
										if (data != ":") {
											fx4 = data;
											channel->fx4 = fx4;
											ss >> data;
										}
									}
								}

								channel->note = note;
								channel->instrument = instrument;
								channel->volume = volume;
								channel->fx1 = fx1;
								row->channels.push_back(channel);
							}
							if (pattern->rows.size() < MAX_ROWS) {
								pattern->rows.push_back(row);
							}
						}
						else if (data == "PATTERN" || data == "TRACK") {
							file.seekg(prevLine);
							break;
						}
						else {
							prevLine = file.tellg();
						}
					}
					patterns[patternID] = pattern;
				}
				else if (data == "TRACK") {
					file.seekg(prevLine);
					break;
				}
			}

			//z = 0;
			//for (auto x : patterns) {
			//	cout << "Pattern " << z++ << endl;
			//	int i = 0;
			//	for (auto y : x->rows) {
			//		cout << "Row " << i++ << ". ";
			//		for (auto p : y->channels) {
			//			cout << p->note;
			//		}
			//		cout << endl;
			//	}
			//}

			//PROCESS ALL TRACK DATA FOR OUTPUT
			vector<int> channelsUsedPatterns[MAX_CHANNELS];

			output << dec << "song" << songNumber << "_frames:" << endl; //song frames
			output << hex << "\t.dw 0x" << setfill('0') << setw(4) << frames.size() * 10 + 2 << dec << endl; //song size
			for (int frame = 0; frame < (MAX_FRAMES>frames.size() ? frames.size() : MAX_FRAMES); frame++) {
				output << "\t.dw ";
				for (int i = 0; i < MAX_CHANNELS; i++) {
					output << "song" << songNumber << "_channel" << i << "_pattern" << frames[frame]->channel[i];
					channelsUsedPatterns[i].push_back(frames[frame]->channel[i]);
					if (i == MAX_CHANNELS - 1) {
						output << endl;
					}
					else {
						output << ", ";
					}
				}
			}
			output << endl;
			
			for (int i = 0; i < MAX_CHANNELS; i++) { //filter any unused patterns
				sort(channelsUsedPatterns[i].begin(), channelsUsedPatterns[i].end());
				channelsUsedPatterns[i].erase(unique(channelsUsedPatterns[i].begin(), channelsUsedPatterns[i].end()), channelsUsedPatterns[i].end());
			}
			
			for (int i = 0; i < MAX_CHANNELS; i++) { //pattern data
				output << "song" << songNumber << "_channel" << i << "_patterns:" << endl;
				int prevVolume = -1; //-1 represents a silenced channel
				int volume = 15;
				for (int pattern = 0; pattern < channelsUsedPatterns[i].size(); pattern++) {
					output << "\tsong" << songNumber << "_channel" << i << "_pattern" << channelsUsedPatterns[i][pattern] << ": .db ";
					int prevInstrument = -1;
					int instrument = 0;
					if (i == 0 && pattern == 0) {
						output << "0xf0, " << "0x" << setfill('0') << setw(2) << hex << speed << ", ";
					}
					processRows(output, &patterns[channelsUsedPatterns[i][pattern]]->rows, i, speed, numOfRows, &prevVolume, &volume, &instruments, &prevInstrument, &instrument, &dpcmKeys);
					output << dec;
				}
				output << endl;
			}
		}
	}

	//PROCESS ALL DPCM SMAPLES
	output << "dpcm_samples:" << endl;
	for (int i = 0; i < dpcmSamples.size(); i++) {
		output << "\t.dw dcpm_sample_" << i << endl;
	}
	output << endl;
	for (int i = 0; i < dpcmSamples.size(); i++) {
		output << "dcpm_sample_" << i << ":" << endl;
		vector<int> sample = dpcmSamples.at(i)->samples;
		output << hex << "\t.db " << "0x" << setfill('0') << setw(2) << static_cast <int>(dpcmSamples.at(i)->length/16) << endl;
		for (int o = 0; o < sample.size(); o++) {
			if (o % 16 == 0) {
				output << "\t.db ";
			}
			output << "0x" << setfill('0') << setw(2) << static_cast <int>(sample[o]);
			if (o % 16 != 15 && o != sample.size()-1) {
				output << ", ";
			}
			else {
				output << endl;
			}
		}
		output << dec;
	}
	output << endl;

	//PROCESS ALL INSTRUMENT/MACRO DATA
	output << "instruments:" << endl;
	for (int i = 0; i < instruments.size(); i++) {
		output << "\t.dw instrument" << i << endl;
	}
	output << endl;

	int index = 0;
	for (auto instrument : instruments) {
		output << dec << "instrument" << index++ << ": ";
		
		string macroLabels = "";
		int usedMacros = 0; //each macro type will be specified by a binary bit
		if (instrument.second->volume != NULL) {
			usedMacros += 1; //bit 0
			macroLabels += ", volume_macro" + to_string(findMacroIndex(&volumeMacros, instrument.second->volume));
		}
		if (instrument.second->arpeggio != NULL) {
			usedMacros += 2; //bit 1
			macroLabels += ", arpeggio_macro" + to_string(findMacroIndex(&arpeggioMacros, instrument.second->arpeggio));
		}
		if (instrument.second->pitch != NULL) {
			usedMacros += 4; //bit 2
			macroLabels += ", pitch_macro" + to_string(findMacroIndex(&pitchMacros, instrument.second->pitch));
		}
		if (instrument.second->hiPitch != NULL) {
			usedMacros += 8; //bit 3
			macroLabels += ", hi_pitch_macro" + to_string(findMacroIndex(&hiPitchMacros, instrument.second->hiPitch));
		}
		if (instrument.second->duty != NULL) {
			usedMacros += 16; //bit 4
			macroLabels += ", duty_macro" + to_string(findMacroIndex(&dutyMacros, instrument.second->duty));
		}

		output << ".dw 0b" << bitset<8>(usedMacros) << macroLabels << endl;;
	}
	output << endl;

	processMacros(output, &volumeMacros, 0);
	processMacros(output, &arpeggioMacros, 1);
	processMacros(output, &pitchMacros, 2);
	processMacros(output, &hiPitchMacros, 3);
	processMacros(output, &dutyMacros, 4);
	cout << "Process complete" << endl;
}

int calculateTicksPerRow(int tempo, int ticksPerRow) {
	double multiplierTempo = (double)150 / tempo;
	if (floor(multiplierTempo) != multiplierTempo) {
		cout << "WARNING: Tempo does not divide evenly. Song BPM playback may be off. Read more at: http://famitracker.com/wiki/index.php?title=Effect_Fxx" << endl;
	}
	return multiplierTempo * ticksPerRow;
}

void processRows(ofstream &output, vector<Row*>* rows, int channel, int speed, int numOfRows, int *prevVolume, int* volume, map<int, Instrument*>* instruments, int* prevInstrument, int* instrument, map<int, vector<DPCMKey*>*>* dpcmKeys) {
	output << hex;
	bool emptyPattern = true;
	int noteNum = -1;
	int delay = 0;
	int prevVol = *prevVolume;
	int vol = *volume;
	int prevInst = *prevInstrument;
	int inst = *instrument;
	int dcpm = -1;
	for (int row = 0; row < rows->size(); row++) { //loop through each row
		if (rows->at(row)->channels.at(channel)->instrument != "..") { //get instrument data
			inst = stoi(rows->at(row)->channels.at(channel)->instrument, NULL, 16);
		}

		if (channel == 3) {							//noise channel
			if (NOISE_NOTES.find(rows->at(row)->channels.at(channel)->note) == NOISE_NOTES.end()) { //get note data
				cout << "WARNING: Noise periods must range between 0-# to F-#. An invalid period was found and will be ignored." << endl;
				noteNum = -1;
			}
			else {
				noteNum = NOISE_NOTES.at(rows->at(row)->channels.at(channel)->note);
			}
		}
		else if (channel == 4) {					//DPCM channel
			string dpcmNote = rows->at(row)->channels.at(channel)->note;
			if (DPCM_NOTES.find(dpcmNote.substr(0, 2)) == DPCM_NOTES.end()) {	//get note data
				cout << "WARNING: DPCM notes must range between C-0 to B-7. An invalid note was found and will be ignored." << endl;
				noteNum = -1;
			}
			else {
				noteNum = DPCM_NOTES.at(dpcmNote.substr(0, 2));
				if (noteNum != -1 && noteNum != -2 && noteNum != -3) {
					int dpcmNoteInt = noteNum;
					int dpcmOctave = stoi(dpcmNote.substr(2, 3));
					if (dpcmKeys->find(inst) == dpcmKeys->end()) {					//find corresponding DPCM key with instrument
						cout << "Warning: No existing DPCM keys associated with instrument " << inst << ". DPCM sample will be ignored" << endl;
						noteNum = -1;
					}
					else {
						for (auto x : *dpcmKeys->at(inst)) {
							if (x->note == dpcmNoteInt && x->octave == dpcmOctave) {
								noteNum = x->dpcmID;
								dcpm = x->pitch;
							}
						}
					}
				}
			}
		}
		else {										//pulse, triangle channel
			if (NOTES.find(rows->at(row)->channels.at(channel)->note) == NOTES.end()) { //get note data
				cout << "WARNING: Notes must range between C-0 to B-7. A note higher than B-7 was found and will be ignored." << endl;
				noteNum = -1;
			}
			else {
				noteNum = NOTES.at(rows->at(row)->channels.at(channel)->note);
			}
		}

		if (rows->at(row)->channels.at(channel)->volume != ".") { //get volume data
			if (vol != -1) {
				vol = stoi(rows->at(row)->channels.at(channel)->volume, NULL, 16);
			}
			else { //if the channel is silenced, only change the prevVol value. vol has to stay -1 in order to represent a silenced channel.
				prevVol = stoi(rows->at(row)->channels.at(channel)->volume, NULL, 16);
			}
		}

		string fx1 = rows->at(row)->channels.at(channel)->fx1; //get fx data
		string fx2 = rows->at(row)->channels.at(channel)->fx2;
		string fx3 = rows->at(row)->channels.at(channel)->fx3;
		string fx4 = rows->at(row)->channels.at(channel)->fx4;

		if (fx4.substr(0, 1) == "G") { //check for Gxx flag
			calculateDelay(output, &delay);
			output << "0xf1, " << "0x" << setfill('0') << setw(2) << fx4.substr(1, 2) << ", ";
		}
		else if (fx3.substr(0, 1) == "G") { //check for Gxx flag
			calculateDelay(output, &delay);
			output << "0xf1, " << "0x" << setfill('0') << setw(2) << fx3.substr(1, 2) << ", ";
		}
		else if (fx2.substr(0, 1) == "G") { //check for Gxx flag
			calculateDelay(output, &delay);
			output << "0xf1, " << "0x" << setfill('0') << setw(2) << fx2.substr(1, 2) << ", ";
		}
		else if (fx1.substr(0, 1) == "G") { //check for Gxx flag
			calculateDelay(output, &delay);
			output << "0xf1, " << "0x" << setfill('0') << setw(2) << fx1.substr(1, 2) << ", ";
		}

		if (noteNum != -1) { //output processed note data
			if (noteNum == -2) {
				calculateDelay(output, &delay);
				if (vol != -1) { //if the channel is not silenced, silence the channel and store the most recent volume level in the pattern
					prevVol = vol;
					vol = -1; //-1 represents a silenced channel
				}
				output << "0x" << setfill('0') << setw(2) << VOLUME_LEVELS::Zero << ", "; //"---" silences the channel
			}
			else if (noteNum == -3) {
				calculateDelay(output, &delay);
				output << "0xe4, "; //note release flag
			}
			else {
				calculateDelay(output, &delay);
				if (vol == -1) { //if the channel was silenced, unsilence the channel and use the most recent volume level in the pattern
					vol = prevVol;
					prevVol = -1;
				}
				output << "0x" << setfill('0') << setw(2) << noteNum << ", ";
				if (channel == 4) {							//DPCM channel sample rate
					output << "0x" << setfill('0') << setw(2) << dcpm << ", ";
				}
				emptyPattern = false;
			}
		}

		if ((prevVol != vol || rows->at(row)->channels.at(channel)->volume != ".") && vol != -1 && !emptyPattern) { //output volume data
			calculateDelay(output, &delay);
			prevVol = vol;
			output << "0x" << setfill('0') << setw(2) << vol + VOLUME_LEVELS::Zero << ", ";
		}

		if (prevInst != inst) { //output instrument data
			calculateDelay(output, &delay);
			prevInst = inst;
			output << "0xe3, "; //instrument change flag
			output << "0x" << setfill('0') << setw(2) << distance(instruments->begin(), instruments->find(inst)) << ", ";
		}

		if (fx1 != "...") { //output fx data
			calculateDelay(output, &delay);
			processEffect(output, fx1, &vol, &prevVol);
		}
		if (fx2 != "...") {
			calculateDelay(output, &delay);
			processEffect(output, fx2, &vol, &prevVol);
		}
		if (fx3 != "...") {
			calculateDelay(output, &delay);
			processEffect(output, fx3, &vol, &prevVol);
		}
		if (fx4 != "...") {
			calculateDelay(output, &delay);
			processEffect(output, fx4, &vol, &prevVol);
		}

		delay++;
	}
	calculateDelay(output, &delay);
	output << "0xff" << endl; //end of row pattern flag
	*prevVolume = prevVol;
	*volume = vol;
	*prevInstrument = prevInst;
	*instrument = inst;
}

void processEffect(ofstream& output, string fx, int* volume, int* prevVolume) {
	int vol = *volume;
	int prevVol = *prevVolume;
	string type = fx.substr(0, 1);
	int flag = EFFECTS.at(type);
	if (type != "G" && type != "H" && type != "I" && type != "J" && type != "W" && type != "X" && type != "Y" && type != "Z") {
		output << "0x" << setfill('0') << setw(2) << flag << ", "; //output the flag for the fx
	}

	uint8_t parameter = stoi(fx.substr(1, 3), NULL, 16);
	int x, y, z;
	switch (flag) {
	case 0xE5: //0xy arpeggio
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xE6: //1xx pitch slide up
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xE7: //2xx pitch slide down
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xE8: //3xx automatic portamento
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xE9: //4xy vibrato
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xEA: //7xy tremelo
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xEB: //Axy volume slide
		x = -(parameter & 0x0F); //slide down
		y = (parameter & 0xF0) >> 4; //slide up
		z = (x + y) * 10000 / 8; //*1000 to scale the total with 0.0625. /8 as per the Axy documentation on famitracker.

		z /= 625; //lowest binary fraction denomination with 4 bits is 0.0625
		parameter = z;
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xEC: //Bxx pattern jump
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xED: //Cxx halt
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xEE: //Dxx frame skip
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xEF: //Exx volume
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xF0: //Fxx speed and tempo
		if (parameter > 0x1F) {
			output << "ERROR: Fxx effect with a value not 0x00-0x1F" << endl;
		}
		else {
			output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		}
		break;
	case 0xF1: //Gxx note delay
		break;
	case 0xF2: //Hxy hardware sweep up
		break;
	case 0xF3: //Ixy hardware sweep down
		break;
	case 0xF4: //Hxx FDS modulation depth
		break;
	case 0xF5: //Jxx FDS modulation speed
		break;
	case 0xF6: //Pxx fine pitch
		output << "0x" << setfill('0') << setw(2) << static_cast <int>((uint8_t)(0x80-parameter)) << ", ";
		break;
	case 0xF7: //Qxy note slide up
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xF8: //Rxy note slide down
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xF9: //Sxx mute delay
		if (vol != -1) { //if the channel is not silenced, silence the channel and store the most recent volume level in the pattern
			prevVol = vol;
			vol = -1; //-1 represents a silenced channel
		}
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		output << "0x" << setfill('0') << setw(2) << VOLUME_LEVELS::Zero << ", ";
		break;
	case 0xFA: //Vxx duty or noise mode
		output << "0x" << setfill('0') << setw(2) << static_cast <int>(parameter) << ", ";
		break;
	case 0xFB: //Wxx DPCM sample speed
		break;
	case 0xFC: //Xxx DPCM sample retrigger
		break;
	case 0xFD: //DPCM sample offset
		break;
	case 0xFE: //DPCM delta counter
		break;
	}
	*volume = vol;
	*prevVolume = prevVol;
}

void calculateDelay(ofstream& output, int* delay) {
	int rows = *delay;
	if (rows != 0) {
		rows += VOLUME_LEVELS::Fifteen; //delay levels must range between the highest volume level (0x66) and the instrument flag (0xE3)
		for (rows; rows >= 0xE3; rows -= (0xE3 - VOLUME_LEVELS::Fifteen)) {
			output << "0xe2, ";
		}
		output << "0x" << setfill('0') << setw(2) << hex << rows << ", ";
		rows = 0;
	}
	*delay = rows;
}

int findMacroIndex(map<int, Macro*>* macros, Macro* macroTarget) {
	int index = 0;
	for (auto x : *macros) {
		if (x.second == macroTarget) {
			return index;
		}
		index++;
	}
	return -1;
}

void processMacros(ofstream& output, map<int, Macro*>* macros, int macroType) {
	if (macros->empty()) {
		return;
	}
	int index = 0;
	int offset = 2;
	string macroLabel = "";
	string endFlag = "0xff";

	switch (macroType) {
	case 0:
		macroLabel = "volume_macro";
		break;
	case 1:
		macroLabel = "arpeggio_macro";
		endFlag = "0x80"; //-1 in two's complement is 0xff, so we use the value for -128 (0x80) as the end flag
		offset = 3; //since the mode flag is stored at the 0th index, we have to offset all value indexes by an extra 1
		break;
	case 2:
		macroLabel = "pitch_macro";
		endFlag = "0x80";
		break;
	case 3:
		macroLabel = "hi_pitch_macro";
		endFlag = "0x80";
		break;
	case 4:
		macroLabel = "duty_macro";
		break;
	}

	for (auto macro: *macros) {
		output << dec << macroLabel << index++ << ": .db ";
		if (macro.second->release != (uint8_t)-1) { //check for release flag
			output << "0x" << setfill('0') << setw(2) << hex << (static_cast<int>(macro.second->release) + offset) << ", ";
		}
		else { //if there is no release flag
			output << "0xff, ";
		}
		if (macro.second->loop != (uint8_t)-1) { //check for loop flag
			output << "0x" << setfill('0') << setw(2) << hex << (static_cast<int>(macro.second->loop) + offset) << ", ";
		}
		else {
			if (macroType == 0) { //the volume loop flag will be set to loop back to the last available value
				output << "0x" << setfill('0') << setw(2) << hex << (macro.second->values.size() - 1 + offset) << ", ";
			}
			else {
				output << "0xff, ";
			}
		}
		if (macroType == 1) { //if the macro is an arpeggio macro
			output << "0x" << setfill('0') << setw(2) << hex << static_cast<int>(macro.second->mode) << ", "; //flag to store arpeggio mode is written first
		}

		for (int i = 0; i < macro.second->values.size(); i++) {
			uint8_t value = macro.second->values.at(i);
			output << "0x" << setfill('0') << setw(2) << hex << static_cast<int>(value) << ", ";
		}
		output << endFlag << endl;
	}
	output << endl;
}

void generateNoteTable(ofstream& output) {
	double a = 27.5;
	double ratio = pow(2.0, 1.0 / 12.0);

	output << "note_table: " << endl;
	output << "\t.dw ";
	for (int i = 0; i < 3; i++) {
		double frequency = a * pow(ratio, i) * 2;  //*2 because famitracker notes are calculated with twice the frequency of the note
		int timerPeriod = round((11.1746014718 * ( (1789773.0) / (16 * frequency) )));
		output << "0x" << setfill('0') << setw(4) << hex << timerPeriod;

		if (i != 2) {
			output << ", ";
		}
	}
	output << endl;

	for (int i = 1; i <= 7; i++) {
		output << "\t.dw ";
		for (int o = 3; o < 15; o++) {
			if (o == 12) {
				a *= 2;
			}

			double frequency = a * pow(ratio, o%12) * 2; //*2 because famitracker notes are calculated with twice the frequency of the note
			int timerPeriod = round(11.1746014718 * ((1789773.0) / (16 * frequency)));
			output << "0x" << setfill('0') << setw(4) << hex << timerPeriod;

			if (o != 14) {
				output << ", ";
			}
		}
		output << endl;
	}
	output << endl;
}

//unsure if the period table on the NES wiki is incorrect or not, but dividing the periods by 2 hits the correct frequencies.
void generateNoisePeriodTable(ofstream& output) {
	output << "noise_period_table: .dw ";
	for (int i = 0; i < 16; i++) {
		output << "0x" << setfill('0') << setw(4) << hex << (uint16_t)((NOISE_PERIOD[i]/2+1) * 11.1746014718);
		if (i != 15) {
			output << ", ";
		}
		else {
			output << endl;
		}
	}
	output << endl;
}

void generateVibratoTable(ofstream& output) {
	output << "vibrato_table: " << endl;
	for (int i = 0; i < 16; ++i) {   // depth
		output << "\t.db ";
		for (int j = 0; j < 16; ++j) {   // phase
			int value = 0;
			double angle = (double(j) / 16.0) * (3.1415 / 2.0);
			value = int(sin(angle) * VIBRATO_DEPTH[i]); //new vibrato table
			//value = (int)((double(j * OLD_VIBRATO_DEPTH[i]) / 16.0) + 1);
			output << "0x" << setfill('0') << setw(2) << hex << value;

			if (j != 15) {
				output << ", ";
			}
		}
		output << endl;
	}
	output << endl;
}

//https://wiki.nesdev.com/w/index.php/APU_Mixer
void generatePulseVolumeTable(ofstream& output) {
	output << "pulse_volume_table: " << endl;
	for (int i = 0; i < 31; i++) {
		if (i % 16 == 0) {
			output << "\t.db ";
		}

		int volume = round((95.52 / (8128.0 / i + 100)) * 255);
		output << "0x" << setfill('0') << setw(2) << hex << volume;

		if (i % 16 != 15 && i != 30) {
			output << ", ";
		}
		else {
			output << endl;
		}
	}
	output << endl;
}

void generateTNDVolumeTable(ofstream& output) {
	output << "tnd_volume_table: " << endl;
	for (int i = 0; i < 203; i++) {
		if (i % 16 == 0) {
			output << "\t.db ";
		}

		int volume = round((163.67 / (24329.0 / i + 100)) * 255);
		output << "0x" << setfill('0') << setw(2) << hex << volume;

		if (i % 16 != 15 && i != 202) {
			output << ", ";
		}
		else {
			output << endl;
		}
	}
	output << endl;
}