		#include "SKSE/API.h"
		#include "SKSE/Skse.h"
		#include <atomic>
		#include <chrono>
		#include <iostream>
		#include <stdio.h>
		#include <string>
		#include <thread>
		#include <windows.h>

		const wchar_t* sharedMemoryName = L"Global\\MySharedMemory";
		const wchar_t* semaphoreName = L"Global\\MySharedMemorySemaphore";
		const size_t sharedMemorySize = 1024;

		std::atomic<bool> stopThread = false;
		std::thread readerThread;

		HANDLE hSemaphore = NULL;
		HANDLE hMapObject = NULL;
		LPVOID pSharedMem = NULL;

		void SharedMemoryReader()
		{
			// Create a console window for this thread
			AllocConsole();
			FILE* fDummy;
			freopen_s(&fDummy, "CONOUT$", "w", stdout);
			freopen_s(&fDummy, "CONOUT$", "w", stderr);

			std::cout << "Shared Memory Monitor Started\n";
			std::cout << "Press Ctrl+C in this window to stop monitoring\n\n";

			while (!stopThread) {
				if (pSharedMem != NULL) {
					// Wait for semaphore
					WaitForSingleObject(hSemaphore, INFINITE);

					// Print the shared memory content
					std::cout << "Shared Memory Content: " << (char*)pSharedMem << "\n";

					// Release semaphore
					ReleaseSemaphore(hSemaphore, 1, NULL);
				}

				// Sleep to prevent high CPU usage
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}

			// Cleanup console
			fclose(fDummy);
			FreeConsole();
		}

		void InitializeSharedMemoryAndSemaphore()
		{
			// Set up security attributes to allow cross-process access
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = FALSE;  // Handles are not inherited by child processes

			// Create a NULL DACL (allow all access)
			SECURITY_DESCRIPTOR sd;
			InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
			sa.lpSecurityDescriptor = &sd;

			// Create or open the semaphore with the security attributes
			hSemaphore = CreateSemaphoreW(
				&sa,  // Security attributes
				1,    // Initial count
				1,    // Maximum count
				semaphoreName);

			if (hSemaphore == NULL) {
				DWORD error = GetLastError();
				MessageBoxA(NULL, ("Semaphore creation failed: " + std::to_string(error)).c_str(), "Error", MB_OK);
				return;
			}

			// Create shared memory with the same security attributes
			hMapObject = CreateFileMappingW(
				INVALID_HANDLE_VALUE,
				&sa,  // Security attributes
				PAGE_READWRITE,
				0,
				sharedMemorySize,
				sharedMemoryName);

			if (hMapObject == NULL) {
				DWORD error = GetLastError();
				CloseHandle(hSemaphore);
				MessageBoxA(NULL, ("Shared memory creation failed: " + std::to_string(error)).c_str(), "Error", MB_OK);
				return;
			}

			// Map the shared memory
			pSharedMem = MapViewOfFile(
				hMapObject,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				sharedMemorySize);

			if (pSharedMem == NULL) {
				DWORD error = GetLastError();
				CloseHandle(hMapObject);
				CloseHandle(hSemaphore);
				MessageBoxA(NULL, ("Memory mapping failed: " + std::to_string(error)).c_str(), "Error", MB_OK);
				return;
			}

			// Start the reader thread
			readerThread = std::thread(SharedMemoryReader);
		}

		void WriteToSharedMemory(const std::string& message)
		{
			if (pSharedMem != NULL) {
				// Wait 
				WaitForSingleObject(hSemaphore, INFINITE);

				// P
				memcpy(pSharedMem, message.c_str(), message.size() + 1);

				// V
				ReleaseSemaphore(hSemaphore, 1, NULL);
			}
		}


		//Clears sharedmemory (if used)
		void Cleanup()
		{
			if (pSharedMem != NULL) {
				UnmapViewOfFile(pSharedMem);
			}
			if (hMapObject != NULL) {
				CloseHandle(hMapObject);
			}
			if (hSemaphore != NULL) {
				CloseHandle(hSemaphore);
			}
		}


		//Sets the plugin version in the dll
		extern "C" DLLEXPORT constinit SKSE::PluginVersionData SKSEPlugin_Version = [] {
			SKSE::PluginVersionData v{};
			v.PluginName("SharedMemory");
			v.PluginVersion({ 1, 0, 0 });
			v.UsesAddressLibrary(true);
			v.CompatibleVersions({ SKSE::RUNTIME_LATEST});
			return v;

		}();




		//Called on plugin loading
		extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
		{
			

			SKSE::Init(a_skse);
			SKSE::log::info("Plugin loaded!");

			REL::Version runtimeVersion = a_skse->RuntimeVersion();

			
			//wont even log that since the plugin wont be launched by the game...
			//SKSE::log::info("Runtime version: {}.{}.{}", runtimeVersion.get_major(), runtimeVersion.get_minor(), runtimeVersion.get_build());

			InitializeSharedMemoryAndSemaphore();
			WriteToSharedMemory("Hello from SKSE Plugin!");
			return true;
		}

		//Stops the thread, and cleans the shared memory segment to avoid any leakage.
		extern "C" DLLEXPORT void SKSEPlugin_Unload()
		{
			stopThread = true;

			if (readerThread.joinable()) {
				readerThread.join();
			}

			Cleanup();
		}
