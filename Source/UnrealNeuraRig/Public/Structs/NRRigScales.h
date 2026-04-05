#pragma once

#include "CoreMinimal.h"
#include "NRRigScales.generated.h"

USTRUCT(BlueprintType)
struct FNRRigScales
{
	GENERATED_BODY()

	// --- BALL OFFSET POSITION (sme) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Ball rotation offset (X)", ToolTip = "Ball rotation offset (X)"))
	float BallOffsetPitch = 0.0f;

	// --- FOOT OFFSET POSITION (sme) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Forward Offset (X)", ToolTip = "Subtracted from the AI data to center the step. Default is 0.0 since the AI is already centered."))
	float FootOffsetX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Lateral Offset (Y)", ToolTip = "Subtracted from the AI data to center the step. Default is 0.0 since the AI is already centered."))
	float FootOffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Height Offset (Z)", ToolTip = "Subtracted from the AI data to center the step. Default is 0.0 since the AI is already centered."))
	float FootOffsetZ = 0.0f;

	// --- FOOT IK (position in cm) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Stride X", Description = "Control the stride forward/backward."))
	float MaxFootStrideX = 65.0f; // Maximum stride forward/backward

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Width Y", Description = "Control the side leg opening."))
	float MaxFootWidthY = 3.5f; // Side leg opening

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Height Z", Description = "Control the height of the foot when it leaves the ground."))
	float MaxFootHeightZ = 90.0f;

	// --- FOOT ROTATION (Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Pitch", Description = "Control the toe/heel rotation."))
	float MaxFootPitch = 65.0f; // Toe/heel rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Roll", Description = "Control the foot roll (inversion/eversion)."))
	float MaxFootRoll = 5.0f; // Ankle sprain

	// --- BALL ROTATION (Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Pitch", Description = "Control the toe/heel rotation."))
	float MaxBallPitch = 90.0f; // Toe/heel rotation

	// -- LegIK (Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Legs", meta = (DisplayName = "Max Thigh Pitch", Description = "Rotação máxima da coxa para frente/trás em graus."))
	float MaxThighPitch = 165.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Legs", meta = (DisplayName = "Max Calf Pitch", Description = "Dobra máxima do joelho (panturrilha) para trás em graus."))
	float MaxCalfPitch = 90.0f;

	// --- PELVIS (Position in cm) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Forward X", ToolTip = "Control the forward/backward movement of the pelvis."))
	float MaxPelvisForwardX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Drop Z", ToolTip = "Control the pelvic drop (squat)."))
	float MaxPelvisDropZ = 96.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Sway Y", ToolTip = "Control the lateral balance (weight transfer)."))
	float MaxPelvisSwayY = 0.4f;

	// --- PELVIS (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Pitch", ToolTip = "Control the forward/backward tilt of the pelvis."))
	float MaxPelvisPitch = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Yaw", ToolTip = "Control the left/right twist of the pelvis."))
	float MaxPelvisYaw = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Roll", ToolTip = "Control the hip drop/tilt (contra-lateral drop)."))
	float MaxPelvisRoll = 360.0f; // Queda natural do quadril durante a passada

	// --- SPINE (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Pitch", ToolTip = "Control the forward tilt of the body."))
	float MaxSpinePitch = 45.0f; // Reduzido de 30.0 para evitar que o personagem dobre muito para frente

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Yaw", ToolTip = "Control the shoulder twist."))
	float MaxSpineYaw = 30.0f; // Mantido, valor ideal para compensar a perna

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Roll", ToolTip = "Control the lateral tilt of the spine."))
	float MaxSpineRoll = 8.0f; // Reduzido de 15.0 para um contra-balanço mais sutil e realista

	// --- ARMS (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Swing", Description = "Control the forward and backward swing of the arms."))
	float MaxArmSwing = 60.0f; // Arm swing forward/backward

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Open", Description = "Control the opening of the arms."))
	float MaxArmOpen = 10.0f; // Open your arm to avoid hitting your body.
};
