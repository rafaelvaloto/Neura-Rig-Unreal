#pragma once

#include "CoreMinimal.h"
#include "NRParameters.generated.h"

USTRUCT(BlueprintType)
struct FNRParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Spacing Foot R", Description = "leg spacing control."))
	float SpacingFootR = -0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Spacing Foot L", Description = "leg spacing control."))
	float SpacingFootL = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Offset Foot L", Description = "Control the foot offset to the center of the body.", min = 0.0f, max = 1.0f, UIMin = 0.0f, UIMax = 1.0f))
	float OffsetFootL = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Offset Foot R", Description = "Control the foot offset to the center of the body.", min = 0.0f, max = 1.0f, UIMin = 0.0f, UIMax = 1.0f))
	float OffsetFootR = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Velocity", Description = "Control the velocity of the foot.", min = 0.0f, max = 600.0f, UIMin = 0.0f, UIMax = 600.0f))
	float Velocity = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Parameters", meta = (DisplayName = "Interpolation Speed", ToolTip = "Control the interpolation speed of the foot movement. Higher values will result in snappier movements, while lower values will create smoother transitions."))
	float S_interpolation = 15.0f;

};
