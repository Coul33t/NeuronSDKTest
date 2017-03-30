#pragma once
// NeuronSDK includes
#include "DataType.h"
#include "NeuronDataReader.h"
#include <iostream>

#pragma comment(lib, "NeuronDataReader.lib")

class NeuronConnection {

public:
	// ctor & dtor
	NeuronConnection();
	~NeuronConnection();

	// Connect to the Neuron
	bool Connect();

	// Kill the connection
	void KillConnection();

	// Callback functions to receive the BVH data
	static void __stdcall BvhFrameDataReceived(void*, SOCKET_REF, BvhDataHeader*, float*);
	// Callback functions to receive the Calculation data
	static void __stdcall CalcFrameDataReceive(void*, SOCKET_REF, CalcDataHeader*, float*);

	// Display information about the received BVH data
	void ShowBvhBoneInfo(SOCKET_REF, BvhDataHeader*, float*);
	// Display information about the received Calc data
	void ShowBvhCalcInfo(SOCKET_REF, CalcDataHeader*, float*);

private:
	// Socket used to connect to the Neuron
	SOCKET_REF sockTCPRef;
};

