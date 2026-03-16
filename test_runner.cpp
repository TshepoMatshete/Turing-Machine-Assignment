#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;
using namespace std;

/* Terminal colors */
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string CYAN = "\033[36m";
const string BOLD = "\033[1m";

/* Read entire file */
string readFile(const string& path) {
    ifstream file(path);
    return string((istreambuf_iterator<char>(file)),
                   istreambuf_iterator<char>());
}

int main() {

    string sourceFile = "solution.cpp";
    string testsDir = "tests";
    string executable = "solution_exec";

    cout << CYAN << BOLD << "\n========== C++ Test Runner ==========\n" << RESET;

    /* Compile */
    cout << CYAN << "Compiling " << sourceFile << "...\n" << RESET;

    string compileCmd = "g++ -std=c++17 " + sourceFile + " -o " + executable;
    int compileResult = system(compileCmd.c_str());

    if (compileResult != 0) {
        cout << RED << "Compilation Failed ❌\n" << RESET;
        return 1;
    }

    cout << GREEN << "Compilation Successful ✔\n\n" << RESET;

    vector<string> tests;

    for (auto& entry : fs::directory_iterator(testsDir)) {
        if (entry.path().extension() == ".in") {
            tests.push_back(entry.path().stem().string());
        }
    }

    int passed = 0;
    int total = tests.size();

    for (int i = 0; i < total; i++) {

        string name = tests[i];
        string inputFile = testsDir + "/" + name + ".in";
        string outputFile = testsDir + "/" + name + ".out";

        string tempOutput = "temp_output.txt";

        cout << BOLD << CYAN << "Running Test: " << name << RESET << endl;

        string runCmd = "./" + executable + " < " + inputFile + " > " + tempOutput;
        system(runCmd.c_str());

        string expected = readFile(outputFile);
        string actual = readFile(tempOutput);

        if (expected == actual) {
            cout << GREEN << "PASS ✔\n\n" << RESET;
            passed++;
        } else {

            cout << RED << "FAIL ❌\n" << RESET;

            cout << YELLOW << "\nExpected:\n" << RESET;
            cout << expected << endl;

            cout << YELLOW << "Got:\n" << RESET;
            cout << actual << endl;

            cout << endl;
        }
    }

    cout << CYAN << BOLD << "========== SUMMARY ==========\n" << RESET;

    cout << "Passed: " << GREEN << passed << RESET
         << " / " << total << endl;

    if (passed == total) {
        cout << GREEN << BOLD << "All tests passed 🎉\n" << RESET;
    } else {
        cout << RED << BOLD << "Some tests failed\n" << RESET;
    }

    return 0;
}