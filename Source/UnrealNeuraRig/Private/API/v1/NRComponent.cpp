// Copyright (C) 2026 Rafael Valoto.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.


#include "API/v1/NRComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Network/NRNetwork.h"


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

	frameCounter = 0;
	ConvergenceFrame = 0;

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

	SpacingR = RigParameters.SpacingFootR;
	SpacingL = RigParameters.SpacingFootL;

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

	if (ConvergenceFrame == 0)
	{
		frameCounter++;
	}

	float CurrentDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

	float Velocity01 = FMath::Clamp(Actor->GetVelocity().Size() / 600.f, 0.0f, 1.f);
	if (Velocity01 <= 0.0f)
	{
		Velocity01 = FMath::Clamp(RigParameters.Velocity / 600.f, 0.0f, 1.f);
	}
	Push1(Velocity01);

	// JSON Offsets: []
	Push1(L1_R); // mse
	Push1(L2_R);
	Push1(RigParameters.OffsetFootR);  // offset
	Push1(RigParameters.SpacingFootR); // spacing

	// JSON Offsets: []
	Push1(L1_L); // mse
	Push1(L2_L);
	Push1(RigParameters.OffsetFootL);  // offset
	Push1(RigParameters.SpacingFootL); // spacing

	Push1(DeltaTime);
	Push1(CurrentDilation);
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
		if (dPacketRecive.Num() == 12)
		{
			FVector dLocalPosR = FVector(dPacketRecive[0].X - RigScales.FootOffsetX, dPacketRecive[0].Y - RigScales.FootOffsetY, dPacketRecive[0].Z - RigScales.FootOffsetZ) * FVector(RigScales.MaxFootStrideX);
			FVector dLocalPosL = FVector(dPacketRecive[2].X - RigScales.FootOffsetX, dPacketRecive[2].Y - RigScales.FootOffsetY, dPacketRecive[2].Z - RigScales.FootOffsetZ) * FVector(RigScales.MaxFootStrideX);
			
			dLocalPosR.X = FMath::Clamp(dLocalPosR.X, -RigScales.MaxFootStrideX, RigScales.MaxFootStrideX);
			dLocalPosR.Y = FMath::Clamp(dLocalPosR.Y, -5.0f, RigScales.MaxFootWidthY);
			dLocalPosR.Z = FMath::Clamp(dLocalPosR.Z, 0.0, RigScales.MaxFootHeightZ);

			dLocalPosL.X = FMath::Clamp(dLocalPosL.X, -RigScales.MaxFootStrideX, RigScales.MaxFootStrideX);
			dLocalPosL.Y = FMath::Clamp(dLocalPosL.Y, -5.0, RigScales.MaxFootWidthY);
			dLocalPosL.Z = FMath::Clamp(dLocalPosL.Z, 0.0, RigScales.MaxFootHeightZ);
			
			// UE_LOG(LogTemp, Warning, TEXT("dLocalPosR: %s"), *dLocalPosR.ToString());
			// UE_LOG(LogTemp, Log, TEXT("dLocalPosL: %s"), *dLocalPosL.ToString());
			
			FVector DebugWorldR = GetOwner()->GetActorTransform().TransformPosition(dLocalPosR);
			FVector DebugWorldL = GetOwner()->GetActorTransform().TransformPosition(dLocalPosL);

			DebugWorldR.Y = CharacterMesh->GetBoneLocation("foot_r").Y;
			DebugWorldR.Z -= 89.0f;
			DebugWorldL.Y = CharacterMesh->GetBoneLocation("foot_l").Y;
			DebugWorldL.Z -= 89.0f;

			DrawDebugSphere(GetWorld(), DebugWorldR, 4.0f, 8, FColor::Yellow, false, 0.05f);
			DrawDebugSphere(GetWorld(), DebugWorldL, 4.0f, 8, FColor::Red, false, 0.05f);
		}

		TArray<FVector> PacketRecive;
		PacketRecive.Reset();
		UNRNetwork::ReciveDataIK(PacketRecive);
		if (PacketRecive.Num() == 12)
		{
			bHasConverged = true;
			ConvergenceFrame = frameCounter;

			FVector LocalPosR = FVector(PacketRecive[0].X - RigScales.FootOffsetX, PacketRecive[0].Y - RigScales.FootOffsetY, PacketRecive[0].Z - RigScales.FootOffsetZ) * FVector(RigScales.MaxFootStrideX);
			FVector LocalPosL = FVector(PacketRecive[2].X - RigScales.FootOffsetX, PacketRecive[2].Y - RigScales.FootOffsetY, PacketRecive[2].Z - RigScales.FootOffsetZ) * FVector(RigScales.MaxFootStrideX);
			
			LocalPosR.X = FMath::Clamp(LocalPosR.X, -RigScales.MaxFootStrideX, RigScales.MaxFootStrideX);
			LocalPosR.Y = FMath::Clamp(LocalPosR.Y, -5.0f, RigScales.MaxFootWidthY);
			LocalPosR.Z = FMath::Clamp(LocalPosR.Z, 0.0, RigScales.MaxFootHeightZ);

			LocalPosL.X = FMath::Clamp(LocalPosL.X, -RigScales.MaxFootStrideX, RigScales.MaxFootStrideX);
			LocalPosL.Y = FMath::Clamp(LocalPosL.Y, -5.0, RigScales.MaxFootWidthY);
			LocalPosL.Z = FMath::Clamp(LocalPosL.Z, 0.0, RigScales.MaxFootHeightZ);
			
			float S_intpl = RigParameters.S_interpolation;
			float T_intpl = DeltaTime;

			// Foot position
			OutFootR_Pos = FMath::VInterpTo(OutFootR_Pos, -LocalPosR, T_intpl, S_intpl);
			OutFootL_Pos = FMath::VInterpTo(OutFootL_Pos, LocalPosL, T_intpl, S_intpl);

			// Foot rotate
			FRotator Fr_rot = FRotator(PacketRecive[1].X * RigScales.MaxFootPitch, 0.0f, 0.0f);
			FRotator Fl_rot = FRotator(PacketRecive[3].X * RigScales.MaxFootPitch, 0.0f, 0.0f);
			OutFootR_Rot = FMath::RInterpTo(OutFootR_Rot, Fr_rot, T_intpl, S_intpl);
			OutFootL_Rot = FMath::RInterpTo(OutFootL_Rot, Fl_rot, T_intpl, S_intpl);

			// Ball rotate
			FRotator Br_root = FRotator((PacketRecive[5].X + 0.45) * RigScales.MaxBallPitch, PacketRecive[5].Y, PacketRecive[5].Z);
			FRotator Bl_root = FRotator((PacketRecive[7].X + 0.45) * RigScales.MaxBallPitch, PacketRecive[7].Y, PacketRecive[7].Z);
			OutBallR_Rot = FMath::RInterpTo(OutBallR_Rot, Br_root, T_intpl, S_intpl);
			OutBallL_Rot = FMath::RInterpTo(OutBallL_Rot, Bl_root, T_intpl, S_intpl);
			
			UE_LOG(LogTemp, Warning, TEXT("BallR_Rot: %s"), *OutBallR_Rot.ToString());
			UE_LOG(LogTemp, Warning, TEXT("BallR_Rot: %s"), *OutBallL_Rot.ToString());

			// Pelvis
			FVector P_pos = FVector(
				PacketRecive[8].X * RigScales.MaxPelvisForwardX,
				PacketRecive[8].Y * RigScales.MaxPelvisSwayY,
				PacketRecive[8].Z * RigScales.MaxPelvisDropZ
				);
			FRotator P_rot = FRotator(
				PacketRecive[9].X * RigScales.MaxPelvisPitch,
				PacketRecive[9].Y * RigScales.MaxPelvisYaw,
				PacketRecive[9].Z * RigScales.MaxPelvisRoll
				);
			OutPelvis_Pos = FMath::VInterpTo(OutPelvis_Pos, P_pos, T_intpl, S_intpl);
			OutPelvis_Rot = FMath::RInterpTo(OutPelvis_Rot, P_rot, T_intpl, S_intpl);

			// Spine
			FRotator S_rot = FRotator(FMath::Clamp(PacketRecive[11].X * RigScales.MaxSpinePitch, 0.0, RigScales.MaxSpinePitch), FMath::Clamp(PacketRecive[11].Y * RigScales.MaxSpineYaw, 0.0, RigScales.MaxSpineYaw), FMath::Clamp(PacketRecive[11].Z * RigScales.MaxSpineRoll, 0.0, RigScales.MaxSpineRoll));
			OutSpine_Rot = FMath::RInterpTo(OutSpine_Rot, S_rot, T_intpl, S_intpl);

			FVector WorldR = GetOwner()->GetActorTransform().TransformPosition(LocalPosR);
			FVector WorldL = GetOwner()->GetActorTransform().TransformPosition(LocalPosL);

			WorldR.Y = CharacterMesh->GetBoneLocation("foot_r").Y;
			WorldR.Z -= 89.0f;
			WorldL.Y = CharacterMesh->GetBoneLocation("foot_l").Y;
			WorldL.Z -= 89.0f;

			DrawDebugSphere(GetWorld(), WorldR, 5.0f, 8, FColor::White, false, 0.05f);
			DrawDebugSphere(GetWorld(), WorldL, 5.0f, 8, FColor::White, false, 0.05f);
		}
	}
}
