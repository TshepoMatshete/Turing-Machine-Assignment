#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>

using namespace std;

unordered_map<char, string> parseLibrary(const string& lib) {
    unordered_map<char, string> encoding;
    stringstream ss(lib);
    string pair;

    while (getline(ss, pair, '|')) {
        char key = pair[0];                
        string value = pair.substr(2);    
        encoding[key] = value;
    }

    return encoding;
}

string encode(const string& lib, const string& text) {
    unordered_map<char, string> encoding = parseLibrary(lib);
    string result = "";

    for (char c : text) {
        if (encoding.count(c)) {
            result += encoding[c];
        }
    }

    return result;
}

int main() {
    string library,text;

    
    getline(cin, library);


    getline(cin, text);

    string encoded = encode(library, text);

    cout << encoded << endl;

    return 0;
}