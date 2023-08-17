#pragma once

#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>

namespace transport_catalogue {
namespace detail {
namespace string_refactoring {

std::string MakeName(std::string_view& str);
std::string_view::iterator FindSimbol(std::string_view str, char simbol); 
Stop MakeStop(std::string_view command);
Bus MakeBus(std::string_view command, TransportCatalogue& transport);
void MarkDistance(TransportCatalogue& transport);

} //end string_refactoring
} //end detail
namespace input {

struct waitlist {
    std::vector<std::string> stop;
    std::vector<std::string> bus;
};

std::string ReadLine();

int ReadLineWithNumber();

void Add(TransportCatalogue& transport);

} //end input
} //end transport_catalogue

