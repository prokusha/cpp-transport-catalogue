#include "input_reader.h"

#include <iostream>

using namespace std;

namespace transport_catalogue {
namespace input {

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

void Add(waitlist& list) {
    int count = ReadLineWithNumber();
    for (int it = 0; it < count; ++it) {
        string command = ReadLine();
        if (command[0] == 'S') {
            list.stop.push_back(command);
        } else {
            list.bus.push_back(command);
        }
    }
}

} //end input
} //end transport_catalogue
