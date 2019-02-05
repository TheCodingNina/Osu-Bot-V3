#include <Common/Pch.h>

#include <Content/OsuBot/SigScan.h>
#include <Common/SplitString.h>

#include <TlHelp32.h>
#include <sstream>


using namespace OsuBot::SigScan;


// Get the process handle and ID with a process name.
bool SigScanner::GetProcess(_In_ std::wstring processName) {
	// Get a handle of a process.
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	
	PROCESSENTRY32W processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32W);

	// Find the process name with a PROCESSENTRY32W object.
	do {
		if (processEntry.szExeFile == processName) {
			// Process name found, set the variables of the SigScanner.
			m_targetID = processEntry.th32ProcessID;
			m_targetProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_targetID);

			// Close the handle and return TRUE.
			CloseHandle(hProcess);
			return TRUE;
		}
	} while (Process32NextW(hProcess, &processEntry));

	// No process found with the same name.
	// Close the handle and return FALSE.
	CloseHandle(hProcess);
	return FALSE;
}

// Fills the m_targetRegion struct.
void SigScanner::GetRegion(_In_opt_ const UINT& startAddress) {
	MEMORY_BASIC_INFORMATION mbi;
	LPVOID address = NULL;

	// If given set the address to the argument.
	if (startAddress != NULL) {
		address = reinterpret_cast<LPVOID>(startAddress);
	}

	// Find a memory region with Commit state.
	do {
		// Get a region in the target process.
		VirtualQueryEx(m_targetProcess, address, &mbi, sizeof(mbi));

		// Fill the region struct.
		m_targetRegion.dwBase = reinterpret_cast<DWORD>(mbi.BaseAddress);
		m_targetRegion.dwSize = (DWORD)mbi.RegionSize;

		// Set the address to the next region.
		address = reinterpret_cast<LPVOID>(m_targetRegion.dwBase + m_targetRegion.dwSize);
	} while (mbi.State != MEM_COMMIT || mbi.Protect != PAGE_EXECUTE_READWRITE);


	//// Get the module base (entry point).
	//MODULEINFO mInfo;
	//if (K32GetModuleInformation(m_targetProcess, nullptr, &mInfo, sizeof(mInfo))) {
	//	// Info get, set the base address.
	//	m_targetModule.dwBase = reinterpret_cast<DWORD>(mInfo.EntryPoint);
	//}
	//else {
	//	// When not found fill with NULL.
	//	m_targetModule.dwBase = NULL;
	//}
	//
	//// Get the module size (working set size).
	//PROCESS_MEMORY_COUNTERS pmcInfo;
	//if (K32GetProcessMemoryInfo(m_targetProcess, &pmcInfo, sizeof(pmcInfo))) {
	//	// Info get, set the module size.
	//	m_targetModule.dwSize = static_cast<DWORD>(pmcInfo.WorkingSetSize);
	//}
	//else {
	//	// When not found fill with NULL.
	//	m_targetModule.dwSize = NULL;
	//}
}


// Finding the signature and returns the address in the memory.
void SigScanner::FindSignature(
	_In_ const std::wstring* signatureString
) {
	// Spilt the sig string into tokens.
	std::vector<std::wstring> tokens = SplitString(*signatureString, L"\\");

	// Prepair the tokens for conversion.
	if (tokens.front() == L"\\") tokens.at(0) = tokens.begin()->substr(1);
	if (tokens.back() == L"\n") tokens.end()->pop_back();


	// Make BYTE array.
	std::vector<BYTE> signature;
	std::wstringstream ss;
	for (auto set : tokens) {
		ss << std::hex << set;
		UINT x; ss >> x;

		signature.push_back((BYTE)x);

		ss.clear();
	}


	const DWORD mult = 4096UL;
	BYTE data[mult];
	DWORD startAddress;
	DWORD endAddress = NULL;		// This is set the NULL, for the first memory region.
	bool hit = TRUE;

	// Iterate trough memory regions.
	do {
		// Get the start and end addresses.
		GetRegion(endAddress);

		// Set the start and end addresses.
		startAddress = m_targetRegion.dwBase;
		endAddress = m_targetRegion.dwBase + m_targetRegion.dwSize;

		// Search the memory region.
		for (DWORD i = startAddress; i < endAddress; i += mult) {
			// Read the data in.
			ReadProcessMemory(m_targetProcess, reinterpret_cast<LPCVOID>(i), &data, mult, nullptr);
			for (DWORD a = 0UL; a < mult; a++) {
				hit = TRUE;

				// Search the data block for the signature.
				for (DWORD j = 0UL; j < (DWORD)signature.size() && hit; j++) {
					// Check if current byte is a wildcard.
					if (signature[j] != 0x00) {
						// Check for mis matching bytes.
						if (data[a + j] != signature[j]) {
							// Bytes are not equal.
							hit = FALSE;
						}
					}
				}
				if (hit) {
					// Signature found, set the result and exit function.
					m_resultAddress = i + a;
					m_sigFound = TRUE;
					return;
				}
			}
		}
	} while (!hit);

	// Signature not found.
	m_sigFound = FALSE;
}