# Sand Simulation with Elements

This project is an interactive sand simulation game that implements various materials with unique physics and interactions. Built using the Raylib library, the simulation allows users to create, manipulate, and observe complex interactions between different elements in a dynamic grid environment.

## Features

- **Multiple Materials**: Simulate 10 different elements with unique behaviors
- **Dynamic Physics**: Material-specific movement patterns and state changes
- **Interactive Tools**: Adjustable brush size, material selection, erase functions
- **Real-time Visuals**: Color transitions that indicate material states
- **Resizable Window**: Dynamic grid adjustment when resizing

## Controls

- **Left Mouse Button**: Place selected material
- **Mouse Wheel**: Adjust brush size (1-10 pixels)
- **UI Buttons**: Select material or tool (sand, water, stone, acid, gas, fire, erase, clear all)

## Material Interactions

| Material     | Behavior                  | Special Properties                          |
|--------------|---------------------------|---------------------------------------------|
| **Sand**     | Falls downward            | Displaces water and gas                     |
| **Water**    | Flows downward/sideways   | Converts to acid when near acid             |
| **Stone**    | Immovable solid           | Eroded by acid                              |
| **Acid**     | Falls, erodes materials   | Converts water to acid, evaporates over time|
| **Gas**      | Rises and spreads         | Ignites when near fire                      |
| **Fire**     | Spreads upward            | Evaporates water into steam                 |
| **Acid Gas** | Rises and spreads         | Dissipates over time                        |
| **Steam**    | Rises and spreads         | Condenses into rain over time               |
| **Rain**     | Falls downward            | Converts to water on impact                 |

## Building and Running

### Prerequisites
- Raylib library installed
- C compiler (gcc or similar)

### Compilation
```bash
gcc -o run game.c -lraylib -lm
