#include <iostream>
#include "../ConsoleClient.h"

int main(){
	std::cout << "Client Program\n\n";
	ConsoleClient c;
	c.connectToServer("ws://localhost:9002");
	//c.startTurn();
}
