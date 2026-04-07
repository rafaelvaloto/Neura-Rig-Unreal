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

	// Initialize sockets (Already handled by Module Startup, but safe to re-init if needed)
	UNRNetwork::InitSocket();

	SpacingR = RigParameters.SpacingFootR;
	SpacingL = RigParameters.SpacingFootL;
	
	auto MeasureBoneDistance = [CharacterMesh](const FName& A, const FName& B) -> float
	{
		const FVector PosA = CharacterMesh->GetSocketLocation(A);
		const FVector PosB = CharacterMesh->GetBoneLocation(B);
		return FVector::Dist(PosA, PosB) * 0.01f; // cm -> m
	};

	constexpr float DefaultFootTipLength = 0.05f;
	
	// Right leg
	L1_R = MeasureBoneDistance(TEXT("thigh_r"), TEXT("calf_r")); // femur
	L2_R = MeasureBoneDistance(TEXT("calf_r"), TEXT("foot_r"));  // tibia
	L3_R = MeasureBoneDistance(TEXT("foot_r"), TEXT("ball_r"));  // foot
	L4_R = DefaultFootTipLength;


	// Left leg
	L1_L = MeasureBoneDistance(TEXT("thigh_l"), TEXT("calf_l"));
	L2_L = MeasureBoneDistance(TEXT("calf_l"), TEXT("foot_l"));
	L3_L = MeasureBoneDistance(TEXT("foot_l"), TEXT("ball_l"));
	L4_L = DefaultFootTipLength;

	UE_LOG(LogTemp, Log, TEXT("R: L1=%f L2=%f | L: L1=%f L2=%f"), L1_R, L2_R, L1_L, L2_L);
	UE_LOG(LogTemp, Log, TEXT("R: L3=%f L4=%f | L: L3=%f L4=%f"), L3_R, L4_R, L3_L, L4_L);

	bHasConverged = false;
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
	
	auto DetectFootHit = [this](USkeletalMeshComponent* Mesh, const FName& FootBoneName) -> float
	{
		if (!Mesh || !GetWorld())
		{
			return 0.0f;
		}

		const FVector Start = Mesh->GetSocketLocation(FootBoneName);
		const FVector End = Start - FVector(0.0f, 0.0f, 20.0f); // distância de checagem para baixo

		FCollisionQueryParams Params(SCENE_QUERY_STAT(DetectFootHit), false);
		Params.AddIgnoredActor(GetOwner());

		FHitResult Hit;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_Visibility,
			Params
		);

		return bHit ? 1.0f : 0.0f;
	};
	
	const float IsHit0 = DetectFootHit(CharacterMesh, TEXT("foot_r"));
	const float IsHit1 = DetectFootHit(CharacterMesh, TEXT("foot_l"));

	float Velocity01 = FMath::Clamp(Actor->GetVelocity().Size() / 600.f, 0.0f, 1.f);
	if (Velocity01 <= 0.0f)
	{
		Velocity01 = FMath::Clamp(RigParameters.Velocity / 600.f, 0.0f, 1.f);
	}
	Push1(Velocity01);

	// JSON Offsets: []
	Push1(L1_R); // mse
	Push1(L2_R);
	Push1(L3_R);                     // bone_l3_r
	Push1(L4_R);                     // bone_l4_r
	Push1(IsHit0);                   // IsHit0
	Push1(RigParameters.OffsetFootR);  // offset
	Push1(RigParameters.SpacingFootR); // spacing

	// JSON Offsets: []
	Push1(L1_L); // mse
	Push1(L2_L);
	Push1(L3_L);                     // bone_l3_l
	Push1(L4_L);                     // bone_l4_l
	Push1(IsHit1);                   // IsHit1
	Push1(RigParameters.OffsetFootL);  // offset
	Push1(RigParameters.SpacingFootL); // spacing

	Push1(DeltaTime);
	Push1(CurrentDilation);
	Push1(0.0); // reserved

	if (Raw.Num() > 0)
	{
		TArray<float> Packet;
		const int32 DataSize = Raw.Num() * sizeof(float);
		UNRNetwork::SendDataIK(Raw.GetData(), DataSize);
		
		TArray<FVector> dPacketRecive;
		dPacketRecive.Reset();
		UNRNetwork::ReciveDataIKDebug(dPacketRecive);
		if (dPacketRecive.Num() == 20 && !bHasConverged)
		{
			//UpdateIK(CharacterMesh, dPacketRecive, DeltaTime);
		}
		
		TArray<FVector> PacketRecive;
		PacketRecive.Reset();
		UNRNetwork::ReciveDataIK(PacketRecive);
		if (PacketRecive.Num() == 20)
		{
			bHasConverged = true;
			ConvergenceFrame = frameCounter;
			UpdateIK(CharacterMesh, PacketRecive, DeltaTime);
		}
	}
}

void UNRComponent::UpdateIK(USkeletalMeshComponent* CharacterMesh, TArray<FVector> PacketRecive, float DeltaTime)
{
	FVector FootScale = FVector(RigScales.MaxFootHeightZ,  RigScales.MaxFootStrideX, RigScales.MaxFootWidthY);
	FVector LocalPosR = FVector(PacketRecive[0].X, PacketRecive[0].Y, PacketRecive[0].Z) * FootScale;
	FVector LocalPosL = FVector(PacketRecive[2].X, PacketRecive[2].Y,  PacketRecive[2].Z) * FootScale;
	
	float S_intpl = RigParameters.S_interpolation;
	float T_intpl = DeltaTime;
	
	// Foot position
	OutFootR_Pos = FMath::VInterpTo(OutFootR_Pos, LocalPosR, T_intpl, S_intpl);
	OutFootL_Pos = FMath::VInterpTo(OutFootL_Pos, -LocalPosL, T_intpl, S_intpl);
	
	// Foot rotate
	FRotator Fr_rot = FRotator(PacketRecive[1].X * RigScales.MaxFootPitch, 0.0f, 0.0f);
	FRotator Fl_rot = FRotator(PacketRecive[3].X * RigScales.MaxFootPitch, 0.0f, 0.0f);
	OutFootR_Rot = FMath::RInterpTo(OutFootR_Rot, Fr_rot, T_intpl, S_intpl);
	OutFootL_Rot = FMath::RInterpTo(OutFootL_Rot, Fl_rot, T_intpl, S_intpl);
	
	// Ball rotate
	float TargetPitchR = PacketRecive[5].X * RigScales.MaxBallPitch;
	float TargetPitchL = PacketRecive[7].X * RigScales.MaxBallPitch;
	FRotator Br_rot = FRotator(TargetPitchR, PacketRecive[5].Y, PacketRecive[5].Z);
	FRotator Bl_rot = FRotator(TargetPitchL, PacketRecive[7].Y, PacketRecive[7].Z);
	
	OutBallR_Rot = FMath::RInterpTo(OutBallR_Rot, Br_rot, T_intpl, S_intpl);
	OutBallL_Rot = FMath::RInterpTo(OutBallL_Rot, Bl_rot, T_intpl, S_intpl);
	
	// LegIK rotate
	float TargetCalfPitchR = PacketRecive[9].X * RigScales.MaxCalfPitch; // Calf
	float TargetCalfPitchL = PacketRecive[11].X * RigScales.MaxCalfPitch;
	
	float TargetThighPitchR = PacketRecive[13].X * RigScales.MaxThighPitch; // Thigh
	float TargetThighPitchL = PacketRecive[15].X * RigScales.MaxThighPitch;
	
	FRotator TargetThighRotR = FRotator(TargetThighPitchR, PacketRecive[13].Y, PacketRecive[13].Z);
	FRotator TargetThighRotL = FRotator(TargetThighPitchL, PacketRecive[15].Y, PacketRecive[15].Z);
	
	FRotator TargetCalfRotR  = FRotator(TargetCalfPitchR, PacketRecive[9].Y, PacketRecive[9].Z);
	FRotator TargetCalfRotL  = FRotator(TargetCalfPitchL, PacketRecive[11].Y, PacketRecive[11].Z);
	
	OutThighR_Rot = FMath::RInterpTo(OutThighR_Rot, TargetThighRotR, T_intpl, S_intpl);
	OutThighL_Rot = FMath::RInterpTo(OutThighL_Rot, TargetThighRotL, T_intpl, S_intpl);
	
	OutCalfR_Rot  = FMath::RInterpTo(OutCalfR_Rot, TargetCalfRotR, T_intpl, S_intpl);
	OutCalfL_Rot  = FMath::RInterpTo(OutCalfL_Rot, TargetCalfRotL, T_intpl, S_intpl);
	// End -- LegIK --
	
	// Pelvis
	FVector P_pos = FVector(
	0.0,
		PacketRecive[16].Y * RigScales.MaxPelvisSwayY,
		PacketRecive[16].Z * RigScales.MaxPelvisDropZ
		);
	FRotator P_rot = FRotator(
		PacketRecive[17].X * RigScales.MaxPelvisPitch,
		PacketRecive[17].Y * RigScales.MaxPelvisYaw,
		PacketRecive[17].Z * RigScales.MaxPelvisRoll
		);
	
	
	OutPelvis_Pos = FMath::VInterpTo(OutPelvis_Pos, P_pos, T_intpl, S_intpl);
	OutPelvis_Rot = FMath::RInterpTo(OutPelvis_Rot, P_rot, T_intpl, S_intpl);
	
	// Spine
	FRotator S_rot = FRotator(PacketRecive[19].X * RigScales.MaxSpinePitch, PacketRecive[19].Y * RigScales.MaxSpineYaw, PacketRecive[19].Z * RigScales.MaxSpineRoll);
	OutSpine_Rot = FMath::RInterpTo(OutSpine_Rot, S_rot, T_intpl, S_intpl);
	
	FVector WorldR = GetOwner()->GetActorTransform().TransformPosition(FVector(-OutFootR_Pos.Y, OutFootR_Pos.Z, -OutFootR_Pos.X));
	FVector WorldL = GetOwner()->GetActorTransform().TransformPosition(FVector(OutFootL_Pos.Y, OutFootL_Pos.Z, OutFootL_Pos.X));
	
	WorldR.Y = CharacterMesh->GetBoneLocation("foot_r").Y;
	WorldL.Y = CharacterMesh->GetBoneLocation("foot_l").Y;
	
	DrawDebugSphere(GetWorld(), WorldR, 5.0f, 8, FColor::White, false, 0.05f);
	DrawDebugSphere(GetWorld(), WorldL, 5.0f, 8, FColor::White, false, 0.05f);
}
