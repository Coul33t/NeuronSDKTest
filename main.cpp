#include <iostream>
#include "NeuronConnection.h"

// ------------------------------------------------------------
// Those 3 includes are NOT related to the Neuron SDK
// ------------------------------------------------------------
#include <thread>	// Used for a non blocking input
#include <future>	// Same
#include <conio.h>	// Input without enter (_getch() function)
// ------------------------------------------------------------


// Small function used to asynchronously check if Esc is pressed
// (used to kill the communication and stop the program)
// (NOT related to the Neuron SDK)
bool isEsc() {
	char ch = _getch();
	if (ch == 27)
		return true;
	return false;
}

int main() {
	// Used for checking if ESC key is pressed asynchronously
	// (NOT related to the Neuron SDK)
	std::future<bool> ret = std::async(&isEsc);


	NeuronConnection NCon;

	if (NCon.Connect()) {
		std::cout << "Connection succesful." << std::endl << std::endl;
		
		// Asynchronously checking for Esc press
		// (NOT related to the Neuron SDK)
		while(!ret.get());

		NCon.KillConnection();
	}
		
	else {
		std::cout << "Failed to connect. Please check if the Neuron is connected to the computer, and that Axis Neuron is running." << std::endl << std::endl;
		std::cout << "Steps required : " << std::endl;
		std::cout << "\t- Connect the Neuron to the computer" << std::endl;
		std::cout << "\t- Launch the Axis Neuron software" << std::endl;
		std::cout << "\t- Connect the Neuron to the Axis Neuron software" << std::endl;
		std::cout << "\t- Launch this program" << std::endl << std::endl;
		std::cout << "Press any key to quit." << std::endl;
		_getch();

		return 1;
	}

	return 0;
}