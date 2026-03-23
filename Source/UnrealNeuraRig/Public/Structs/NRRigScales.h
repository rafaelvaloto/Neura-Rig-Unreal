#pragma once

#include "CoreMinimal.h"
#include "NRRigScales.generated.h"

USTRUCT(BlueprintType)
struct FNRRigScales
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Forward Offset (X)", ToolTip = "Subtracted from the AI data to center the step on the X-axis before applying the scale."))
	float FootOffsetX = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Lateral Offset (Y)", ToolTip = "Offset for the Y-axis of the foot."))
	float FootOffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Conversion Range", meta = (DisplayName = "Foot Height Offset (Z)", ToolTip = "Offset for the Z-axis of the foot."))
	float FootOffsetZ = 0.0f;

	// --- FOOT IK (position in cm) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Stride X", Description = "Control the stride forward/backward."))
	float MaxFootStrideX = 65.0f; // Maximum stride forward/backward

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Width Y", Description = "Control the side leg opening."))
	float MaxFootWidthY = 15.0f; // Side leg opening

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Height Z", Description = "Control the height of the foot when it leaves the ground."))
	float MaxFootHeightZ = 30.0f; // Maximum height of the foot when it leaves the ground.

	// --- FOOT ROTATION (Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Pitch", Description = "Control the toe/heel rotation."))
	float MaxFootPitch = 45.0f; // Toe/heel rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Roll", Description = "Control the foot roll (inversion/eversion)."))
	float MaxFootRoll = 15.0f; // Ankle sprain

	// --- BALL ROTATION (Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Foot", meta = (DisplayName = "Foot Pitch", Description = "Control the toe/heel rotation."))
	float MaxBallPitch = 45.0f; // Toe/heel rotation

	// --- PELVIS (Position in cm) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Forward X", ToolTip = "Control the forward/backward movement of the pelvis."))
	float MaxPelvisForwardX = 6.0f; // Reduzido para evitar avanço exagerado

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Drop Z", ToolTip = "Control the pelvic drop (squat)."))
	float MaxPelvisDropZ = 8.0f; // Reduzido de 8.0 para remover o aspecto agachado

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Sway Y", ToolTip = "Control the lateral balance (weight transfer)."))
	float MaxPelvisSwayY = 12.0f; // Reduzido de 12.0 para eliminar o rebolado

	// --- PELVIS (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Pitch", ToolTip = "Control the forward/backward tilt of the pelvis."))
	float MaxPelvisPitch = 1.5f; // Mantém o quadril mais estável

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Yaw", ToolTip = "Control the left/right twist of the pelvis."))
	float MaxPelvisYaw = 5.0f; // Reduzido de 20.0 para alinhar com o passo sem torcer demais

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Pelvis", meta = (DisplayName = "Pelvis Roll", ToolTip = "Control the hip drop/tilt (contra-lateral drop)."))
	float MaxPelvisRoll = 3.0f; // Queda natural do quadril durante a passada

	// --- SPINE (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Pitch", ToolTip = "Control the forward tilt of the body."))
	float MaxSpinePitch = 30.0f; // Reduzido de 30.0 para evitar que o personagem dobre muito para frente

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Yaw", ToolTip = "Control the shoulder twist."))
	float MaxSpineYaw = 15.0f; // Mantido, valor ideal para compensar a perna

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Spine", meta = (DisplayName = "Spine Roll", ToolTip = "Control the lateral tilt of the spine."))
	float MaxSpineRoll = 8.0f; // Reduzido de 15.0 para um contra-balanço mais sutil e realista

	// --- ARMS (Rotation in Degrees) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Swing", Description = "Control the forward and backward swing of the arms."))
	float MaxArmSwing = 60.0f; // Arm swing forward/backward

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Rig|Arms", meta = (DisplayName = "Arm Open", Description = "Control the opening of the arms."))
	float MaxArmOpen = 10.0f; // Open your arm to avoid hitting your body.
};
