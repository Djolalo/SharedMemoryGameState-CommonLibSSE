This plugin aims to write in shared memory, in order to be able to access externally game's state (overall considering the use of ice and fire magics). 
It's using the example plugin provided by @Ryan-rsm-MccKenzie [available here](https://github.com/Ryan-rsm-McKenzie/ExamplePlugin-CommonLibSSE) 
The following instructions have been taken from his README.md aswell, so him and the team behind CommonlibSSE and SKSE have provided most of the code that we're trying to use here.

------------------------

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [The Elder Scrolls V: Skyrim Special Edition](https://store.steampowered.com/app/489830)
	* Add the environment variable `Skyrim64Path` to point to the root installation of your game directory (the one containing `SkyrimSE.exe`).
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Building
```
git clone https://github.com/Ryan-rsm-McKenzie/SharedMemoryGameState-CommonLibSSE
cd SharedMemoryGameState-CommonLibSSE
git submodule init
git submodule update
cmake --preset vs2022-windows
cmake --build build --config Release
```

## Tips
* Set `COPY_OUTPUT` to `ON` to automatically copy the built dll to the game directory, i.e. `cmake --preset vs2022-windows -DCOPY_OUTPUT=ON`
* Build the `package` target to automatically build and zip up your dll in a ready-to-distribute format.
