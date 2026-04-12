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
	
	Push1(L1_R); // femur
	Push1(L2_R); // tibia
	Push1(L3_R); // foot
	Push1(L4_R); // ball
	Push1(IsHit0);                   // IsHit0
	Push1(RigParameters.OffsetFootR);  // offset
	Push1(RigParameters.SpacingFootR); // spacing

	// JSON Offsets: []
	Push1(L1_L);
	Push1(L2_L);
	Push1(L3_L);
	Push1(L4_L);
	Push1(IsHit1);                   // IsHit1
	Push1(RigParameters.OffsetFootL);  // offset
	Push1(RigParameters.SpacingFootL); // spacing
	Push1(L_Pelvis);

	Push1(DeltaTime);
	Push1(CurrentDilation);
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
	if (PacketRecive.Num() < 49) return;

	float S_intpl = RigParameters.S_interpolation;
	float T_intpl = DeltaTime;

	auto GetRot = [](const float* p) {
		return FQuat(p[0], p[1], p[2], p[3]).Rotator();
	};

	auto GetQuat = [](const float* p) {
		return FQuat(p[0], p[1], p[2], p[3]);
	};

	auto InterpQuat = [](FQuat Current, FQuat Target, float DeltaTime, float InterpSpeed) {
		if (InterpSpeed <= 0.f) return Target;
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
	
	// Foot IK (Offset 7, Size 14: 2x vec3 + Quat)
	FVector FootScale = FVector(RigScales.MaxFootHeightZ, RigScales.MaxFootStrideX, RigScales.MaxFootWidthY);
	
	// Foot R (Offset 7)
	FVector LocalPosR = FVector(PacketRecive[7], PacketRecive[8], PacketRecive[9]) * FootScale;
	OutFootR_Pos = FMath::VInterpTo(OutFootR_Pos, LocalPosR, T_intpl, S_intpl);
	OutFootR_Quat = InterpQuat(OutFootR_Quat, GetQuat(&PacketRecive[10]), T_intpl, S_intpl);
	OutFootR_Rot = OutFootR_Quat.Rotator();

	// Foot L (Offset 14)
	FVector LocalPosL = FVector(PacketRecive[14], PacketRecive[15], PacketRecive[16]) * FootScale;
	OutFootL_Pos = FMath::VInterpTo(OutFootL_Pos, LocalPosL, T_intpl, S_intpl);
	OutFootL_Quat = InterpQuat(OutFootL_Quat, GetQuat(&PacketRecive[17]), T_intpl, S_intpl);
	OutFootL_Rot = OutFootL_Quat.Rotator();
	
	// Leg R Calf (Offset 21 + 0*7)
	OutCalfR_Quat = InterpQuat(OutCalfR_Quat, GetQuat(&PacketRecive[24]), T_intpl, S_intpl);
	OutCalfR_Rot = OutCalfR_Quat.Rotator();
	// Leg L Calf (Offset 21 + 1*7)
	OutCalfL_Quat = InterpQuat(OutCalfL_Quat, GetQuat(&PacketRecive[31]), T_intpl, S_intpl);
	OutCalfL_Rot = OutCalfL_Quat.Rotator();
	
	// Leg R Thigh (Offset 21 + 2*7)
	OutThighR_Quat = GetQuat(&PacketRecive[38]); 
	OutThighR_Rot = OutThighR_Quat.Rotator();
	// Leg L Thigh (Offset 21 + 3*7)
	OutThighL_Quat = GetQuat(&PacketRecive[45]);
	OutThighL_Rot = OutThighL_Quat.Rotator();
	
	// UE_LOG(LogTemp, Warning, TEXT("OutPelvis_Rot: %s"), *OutPelvis_Rot.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutFootL_Rot: %s"), *OutFootL_Rot.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutFootR_Rot: %s"), *OutFootR_Rot.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutThighR_Rot: %s"), *OutThighR_Rot.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutThighL_Rot: %s"), *OutThighL_Rot.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutCalfR_Quat: %s"), *OutCalfR_Quat.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("OutCalfL_Rot: %s"), *OutCalfL_Rot.ToString());

	if (CharacterMesh && GetWorld())
	{
		const FTransform ActorTrans = GetOwner()->GetActorTransform();
		
		auto DrawBone = [this, &ActorTrans](FVector Direction, FVector Start, float Length, FColor Color) -> FVector {
			FVector End = Start + (Direction * Length * 100.0f); // m -> cm
			DrawDebugLine(GetWorld(), ActorTrans.TransformPosition(Start), ActorTrans.TransformPosition(End), Color, false, 0.05f, 0, 2.0f);
			return End;
		};
		FVector PelvisCenter = OutPelvis_Pos;
		PelvisCenter.Z = 0.0f;
		PelvisCenter.X = 0.0f;
		FVector RightThighBase = PelvisCenter + (OutPelvis_Quat.GetRightVector() * (L_Pelvis * 50.0f)); 
		FVector LeftThighBase = PelvisCenter - (OutPelvis_Quat.GetRightVector() * (L_Pelvis * 50.0f));
		DrawDebugLine(GetWorld(), ActorTrans.TransformPosition(LeftThighBase), ActorTrans.TransformPosition(RightThighBase), FColor::White, false, 0.05f, 0, 3.0f);

		FVector RDirectionT = OutThighR_Quat.GetAxisZ();
		FVector RDirectionC = -OutCalfR_Quat.GetAxisZ();
		FVector RDirectionF = -OutFootR_Quat.GetForwardVector();
		
		FVector KneeR = DrawBone(RDirectionT, RightThighBase, L1_R, FColor::Blue);
		FVector AnkleR = DrawBone(RDirectionC, KneeR, L2_R, FColor::Blue);
		FVector FootR = DrawBone(RDirectionF, AnkleR, L3_R, FColor::Blue);

		FVector LDirectionT = OutThighL_Quat.GetAxisZ();
		FVector LDirectionC = OutCalfL_Quat.GetAxisZ();
		FVector LDirectionF = OutFootL_Quat.GetForwardVector();
		FVector KneeL = DrawBone(LDirectionT, LeftThighBase, L1_L, FColor::Red);
		FVector AnkleL = DrawBone(LDirectionC, KneeL, L2_L, FColor::Red);
		FVector FootL = DrawBone(LDirectionF, AnkleL, L3_L, FColor::Red);

		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(RightThighBase), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(LeftThighBase), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(KneeR), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(KneeL), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(AnkleR), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(AnkleL), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(FootR), 3.0f, 8, FColor::White, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(FootL), 3.0f, 8, FColor::White, false, 0.05f);

		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(FVector(OutFootR_Pos.Y, OutFootR_Pos.Z, OutFootR_Pos.X)), 4.0f, 8, FColor::Magenta, false, 0.05f);
		DrawDebugSphere(GetWorld(), ActorTrans.TransformPosition(FVector(OutFootL_Pos.Y, OutFootL_Pos.Z, OutFootL_Pos.X)), 4.0f, 8, FColor::White, false, 0.05f);
	}
}
