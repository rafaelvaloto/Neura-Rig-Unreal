// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/NRNetwork.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

FSocket* UNRNetwork::NRSocket = nullptr;
FSocket* UNRNetwork::dNRSocket = nullptr;
void UNRNetwork::SetSocket()
{
	FIPv4Address IP;
	FIPv4Address::Parse(TEXT("127.0.0.1"), IP);
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	
	if (dNRSocket)
	{
		NRSocket->Close();
		NRSocket = nullptr;
	}
	
	if (!NRSocket)
	{
		NRSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("NRSocket"), true);
		Addr->SetIp(IP.Value);
		Addr->SetPort(6003);
		NRSocket->SetNonBlocking(true);

		int32 BytesSent = 0;
		if (NRSocket->SendTo(0, 0, BytesSent, *Addr))
		{
			UE_LOG(LogTemp, Log, TEXT("[UnrealNeuraRig][UDP Socket] ::6003 ping bound successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UnrealNeuraRig][UDP Socket] ::6003 bind fail"));
		}
	}
	
	if (dNRSocket)
	{
		dNRSocket->Close();
		dNRSocket = nullptr;
	}
	
	if (!dNRSocket)
	{
		dNRSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("dNRSocket"), true);
		Addr->SetIp(IP.Value);
		Addr->SetPort(6004);
		dNRSocket->SetNonBlocking(true);

		if (int32 BytesSent = 0; dNRSocket->SendTo({0}, 0, BytesSent, *Addr))
		{
			UE_LOG(LogTemp, Log, TEXT("[UnrealNeuraRig][UDP Socket] ::6004 ping bound successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UnrealNeuraRig][UDP Socket] ::6004 bind fail"));
		}
	}
}

void UNRNetwork::InitSocket()
{
	SetSocket();
}

void UNRNetwork::SendDataIK(uint8* DataBuffer, int32 Size)
{
	FSocket* Socket = NRSocket;
	if (!Socket)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnrealNeuraRig][UDP Socket] Socket not initialized"));
		return;
	}

	FIPv4Address IP;
	FIPv4Address::Parse(TEXT("127.0.0.1"), IP);

	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(IP.Value);
	Addr->SetPort(6003);

	int32 BytesSent = 0;
	if (!Socket->SendTo(DataBuffer, Size, BytesSent, *Addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnrealNeuraRig][UDP Socket] ::6003 send fail"));
	}
}

bool UNRNetwork::ReciveDataIK(TArray<FVector>& OutVectors)
{
	FSocket* Socket = NRSocket;
	if (!Socket)
	{
		return false;
	}

	uint8 ReceiveBuffer[1024];
	int32 BytesRead = 0;
	TSharedRef<FInternetAddr> SenderAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	uint32 PendingDataSize = 0;
	if (!Socket->HasPendingData(PendingDataSize) || PendingDataSize == 0)
	{
		return false;
	}

	if (!Socket->RecvFrom(ReceiveBuffer, sizeof(ReceiveBuffer), BytesRead, *SenderAddr))
	{
		return false;
	}

	if (ReceiveBuffer[0] != 3)
	{
		return false;
	}

	const float* PredData = reinterpret_cast<const float*>(&ReceiveBuffer[1]);

	TArray<FVector> Vs;
	Vs.Reserve(10);
	for (int32 j = 0; j < 10; j++)
	{
		int32 id = j * 3;

		FVector V(
		    PredData[id],     // X
		    PredData[id + 1], // Y
		    PredData[id + 2]  // Z
		);
		Vs.Add(V);
	}
	OutVectors.Empty();

	OutVectors = Vs;
	return true;
}

bool UNRNetwork::ReciveDataIKDebug(TArray<FVector>& OutVectors)
{
	FSocket* Socket = dNRSocket;
	if (!Socket)
	{
		return false;
	}

	uint8 dReceiveBuffer[1024];
	int32 BytesRead = 0;
	TSharedRef<FInternetAddr> dSenderAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	if (!Socket->RecvFrom(dReceiveBuffer, sizeof(dReceiveBuffer), BytesRead, *dSenderAddr))
	{
		return false;
	}

	if (dReceiveBuffer[0] != 4)
	{
		return false;
	}

	const float* PredData = reinterpret_cast<const float*>(&dReceiveBuffer[1]);

	TArray<FVector> Vs;
	Vs.Reserve(10);
	for (int32 j = 0; j < 10; j++)
	{
		int32 id = j * 3;

		FVector V(
			PredData[id],     // X
			PredData[id + 1], // Y
			PredData[id + 2]  // Z
		);
		// UE_LOG(LogTemp, Log, TEXT("Vs %s"), *V.ToString());
		Vs.Add(V);
	}
	
	OutVectors.Empty();
	
	OutVectors = Vs;
	return true;
}
