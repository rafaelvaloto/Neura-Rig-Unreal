// Copyright (C) 2026 Rafael Valoto.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/NRParameters.h"
#include "Structs/NRRigScales.h"

#include "NRComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALNEURARIG_API UNRComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNRComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Debug")
	class UTextRenderComponent* AIDebugText;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Status")
	int32 ConvergenceFrame = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Status")
	bool bHasConverged = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Pelvis Pos")
	FVector OutPelvis_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Pelvis Rot")
	FRotator OutPelvis_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Pelvis Rot")
	FQuat OutPelvis_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Spine Rot")
	FRotator OutSpine_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Spine Rot")
	FQuat OutSpine_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutUpperArmR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FQuat OutUpperArmR_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutUpperArmL_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FQuat OutUpperArmL_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutClavicleR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FQuat OutClavicleR_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutClavicleL_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FQuat OutClavicleL_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Pos")
	FVector OutFootR_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Pos")
	FVector OutFootL_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Rot")
	FRotator OutFootR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Rot")
	FQuat OutFootR_Quat;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Rot")
	FRotator OutFootL_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Rot")
	FQuat OutFootL_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallR Rot")
	FRotator OutBallR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallR Rot")
	FQuat OutBallR_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallL Rot")
	FRotator OutBallL_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallL Rot")
	FQuat OutBallL_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfR Pos")
	FRotator OutCalfR_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfL Pos")
	FRotator OutCalfL_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfR Rot")
	FRotator OutCalfR_Rot = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfR Rot")
	FQuat OutCalfR_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfL Rot")
	FRotator OutCalfL_Rot = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig CalfL Rot")
	FQuat OutCalfL_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig ThighR Rot")
	FRotator OutThighR_Rot = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig ThighR Rot")
	FQuat OutThighR_Quat;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig ThighL Rot")
	FRotator OutThighL_Rot = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig ThighL Rot")
	FQuat OutThighL_Quat;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeuraRig Scales", meta = (DisplayName = "Rig Scales", Description = "Control the scales of the neural rig."))
	FNRRigScales RigScales;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeuraRig Parameters", meta = (DisplayName = "Rig Parameters", Description = "Control the parameters of the neural rig."))
	FNRParameters RigParameters;

protected:
	float L_Pelvis; // Pelvis (distância entre coxas)
	float L1_R; // femur R
	float L2_R; // tíbia R
	float L3_R; // foot R
	float L4_R; // ball R
	float L1_L;
	float L2_L;
	float L3_L;
	float L4_L;
	float LastTime;
	float SpacingR;
	float SpacingL;
	int32 frameCounter;
	
	FVector AxisR;
	FVector AxisL;
	
private:
	void UpdateIK(USkeletalMeshComponent* CharacterMesh, const TArray<float>& PacketRecive, float DeltaTime);
};
