// Copyright (C) 2026 Rafael Valoto.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Spine Rot")
	FRotator OutSpine_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutUpperArmR_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutUpperArmL_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutClavicleR_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig Arm Rot")
	FRotator OutClavicleL_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Pos")
	FVector OutFootR_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Pos")
	FVector OutFootL_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Rot")
	FRotator OutFootR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Rot")
	FRotator OutFootL_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallR Rot")
	FRotator OutBallR_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallL Rot")
	FRotator OutBallL_Rot;

protected:
	float L1_R; // femur R
	float L2_R; // tíbia R
	float L1_L;
	float L2_L;
	float LastTime;

	float SpacingR;
	float SpacingL;
	
	int32 frameCounter = 0;
	
	FVector AxisR;
	FVector AxisL;
	FVector LastFootPosR;
	FVector LastFootPosL;
};
