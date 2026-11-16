# Blackjack-Project

A C++ application for tracking blackjack shoe composition and calculating composition-dependent expected values (EV) for blackjack side bets.

The project includes **Felt**, a Qt 6 desktop interface for manually tracking a shoe and exploring exact 21+3 side-bet probabilities and expected value. Additional side bets and computer vision are future work.

## Run the desktop UI

Install Qt 6 with its Widgets module and CMake. On macOS with Homebrew, the minimal Qt package is `brew install qtbase`.

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qtbase
cmake --build build
./build/BlackjackUI
```

On Windows, install Qt 6 for your compiler using the Qt installer, point `CMAKE_PREFIX_PATH` at that Qt kit (for example `C:/Qt/6.x.x/msvc2022_64`), and run `build/Debug/BlackjackUI.exe` after a Debug build. Qt DLLs must be on your PATH, or deploy them alongside the executable using Qt's `windeployqt` tool.

The setup screen offers 1–16 decks, 21+3 selection, classic/tiered/custom net payouts, and automatic or manual EV updates. Starting a session initializes a fresh shoe. In the tracker:

- Click any of the 52 rank/suit buttons to remove one physical copy. Depleted cards are disabled.
- Use **Undo last card** (Ctrl+Z / Cmd+Z) to restore the most recently removed card.
- **Shuffle shoe** immediately resets the current shoe and removal history.
- In manual mode, shoe counts change immediately and the displayed results are marked **OUTDATED** until **Update EV** is pressed.
- EV, outcome probabilities, and the session trend use the actual C++ enumeration engine. Below three cards, EV is unavailable.
- Table settings let you configure a new session; **Start a fresh session** applies them and resets the shoe.

Perfect Pairs is shown as coming later and cannot be selected. The reusable `ShoeController` is the shared entry point for future computer vision input.

Qt is optional for the original CLI and tests. Without it, CMake builds those targets and explains how to enable the UI.

```sh
ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/BlackjackUI --smoke-test
```

The UI smoke test checks card removal, undo, depletion, shuffle, and manual refresh, and writes setup/tracker screenshots to `/tmp/felt-*.png` (macOS/Linux).

## 1. Prerequisites

The project is developed on Windows using Visual Studio Code and CMake.

Before building, install:

- **Git:** https://git-scm.com/downloads
- **Visual Studio Code:** https://code.visualstudio.com/
- **CMake (3.20 or newer):** https://cmake.org/download/
- **Microsoft C++ Build Tools:** https://visualstudio.microsoft.com/downloads/

When installing Microsoft C++ Build Tools, select the **Desktop development with C++** workload, including the MSVC compiler and Windows SDK.

### Recommended VS Code Extensions

Install the following extensions from the VS Code Extensions marketplace:

- C/C++ — Microsoft
- CMake — Microsoft
- CMake Tools — Microsoft
- C/C++ Extension Pack — Microsoft (optional)

### Verify Installation

Open a Developer PowerShell for Visual Studio terminal and run:

```powershell
cmake --version
git --version
cl
```

These commands should confirm that CMake, Git, and the MSVC C++ compiler are available.

If `cl` isn't recognized in a standard PowerShell terminal, try using Developer PowerShell for Visual Studio.

## 2. Clone the Repository

Clone the private GitHub repository using its URL:

```powershell
git clone <repository-url>
cd Blackjack-Project
```

Open the repository folder in VS Code:

```powershell
code .
```

You must have access to the private repository before cloning.

## 3. Project Structure

The current project is organized as follows:

```text
Blackjack-Project/
├── include/
│   ├── Shoe.h
│   ├── SideBetEV.h
│   └── CardRecognition.h
├── src/
│   ├── main.cpp
│   ├── Shoe.cpp
│   ├── SideBetEV.cpp
│   └── CardRecognition.cpp
├── cv/
│   └── main.cpp
├── recordings/ (to be added)
├── data/ (to be added)
└── CMakeLists.txt
```

**Main components:**

- `Shoe.h / Shoe.cpp` — Represents the remaining shoe using 52 rank-and-suit card categories. Supports different deck counts and removing individual cards.
- `SideBetEV.h / SideBetEV.cpp` — Calculates exact 21+3 outcome probabilities and EV using configurable payout structures.
- `src/main.cpp` — Entry point for testing and running the existing EV calculations.
- `CardRecognition.h / CardRecognition.cpp` — Files reserved for the developing computer vision component.
- `cv/` — Separate workspace for future computer vision testing.
- `recordings/` — Local video recordings for development and testing. (to be added)
- `data/` — Supporting datasets and related resources. (to be added)

Some CV files are placeholders and may not yet contain functional implementations.

## 4. Configure the Project with CMake

From the repository root, run:

```powershell
cmake -S . -B build
```

This configures the CMake project and generates the necessary build files inside the `build/` directory.

On Windows, CMake may automatically select a Visual Studio/MSVC generator.

If CMake cannot find a compiler, verify that Microsoft C++ Build Tools is installed and try configuring from Developer PowerShell for Visual Studio.

## 5. Build the Project

After configuration, run:

```powershell
cmake --build build --config Debug
```

This compiles the C++ source files and generates the executable.

When using the Visual Studio CMake generator, the executable is expected at:

```text
build/Debug/BlackjackProject.exe
```

## 6. Run the Project

From the repository root:

```powershell
.\build\Debug\BlackjackProject.exe
```

To build and run in one command:

```powershell
cmake --build build --config Debug; if ($LASTEXITCODE -eq 0) { .\build\Debug\BlackjackProject.exe }
```

This executable runs the original terminal demo. Run `BlackjackUI` for the graphical tracker (see the desktop quick start above).

## 7. Implemented Features

### Shoe Composition Tracking

The `Shoe` class maintains an array of 52 integers, representing the number of remaining cards of each rank-and-suit combination.

The card indexing system is:

```cpp
index = rank * 4 + suit;
```

Ranks:

- `0` = Ace
- `1` = 2
- ...
- `12` = King

Suits:

- `0` = Clubs
- `1` = Diamonds
- `2` = Hearts
- `3` = Spades

The shoe supports configurable deck counts, with six decks as the default.

### 21+3 Side-Bet EV

The EV engine evaluates the following mutually exclusive outcomes:

- Flush
- Straight
- Three of a kind
- Straight flush
- Suited three of a kind
- Losing hand

It enumerates possible three-card combinations and weights them according to the remaining physical copies of each card.

Outcome probabilities are then combined with a configurable payout structure to calculate the expected net profit for a $1 side bet.

### Configurable Payout Structures

The `PayoutStructure21Plus3` class allows multiple payout configurations.

Example:

```cpp
Shoe shoe(6);

// Default: 9:1 for every winning outcome
PayoutStructure21Plus3 defaultPayouts;

// Alternative payout structure
PayoutStructure21Plus3 customPayouts(
    5, 10, 30, 40, 100
);

double ev1 = calculateEV21Plus3(
    shoe, defaultPayouts
);

double ev2 = calculateEV21Plus3(
    shoe, customPayouts
);
```

Payout configurations can also be updated using `setPayouts()`.

## 8. Testing

The initial implementation has been checked against the standard six-deck, flat 9:1 21+3 benchmark.

Expected EV per $1 wagered:

```text
Approximately -0.032386
```

This corresponds to a house edge of approximately 3.2386%.

Additional testing should verify:

- Fresh shoes with different deck counts.
- Depleted shoe compositions.
- Probability totals summing to 1.
- Correct classification of every winning hand.
- Different payout configurations.
- Card-removal boundary conditions.

Core regression tests cover the fresh-shoe benchmark, probability normalization, chronological undo, depleted cards, configuration validation, and small-shoe boundaries. Run them with `ctest --test-dir build --output-on-failure`.

## 9. Planned Features

### Manual Shoe Tracking UI (implemented)

The Qt desktop interface allows users to:

- Configure the number of decks.
- Select supported side bets and payout structures.
- View all 52 card categories.
- Remove exposed cards by clicking them.
- Reset the shoe using a shuffle button.
- Undo incorrect card removals.
- Display EV calculations.
- Choose automatic or manual EV updates.

### Additional Side Bets

Extend the EV engine to calculate other blackjack side bets using the existing shoe representation.

### Computer Vision

Use OpenCV and potentially trained machine-learning models to identify exposed cards from recorded blackjack footage.

The objective is for automatic card recognition and manual card entry to use the same shoe-management system.

## 10. Development and Git Workflow

Before making changes, verify your current branch:

```powershell
git branch
git status
```

After making and testing changes, stage and commit them on the appropriate development branch:

```powershell
git add .
git status
git commit -m "Describe changes made"
```

Push the current branch:

```powershell
git push
```

If the branch does not yet have an upstream configured, use:

```powershell
git push -u origin <branch-name>
```

Use pull requests when merging collaborative changes into `main`.

Do not commit generated build files, executables, or large recordings. These should be excluded using `.gitignore`.

## 11. Troubleshooting

**CMake is not recognized**

Verify the installation using `cmake --version` and confirm CMake is available in your system PATH.

**MSVC compiler is not recognized**

Open Developer PowerShell for Visual Studio and run `cl`. Confirm the Desktop development with C++ workload is installed.

**Executable not found**

Ensure the build succeeded, then locate generated executables using:

```powershell
Get-ChildItem .\build -Recurse -Filter *.exe
```

**Linker error: unresolved external symbol**

Verify that the function has a matching implementation and that its `.cpp` source file is included in `CMakeLists.txt`.

**Changes are not appearing when running**

Save all source files and rebuild:

```powershell
cmake --build build --config Debug
```

If `CMakeLists.txt` was modified, reconfigure first:

```powershell
cmake -S . -B build
```

---

**Project status:** Core shoe tracking, exact 21+3 EV, and the manual Qt interface are implemented. Computer vision and additional side bets are future work.
