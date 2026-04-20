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

	frameCounter = 0;
	ConvergenceFrame = 0;


	CharacterMesh->SetHiddenInGame(false);

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
	
	FTransform P_p = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_World);
	UE_LOG(LogTemp, Log, TEXT("P_p: %s"), *P_p.ToString());
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
	
	auto PushVec = [&Raw](const FVector& V) {
		Raw.Add(V.X);
		Raw.Add(V.Y);
		Raw.Add(V.Z);
	};

	auto PushQuat = [&Raw](const FQuat& Q) {
		Raw.Add(Q.X);
		Raw.Add(Q.Y);
		Raw.Add(Q.Z);
		Raw.Add(Q.W);
	};

	FVector RightThighBaseOffset = PelvisW_Transform.GetLocation() + (PelvisW_Transform.GetRotation().GetRightVector() * (L_Pelvis * 50.0f));
	FVector LeftThighBaseOffset = PelvisW_Transform.GetLocation() - (PelvisW_Transform.GetRotation().GetRightVector() * (L_Pelvis * 50.0f));
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

	float Velocity01 = FMath::Clamp(Actor->GetVelocity().Size() / 600.05f, 0.0f, 1.f);
	if (Velocity01 <= 0.0f)
	{
		Velocity01 = FMath::Clamp(RigParameters.Velocity / 600.05f, 0.0f, 1.f);
	}
	Push1(Velocity01); // 0

	// JSON Offsets: []
	Push1(L1_R);                       // femur 1
	Push1(L2_R);                       // tibia 2
	Push1(L3_R);                       // foot 3
	Push1(L4_R);                       // ball 4
	Push1(IsHit0);                     // IsHit0 5
	Push1(RigParameters.OffsetFootR);  // offset 6
	Push1(RigParameters.SpacingFootR); // spacing 7

	// JSON Offsets: []
	Push1(L1_L); // 8
	Push1(L2_L); // 9
	Push1(L3_L); // 10
	Push1(L4_L); // 11
	Push1(IsHit1); // 12
	Push1(RigParameters.OffsetFootL);  // offset 13
	Push1(RigParameters.SpacingFootL); // spacing 14
	Push1(L_Pelvis); // 15
	
	PelvisW_Transform = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_Component);
	PushVec(PelvisW_Transform.GetLocation() * 0.01); // 16 17 18
	PushQuat(PelvisW_Transform.GetRotation()); // 19 20 21 22
	
	// FTransform pelvis = CharacterMesh->GetBoneTransform(TEXT("pelvis"), RTS_ParentBoneSpace);
	// FTransform thigh_r = CharacterMesh->GetBoneTransform(TEXT("thigh_r"), RTS_ParentBoneSpace);
	// FTransform thigh_l = CharacterMesh->GetBoneTransform(TEXT("thigh_l"), RTS_ParentBoneSpace);
	// FTransform calf_r = CharacterMesh->GetBoneTransform(TEXT("calf_r"), RTS_ParentBoneSpace);
	// FTransform calf_l = CharacterMesh->GetBoneTransform(TEXT("calf_l"), RTS_ParentBoneSpace);
	// FTransform foot_r = CharacterMesh->GetBoneTransform(TEXT("foot_r"), RTS_ParentBoneSpace);
	// FTransform foot_l = CharacterMesh->GetBoneTransform(TEXT("foot_l"), RTS_ParentBoneSpace);
	//
	// UE_LOG(LogTemp, Warning, TEXT("Pelvis: %s"), *pelvis.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("ThighR: %s"), *thigh_r.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("ThighL: %s"), *thigh_l.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("CalfR: %s"), *calf_r.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("CalfL: %s"), *calf_l.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("FootR: %s"), *foot_r.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("FootL: %s"), *foot_l.ToString());

	Push1(DeltaTime); // 23
	Push1(CurrentDilation); // 24
	Push1(0.0); // reserved 25

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
		if (InterpSpeed <= 0.05f)
			return Target;
		return FQuat::Slerp(Current, Target, FMath::Clamp(DeltaTime * InterpSpeed, 0.05f, 1.f));
	};

	// Pelvis (Offset 0, Size 7: vec3 + Quat)
	FVector PelvisPos = FVector(PacketRecive[0], PacketRecive[1], PacketRecive[2]) * 100.0f;
	OutPelvis_Pos = FMath::VInterpTo(OutPelvis_Pos, PelvisPos, T_intpl, S_intpl);

	FQuat PelvisQuat = GetQuat(&PacketRecive[3]);
	OutPelvis_Quat = InterpQuat(OutPelvis_Quat, PelvisQuat, T_intpl, S_intpl);
	OutPelvis_Rot = OutPelvis_Quat.Rotator();

	// Leg R (Offset 7, Size 21: 3x vec3+Quat)
	// Leg R Thigh (Offset 7 + 0*7)
	OutThighR_Pos = FVector(PacketRecive[7], PacketRecive[8], PacketRecive[9]) * 100.0f;
	OutThighR_Quat = InterpQuat(OutThighR_Quat, GetQuat(&PacketRecive[10]), T_intpl, S_intpl);
	OutThighR_Rot = OutThighR_Quat.Rotator();

	// Leg R Calf (Offset 7 + 1*7)
	OutCalfR_Pos = FVector(PacketRecive[14], PacketRecive[15], PacketRecive[16]) * 100.0f;
	OutCalfR_Quat = InterpQuat(OutCalfR_Quat, GetQuat(&PacketRecive[17]), T_intpl, S_intpl);
	OutCalfR_Rot = OutCalfR_Quat.Rotator();

	// Leg R Foot (Offset 7 + 2*7)
	FVector LocalPosR = FVector(PacketRecive[21], PacketRecive[22], PacketRecive[23]) * 100.0f;
	OutFootR_Pos = FMath::VInterpTo(OutFootR_Pos, LocalPosR, T_intpl, S_intpl);
	OutFootR_Quat = InterpQuat(OutFootR_Quat, GetQuat(&PacketRecive[24]), T_intpl, S_intpl);
	OutFootR_Rot = OutFootR_Quat.Rotator();

	// Leg L (Offset 28, Size 21: 3x vec3+Quat)
	// Leg L Thigh (Offset 28 + 0*7)
	OutThighL_Pos = FVector(PacketRecive[28], PacketRecive[29], PacketRecive[30]) * 100.0f;
	OutThighL_Quat = InterpQuat(OutThighL_Quat, GetQuat(&PacketRecive[31]), T_intpl, S_intpl);
	OutThighL_Rot = OutThighL_Quat.Rotator();

	// Leg L Calf (Offset 28 + 1*7)
	OutCalfL_Pos = FVector(PacketRecive[35], PacketRecive[36], PacketRecive[37]) * 100.0f;
	OutCalfL_Quat = InterpQuat(OutCalfL_Quat, GetQuat(&PacketRecive[38]), T_intpl, S_intpl);
	OutCalfL_Rot = OutCalfL_Quat.Rotator();

	// Leg L Foot (Offset 28 + 2*7)
	FVector LocalPosL = FVector(PacketRecive[42], PacketRecive[43], PacketRecive[44]) * 100.0f;
	OutFootL_Pos = FMath::VInterpTo(OutFootL_Pos, LocalPosL, T_intpl, S_intpl);
	OutFootL_Quat = InterpQuat(OutFootL_Quat, GetQuat(&PacketRecive[45]), T_intpl, S_intpl);
	OutFootL_Rot = OutFootL_Quat.Rotator();

	if (CharacterMesh && GetWorld())
	{
		#include "DrawDebugHelpers.h"

		const float AxisLen = 10.05f;
		const float SphereSize = 3.f;
		const float LineThickness = 0.5f;
		
		FTransform pW_T = FTransform(OutPelvis_Quat, OutPelvis_Pos, FVector::OneVector);
		UE_LOG(LogTemp, Log, TEXT("pW_Transform ( w * l ): %s"), *pW_T.ToString());
		
		FTransform thighRW_T = FTransform(OutThighR_Quat, OutThighR_Pos, FVector::OneVector) * pW_T;
		FTransform thighLW_T = FTransform(OutThighL_Quat, OutThighL_Pos, FVector::OneVector) * pW_T;
		
		FTransform calfRW_T = FTransform(OutCalfR_Quat, OutCalfR_Pos, FVector::OneVector) * thighRW_T;
		FTransform calfLW_T = FTransform(OutCalfL_Quat, OutCalfL_Pos, FVector::OneVector) * thighLW_T;
		FTransform footRW_T = FTransform(OutFootR_Quat, OutFootR_Pos, FVector::OneVector) * calfRW_T;
		FTransform footLW_T = FTransform(OutFootL_Quat, OutFootL_Pos, FVector::OneVector) * calfLW_T;
		
		
		// UE_LOG(LogTemp, Log, TEXT("Local OutPelvis_Pos: %s"), *OutPelvis_Pos.ToString());
		// UE_LOG(LogTemp, Log, TEXT("Local OutPelvis_Rot: %s"), *OutPelvis_Quat.Rotator().ToString());
		//
		// UE_LOG(LogTemp, Warning, TEXT("thighRW_T ( w * l ): %s"), *thighRW_T.ToString());
		// UE_LOG(LogTemp, Warning, TEXT("thighLW_T ( w * l ): %s"), *thighLW_T.ToString());
		// UE_LOG(LogTemp, Log, TEXT("Local OutThigh_R_Rot: %s"), *OutThighR_Quat.Rotator().ToString());
		// UE_LOG(LogTemp, Log, TEXT("Local OutThigh_L_Rot: %s"), *OutThighL_Quat.Rotator().ToString());

		DrawDebugSphere(GetWorld(), footRW_T.GetLocation(), 5.0f, 12, FColor::Black, false, 0.025f, 0, 0.5f);
		DrawDebugSphere(GetWorld(), footLW_T.GetLocation(), 5.0f, 12, FColor::Black, false, 0.025f, 0, 0.5f);
		
		// Joints, Axes [Pelvis]
		DrawDebugSphere(GetWorld(), pW_T.GetLocation(), 8.0f, 12, FColor::Yellow, false, 0.2f, 0, 0.1f);
		DrawDebugCoordinateSystem(GetWorld(), pW_T.GetLocation(), pW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		
		// Joints [Leg R]
		DrawDebugSphere(GetWorld(), thighRW_T.GetLocation(), SphereSize, 12, FColor::Red, false, 0.2f, 0, 0.1f);
		DrawDebugSphere(GetWorld(), calfRW_T.GetLocation(), SphereSize, 12, FColor::Red, false, 0.2f, 0, 0.1f);
		// Bones [Leg R]
		DrawDebugLine(GetWorld(), pW_T.GetLocation(), thighRW_T.GetLocation(), FColor::Red, false, 0.2f, 0, LineThickness);
		DrawDebugLine(GetWorld(), thighRW_T.GetLocation(), calfRW_T.GetLocation(), FColor::Red, false, 0.2f, 0, LineThickness);
		DrawDebugLine(GetWorld(), calfRW_T.GetLocation(), footRW_T.GetLocation(), FColor::Red, false, 0.2f, 0, LineThickness);
		// Axes [Leg R]
		DrawDebugCoordinateSystem(GetWorld(), thighRW_T.GetLocation(), thighRW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		DrawDebugCoordinateSystem(GetWorld(), calfRW_T.GetLocation(), calfRW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		DrawDebugCoordinateSystem(GetWorld(), footRW_T.GetLocation(), footRW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		
		// Joints [Leg L]
		DrawDebugSphere(GetWorld(), thighLW_T.GetLocation(), SphereSize, 12, FColor::Blue, false, 0.2f, 0, 0.1f);
		DrawDebugSphere(GetWorld(), calfLW_T.GetLocation(), SphereSize, 12, FColor::Blue, false, 0.2f, 0, 0.1f);
		// Bones [Leg L]
		DrawDebugLine(GetWorld(), pW_T.GetLocation(), thighLW_T.GetLocation(), FColor::Blue, false, 0.2f, 0, LineThickness);
		DrawDebugLine(GetWorld(), thighLW_T.GetLocation(), calfLW_T.GetLocation(), FColor::Blue, false, 0.2f, 0, LineThickness);
		DrawDebugLine(GetWorld(), calfLW_T.GetLocation(), footLW_T.GetLocation(), FColor::Blue, false, 0.2f, 0, LineThickness);
		// Axes [Leg L]
		DrawDebugCoordinateSystem(GetWorld(), thighLW_T.GetLocation(), thighLW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		DrawDebugCoordinateSystem(GetWorld(), calfLW_T.GetLocation(), calfLW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);
		DrawDebugCoordinateSystem(GetWorld(), footLW_T.GetLocation(), footLW_T.Rotator(), AxisLen, false, 0.2f, 0, 0.1f);

	}
}
