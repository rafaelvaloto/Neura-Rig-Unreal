// Project: NeuraRig
// Copyright (c) 2026 Rafael Valoto

#pragma once

#include "CoreMinimal.h"
#include "Sockets.h"
#include "IPAddress.h"
#include <vector>

namespace NR
{
	class UNREALNEURARIG_API NRNetworkServer
	{
	public:
		NRNetworkServer();
		~NRNetworkServer();

		bool Start(int port);
		bool Receive(std::vector<float>& outData);
		void Stop();

	private:
		FSocket* serverSocket;
		bool bIsRunning;
		float buffer[512]; // Matches user provided buffer size logic implicitly
	};
}
