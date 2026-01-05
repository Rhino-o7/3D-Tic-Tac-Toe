#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>


#include <iostream>
#include "consoleClient.h"

int main(){
	std::cout << "Client Program\n\n";
	ConsoleClient c;
	c.connectToServer("ws://localhost:9002");
	//c.startTurn();
}
