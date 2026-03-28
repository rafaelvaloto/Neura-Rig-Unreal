// Project: NeuraRig
// Copyright (c) 2026 Rafael Valoto

#include "Network/NRNetworkClient.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/UdpSocketBuilder.h"

namespace NR
{
	NRNetworkClient::NRNetworkClient()
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (SocketSubsystem)
		{
			clientSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("NRNetworkClient"), true);
			if (clientSocket)
			{
				clientSocket->SetNonBlocking(true);
				clientSocket->SetReuseAddr(true);
				
				// Port 0 means ephemeral local port for sending
				TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
				Addr->SetAnyAddress();
				Addr->SetPort(0);
				clientSocket->Bind(*Addr);
			}
		}
		else
		{
			clientSocket = nullptr;
		}
	}

	NRNetworkClient::~NRNetworkClient()
	{
		if (clientSocket)
		{
			ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
			if (SocketSubsystem)
			{
				clientSocket->Close();
				SocketSubsystem->DestroySocket(clientSocket);
			}
			clientSocket = nullptr;
		}
	}

	bool NRNetworkClient::Send(const std::vector<float>& data, const std::string& ip, int port)
	{
		if (!clientSocket || data.empty())
		{
			return false;
		}

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		TSharedRef<FInternetAddr> destAddr = SocketSubsystem->CreateInternetAddr();
		
		FIPv4Address IPAddr;
		if (!FIPv4Address::Parse(FString(ip.c_str()), IPAddr))
		{
			return false;
		}
		
		destAddr->SetIp(IPAddr.Value);
		destAddr->SetPort(port);

		int32 bytesSent = 0;
		bool bSuccess = clientSocket->SendTo(reinterpret_cast<const uint8*>(data.data()), static_cast<int32>(data.size() * sizeof(float)), bytesSent, *destAddr);

		return bSuccess && bytesSent > 0;
	}
}
