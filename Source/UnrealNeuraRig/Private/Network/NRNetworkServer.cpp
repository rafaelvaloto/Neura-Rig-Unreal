// Project: NeuraRig
// Copyright (c) 2026 Rafael Valoto

#include "Network/NRNetworkServer.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

namespace NR
{
	NRNetworkServer::NRNetworkServer()
		: serverSocket(nullptr)
		, bIsRunning(false), buffer{}
	{
	}

	NRNetworkServer::~NRNetworkServer()
	{
		Stop();
	}

	bool NRNetworkServer::Start(int port)
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (!SocketSubsystem) return false;

		serverSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("NRNetworkServer"), true);
		if (!serverSocket)
		{
			return false;
		}

		serverSocket->SetNonBlocking(true);
		serverSocket->SetReuseAddr(true);

		TSharedRef<FInternetAddr> serverAddr = SocketSubsystem->CreateInternetAddr();
		serverAddr->SetAnyAddress();
		serverAddr->SetPort(port);

		if (!serverSocket->Bind(*serverAddr))
		{
			SocketSubsystem->DestroySocket(serverSocket);
			serverSocket = nullptr;
			return false;
		}

		bIsRunning = true;
		return true;
	}

	bool NRNetworkServer::Receive(std::vector<float>& outData)
	{
		if (!serverSocket)
		{
			return false;
		}

		uint32 PendingDataSize = 0;
		if (!serverSocket->HasPendingData(PendingDataSize) || PendingDataSize == 0)
		{
			return false;
		}

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		TSharedRef<FInternetAddr> clientAddr = SocketSubsystem->CreateInternetAddr();
		int32 bytesReceived = 0;

		bool bSuccess = serverSocket->RecvFrom(reinterpret_cast<uint8*>(buffer), sizeof(buffer), bytesReceived, *clientAddr);

		if (!bSuccess || bytesReceived <= 0)
		{
			return false;
		}

		const int count = bytesReceived / sizeof(float);
		if (count > 0)
		{
			outData.assign(buffer, buffer + count);
			return true;
		}
		
		return false;
	}

	void NRNetworkServer::Stop()
	{
		if (serverSocket)
		{
			ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
			if (SocketSubsystem)
			{
				serverSocket->Close();
				SocketSubsystem->DestroySocket(serverSocket);
			}
			serverSocket = nullptr;
		}
		bIsRunning = false;
	}
}
