#pragma once

#include "transport_catalogue.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

namespace transport_catalogue {
namespace output {

void ReturnStats(TransportCatalogue& transport, std::istream& in, std::ostream& out);

void DisplayStatBus(TransportCatalogue& transport, std::string_view command, std::ostream& out);
void DisplayStatStop(TransportCatalogue& transport, std::string_view command, std::ostream& out);

} //end output
} //end transport_catalogue
