// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rigs/RigHierarchyDefines.h"

class UNREALNEURARIG_API UNRNetwork
{

public:
	static void SetSocket();
	static void InitSocket();

	static void SendDataIK(uint8* DataBuffer, int32 Size);
	static bool ReciveDataIK(TArray<FVector>& OutVectors);
	static bool ReciveDataIKDebug(TArray<FVector>& OutVectors);

private:
	static FSocket* NRSocket;
	static FSocket* dNRSocket;
};
