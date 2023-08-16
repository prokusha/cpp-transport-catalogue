#pragma once

#include <string>
#include <vector>

namespace transport_catalogue {
namespace input {

struct waitlist {
    std::vector<std::string> stop;
    std::vector<std::string> bus;
};

std::string ReadLine();

int ReadLineWithNumber();

void Add(waitlist&);

} //end input
} //end transport_catalogue

