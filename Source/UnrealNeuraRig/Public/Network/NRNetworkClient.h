// Project: NeuraRig
// Copyright (c) 2026 Rafael Valoto

#pragma once

#include "CoreMinimal.h"
#include "Sockets.h"
#include "IPAddress.h"
#include <vector>
#include <string>

namespace NR
{
	class UNREALNEURARIG_API NRNetworkClient
	{
	public:
		NRNetworkClient();
		~NRNetworkClient();

		bool Send(const std::vector<float>& data, const std::string& ip, int port);

	private:
		FSocket* clientSocket;
	};
}
