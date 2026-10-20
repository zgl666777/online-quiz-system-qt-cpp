#include "myserver.h"
#include <exception>
#include <iostream>

int main()
{
    try {
        MyServer server;
        if (server.listen(9999) != 0) return 1;
        std::cout << "Quiz server listening on port 9999" << std::endl;
        server.start();
    } catch (const std::exception &error) {
        std::cerr << "Server startup failed: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
