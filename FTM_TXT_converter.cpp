#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using namespace std;

int calculateTicksPerRow(int tempo, int speed);

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
	int channel0 = 0; //Pulse 1
	int channel1 = 0; //Pulse 2
	int channel2 = 0; //Triangle
	int channel3 = 0; //Noise
	int channel4 = 0; //DPCM
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

int main() {
	string fileName = "Touhou 6 - Shanghai Teahouse -Chinise Tea-.txt";
	fstream file(fileName);

	if (file.fail()) {
		cout << "Could not open" << endl;
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

	int z = 0;
	for (auto x : volumeMacros) {
		cout << z++ << ". ";
		cout << x.first << " " << x.second->values.back() << endl;
	}

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

	z = 0;
	for (auto x : instruments) {
		cout << z++ << ". " << x.first;
		if (x.second->volume != NULL) {
			cout << " " << x.second->volume->values.back();
		}
		cout << endl;
	}

	//TRACK DATA
	int tempo;
	int speed; //# of ticks per row in FTM
	int songNumber = 0;

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
			//TIMING DATA
			ss >> data >> speed >> tempo >> data;
			speed = calculateTicksPerRow(tempo, speed);
			
			//COLUMN DATA
			getline(file, line);
			ss.str(line);
			ss.clear();
			ss >> data >> data >> data >> data >> data >> data >> data;
			if (!ss.eof()) {
				cout << "WARNING: More than 5 channels detected. Extra channels will be ignored." << endl;
			}

			//FRAME DATA
			vector<Frame*> frames;
			streampos prevLine = file.tellg();

			while (getline(file, line)) {
				ss.str(line);
				ss.clear();
				ss >> data;
				if (data == "ORDER") {
					int channel0, channel1, channel2, channel3, channel4;
					ss >> data >> data >> hex >> channel0 >> channel1 >> channel2 >> channel3 >> channel4;
					
					Frame* frame = new Frame;
					frame->channel0 = channel0;
					frame->channel1 = channel1;
					frame->channel2 = channel2;
					frame->channel3 = channel3;
					frame->channel4 = channel4;
					frames.push_back(frame);
				}
				else if (data == "PATTERN") {
					file.seekg(prevLine);
					break;
				}
				else {
					prevLine = file.tellg();
				}
			}

			z = 0;
			for (auto x : frames) {
				cout << z++ << ". " << x->channel1 << endl;
			}

			//PATTERN DATA
			vector<Pattern*> patterns;

			while (getline(file, line)) {
				Pattern* pattern = new Pattern;
				ss.str(line);
				ss.clear();
				ss >> data;
				if (data == "PATTERN") {
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
						else if (data == "PATTERN") {
							file.seekg(prevLine);
							break;
						}
						else {
							prevLine = file.tellg();
						}
					}
					patterns.push_back(pattern);
				}
			}

			z = 0;
			for (auto x : patterns) {
				cout << "Pattern " << z++ << endl;
				int i = 0;
				for (auto y : x->rows) {
					cout << "Row " << i++ << ". ";
					for (auto p : y->channels) {
						cout << p->note;
					}
					cout << endl;
				}
			}

			//PROCESS ALL TRACK DATA FOR OUTPUT

		}
	}
	cout << "Process complete" << endl;
}

int calculateTicksPerRow(int tempo, int ticksPerRow) {
	double multiplierTempo = (double)150 / tempo;
	return multiplierTempo * ticksPerRow;
}