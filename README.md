# NFDRS4 - National Fire Danger Rating System 4.0

This library provides all of the source code for NFDRS Version 4.0 including the Nelson Dead Fuel Moisture Model, the Growing Season Index-based Live Fuel Moisture Model, the NFDRS calculator, and NFDRS Spatial.

Also produces three apps: the `FireWxConverter`, `NFDRS4_cli` (command line interface), and `NFDRS4_spatial`. 

- `FireWxConverter`: Converts FW13 fire weather data files to FW21 fire weather data files.
- `NFDRS4_cli`: Produces live and dead fuel moistures as well as NFDRS indexes from FW21 fire weather data files.
- `NFDRS4_spatial`: Allows running the NFDRS4 code in a spatial grid (with NETCDF files for I/O).

## Table of Contents

- [NFDRS4 - National Fire Danger Rating System 4.0](#nfdrs4---national-fire-danger-rating-system-40)
  - [Table of Contents](#table-of-contents)
  - [Dependencies](#dependencies)
  - [Building](#building)
    - [Windows](#windows)
      - [Building config4cpp (Windows)](#building-config4cpp-windows)
      - [Building NFDRS4 from Visual Studio 2022 (Windows)](#building-nfdrs4-from-visual-studio-2022-windows)
    - [Linux](#linux)
  - [How to run](#how-to-run)
    - [NFDRS4\_spatial](#nfdrs4_spatial)
  - [License](#license)

## Dependencies

- **CMake:** Requires CMake version 3.8 or higher.
- **Config4cpp:** See [http://www.config4star.org/](http://www.config4star.org/). Used for defining configuration files for `NFDRS4_cli` and NFDRS4 initialization (station) parameters.  The complete source is included in the `extern/` directory and must be built to produce a static library.
- **utctime:** See [http://paulgriffiths.github.io/utctime/documentation/index.html](http://paulgriffiths.github.io/utctime/documentation/index.html). Used for handling time in NFDRS4. The complete source is in the `lib/utctime` directory.

## Building

### Windows

#### Building config4cpp (Windows)

1. Open the x64 Native Tools Command Prompt for VS 2022.
2. Navigate to the `NFDRS4/extern/config4cpp` directory.
3. Run: `nmake -f Makefile.win all64`
4. This produces `config4cpp.lib` in `NFDRS4/extern/config4cpp/lib`.

#### Building NFDRS4 from Visual Studio 2022 (Windows)

1. Open the NFDRS4 folder in Visual Studio 2022 (it will load as a CMake project).
2. Select **Project** -> **CMake Settings for NFDRS4**.
3. Create an x64-Release configuration.
4. Save the settings (CMake will run).
5. Edit `CMakeSettings.json`:
   - Set `CONFIG4CPP_DIR` to `<REPO_LOCATION>/NFDRS4/extern/config4cpp/include` (e.g., `D:/Repos/NFDRS4/extern/config4cpp/include`).
   - Set `CONFIG4CPP_LIB` to `<REPO_LOCATION>/NFDRS4/extern/config4cpp/lib/config4cpp.lib` (e.g., `D:/Repos/NFDRS4/extern/config4cpp/lib/config4cpp.lib`).
6. Save `CMakeSettings.json` (CMake will run).
7. Select **Build** -> **Build All**.
8. Select **Build** -> **Install NFDRS4**.

This creates an install folder with include and lib files for NFDRS4 and fw21, along with `NFDRS4_cli.exe` and `FireWxConverter.exe`.

### Linux

1. Navigate to `NFDRS4/extern/config4cpp` in a terminal.
2. Run `make`.  This creates `libconfig4cpp.a` in `NFDRS4/extern/config4cpp/lib`.
3. Navigate back to the NFDRS4 directory.
4. Create a `build/` directory and `cd` into it
5. Run `cmake ..`
6. Edit `CMakeCache.txt`:
   - Set `CONFIG4CPP_DIR` (e.g., `/home/<user>/src/NFDRS4/extern/config4cpp/include`).
   - Set `CONFIG4CPP_LIB` (e.g., `/home/<user>/src/NFDRS4/extern/config4cpp/lib/libconfig4cpp.a`).
7. Run `make`.
8. Run `sudo make install` (Optional).

## How to run

### NFDRS4_spatial

1. `cd` into `build/bin`
2. Run `./NFDRS4_spatial`

## License

NFDRS4 is public domain software, still under development.