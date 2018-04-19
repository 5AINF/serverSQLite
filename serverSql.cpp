#include "Socket.hpp"

#define PORT 8080

int main(int argc, char* argv[]) {
	ServerTCP* myself = new ServerTCP(PORT, true);
	Connection* conn = myself->accetta();
	char* request = conn->ricevi();
	
	
	
	
	return 0;
}
