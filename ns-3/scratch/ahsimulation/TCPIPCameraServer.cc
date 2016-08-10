/*
 * TCPIPCameraServer.cc
 *
 *  Created on: Aug 9, 2016
 *      Author: dwight
 */

#include "TCPIPCameraServer.h"

using namespace ns3;

TCPIPCameraServer::TCPIPCameraServer() {

}

TCPIPCameraServer::~TCPIPCameraServer() {

}

ns3::TypeId TCPIPCameraServer::GetTypeId(void) {
	static ns3::TypeId tid = ns3::TypeId("TCPIPCameraServer")
			.SetParent<TcpServer>()
			.AddConstructor<TCPIPCameraServer>()
	;
	return tid;
}

void TCPIPCameraServer::OnDataReceived(ns3::Address from) {


	char* buf = new char[1024];
	int nrOfBytesRead;

	while(nrOfBytesRead = Read(from, buf, 1024)) {
		// discard
	}
	delete buf;

}


