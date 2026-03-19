# Neura RIG Plugin for Unreal Engine

Neura RIG is an Unreal Engine plugin designed for **Neural Motion Matching** and **procedural animation**. It provides efficient tools for character rigging and animation control.

## Installation

1. Copy the `UnrealNeuraRig` folder to your project's `Plugins` directory.
2. Restart the Unreal Engine editor.
3. Enable the plugin in the **Plugins** window (Edit > Plugins).

---

## 📺 Development Progress

![Neural IK Demo](AtGif.gif)

*The AI successfully learns to optimize the stride curve, achieving fluid movement and maintaining balance through real-time weight shift predictions.*

---

## Usage

To use the Neura RIG component in your C++ classes, include the relevant headers and initialize the `UNRComponent`.

### C++ Example

Add the following to your Character or Actor class:

```cpp
#include "API/v1/NRComponent.h"

// In your class constructor or where you initialize components:
NeuraRig = CreateDefaultSubobject<UNRComponent>(TEXT("UNRComponent"));
if (NeuraRig->IsActive() == false)
{
    NeuraRig->Activate();
}
```

Make sure your project's `.Build.cs` file includes `"UnrealNeuraRig"` in the `PublicDependencyModuleNames` or `PrivateDependencyModuleNames`.

## Features

- Neural Motion Matching support.
- Procedural animation components.
- Easy-to-use API for real-time rigging adjustments.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Created by Rafael Valoto.
