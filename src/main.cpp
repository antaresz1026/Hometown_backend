#include <iostream>
#include "httpServer.hpp"

int main() {
    httpServer server("127.0.0.1", 23030);
    server.start();
    return 0;
}