// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealNeuraRig.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Network/NRNetwork.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include <vector>

#define LOCTEXT_NAMESPACE "FUnrealNeuraRigModule"

void FUnrealNeuraRigModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[UnrealNeuraRig] Startup Module"));
	UNRNetwork::InitSocket();
}

void FUnrealNeuraRigModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[UnrealNeuraRig] Shutdown Module"));
	UNRNetwork::CloseSocket();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealNeuraRigModule, UnrealNeuraRig);
