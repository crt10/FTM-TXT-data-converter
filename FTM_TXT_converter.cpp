#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using namespace std;

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
}