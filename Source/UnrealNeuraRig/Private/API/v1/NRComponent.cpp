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

	OutPelvis_Quat = FQuat::Identity;
	OutSpine_Quat = FQuat::Identity;
	OutUpperArmR_Quat = FQuat::Identity;
	OutUpperArmL_Quat = FQuat::Identity;
	OutClavicleR_Quat = FQuat::Identity;
	OutClavicleL_Quat = FQuat::Identity;
	OutFootR_Quat = FQuat::Identity;
	OutFootL_Quat = FQuat::Identity;
	OutBallR_Quat = FQuat::Identity;
	OutBallL_Quat = FQuat::Identity;
	OutCalfR_Quat = FQuat::Identity;
	OutCalfL_Quat = FQuat::Identity;
	OutThighR_Quat = FQuat::Identity;
	OutThighL_Quat = FQuat::Identity;
}

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

	CharacterMesh->SetHiddenInGame(true);

	UNRNetwork::InitSocket();

	SpacingR = RigParameters.SpacingFootR;
	SpacingL = RigParameters.SpacingFootL;

	auto MeasureBoneDistance = [CharacterMesh](const FName& A, const FName& B) -> float {
		const FVector PosA = CharacterMesh->GetSocketLocation(A);
		const FVector PosB = CharacterMesh->GetBoneLocation(B);
		return FVector::Dist(PosA, PosB) * 0.01f; // cm -> m
	};

	constexpr float DefaultFootTipLength = 0.05f;

	// Measure bones
	L_Pelvis = MeasureBoneDistance(TEXT("thigh_r"), TEXT("thigh_l"));

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

	UE_LOG(LogTemp, Log, TEXT("Pelvis: L=%f"), L_Pelvis);
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

	auto PushQuat = [&Raw](const FQuat& Q) {
		Raw.Add(Q.X);
		Raw.Add(Q.Y);
		Raw.Add(Q.Z);
		Raw.Add(Q.W);
	};

	FVector PelvisCenter = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_Component).GetLocation();
	FQuat PelvisRotator = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_Component).InverseTransformRotation(FQuat::Identity);
	FVector RightThighBaseOffset = PelvisCenter + (PelvisRotator.GetRightVector() * (L_Pelvis * 50.0f));
	FVector LeftThighBaseOffset = PelvisCenter - (PelvisRotator.GetRightVector() * (L_Pelvis * 50.0f));
	RigParameters.SpacingFootR = RightThighBaseOffset.X * 0.01;
	RigParameters.SpacingFootL = LeftThighBaseOffset.X * 0.01;

	if (ConvergenceFrame == 0)
	{
		frameCounter++;
	}

	float CurrentDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

	auto DetectFootHit = [this](USkeletalMeshComponent* Mesh, const FName& FootBoneName) -> float {
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
	Push1(L1_R);                       // femur
	Push1(L2_R);                       // tibia
	Push1(L3_R);                       // foot
	Push1(L4_R);                       // ball
	Push1(IsHit0);                     // IsHit0
	Push1(RigParameters.OffsetFootR);  // offset
	Push1(RigParameters.SpacingFootR); // spacing

	// JSON Offsets: []
	Push1(L1_L);
	Push1(L2_L);
	Push1(L3_L);
	Push1(L4_L);
	Push1(IsHit1);                     // IsHit1
	Push1(RigParameters.OffsetFootL);  // offset
	Push1(RigParameters.SpacingFootL); // spacing
	Push1(L_Pelvis);

	Push1(DeltaTime);
	Push1(CurrentDilation);
	Push1(0.08f);
	Push1(-0.08f);
	Push1(0.0); // reserved

	if (Raw.Num() > 0)
	{
		const int32 DataSize = Raw.Num() * sizeof(float);
		UNRNetwork::SendDataIK(Raw.GetData(), DataSize);

		TArray<float> PacketRecive;
		if (UNRNetwork::ReciveDataIK(PacketRecive))
		{
			if (PacketRecive.Num() >= 49)
			{
				bHasConverged = true;
				ConvergenceFrame = frameCounter;
				UpdateIK(CharacterMesh, PacketRecive, DeltaTime);
			}
		}
	}
}

void UNRComponent::UpdateIK(USkeletalMeshComponent* CharacterMesh, const TArray<float>& PacketRecive, float DeltaTime)
{
	if (PacketRecive.Num() < 49)
		return;

	float S_intpl = RigParameters.S_interpolation;
	float T_intpl = DeltaTime;

	auto GetRot = [](const float* p) {
		return FQuat(p[0], p[1], p[2], p[3]).Rotator();
	};

	auto GetQuat = [](const float* p) {
		return FQuat(p[0], p[1], p[2], p[3]);
	};

	auto InterpQuat = [](FQuat Current, FQuat Target, float DeltaTime, float InterpSpeed) {
		if (InterpSpeed <= 0.f)
			return Target;
		return FQuat::Slerp(Current, Target, FMath::Clamp(DeltaTime * InterpSpeed, 0.f, 1.f));
	};

	// Pelvis (Offset 0, Size 7: vec3 + Quat)
	FVector PelvisPos = FVector(PacketRecive[0], PacketRecive[1], PacketRecive[2]);
	PelvisPos.X *= RigScales.MaxPelvisForwardX;
	PelvisPos.Y *= RigScales.MaxPelvisSwayY;
	PelvisPos.Z *= RigScales.MaxPelvisDropZ;
	OutPelvis_Pos = FMath::VInterpTo(OutPelvis_Pos, PelvisPos, T_intpl, S_intpl);

	FQuat PelvisQuat = GetQuat(&PacketRecive[3]);
	OutPelvis_Quat = InterpQuat(OutPelvis_Quat, PelvisQuat, T_intpl, S_intpl);
	OutPelvis_Rot = OutPelvis_Quat.Rotator();

	// Leg R (Offset 7, Size 21: 3x vec3+Quat)
	// Leg R Thigh (Offset 7 + 0*7)
	OutThighR_Pos = FVector(PacketRecive[7], PacketRecive[8], PacketRecive[9]) * 100.f;
	OutThighR_Quat = InterpQuat(OutThighR_Quat, GetQuat(&PacketRecive[10]), T_intpl, S_intpl);
	OutThighR_Rot = OutThighR_Quat.Rotator();

	// Leg R Calf (Offset 7 + 1*7)
	OutCalfR_Pos = FVector(PacketRecive[14], PacketRecive[15], PacketRecive[16]) * 100.f;
	OutCalfR_Quat = InterpQuat(OutCalfR_Quat, GetQuat(&PacketRecive[17]), T_intpl, S_intpl);
	OutCalfR_Rot = OutCalfR_Quat.Rotator();

	// Leg R Foot (Offset 7 + 2*7)
	FVector LocalPosR = FVector(PacketRecive[21], PacketRecive[22], PacketRecive[23]) * 100.f;
	OutFootR_Pos = FMath::VInterpTo(OutFootR_Pos, LocalPosR, T_intpl, S_intpl);
	OutFootR_Quat = InterpQuat(OutFootR_Quat, GetQuat(&PacketRecive[24]), T_intpl, S_intpl);
	OutFootR_Rot = OutFootR_Quat.Rotator();

	// Leg L (Offset 28, Size 21: 3x vec3+Quat)
	// Leg L Thigh (Offset 28 + 0*7)
	OutThighL_Pos = FVector(PacketRecive[28], PacketRecive[29], PacketRecive[30]) * 100.f;
	OutThighL_Quat = InterpQuat(OutThighL_Quat, GetQuat(&PacketRecive[31]), T_intpl, S_intpl);
	OutThighL_Rot = OutThighL_Quat.Rotator();

	// Leg L Calf (Offset 28 + 1*7)
	OutCalfL_Pos = FVector(PacketRecive[35], PacketRecive[36], PacketRecive[37]) * 100.f;
	OutCalfL_Quat = InterpQuat(OutCalfL_Quat, GetQuat(&PacketRecive[38]), T_intpl, S_intpl);
	OutCalfL_Rot = OutCalfL_Quat.Rotator();

	// Leg L Foot (Offset 28 + 2*7)
	FVector LocalPosL = FVector(PacketRecive[42], PacketRecive[43], PacketRecive[44]) * 100.f;
	OutFootL_Pos = FMath::VInterpTo(OutFootL_Pos, LocalPosL, T_intpl, S_intpl);
	OutFootL_Quat = InterpQuat(OutFootL_Quat, GetQuat(&PacketRecive[45]), T_intpl, S_intpl);
	OutFootL_Rot = OutFootL_Quat.Rotator();

	if (CharacterMesh && GetWorld())
	{
		#include "DrawDebugHelpers.h"

		FTransform ZOffset = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_Component);
		FVector PelvisWorld = OutPelvis_Pos + ZOffset.GetLocation();
		PelvisWorld.Z += 200.f;
		const FQuat PelvisRotWorld = OutPelvis_Quat;

		auto ToWorldPos = [&](const FVector& RelativePos, const FVector& ParentWorldPos, const FQuat& ParentWorldRot) -> FVector {
			return ParentWorldPos + ParentWorldRot.RotateVector(RelativePos);
		};

		auto ToWorldRot = [&](const FQuat& ParentWorldRot, const FQuat& LocalRot) -> FQuat {
			return ParentWorldRot * LocalRot;
		};

		const FVector Pelvis = PelvisWorld;
		const FRotator PelvisDrawRot = PelvisRotWorld.Rotator();

		// Use bone lengths instead of AI positions (L1, L2, L3 are in meters, convert to cm)
		// Assume bone axis is X (common in UE Mannequin)
		const FVector OffsetThighR = FVector(0.f, L_Pelvis * 50.f, 0.f); // Half pelvis distance to the right
		const FVector OffsetThighL = FVector(0.f, -L_Pelvis * 50.f, 0.f);
		
		UE_LOG(LogTemp, Warning, TEXT("Pelvis: %s"), *Pelvis.ToString());
		UE_LOG(LogTemp, Warning, TEXT("OutThighR_Rot: %s"), *OutThighR_Rot.ToString());
		UE_LOG(LogTemp, Warning, TEXT("OutThighR_Pos: %s"), *OutThighR_Pos.ToString());
		
		UE_LOG(LogTemp, Warning, TEXT("OutThighL_Pos: %s"), *OutThighL_Pos.ToString());
		UE_LOG(LogTemp, Warning, TEXT("OutThighL_Rot: %s"), *OutThighL_Rot.ToString());
		

		const FVector ThighRWorld = ToWorldPos(OffsetThighR, Pelvis, PelvisRotWorld);
		const FQuat ThighRRotW = ToWorldRot(PelvisRotWorld, OutThighR_Quat);

		const FVector CalfRWorld = ToWorldPos(FVector(L1_R * 100.f, 0.f, 0.f), ThighRWorld, ThighRRotW);
		const FQuat CalfRRotW = ToWorldRot(ThighRRotW, OutCalfR_Quat);

		const FVector FootRWorld = ToWorldPos(FVector(L2_R * 100.f, 0.f, 0.f), CalfRWorld, CalfRRotW);
		const FQuat FootRRotW = ToWorldRot(CalfRRotW, OutFootR_Quat);

		const FVector ThighLWorld = ToWorldPos(OffsetThighL, Pelvis, PelvisRotWorld);
		const FQuat ThighLRotW = ToWorldRot(PelvisRotWorld, OutThighL_Quat);

		const FVector CalfLWorld = ToWorldPos(FVector(L1_L * 100.f, 0.f, 0.f), ThighLWorld, ThighLRotW);
		const FQuat CalfLRotW = ToWorldRot(ThighLRotW, OutCalfL_Quat);

		const FVector FootLWorld = ToWorldPos(FVector(L2_L * 100.f, 0.f, 0.f), CalfLWorld, CalfLRotW);
		const FQuat FootLRotW = ToWorldRot(CalfLRotW, OutFootL_Quat);

		const float AxisLen = 12.f;
		
		UE_LOG(LogTemp, Log, TEXT("FootRWorld: %s"), *FootRWorld.ToString());
		UE_LOG(LogTemp, Log, TEXT("FootL: %s"), *FootLWorld.ToString());
		
		FVector Fw_r = Pelvis + FVector(OutFootR_Pos.Y, OutFootR_Pos.Z, OutFootR_Pos.X);
		FVector Fw_l = Pelvis + FVector(OutFootL_Pos.Y, OutFootL_Pos.Z, OutFootL_Pos.X);
		
		DrawDebugSphere(GetWorld(), Fw_r,  5.0, 12, FColor::Black, false, 0.2f);
		DrawDebugSphere(GetWorld(), Fw_l,  5.0, 12, FColor::Black, false, 0.2f);

		DrawDebugSphere(GetWorld(), Pelvis, 3.f, 8, FColor::Yellow, false, 0.f, 0, 1.f);

		DrawDebugSphere(GetWorld(), ThighRWorld, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);
		DrawDebugSphere(GetWorld(), CalfRWorld, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);
		DrawDebugSphere(GetWorld(), Fw_r, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);

		DrawDebugSphere(GetWorld(), ThighLWorld, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);
		DrawDebugSphere(GetWorld(), CalfLWorld, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);
		DrawDebugSphere(GetWorld(), Fw_l, 3.f, 8, FColor::White, false, 0.f, 0, 1.f);

		DrawDebugLine(GetWorld(), Pelvis, ThighRWorld, FColor::Red, false, 0.f, 0, 2.f);
		DrawDebugLine(GetWorld(), ThighRWorld, CalfRWorld, FColor::Red, false, 0.f, 0, 2.f);
		DrawDebugLine(GetWorld(), CalfRWorld, Fw_r, FColor::Blue, false, 0.f, 0, 2.f);

		DrawDebugLine(GetWorld(), Pelvis, ThighLWorld, FColor::Blue, false, 0.f, 0, 2.f);
		DrawDebugLine(GetWorld(), ThighLWorld, CalfLWorld, FColor::Blue, false, 0.f, 0, 2.f);
		DrawDebugLine(GetWorld(), CalfLWorld, Fw_l, FColor::White, false, 0.f, 0, 2.f);

		DrawDebugCoordinateSystem(GetWorld(), Pelvis, PelvisDrawRot, AxisLen, false, 0.f, 0, 1.f);
		DrawDebugCoordinateSystem(GetWorld(), ThighRWorld, ThighRRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);
		DrawDebugCoordinateSystem(GetWorld(), CalfRWorld, CalfRRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);
		DrawDebugCoordinateSystem(GetWorld(), Fw_r, FootRRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);

		DrawDebugCoordinateSystem(GetWorld(), ThighLWorld, ThighLRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);
		DrawDebugCoordinateSystem(GetWorld(), CalfLWorld, CalfLRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);
		DrawDebugCoordinateSystem(GetWorld(), Fw_l, FootLRotW.Rotator(), AxisLen, false, 0.f, 0, 1.f);
	}
}
