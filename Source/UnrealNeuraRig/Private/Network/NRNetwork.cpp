// Project: NeuraRig
// Copyright (c) 2026 Rafael Valoto

#include "Network/NRNetwork.h"

NR::NRNetworkClient* UNRNetwork::Client = nullptr;
NR::NRNetworkServer* UNRNetwork::ServerIK = nullptr;
NR::NRNetworkServer* UNRNetwork::ServerDebug = nullptr;

void UNRNetwork::InitSocket()
{
	if (!Client)
	{
		Client = new NR::NRNetworkClient();
	}
	if (!ServerIK)
	{
		ServerIK = new NR::NRNetworkServer();
		ServerIK->Start(8006);
	}
	if (!ServerDebug)
	{
		ServerDebug = new NR::NRNetworkServer();
		ServerDebug->Start(8007);
	}
}

void UNRNetwork::CloseSocket()
{
	if (Client)
	{
		delete Client;
		Client = nullptr;
	}
	if (ServerIK)
	{
		ServerIK->Stop();
		delete ServerIK;
		ServerIK = nullptr;
	}
	if (ServerDebug)
	{
		ServerDebug->Stop();
		delete ServerDebug;
		ServerDebug = nullptr;
	}
}

void UNRNetwork::SendDataIK(const float* DataBuffer, int32 Size)
{
	if (!Client)
		return;

	std::vector<float> Data;
	Data.resize(Size / sizeof(float));
	
	FMemory::Memcpy(Data.data(), DataBuffer, Size);
	Client->Send(Data, "127.0.0.1", 8005);
}

bool UNRNetwork::ReciveDataIK(TArray<float>& OutFloats)
{
	if (!ServerIK)
		return false;

	std::vector<float> ReceivedData;
	if (ServerIK->Receive(ReceivedData))
	{
		OutFloats.Empty(ReceivedData.size());
		OutFloats.Append(ReceivedData.data(), ReceivedData.size());
		return true;
	}
	return false;
}

bool UNRNetwork::ReciveDataIKDebug(TArray<float>& OutFloats)
{
	if (!ServerDebug)
		return false;

	std::vector<float> ReceivedData;
	if (ServerDebug->Receive(ReceivedData))
	{
		OutFloats.Empty(ReceivedData.size());
		OutFloats.Append(ReceivedData.data(), ReceivedData.size());
		return true;
	}
	return false;
}
