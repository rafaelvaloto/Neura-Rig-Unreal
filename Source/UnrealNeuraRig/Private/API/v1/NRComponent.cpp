// Copyright (C) 2026 Rafael Valoto.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.


#include "API/v1/NRComponent.h"

#include "GameFramework/Character.h"
#include "Network/NRNetwork.h"
#include <cmath>


// Sets default values for this component's properties
UNRComponent::UNRComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UNRComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Actor = GetOwner();
	USkeletalMeshComponent* CharacterMesh = nullptr;
	if (Actor->IsA(ACharacter::StaticClass()))
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		CharacterMesh = Character->GetMesh();
	}
	
	if (!CharacterMesh)
	{
		return;
	}
	
	// Initialize sockets
	UNRNetwork::InitSocket();
	
	SpacingR = 0.05;
	SpacingL = 0.05;
	
	UE_LOG(LogTemp, Warning, TEXT("Spacing R: %f, Spacing L: %f"), SpacingR, SpacingL);

	const FVector ThighR_Pos = CharacterMesh->GetSocketLocation("thigh_r");
	const FVector CalfR_Pos = CharacterMesh->GetSocketLocation("calf_r");
	const FVector FootR_Pos = CharacterMesh->GetSocketLocation("foot_r");
	L1_R = FVector::Dist(CalfR_Pos, ThighR_Pos) * 0.01;
	L2_R = FVector::Dist(FootR_Pos, CalfR_Pos) * 0.01;

	const FVector ThighL_Pos = CharacterMesh->GetSocketLocation("thigh_l");
	const FVector CalfL_Pos = CharacterMesh->GetSocketLocation("calf_l");
	const FVector FootL_Pos = CharacterMesh->GetSocketLocation("foot_l");
	L1_L = FVector::Dist(CalfL_Pos, ThighL_Pos) * 0.01;
	L2_L = FVector::Dist(FootL_Pos, CalfL_Pos) * 0.01;

	UE_LOG(LogTemp, Log, TEXT("L1 R: %f, L2 R: %f"), L1_R, L2_R);
	UE_LOG(LogTemp, Log, TEXT("L1 L: %f, L2 L: %f"), L1_L, L2_L);

	FVector ThighToKnee = (CalfR_Pos - ThighR_Pos).GetSafeNormal();
	FQuat ThighRotWS = CharacterMesh->GetBoneQuaternion("thigh_r");

	FVector ThighToKneeL = (CalfL_Pos - ThighL_Pos).GetSafeNormal();
	FQuat ThighRotWSL = CharacterMesh->GetBoneQuaternion("thigh_l");

	AxisR = ThighRotWS.UnrotateVector(ThighToKnee);
	AxisL = ThighRotWSL.UnrotateVector(ThighToKneeL);
	UE_LOG(LogTemp, Warning, TEXT("Bone axis AxisR: %s"), *AxisR.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Bone axis AxisL: %s"), *AxisL.ToString());

	LastFootPosR = FVector::ZeroVector;
	LastFootPosL = FVector::ZeroVector;
}

// Called every frame
void UNRComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	AActor* Actor = GetOwner();
	USkeletalMeshComponent* CharacterMesh = nullptr;
	if (Actor->IsA(ACharacter::StaticClass()))
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		CharacterMesh = Character->GetMesh();
	}
	
	if (!CharacterMesh)
	{
		return;
	}
	
	TArray<float> Raw;
	Raw.Reserve(256);
	auto Push1 = [&Raw](float A) {
		Raw.Add(A);
	};
	
	const float SpeedCmS = Actor->GetVelocity().Size();
	const float Velocity01 = FMath::Clamp(SpeedCmS / 600.f, 0.3f, 1.f);
	Push1(Velocity01);

	// JSON Offsets: []
	Push1(L1_R); // mse
	Push1(L2_R);
	Push1(0.0); // offset
	Push1(-SpacingR); // spacing

	// JSON Offsets: []
	Push1(L1_L); // mse
	Push1(L2_L);
	Push1(0.5); // offset
	Push1(SpacingL); // spacing
	
	Push1(DeltaTime);
	Push1(0.0); // reserved
	
	if (Raw.Num() > 0)
	{
		TArray<uint8> Packet;
		const int32 DataSize = Raw.Num() * sizeof(float);

		Packet.AddUninitialized(DataSize + 1);
		Packet[0] = 2;

		FMemory::Memcpy(Packet.GetData() + 1, Raw.GetData(), DataSize);
		UNRNetwork::SendDataIK(Packet.GetData(), Packet.Num());
		
		TArray<FVector> dPacketRecive;
		dPacketRecive.Reset();
		UNRNetwork::ReciveDataIKDebug(dPacketRecive);
	
		float Offset = 0.45f;
		float Amplitude2 = 120.0f;
		float Amplitude = 100.0f;
		if (dPacketRecive.Num() == 10)
		{
			float dX1 = (dPacketRecive[0].X - Offset) * Amplitude2;
			float dY1 = (dPacketRecive[0].Y) * Amplitude;
			float dZ1 = (dPacketRecive[0].Z * Amplitude2);
			float dX2 = (dPacketRecive[2].X - Offset) * Amplitude2;
			float dY2 = (dPacketRecive[2].Y) * Amplitude;
			float dZ2 = (dPacketRecive[2].Z * Amplitude2);
			
			FVector dLocalPosR = FVector(dX1, -dY1, dZ1);
			FVector dLocalPosL = FVector(dX2, dY2, dZ2);
			
			// Foot position
			OutFootR_Pos = -dLocalPosR;
			OutFootL_Pos = dLocalPosL;
			
			// Foot rotate
			OutFootR_Rot = FRotator(FMath::Clamp((dPacketRecive[1].X - 0.40f) * Amplitude, 0.0f, 30.0f), 0.0f, 0.0f);
			OutFootL_Rot = FRotator(FMath::Clamp((dPacketRecive[3].X - 0.40f) * Amplitude, 0.0f, 30.0f), 0.0f, 0.0f);
			
			// Ball rotate
			OutBallR_Rot = FRotator(FMath::Clamp((dPacketRecive[5].X - 0.90f) * Amplitude, -45.0, 45.0), 0.0f, 0.0f);
			OutBallL_Rot = FRotator(FMath::Clamp(-(dPacketRecive[7].X - 0.90f) * Amplitude, -45.0, 45.0), 0.0f, 0.0f);
			
			// Pelvis
			OutPelvis_Pos = dPacketRecive[8];
			OutPelvis_Rot = FRotator(dPacketRecive[9].X, dPacketRecive[9].Y, dPacketRecive[9].Z);
			
			FVector DebugWorldR = GetOwner()->GetActorTransform().TransformPosition(dLocalPosR);
			FVector DebugWorldL = GetOwner()->GetActorTransform().TransformPosition(dLocalPosL);
			
			 DebugWorldR.Y = CharacterMesh->GetBoneLocation("foot_r").Y;
			 DebugWorldR.Z -= 89.0f;
			 DebugWorldL.Y = CharacterMesh->GetBoneLocation("foot_l").Y;
			 DebugWorldL.Z -= 89.0f;
			
			DrawDebugSphere(GetWorld(), DebugWorldR, 2.0f, 8, FColor::Yellow, false, 0.05f);
			DrawDebugSphere(GetWorld(), DebugWorldL, 2.0f, 8, FColor::Red, false, 0.05f);
		}
		
		// TArray<FVector> PacketRecive;
		// PacketRecive.Reset();
		// UNRNetwork::ReciveDataIK(PacketRecive);
		// if (dPacketRecive.Num() == 4)
		// {
		// 	float X1 = (dPacketRecive[0].X - Offset) * Amplitude;
		// 	float Z1 = (dPacketRecive[0].Z * Amplitude);
		// 	float X2 = (dPacketRecive[2].X - Offset) * Amplitude;
		// 	float Z2 = (dPacketRecive[2].Z * Amplitude);
		// 	
		// 	FVector LocalPosR = FVector(X1, 0.0, Z1);
		// 	FVector LocalPosL = FVector(X2, 0.0, Z2);
		// 	
		// 	OutFootR_Pos = -LocalPosR;
		// 	OutFootL_Pos = LocalPosL;
		// 	
		// 	FVector WorldR = GetOwner()->GetActorTransform().TransformPosition(LocalPosR);
		// 	FVector WorldL = GetOwner()->GetActorTransform().TransformPosition(LocalPosL);
		// 	
		// 	WorldR.Y = CharacterMesh->GetBoneLocation("foot_r").Y;
		// 	WorldR.Z -= 89.0f;
		// 	WorldR.Y = CharacterMesh->GetBoneLocation("foot_l").Y;
		// 	WorldR.Z -= 89.0f;
		// 	
		// 	DrawDebugSphere(GetWorld(), WorldR, 5.0f, 8, FColor::Yellow, false, 0.1f);
		// 	DrawDebugSphere(GetWorld(), WorldL, 5.0f, 8, FColor::Red, false, 0.1f);
		// }
	}
}

