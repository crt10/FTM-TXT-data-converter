#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
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
	{"...", -1},  {"---", -2}
};

enum VOLUME_LEVELS {
	Zero = 0x57, One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Eleven, Twelve, Thirteen, Fourteen, Fifteen //Fifteen == 0x66, if Zero == 0x57
};

const int MAX_CHANNELS = 5;
const int MAX_FRAMES = (pow(2, 16)-1)/2 / MAX_CHANNELS; //we are using 16 bytes for offsetting frames. since we are grabbing byte data, we must *2 in avr, which gives us half the maximum number of offsets
const int MAX_ROWS = 255;

struct Macro {
	int loop = -1;
	int unknown = -1;
	int mode = -1;
	vector<int> values;
};

struct Instrument {
	Macro* volume = NULL;
	Macro* arpeggio = NULL;
	Macro* pitch = NULL;
	Macro* hiPitch = NULL;
	Macro* duty = NULL;
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
void processRows(ofstream& output, vector<Row*>* rows, int channel, int speed, int numOfRows, int* prevVolume, int* volume);
void calculateDelay(ofstream& output, int delay, int speed, int numOfRows);
void generateNoteTable(ofstream& output);

int main() {
	string fileName = "Touhou 6 - Shanghai Teahouse -Chinise Tea-.txt";
	ifstream file(fileName); //some .txt files won't read properly without, ios::binary
	ofstream output(fileName.substr(0, fileName.size() - 4) + "_OUTPUT.txt", std::ofstream::out | std::ofstream::trunc);

	if (file.fail()) {
		cout << "Error: Could not open the text file" << endl;
		exit(1);
	}

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
		int id, loop, unknown, mode;
		vector<int> values;
		ss >> id >> loop >> unknown >> mode >> data >> data;
		do {
			values.push_back(stoi(data));
			ss >> data;
		} while (!ss.eof());

		Macro* macro = new Macro;
		macro->loop = loop;
		macro->unknown = unknown;
		macro->mode= mode;
		macro->values = values;
		type->insert({ id, macro });
	}

	//int z = 0;
	//for (auto x : volumeMacros) {
	//	cout << z++ << ". ";
	//	cout << x.first << " " << x.second->values.back() << endl;
	//}

	//READ AND STORE DPCM DATA (unimplemented)

	//READ AND STORE INSTRUMENT DATA
	map<int, Instrument*> instruments;

	while (getline(file, line)) {
		if (line == "# Instruments") {
			break;
		}
	}
	while (getline(file, line)) {
		istringstream ss(line);
		string data;
		ss >> data;
		if (data != "INST2A03") {
			break;
		}

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
		instruments.insert({ id, instrument });
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
							for (int maxChannels = 5; maxChannels > 0; maxChannels--) {
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
							pattern->rows.push_back(row);
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

			output << "song" << songNumber << "_frames:" << endl; //song frames
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
					cout << volume << endl;
					output << "\tsong" << songNumber << "_channel" << i << "_pattern" << channelsUsedPatterns[i][pattern] << ": .dw ";
					processRows(output, &patterns[channelsUsedPatterns[i][pattern]]->rows, i, speed, numOfRows, &prevVolume, &volume);
					output << dec;
				}
				output << endl;
			}
		}
	}
	generateNoteTable(output);
	cout << "Process complete" << endl;
}

int calculateTicksPerRow(int tempo, int ticksPerRow) {
	double multiplierTempo = (double)150 / tempo;
	if (floor(multiplierTempo) != multiplierTempo) {
		cout << "WARNING: Tempo does not divide evenly. Song BPM playback may be off. Read more at: http://famitracker.com/wiki/index.php?title=Effect_Fxx" << endl;
	}
	return multiplierTempo * ticksPerRow;
}

void processRows(ofstream &output, vector<Row*>* rows, int channel, int speed, int numOfRows, int *prevVolume, int* volume) {
	output << hex;
	int noteNum = -1;
	int delay = 0;
	int prevVol = *prevVolume;
	int vol = *volume;
	for (int row = 0; row < rows->size(); row++) { //loop through each row
		if (NOTES.find(rows->at(row)->channels.at(channel)->note) == NOTES.end()) { //get note data
			cout << "WARNING: Notes must range between C-0 to B-7. A note higher than larger B-7 was found and will be ignored." << endl;
		}
		else {
			noteNum = NOTES.at(rows->at(row)->channels.at(channel)->note);
		}
		if (rows->at(row)->channels.at(channel)->volume != ".") { //get volume data
			if (vol != -1) {
				vol = stoi(rows->at(row)->channels.at(channel)->volume, NULL, 16);
			}
			else { //if the channel is silenced, only change the prevVol value. vol has to stay -1 in order to represent a silenced channel.
				prevVol = stoi(rows->at(row)->channels.at(channel)->volume, NULL, 16);
			}
		}

		if (noteNum != -1) {
			if (noteNum == -2) { //output processed note data
				if (delay != 0) {
					calculateDelay(output, delay, speed, numOfRows);
					delay = 0;
				}
				if (vol != -1) { //if the channel is not silenced, silence the channel and store the most recent volume level in the pattern
					prevVol = vol;
					vol = -1; //-1 represents a silenced channel
				}
				output << "0x" << setfill('0') << setw(2) << VOLUME_LEVELS::Zero << ", "; //"---" silences the channel
			}
			else {
				if (delay != 0) {
					calculateDelay(output, delay, speed, numOfRows);
					delay = 0;
				}
				if (vol == -1) { //if the channel was silenced, unsilence the channel and use the most recent volume level in the pattern
					vol = prevVol;
					prevVol = -1;
				}
				output << "0x" << setfill('0') << setw(2) << noteNum << ", ";
			}
		}
		if (prevVol != vol && vol != -1) { //output volume data
			if (delay != 0) {
				calculateDelay(output, delay, speed, numOfRows);
				delay = 0;
			}
			prevVol = vol;
			output << "0x" << setfill('0') << setw(2) << vol + VOLUME_LEVELS::Zero << ", ";
		}
		delay++;
	}
	if (delay != 0) {
		calculateDelay(output, delay, speed, numOfRows);
	}
	output << "0xFF" << endl; //end of row pattern flag
	*prevVolume = prevVol;
	*volume = vol;
}

void calculateDelay(ofstream& output, int delay, int speed, int numOfRows) {
	int numOfCycles = delay * speed; //this is the number of NES frame sequences to wait. NES frame sequencer clocks at 60Hz (NTSC)
	numOfCycles += VOLUME_LEVELS::Fifteen; //delay levels must range between the highest volume level (0x66) and the instrument flag (0xE4)
	for (numOfCycles; numOfCycles >= 0xE4; numOfCycles -= (0xE4-VOLUME_LEVELS::Fifteen)) {
		output << "0xE3, ";
	}
	output << "0x" << setfill('0') << setw(2) << hex << numOfCycles << ", ";
}

void generateNoteTable(ofstream& output) {
	double a = 27.5;
	double ratio = pow(2.0, 1.0 / 12.0);

	output << "note_table: " << endl;
	output << "\t.dw ";
	for (int i = 0; i < 3; i++) {
		double frequency = a * pow(ratio, i);
		int timerPeriod = round(11.1746014718 * ((1789773.0) / (16 * frequency)));
		output << "0x" << setfill('0') << setw(4) << hex << timerPeriod;
		if (i != 2) {
			output << ", ";
		}
	}
	output << endl;

	for (int i = 1; i <= 7; i++) {
		output << "\t.dw ";
		for (int i = 3; i < 15; i++) {
			if (i == 12) {
				a *= 2;
			}
			double frequency = a * pow(ratio, i%12);
			int timerPeriod = round(11.1746014718 * ((1789773.0) / (16 * frequency)));
			output << "0x" << setfill('0') << setw(4) << hex << timerPeriod;
			if (i != 14) {
				output << ", ";
			}
		}
		output << endl;
	}
}