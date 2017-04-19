#include "NeuronConnection.h"

NeuronConnection::NeuronConnection() {
	m_firstWrite = true;
	m_initialFrame = -1;
	m_dataCount = -1;
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

		// Unused
		// TODO: check if it works if we delete this
		BRRegisterCalculationDataCallback(this, CalcFrameDataReceive);

		return true;
	}
		
	// Failed
	return false;
}

void NeuronConnection::KillConnection() {
	BRCloseSocket(sockTCPRef);
	FinalizeBvh();
}

void __stdcall NeuronConnection::BvhFrameDataReceived(void* customedObj, SOCKET_REF sender, BvhDataHeader* header, float* data) {
	NeuronConnection* pthis = (NeuronConnection*)customedObj;
	pthis->ShowBvhBoneInfo(sender, header, data);
}

// Called, but unused
void __stdcall NeuronConnection::CalcFrameDataReceive(void* customedObj, SOCKET_REF sender, CalcDataHeader* header, float* data) {
	NeuronConnection* pthis = (NeuronConnection*)customedObj;
	pthis->ShowBvhCalcInfo(sender, header, data);
}

void NeuronConnection::ShowBvhBoneInfo(SOCKET_REF sender, BvhDataHeader* header, float* data) {
	if (m_initialFrame == -1)
		m_initialFrame = header->FrameIndex;
	
	if(m_dataCount == -1)
		m_dataCount = header->DataCount;

	BvhExport(header, data);
}

// Unused
void NeuronConnection::ShowBvhCalcInfo(SOCKET_REF sender, CalcDataHeader* header, float* data) {
	// Cool callback bro
}

void NeuronConnection::BvhExport(BvhDataHeader* header, float* data) {
	
	// Initial connection :
	// We get the initial offsets and the hierarchy
	if (m_firstWrite) {
		m_firstWrite = false;

		m_perJoint = 3;

		if (header->WithDisp)
			m_perJoint = 6;

		// Stupid way to wipe the content of tmp.bvh
		// TODO: do it the right way
		// EDIT: actually not that stupid.
		m_outfile.open("tmp.bvh", std::ios_base::out);
		m_outfile.close();
	}

	m_outfile.open("tmp.bvh", std::ios::out | std::ios::app);

	if (!m_outfile.fail()) {
		m_currentFrame = header->FrameIndex;

		if(m_perJoint == 3) {
			// Root (Why ?)
			//m_outfile << data[0] << " " << data[1] << " " << data[2] << " ";

			for (unsigned int i=0 ; i<BVHBoneCount ; i++) {

				// data[	3       +  (i * 3)   +      0]
				//		root offset | bone index | rotation index

				m_outfile << data[(i * 3) + 0] << " ";
				m_outfile << data[(i * 3) + 1] << " ";
				m_outfile << data[(i * 3) + 2] << " ";
			}
		}

		else if (m_perJoint == 6){
			for (unsigned int i = 0; i<BVHBoneCount; i++) {
				m_outfile << data[(i * 6) + 0] << " ";
				m_outfile << data[(i * 6) + 1] << " ";
				m_outfile << data[(i * 6) + 2] << " ";
				m_outfile << data[(i * 6) + 3] << " ";
				m_outfile << data[(i * 6) + 4] << " ";
				m_outfile << data[(i * 6) + 5] << " ";
			}
		}

		m_outfile << "\n";
		m_outfile.close();
	}

	else {
		std::cout << "Failed to open file tmp.bvh" << std::endl;
	}
	
}

/***********************************************************************
	This method calls 3 methods one after another. They're used to 
	fill the final bvh file. Due to the very nature of CPP files access, 
	we are forced to use a tmp bvh file so that :

		- We add template informations
		- We add frame number and interframe time
		- We copy the motions informations (position and orientation)

	We can't go back in the middle of the file to add frames n° and
	interframe time without breaking the whole file.

***********************************************************************/
void NeuronConnection::FinalizeBvh() {
	InitializeBvhHeader();
	BvhFrameInformations();
	BvhMotionCopy();
}

// Copy the template of the Neuron bvh header (see appendix A in the doc)
void NeuronConnection::InitializeBvhHeader() {
	m_outfile.open("test.bvh");

	if(m_outfile.is_open()) {
		std::ifstream bvh_header_template("bvh_header_template.bvh");

		if(bvh_header_template.is_open()) {
			
			std::string line;

			// Special case : we need to eliminate the XYZ position
			if (m_perJoint == 3) {
				bool isRoot = true;

				while (std::getline(bvh_header_template, line)) {
					// For the root, we leave the 6 cahnnels
					if (!strncmp(line.c_str(), "JOINT", strlen("JOINT")))
						isRoot = false;

					// If it's the channel line AND it's not the root
					// We write a modified channel line
					if (!strncmp(line.c_str(), "CHANNELS", strlen("CHANNELS")) && !isRoot)
						m_outfile << "CHANNELS 3 Yrotation Xrotation Zrotation\n";

					// Else, just ouput the line
					else
						m_outfile << line << '\n';
				}
			}

			else {
				while (std::getline(bvh_header_template, line))
					m_outfile << line << "\n";
			}
			bvh_header_template.close();

		}

		else {
			std::cout << "Error opening bvh_header_template.bvh" << std::endl;
		}
		
		m_outfile.close();
	}
	
	else {
		std::cout << "Error opening test.bvh" << std::endl;
	}
	
	
}

// Add informations about frame number and interframe time
void NeuronConnection::BvhFrameInformations() {
	m_outfile.open("test.bvh", std::ios_base::app);

	if(!m_outfile.fail()) {

		m_outfile << "Frames: " << m_currentFrame - m_initialFrame << "\n";
		if ((m_dataCount / m_perJoint) < 18)
			m_outfile << "Frame Time: " << 1.0 / 120 << "\n";
		else
			m_outfile << "Frame Time: " << 1.0 / 60 << "\n";
	}

	m_outfile.close();
}

// Copy the motion data (position and orientation)
void NeuronConnection::BvhMotionCopy() {
	m_outfile.open("test.bvh", std::ios_base::app);

	std::ifstream motionInfo("tmp.bvh");
	std::string line;

	while(std::getline(motionInfo, line)) {
		m_outfile << line << "\n";
	}

	motionInfo.close();
	m_outfile.close();

	// Delete the file
	std::remove("tmp.bvh");
}