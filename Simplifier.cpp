// simplifier.cpp
// Compile: g++ -std=c++17 -O2 -o simplifier simplifier.cpp
// Usage: cat input.txt | ./simplifier
//
// The program expects a single machine encoding ⟨M⟩ on stdin, e.g.:
//  (example piece)  0#_#1#_#0;0#0#2#1#11; ...  (no whitespace required)
//
// Notes:
// - State tokens are expected as binary strings (e.g. "0" for q0, "10" for q2).
// - Symbols are single chars: '0', '1', or '_' (underscore for blank).
// - Direction codes are binary strings per assignment spec: "0","10","11","00","01".
// - Output is ⟨M'⟩ as semicolon-separated instructions, same format as input,
//   but with directions restricted to "10" (L) and "00" (R).
//
// The program is defensive about whitespace and an optional trailing semicolon.

#include <bits/stdc++.h>
using namespace std;

struct Inst {
    int src;
    char read;   // '_' '0' '1'
    int dst;
    char write;  // '_' '0' '1'
    string dir;  // binary code like "0","10","11","00","01"
};

// helper: trim whitespace from a string
static inline string trim_all(string s) {
    string out;
    for(char c: s) if(!isspace((unsigned char)c)) out.push_back(c);
    return out;
}

int binToInt(const string &s) {
    // accepts string of '0'/'1'; if it contains other digits, try stoi as decimal
    int val = 0;
    bool all01 = true;
    for(char c: s) if(c!='0' && c!='1') { all01 = false; break; }
    if(!all01) {
        try { return stoi(s); } catch(...) { return 0; }
    }
    for(char c: s) {
        val = (val<<1) + (c - '0');
    }
    return val;
}

string intToBin(int x) {
    if(x == 0) return "0";
    string s;
    while(x>0) {
        s.push_back(char('0' + (x & 1)));
        x >>= 1;
    }
    reverse(s.begin(), s.end());
    return s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // read entire stdin
    string input_all;
    //By the way hei ndi a temporary scope just uri ndi kone u collect all input lines into a
    //single string, preserving potential semicolons across lines (we'll trim spaces later).
    {
        string tmp;
        //The reason why ndi chi co shumisa while loop ndi ngauri ndi loop ine yado vhala
        //everything ca termial zwine zwa allow uri ndi capture even very long turing machines
        while(getline(cin, tmp)) {
            input_all += tmp;
        }
    }
    input_all = trim_all(input_all);
    if(input_all.empty()) {
        return 0;
    }

    //Hafha ndi co tokenize the input string into instructions, splitting on semicolons.
    vector<string> tokens;
    size_t pos = 0;
    //Ca hei loop rico break the input_all string into zwi'token \wine ra zwi store in a vector<string> tokens, each token representing one instruction (the part between semicolons).
    while(pos < input_all.size()) {
        size_t sc = input_all.find(';', pos);
        //string::npos in the code below is a special value indicating "not found"; if we don't find a semicolon, we take the rest of the string as the last token.
        if(sc == string::npos) sc = input_all.size();
        string tk = input_all.substr(pos, sc - pos);
        if(!tk.empty()) tokens.push_back(tk);
        pos = sc + 1;
    }
    //Hei ndi vector ine ya do store Inst structures ze razi define mathomoni a program
    vector<Inst> insts;
    int maxState = 0;
    // parse each token, expecting 5 parts separated by '#'
    for(auto &tk: tokens) {
        //Hafha we're splitting each instruction token into its components: src, read, dst, write, dir. We expect them to be separated by '#' characters. We loop through the token string to find these parts and store them in a vector<string> parts.
        vector<string> parts;
        size_t p = 0;
        while(p < tk.size()) {
            size_t h = tk.find('#', p);
            if(h == string::npos) h = tk.size();
            parts.push_back(tk.substr(p, h - p));
            p = h + 1;
        }
        if(parts.size() != 5) {
            // malformed token: skip silently (or could error)
            continue;
        }
        string s_src = parts[0];
        string s_read = parts[1];
        string s_dst = parts[2];
        string s_write = parts[3];
        string s_dir = parts[4];

        int src = binToInt(s_src);
        int dst = binToInt(s_dst);
        char read = s_read.empty() ? '_' : s_read[0];
        char write = s_write.empty() ? '_' : s_write[0];

        maxState = max(maxState, max(src, dst));
        insts.push_back({src, read, dst, write, s_dir});
    }

    // Build lookup: for each (state, symbol) -> instruction.
    // Symbol order in spec: ⊔ (blank '_' ), 0, 1
    auto symIndex = [&](char c)->int{
        if(c == '_') return 0;
        if(c == '0') return 1;
        return 2; // '1' or others
    };
    const vector<char> symbols = {'_', '0', '1'};

    // map from (state, read) to instruction index
    unordered_map<long long, int> inst_map;
    for(size_t i=0;i<insts.size();++i){
        long long key = ( (long long)insts[i].src << 32 ) | (unsigned long long)(unsigned char)insts[i].read;
        inst_map[key] = (int)i;
    }

    // We'll produce output instructions in two phases:
    //  - updated original states' transitions (states 0..maxState)
    //  - transitions for newly added intermediate states (in creation order)
    vector<Inst> out_insts;           // final output instructions
    vector<Inst> added_state_insts;   // for new states; appended after originals
    int nextNewState = maxState + 1;

    // For deterministic creation of added-state transition groups, keep record of each new state's transitions
    struct NewStateGroup { int id; vector<Inst> transitions; };
    vector<NewStateGroup> newGroups;

    // Process original states in order
    for(int state = 0; state <= maxState; ++state) {
        // For each symbol in order '_', '0', '1' (spec orders ⊔,0,1)
        for(char sym : symbols) {
            long long key = ( (long long)state << 32 ) | (unsigned long long)(unsigned char)sym;
            auto it = inst_map.find(key);
            if(it == inst_map.end()) {
                // If a transition is missing in input, skip (but spec says all will exist).
                continue;
            }
            Inst orig = insts[it->second];
            string d = orig.dir;

            if(d == "0") {
                // S -> simulate with R then L (create new state)
                int newState = nextNewState++;
                // first: src, read -> newState, write, R ("00")
                out_insts.push_back({orig.src, orig.read, newState, orig.write, string("00")});
                // second: for each symbol x: newState, x -> dst, x, L ("10")
                NewStateGroup g;
                g.id = newState;
                for(char x: symbols) {
                    g.transitions.push_back({newState, x, orig.dst, x, string("10")});
                }
                newGroups.push_back(std::move(g));
            } else if(d == "11") {
                // L2 -> simulate as L then L (create new state)
                int newState = nextNewState++;
                out_insts.push_back({orig.src, orig.read, newState, orig.write, string("10")}); // L
                NewStateGroup g;
                g.id = newState;
                for(char x: symbols) {
                    g.transitions.push_back({newState, x, orig.dst, x, string("10")}); // L
                }
                newGroups.push_back(std::move(g));
            } else if(d == "01") {
                // R2 -> simulate as R then R
                int newState = nextNewState++;
                out_insts.push_back({orig.src, orig.read, newState, orig.write, string("00")}); // R
                NewStateGroup g;
                g.id = newState;
                for(char x: symbols) {
                    g.transitions.push_back({newState, x, orig.dst, x, string("00")}); // R
                }
                newGroups.push_back(std::move(g));
            } else {
                // L ("10") or R ("00") or unknown -> keep unchanged (pass-through)
                out_insts.push_back(orig);
            }
        }
    }

    // Append added states' transitions in creation order
    for(auto &g : newGroups) {
        for(auto &t : g.transitions) {
            out_insts.push_back(t);
        }
    }

    // Produce output string: join with ';'
    // For states and directions we must use the binary encodings per spec.
    auto dirEncode = [&](const string &d)->string {
        // we store directions already encoded as "00"/"10" etc.
        return d;
    };

    bool first = true;
    ostringstream oss;
    for(size_t i=0;i<out_insts.size();++i) {
        const Inst &it = out_insts[i];
        if(!first) oss << ";";
        first = false;
        oss << intToBin(it.src) << "#" << it.read << "#" << intToBin(it.dst) << "#" << it.write << "#" << dirEncode(it.dir);
    }
    string out = oss.str();
    out = out + ";"; // optional trailing semicolon per spec
    cout << out << "\n";
    return 0;
}