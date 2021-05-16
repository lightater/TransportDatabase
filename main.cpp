#include "tests.h"
#include "transport_database.h"
#include "requests.h"

using namespace Transport;
using namespace Requests;

int main() {
    TestAll();
    TransportDatabase tdb;
    Json::Print(ProcessRequests(ParseRequests(Json::Load()), tdb));

    return 0;
}
