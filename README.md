# NeuraRIG Plugin for Unreal Engine | Development Phase

NeuraRIG is an advanced Unreal Engine plugin designed for **Neural Motion Matching** and **procedural animation**. Leveraging machine learning, it provides high-fidelity character rigging and real-time motion synthesis.

## 🚀 Current Progress

The AI model currently handles the end-to-end generation of motion paths and local transformations from the **pelvis down to the feet**. It successfully predicts optimal trajectories and joint orientations to ensure natural movement and physiological constraints.

*The system demonstrates real-time inference, calculating precise local positions and rotations for the entire lower body hierarchy, ensuring fluid transitions and grounded foot placement.*

---

## 🧠 Neural Training Core

This plugin works in tandem with the **[NeuraRig](https://github.com/rafaelvaloto/NeuraRig)** project, which contains the deep learning framework, dataset processing, and model training scripts used to generate the weights utilized by this runtime integration.

---

## 🛠 Installation

1. Clone or copy the `UnrealNeuraRig` folder into your Unreal Engine project's `Plugins` directory.
2. Restart the Unreal Engine editor.
3. Enable the plugin via **Edit > Plugins**.

## 📖 Usage

Integrate the `UNRComponent` into your character to enable neural-driven IK and motion matching.

### C++ Integration

Add the component to your Character class:

```cpp
#include "API/v1/NRComponent.h"

// Inside your constructor:
NeuraRig = CreateDefaultSubobject<UNRComponent>(TEXT("UNRComponent"));

// Ensure it is active:
if (NeuraRig && !NeuraRig->IsActive())
{
    NeuraRig->Activate();
}
```

Update your `YourProject.Build.cs` to include the module:
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "UnrealNeuraRig" });
```

## ✨ Key Features

- **Neural Motion Matching:** Real-time motion synthesis based on learned datasets.
- **Procedural IK:** Machine learning-enhanced inverse kinematics for realistic foot placement.
- **Low Latency Inference:** Optimized for real-time gameplay performance.
- **Bi-directional Network Sync:** Seamless communication between Unreal Engine and the neural inference server.

## ⚖️ License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Developed by **Rafael Valoto**.
