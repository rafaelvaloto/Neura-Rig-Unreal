// Copyright (C) 2026 Rafael Valoto.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#pragma once
#include "CoreMinimal.h"

#include "Network/NRNetworkClient.h"
#include "Network/NRNetworkServer.h"

class UNREALNEURARIG_API UNRNetwork
{
public:
	static void InitSocket();
	static void CloseSocket();

	static void SendDataIK(const float* DataBuffer, int32 Size);
	static bool ReciveDataIK(TArray<float>& OutFloats);
	static bool ReciveDataIKDebug(TArray<float>& OutFloats);

private:
	static NR::NRNetworkClient* Client;
	static NR::NRNetworkServer* ServerIK;
	static NR::NRNetworkServer* ServerDebug;
};