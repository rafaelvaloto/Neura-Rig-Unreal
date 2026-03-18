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

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Pos")
	FVector OutFootR_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Pos")
	FVector OutFootL_Pos;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootR Rot")
	FRotator OutFootR_Rot;

	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig FootL Rot")
	FRotator OutFootL_Rot;
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallR Rot")
	FRotator OutBallR_Rot = FRotator(0.0f, 0.0f, 0.0f);
	
	UPROPERTY(BlueprintReadOnly, Category = "NeuraRig BallL Rot")
	FRotator OutBallL_Rot = FRotator(0.0f, 0.0f, 0.0f);


protected:
	float L1_R; // femur R
	float L2_R; // tíbia R
	float L1_L;
	float L2_L;
	float LastTime;

	FVector AxisR;
	FVector AxisL;
	FVector LastFootPosR;
	FVector LastFootPosL;
};
