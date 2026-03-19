#pragma once

#include "CoreMinimal.h"
#include "NRRigScales.generated.h"

USTRUCT(BlueprintType)
struct FNRRigScales
{
    GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Forward Offset (X)", ToolTip = "Subtracted from the AI ​​data to center the step on the X-axis before applying the scale."))
	float FootOffsetX = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Lateral Offset (Y)", ToolTip = "Offset for the Y-axis of the foot."))
	float FootOffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Height Offset (Z)", ToolTip = "Offset for the Z-axis of the foot."))
	float FootOffsetZ = 0.0f;
	
    // --- FOOT IK (position in cm) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Stride X", Description = "Control the stride forward/backward."))
    float MaxFootStrideX = 70.0f; // Maximum stride forward/backward

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Width Y", Description = "Control the side leg opening."))
    float MaxFootWidthY = 15.0f;  // Side leg opening

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Height Z", Description = "Control the height of the foot when it leaves the ground."))
    float MaxFootHeightZ = 25.0f; // Maximum height of the foot when it leaves the ground.

    // --- FOOT ROTATION (Degrees) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Pitch", Description = "Control the toe/heel rotation."))
    float MaxFootPitch = 45.0f;   // Toe/heel rotation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Roll", Description = "Control the foot roll (inversion/eversion)."))
    float MaxFootRoll = 15.0f;    // Ankle sprain

    // --- PELVIS (position in cm) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Drop Z", Description = "Control the pelvic drop (squat)."))
    float MaxPelvisDropZ = 8.0f;  // Pelvic drop (squat)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Sway Y", Description = "Control the lateral balance (weight transfer)."))
    float MaxPelvisSwayY = 12.0f; // Lateral balance (weight transfer)

    // --- SPINE (Rotation in Degrees) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Pitch", Description = "Control the forward tilt of the body."))
    float MaxSpinePitch = 30.0f;  // Forward tilt of the body

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Yaw", Description = "Control the shoulder twist."))
    float MaxSpineYaw = 45.0f;    // Shoulder twist

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Roll", Description = "Control the lateral tilt of the spine."))
    float MaxSpineRoll = 15.0f;   // Lateral tilt of the spine

    // --- ARMS (Rotation in Degrees) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Swing", Description = "Control the forward and backward swing of the arms."))
    float MaxArmSwing = 60.0f;    // Arm swing forward/backward

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Open", Description = "Control the opening of the arms."))
    float MaxArmOpen = 10.0f;     // Open your arm to avoid hitting your body.
};
