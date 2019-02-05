#pragma once

namespace OsuBot
{
	namespace SigScan
	{
		struct MODULE {
			DWORD dwBase;
			DWORD dwSize;
		};

		class SigScanner {
		public:
			// Member functions.
			bool GetProcess(_In_ std::wstring processName);
			void GetRegion(_In_opt_ const UINT& startAddress = NULL);
			
			// Signature functions.
			void FindSignature(
				_In_ const std::wstring* signature
			);

			// Accessor functions.
		public:
			HANDLE GetTargetProcessHandle() const { return m_targetProcess; }
			DWORD GetResultAddress() const { return m_resultAddress; }
			DWORD GetTargetProcessID() const { return m_targetID; }
			bool SigFound() const { return m_sigFound; }


			// Member variables.
		private:
			MODULE m_targetRegion;
			HANDLE m_targetProcess;
			DWORD m_targetID;
			DWORD m_resultAddress;
			bool m_sigFound;
		};
	}
}