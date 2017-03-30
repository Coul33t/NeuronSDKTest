#include "NeuronConnection.h"

NeuronConnection::NeuronConnection() {

}


NeuronConnection::~NeuronConnection() {

}

bool NeuronConnection::Connect() {
	
	// If a connection has already been established
	if (sockTCPRef) {
		// close socket
		BRCloseSocket(sockTCPRef);
		sockTCPRef = 0;
	}

	// Try to connect to the Neuron
	sockTCPRef = BRConnectTo("127.0.0.1", 7001);
	
	// If the connection is ok
	if(sockTCPRef) {

		//calback register used to receive data
		// (we assign BvhFrameDataReceived to the FrameData callback,
		//  and CalcFrameDataReceive to the CalculationData callback)
		BRRegisterFrameDataCallback(this, BvhFrameDataReceived);
		BRRegisterCalculationDataCallback(this, CalcFrameDataReceive);

		return true;
	}
		
	// Failed
	return false;
}

void NeuronConnection::KillConnection() {
	BRCloseSocket(sockTCPRef);
}

void __stdcall NeuronConnection::BvhFrameDataReceived(void* customedObj, SOCKET_REF sender, BvhDataHeader* header, float* data) {
	NeuronConnection* pthis = (NeuronConnection*)customedObj;
	pthis->ShowBvhBoneInfo(sender, header, data);
}
void __stdcall NeuronConnection::CalcFrameDataReceive(void* customedObj, SOCKET_REF sender, CalcDataHeader* header, float* data) {
	NeuronConnection* pthis = (NeuronConnection*)customedObj;
	pthis->ShowBvhCalcInfo(sender, header, data);
}

void NeuronConnection::ShowBvhBoneInfo(SOCKET_REF sender, BvhDataHeader* header, float* data) {
	std::cout << "Frame count : " << header->FrameIndex << std::endl;
	
	int dataIndex = 0;
	int perJoint = 3;
	
	// WithDisp = Position for every joint or not (true or false)
	if(header->WithDisp)
		perJoint = 6;

	// Placeholder value (used to display a specific joint)
	int idx = 14 * 6;

	std::cout << "\nx pos : " << data[0 + idx] << "\ny pos : " << data[1 + idx] << "\nz pos : " << data[2 + idx] << std::endl;
	std::cout << "\nx ori : " << data[3 + idx] << "\ny ori : " << data[4 + idx] << "\nz ori : " << data[5 + idx] << std::endl;		
}

void NeuronConnection::ShowBvhCalcInfo(SOCKET_REF sender, CalcDataHeader* header, float* data) {
	std::cout << header->FrameIndex << "\r";
}