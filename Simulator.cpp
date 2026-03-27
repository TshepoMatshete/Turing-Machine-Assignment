#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Transition {
    int nextState;
    char writeSymbol;
    int direction;
};

struct PairHash {
    size_t operator()(const pair<int, char>& p) const {
        return hash<int>()(p.first) ^ (hash<char>()(p.second) << 16);
    }
};

using DeltaMap = unordered_map<pair<int, char>, Transition, PairHash>;

char binaryChunkToSymbol(const string& chunk) {
    if (chunk == "000") return '0';
    if (chunk == "001") return '1';
    if (chunk == "010") return '_';
    if (chunk == "100") return '#';
    if (chunk == "101") return ';';
    return '?';
}

string decodeBinary(const string& encoded) {
    string decoded;
    for (size_t i = 0; i + 2 < encoded.size(); i += 3) {
        char symbol = binaryChunkToSymbol(encoded.substr(i, 3));
        if (symbol != '?') {
            decoded += symbol;
        }
    }
    return decoded;
}

int decodeState(const string& bits) {
    int value = 0;
    for (char c : bits) {
        value = value * 2 + (c - '0');
    }
    return value;
}

string encodeState(int state) {
    if (state == 0) return "0";

    string bits;
    while (state > 0) {
        bits = char('0' + (state & 1)) + bits;
        state >>= 1;
    }
    return bits;
}

DeltaMap parseMachine(const string& encodedMachine) {
    DeltaMap delta;
    string decoded = decodeBinary(encodedMachine);

    stringstream outer(decoded);
    string transitionToken;

    while (getline(outer, transitionToken, ';')) {
        if (transitionToken.empty()) continue;

        vector<string> parts;
        stringstream inner(transitionToken);
        string part;

        while (getline(inner, part, '#')) {
            parts.push_back(part);
        }

        if (parts.size() != 5) continue;

        int currentState = decodeState(parts[0]);
        char readSymbol = parts[1][0];
        int nextState = decodeState(parts[2]);
        char writeSymbol = parts[3][0];
        int direction = (parts[4] == "10") ? -1 : 1;

        delta[{currentState, readSymbol}] = {nextState, writeSymbol, direction};
    }

    return delta;
}

string buildConfig(const vector<char>& tape, int head, int state) {
    int minPos = head;
    int maxPos = head;

    for (int i = 0; i < (int)tape.size(); i++) {
        if (tape[i] != '_') {
            if (i < minPos) minPos = i;
            if (i > maxPos) maxPos = i;
        }
    }

    minPos = min(minPos, head);
    maxPos = max(maxPos, head);

    string leftPart, rightPart;

    for (int i = minPos; i < head; i++) {
        leftPart += tape[i];
    }

    for (int i = head; i <= maxPos; i++) {
        rightPart += tape[i];
    }

    if (rightPart.empty()) {
        rightPart = "_";
    }

    size_t leftStart = leftPart.find_first_not_of('_');
    if (leftStart == string::npos) {
        leftPart.clear();
    } else {
        leftPart = leftPart.substr(leftStart);
    }

    size_t rightEnd = rightPart.find_last_not_of('_');
    if (rightEnd == string::npos) {
        rightPart = string(1, rightPart[0]);
    } else {
        rightPart = rightPart.substr(0, rightEnd + 1);
    }

    return leftPart + "#" + encodeState(state) + "#" + rightPart;
}

int main() {
    string encodedMachine;
    string inputStr;

    getline(cin, encodedMachine);
    getline(cin, inputStr);

    encodedMachine.erase(remove(encodedMachine.begin(), encodedMachine.end(), '\r'), encodedMachine.end());
    inputStr.erase(remove(inputStr.begin(), inputStr.end(), '\r'), inputStr.end());

    DeltaMap delta = parseMachine(encodedMachine);

    const int OFFSET = 2000000;
    const int TAPE_SIZE = 4200000;
    const int MAX_STEPS = 1000000;

    vector<char> tape(TAPE_SIZE, '_');

    int head = OFFSET;
    if (inputStr != "_") {
        for (int i = 0; i < (int)inputStr.size(); i++) {
            tape[OFFSET + i] = inputStr[i];
        }
    }

    int state = 0;
    const int Q_ACCEPT = 1;
    const int Q_REJECT = 2;

    string lastConfig;
    int stepCount = 0;

    for (int step = 0; step < MAX_STEPS; step++) {
        if (state == Q_ACCEPT || state == Q_REJECT) break;

        char currentSymbol = tape[head];
        auto it = delta.find({state, currentSymbol});

        if (it == delta.end()) break;

        tape[head] = it->second.writeSymbol;
        state = it->second.nextState;
        head += it->second.direction;
        stepCount++;

        if (stepCount == MAX_STEPS) {
            lastConfig = buildConfig(tape, head, state);
            break;
        }
    }

    if (lastConfig.empty()) {
        lastConfig = buildConfig(tape, head, state);
    }

    cout << lastConfig << endl;
    return 0;
}
