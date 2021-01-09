/*
This is a stand alone bypass made by Apxaey. Feel free to use this in your cheats but credit me for the bypass as i put alot of time into this.
If you have some brain cells you will be able to incorporate this into your cheats and remain undetected by user-mode anticheats.
Obviously standard cheat 'recommendations' still apply:
1.) Use self-written or not signatured code
2.) Dont write impossible values
3.) If your going internal use a manual map injector
If you follow the guidelines above and use this bypass you will be safe from usermode anticheats like VAC.
*/
#define VK_ESCAPE	0x1B 

#include <Windows.h> 
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include "main.h"
#include <conio.h>
#include <cstdlib>
#include <stdlib.h>
#include <thread>

using namespace std;

// Bases [Gameversion: 1.7.1.8408002 Only works as HOST/Lobby-Leader
int PlayerBase = 0xFD06548;
int ZMXPScaleBase = 0xFD2E540; // Offsets to Multipliers are written below.
int TimeScaleBase = 0xEE39BF8 + 0x7C;
int CMDBufferBase = 0x116D1548 + 0x50418; // there are much more stuff at this Base ;-)! The Offset was stable for about 3 patches so this reagion is a BIG one with many things have fun!

// Cache Addresses from PlayerBase (more at yourthread-code)
int PlayerCompPtr, PlayerPedPtr, ZMGlobalBase, ZMBotBase, ZMBotListBase; // Used to Cache the Pointers at the PlayerBase by his Offsets (more infos at the yourthread() script)

// Offsets
// PlayerCompPtr - Offsets
int PC_ArraySize_Offset = 0xB830; // Size of Array between Players Data [IDK if it got changed with 1.7.1 if it got changed, please post the new Offset, i will update them here. dont have the time to check it self]
int PC_CurrentUsedWeaponID = 0x28; // Shows Current Used WeaponID (this are only Readable IDs, so change is not working with them on me)
int PC_SetWeaponID = 0xB0; // +(1-5 * 0x40 for WP2 to WP6) Can be used to change a WeaponID correctly from ID 1-300 (! info !, some IDs can result in GameCrashes!).
int PC_InfraredVision = 0xE66; // (byte) On=0x10|Off=0x0
int PC_GodMode = 0xE67; // (byte) On=0xA0|Off=0x20
int PC_RapidFire1 = 0xE6C; // Freeze to 0 how long you press Left Mouse-Key or Reloading and other stuff is not working.
int PC_RapidFire2 = 0xE80; // Freeze to 0 how long you press Left Mouse-Key or Reloading and other stuff is not working.
int PC_MaxAmmo = 0x1360; // +(1-5 * 0x8 for WP1 to WP6) (WP0 Mostly used in MP, ZM first WP is WP1 | WP3-6 Mostly used for Granades and Special) The Game assign the next Free WP Slot so WP1 is MainWeapon, you get a granade, then WP2 is the Granade, you buy a Weapon from wall then this is WP3 and so on..
int PC_Ammo = 0x13D4; // +(1-5 * 0x4 for WP1 to WP6) (WP0 Mostly used in MP, ZM first WP is WP1 | WP3-6 Mostly used for Granades and Special)
int PC_Points = 0x5CE4; // ZM Points / Money
int PC_Name = 0x5BDA; // Playername
int PC_RunSpeed = 0x5C30; // (float)
int PC_ClanTags = 0x605C; // Player Clan/Crew-Tag

// PlayerPedPtr - Offsets
int PP_ArraySize_Offset = 0x5F8; // ArraySize to next Player.
int PP_Health = 0x398;
int PP_MaxHealth = 0x39C; // Max Health dont increase by using Perk Juggernog
int PP_Coords = 0x2D4; // Vector3
int PP_Heading_Z = 0x34; // float
int PP_Heading_XY = 0x38; // float | can be used to TP Zombies in front of you by your Heading Position and Forward Distance.

// ZMGlobalBase - Offsets
// The Move Offset got removed with Patch 1.6.0 so Move Offset is no longer needed! Use 0x0 if you have add this Offset to your code... OLD: public int ZM_Global_MovedOffset = 0x2F20; // Since 1.5.0 The data got moved by this Offset so ZM_Global_MovedOffset + ZM_Global_ZombiesIgnoreAll is the corretly Offset to ZombiesIgnoreAll
int ZM_Global_ZombiesIgnoreAll = 0x14; // Zombies Ignore any Player in the Lobby.
int ZM_Global_ZMLeftCount = 0x3C; // Zombies Left

// ZMBotBase - Offsets
int ZM_Bot_List_Offset = 0x8; // Offset to Pointer at ZMBotBase + 0x8 -> ZMBotListBase

// ZMBotListBase - Offsets
int ZM_Bot_ArraySize_Offset = 0x5F8; // ArraySize to next Zombie.
int ZM_Bot_Health = 0x398;
int ZM_Bot_MaxHealth = 0x39C;
int ZM_Bot_Coords = 0x2D4; // Cam be used to Teleport all Zombies in front of any Player with a Heading Variable from the Players.

// ZMXPScaleBase - Offsets (All Offsets add to Base Address == ZMXPScaleBase + XPEP_Offset as example)
int XPEP_Offset = 0x20; // Needs a higher Value like 1000.00f
int XPUNK01_Offset = 0x24; // K/A Modifier
int XPEP_RealAdd_Offset = 0x28; // K/A Modifier
int XPUNK03_Offset = 0x2c; // K/A Modifier
int XPGun_Offset = 0x30; // works like it is, 1.00f == Normal, 2.00f == x2 etc...
int XPUNK04_Offset = 0x34; // K/A Modifier
int XPUNK05_Offset = 0x38; // K/A Modifier
int XPUNK06_Offset = 0x3c; // K/A Modifier
int XPUNK07_Offset = 0x40; // currently it is 0.00f idk what id do.
int XPUNK08_Offset = 0x44; // K/A Modifier
int XPUNK09_Offset = 0x48; // K/A Modifier
int XPUNK10_Offset = 0x4C; // K/A Modifier

// CMDBufferBase - Offsets
int CMDBB_Exec = -0x1B;

// Godmode bytes
int EnabledGod = 0xA0;
int DisabledGod = 0x20;

//simple function i made that will just initialize our Object_Attributes structure as NtOpenProcess will fail otherwise
OBJECT_ATTRIBUTES InitObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security)
{
	OBJECT_ATTRIBUTES object;

	object.Length = sizeof(OBJECT_ATTRIBUTES);
	object.ObjectName = name;
	object.Attributes = attributes;
	object.RootDirectory = hRoot;
	object.SecurityDescriptor = security;

	return object;
}

SYSTEM_HANDLE_INFORMATION* hInfo; //holds the handle information

//the handles we will need to use later on
HANDLE procHandle = NULL;
HANDLE hProcess = NULL;
HANDLE HijackedHandle = NULL;

//basic function i made that will get a proccess id from a binary name, you dont have to use it. It needs some rework but for now it gets the job done.
DWORD GetPID(LPCSTR procName)
{
	//create a process snapshot
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, false);
	if (hSnap && hSnap != INVALID_HANDLE_VALUE) //check the snapshot succeded
	{
		PROCESSENTRY32 procEntry;

		//zero the memory containing the file names
		ZeroMemory(procEntry.szExeFile, sizeof(procEntry.szExeFile));

		//repeat the loop until a name matches the desired name
		do
		{
			if (lstrcmpi(procEntry.szExeFile, procName) == NULL) {
				return procEntry.th32ProcessID;
				CloseHandle(hSnap);
			}
		} while (Process32Next(hSnap, &procEntry));


	}


}
bool IsHandleValid(HANDLE handle) //i made this to simply check if a handle is valid rather than repeating the if statments
{
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void CleanUpAndExit(LPSTR ErrorMessage) //just a function to clean up and exit. 
{

	delete[] hInfo;

	procHandle ? CloseHandle(procHandle) : 0;

	std::cout << ErrorMessage << std::endl;

	system("pause");

}
HANDLE HijackExistingHandle(DWORD dwTargetProcessId)
{
	HMODULE Ntdll = GetModuleHandleA("ntdll"); // get the base address of ntdll.dll

   //get the address of RtlAdjustPrivilege in ntdll.dll so we can grant our process the highest permission possible
	_RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(Ntdll, "RtlAdjustPrivilege");

	boolean OldPriv; //store the old privileges

	// Give our program SeDeugPrivileges whcih allows us to get a handle to every process, even the highest privileged SYSTEM level processes.
	RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &OldPriv);

	//get the address of NtQuerySystemInformation in ntdll.dll so we can find all the open handles on our system
	_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(Ntdll, "NtQuerySystemInformation");

	//get the address of NtDuplicateObject in ntdll.dll so we can duplicate an existing handle into our cheat, basically performing the hijacking
	_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(Ntdll, "NtDuplicateObject");

	//get the address of NtOpenProcess in ntdll.dll so wecan create a Duplicate handle
	_NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetProcAddress(Ntdll, "NtOpenProcess");


	//initialize the Object Attributes structure, you can just set each member to NULL rather than create a function like i did
	OBJECT_ATTRIBUTES Obj_Attribute = InitObjectAttributes(NULL, NULL, NULL, NULL);

	//clientID is a PDWORD or DWORD* of the process id to create a handle to
	CLIENT_ID clientID = { 0 };


	//the size variable is the amount of bytes allocated to store all the open handles
	DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);

	//we allocate the memory to store all the handles on the heap rather than the stack becuase of the large amount of data
	hInfo = (SYSTEM_HANDLE_INFORMATION*) new byte[size];

	//zero the memory handle info
	ZeroMemory(hInfo, size);

	//we use this for checking if the Native functions succeed
	NTSTATUS NtRet = NULL;

	do
	{
		// delete the previously allocated memory on the heap because it wasn't large enough to store all the handles
		delete[] hInfo;

		//increase the amount of memory allocated by 50%
		size *= 1.5;
		try
		{
			//set and allocate the larger size on the heap
			hInfo = (PSYSTEM_HANDLE_INFORMATION) new byte[size];



		}
		catch (std::bad_alloc) //catch a bad heap allocation.
		{


			std::cout << "Bad Heap Allocation" << std::endl;
			exit(-1);

		}
		Sleep(1); //sleep for the cpu

		//we continue this loop until all the handles have been stored
	} while ((NtRet = NtQuerySystemInformation(SystemHandleInformation, hInfo, size, NULL)) == STATUS_INFO_LENGTH_MISMATCH);

	//check if we got all the open handles on our system
	if (!NT_SUCCESS(NtRet))
	{

		std::cout << "NtQuerySystemInformation failed" << std::endl;
		exit(-1);
	}


	//loop through each handle on our system, and filter out handles that are invalid or cant be hijacked
	for (unsigned int i = 0; i < hInfo->HandleCount; ++i)
	{
		//a variable to store the number of handles OUR cheat has open.
		static DWORD NumOfOpenHandles;

		//get the amount of outgoing handles OUR cheat has open
		GetProcessHandleCount(GetCurrentProcess(), &NumOfOpenHandles);

		//you can do a higher number if this is triggering false positives. Its just to make sure we dont fuck up and create thousands of handles
		if (NumOfOpenHandles > 50)
		{

			std::cout << "Error handle leakage detected" << std::endl;
			exit(-1);
		}

		//check if the current handle is valid, otherwise increment i and check the next handle
		if (!IsHandleValid((HANDLE)hInfo->Handles[i].Handle))
		{
			continue;
		}

		//check the handle type is 0x7 meaning a process handle so we dont hijack a file handle for example
		if (hInfo->Handles[i].ObjectTypeNumber != ProcessHandleType)
		{
			continue;
		}


		//set clientID to a pointer to the process with the handle to out target
		clientID.UniqueProcess = (DWORD*)hInfo->Handles[i].ProcessId;

		//if procHandle is open, close it
		procHandle ? CloseHandle(procHandle) : 0;

		//create a a handle with duplicate only permissions to the process with a handle to our target. NOT OUR TARGET.
		NtRet = NtOpenProcess(&procHandle, PROCESS_DUP_HANDLE, &Obj_Attribute, &clientID);
		if (!IsHandleValid(procHandle) || !NT_SUCCESS(NtRet)) //check is the funcions succeeded and check the handle is valid
		{
			continue;
		}


		//we duplicate the handle another process has to our target into our cheat with whatever permissions we want. I did all access.
		NtRet = NtDuplicateObject(procHandle, (HANDLE)hInfo->Handles[i].Handle, NtCurrentProcess, &HijackedHandle, PROCESS_ALL_ACCESS, 0, 0);
		if (!IsHandleValid(HijackedHandle) || !NT_SUCCESS(NtRet))//check is the funcions succeeded and check the handle is valid
		{

			continue;
		}

		//get the process id of the handle we duplicated and check its to our target
		if (GetProcessId(HijackedHandle) != dwTargetProcessId) {
			CloseHandle(HijackedHandle);
			continue;
		}

		hProcess = HijackedHandle;

		break;
	}

	return hProcess;

}
uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_stricmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

void active_inactive(bool test) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!test) {
		SetConsoleTextAttribute(hConsole, 12);
		std::cout << "INACTIVE";
	}
	else {
		SetConsoleTextAttribute(hConsole, 10);
		std::cout << "ACTIVE";

	}
}
void printLogo() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 15);
	cout << "_________________________________________________" << endl;
	SetConsoleTextAttribute(hConsole, 13);
	cout << "  ____   _                 ______ _   _" << endl;
	cout << " |  _ \\ | |         /\\    |___  /| \\ | |" << endl;
	cout << " | |_) || |        /  \\      / / |  \\| |" << endl;
	cout << " |  _ < | |       / /\\ \\    / /  | . ` |" << endl;
	cout << " | |_) || |____  / ____ \\  / /__ | |\\  |" << endl;
	cout << " |____/ |______|/_/    \\_\\/_____||_| \\_| Software" << endl;
	SetConsoleTextAttribute(hConsole, 14);
	cout << " Call of Duty: Black Ops Cold War Trainer" << endl;
	SetConsoleTextAttribute(hConsole, 15);
	cout << "_________________________________________________" << endl;
	cout << " " << endl;
}

void cheatMenu(string cheat, string key, bool active)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 3);
	std::cout << "\r      [+] ";
	SetConsoleTextAttribute(hConsole, 15);
	std::cout << cheat << " (" << key << ") >> ";
	active_inactive(active);
	std::cout << endl;
	std::cout << endl;
}

////// CHEATS //////

// HANDLE pHandle, DWORD process_id, DWORD base_address

//Booleans for cheats
BOOL godmode = FALSE;
BOOL infinite_ammo = FALSE;
BOOL unlimited_points = FALSE;
BOOL rapid_fire = FALSE;
BOOL teleport_zombies = FALSE;
BOOL insta_kill = FALSE;
BOOL superxp = FALSE;
BOOL leave = FALSE;

void runThread(HANDLE pHandle, DWORD base_address) //Start thread
{

	int value;

	if (PlayerCompPtr != ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase, &PlayerCompPtr, sizeof(PlayerCompPtr), 0))
	{
		cout << "PlayerCompPtr updated." << endl;
		PlayerCompPtr = ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase, &PlayerCompPtr, sizeof(PlayerCompPtr), 0);
	}
	if (PlayerPedPtr != ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x8, &PlayerPedPtr, sizeof(PlayerPedPtr), 0))
	{
		cout << "PlayerPedPtr updated." << endl;
		PlayerPedPtr = ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x8, &PlayerPedPtr, sizeof(PlayerPedPtr), 0);
	}
	if (ZMGlobalBase != ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x60, &ZMGlobalBase, sizeof(ZMGlobalBase), 0))
	{
		cout << "ZMGlobalBase updated." << endl;
		ZMGlobalBase = ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x60, &ZMGlobalBase, sizeof(ZMGlobalBase), 0);
	}
	if (ZMBotBase != ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x68, &ZMBotBase, sizeof(ZMBotBase), 0))
	{
		cout << "ZMBotBase updated." << endl;
		ZMBotBase = ReadProcessMemory(pHandle, (UINT64*)base_address + PlayerBase + 0x68, &ZMBotBase, sizeof(ZMBotBase), 0);
	}
	if (ZMBotBase != 0x0 || ZMBotBase != 0x68 && ZMBotListBase != ReadProcessMemory(pHandle, (UINT64*)base_address + ZMBotBase + ZM_Bot_List_Offset, &ZMBotBase, sizeof(ZMBotBase), 0))
	{
		cout << "ZMBotBase updated." << endl;
		ZMBotListBase = ReadProcessMemory(pHandle, (UINT64*)base_address + ZMBotBase + ZM_Bot_List_Offset, &ZMBotBase, sizeof(ZMBotBase), 0);
	}
}

void instaKill(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (insta_kill == TRUE) {
			cout << "\nSUCCESS" << endl;
			for (int i = 80; i < 300; i++)
			{
				int oneul = 1;
				long num = PlayerPedPtr;
				int num2 = ReadProcessMemory(pHandle, (int*)num + ZM_Bot_Health + ZM_Bot_List_Offset, &num2, sizeof(num2), 0);
				if (ReadProcessMemory(pHandle, (int*)num + ZM_Bot_Health, &num2, sizeof(num2), 0) != 0 && (num2 == 40110400 || num2 == 110400))
				{
					WriteProcessMemory(pHandle, (int*)num + ZM_Bot_Health, &oneul, sizeof(oneul), 0);
				}
			}
		}
	}
}

void maxAmmo(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (infinite_ammo == TRUE)
		{

			cout << "\nSUCCESS" << endl;
			int count = ReadProcessMemory(pHandle, (int*)PlayerPedPtr + PP_ArraySize_Offset, &count, sizeof(count), 0);
			for (int i = 0; i < count; i++)
			{
				long num = PlayerPedPtr;
				int max = 250;
				int max2 = 100;
				WriteProcessMemory(pHandle, (int*)num + PC_MaxAmmo, &max, sizeof(max), 0);
				WriteProcessMemory(pHandle, (int*)num + PC_MaxAmmo + 0x8, &max, sizeof(max), 0);
				WriteProcessMemory(pHandle, (int*)num + PC_MaxAmmo, &max2, sizeof(max2), 0);
				WriteProcessMemory(pHandle, (int*)num + PC_MaxAmmo + 0x4, &max2, sizeof(max2), 0);
			}
		}
	}
}

void enableGodMode(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (godmode == TRUE)
		{
			cout << "\nSUCCESS" << endl;
			int count = ReadProcessMemory(pHandle, (int*)PlayerPedPtr + PP_ArraySize_Offset, &count, sizeof(count), 0);
			for (int i = 0; i < count; i++)
			{
				int god = 160;
				WriteProcessMemory(pHandle, (LPVOID*)PlayerCompPtr + PC_GodMode + 0xA0, &god, sizeof(god), 0);
			}
		}
	}
}

void rapidFire(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (rapid_fire == TRUE)
		{
			cout << "\nSUCCESS" << endl;
			int count = ReadProcessMemory(pHandle, (int*)PlayerPedPtr + PP_ArraySize_Offset, &count, sizeof(count), 0);
			for (int i = 0; i < count; i++)
			{
				long num = PlayerCompPtr;
				WriteProcessMemory(pHandle, (LPVOID*)PlayerCompPtr + PC_RapidFire1, NULL, NULL, 0);
				WriteProcessMemory(pHandle, (LPVOID*)PlayerCompPtr + PC_RapidFire2, NULL, NULL, 0);
			}
		}
	}
}

void unlimitedPoints(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (unlimited_points == TRUE)
		{
			cout << "\nSUCCESS" << endl;
			int count = ReadProcessMemory(pHandle, (int*)PlayerPedPtr + PP_ArraySize_Offset, &count, sizeof(count), 0);
			for (int i = 0; i < count; i++)
			{
				long num = PlayerCompPtr;
				int points = 50000;
				WriteProcessMemory(pHandle, (int*)num + PC_Points, &points, sizeof(points), 0);
			}
		}
	}
}

void superXP(HANDLE pHandle, DWORD process_id, DWORD base_address)
{
	if (process_id && base_address)
	{
		cout << "Writing memory";
		for (int i = 0; i < 3; i++)
		{
			Sleep(34);
			cout << ".";
			Sleep(152);
		}
		if (superxp == TRUE)
		{
			cout << "\nSUCCESS" << endl;
			int xp = 10000.00f;
			int gunxp = 5000.00f;
			WriteProcessMemory(pHandle, (int*)PlayerCompPtr + ZMXPScaleBase + XPEP_Offset, &xp, sizeof(xp), 0);
			WriteProcessMemory(pHandle, (int*)PlayerCompPtr + ZMXPScaleBase + XPEP_Offset, &xp, sizeof(xp), 0);
		}
	}
}

int zombie(int n)
{
	return n * 0x5F8;
}

void ShowConsoleCursor(bool showFlag)
{

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void refresh()
{
	system("CLS");
	printLogo();
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, 15);
	cout << "\rProcess: ";
	SetConsoleTextAttribute(hConsole, 10);
	cout << "BlackOpsColdWar.exe" << endl;
	SetConsoleTextAttribute(hConsole, 4);
	cout << "Press (NUM7) to disable all cheats." << endl;
	cout << endl;

	cheatMenu("GOD MODE", "NUM1", godmode);
	cheatMenu("INFINITE AMMO", "NUM2", infinite_ammo);
	cheatMenu("UNLIMITED POINTS", "NUM3", unlimited_points);
	cheatMenu("RAPID FIRE", "NUM4", rapid_fire);
	cheatMenu("INSTA KILL", "NUM5", insta_kill);
	cheatMenu("SUPER XP", "NUM6", superxp);
	SetConsoleTextAttribute(hConsole, 3);
	std::cout << "\r           [+] ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << "EXIT (ESCAPE)";
	std::cout << endl;
	std::cout << endl;
}

int main()
{

	SetConsoleTitle("BLAZN CW TRAINER");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	system("CLS");
	ShowConsoleCursor(false);
	printLogo();
	Sleep(1000);
	SetConsoleTextAttribute(hConsole, 4);
	cout << "[+] Please make sure the game is open before opening this software!\r";
	Sleep(2000);
	cout << "                                                                                 \r";

	for (int i = 0; i < 1; i++)
	{
		SetConsoleTextAttribute(hConsole, 7);
		cout << "\rWaiting for game   \rWaiting for game";
		for (int i = 0; i < 3; i++)
		{
			Sleep(200);
			cout << ".";
			Sleep(250);
		}
	}

	DWORD process_id = GetPID("BlackOpsColdWar.exe");
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
//	HANDLE pHandle = HijackExistingHandle(process_id);
	DWORD base_address = GetModuleBaseAddress(process_id, "BlackOpsColdWar.exe");

	runThread(pHandle, base_address);
	Sleep(1000);
	if (base_address != NULL)
	{
		Sleep(246);
		SetConsoleTextAttribute(hConsole, 10);
		Sleep(467);
		cout << "\r[FOUND GAME]          ";

		if (process_id == NULL)
		{
			SetConsoleTextAttribute(hConsole, 4);
			cout << "Cannot obtain process";
			Sleep(2314);
			exit(-1);
		}
	}
	else {
		Sleep(500);
		SetConsoleTextAttribute(hConsole, 4);
		Sleep(300);
		cout << "\r[GAME NOT FOUND]          ";
		SetConsoleTextAttribute(hConsole, 15);
		cout << "\nNow closing loader";
		for (int i = 0; i < 3; i++)
		{
			Sleep(500);
			cout << ".";
			Sleep(500);
		}
		exit(-1);
	}

	SetConsoleTextAttribute(hConsole, 10);
	cout << "\nSUCCESS" << endl;
	Sleep(1096);
	SetConsoleTextAttribute(hConsole, 15);
	cout << "\nLoading cheat: ";
	for (int i = 0; i < 101; i++)
	{
		cout << "\rLoading cheat: " << i << "%";
		Sleep(10);
		cout << "\rLoading cheat: " << i << "%";
		Sleep(10);
	}

	SetConsoleTextAttribute(hConsole, 10);
	cout << "\nCHEAT SUCCESSFULLY LOADED" << endl;
	cout << endl;
	Sleep(500);
	SetConsoleTextAttribute(hConsole, 11);
	Sleep(1000);

	char keystroke;

	int unlimitedpoints = 35000;

	//UI Design (Loading cheat 1 by 1).
	system("CLS");
	printLogo();
	SetConsoleTextAttribute(hConsole, 15);
	cout << "\rProcess: ";
	SetConsoleTextAttribute(hConsole, 10);
	cout << "BlackOpsColdWar.exe" << endl;
	SetConsoleTextAttribute(hConsole, 4);
	cout << "Press (NUM7) to disable all cheats." << endl;
	cout << endl;

	cheatMenu("GOD MODE", "NUM1", godmode);
	Sleep(100);
	cheatMenu("INFINITE AMMO", "NUM2", infinite_ammo);
	Sleep(100);
	cheatMenu("UNLIMITED POINTS", "NUM3", unlimited_points);
	Sleep(100);
	cheatMenu("RAPID FIRE", "NUM4", rapid_fire);
	Sleep(100);
	cheatMenu("INSTA KILL", "NUM5", insta_kill);
	Sleep(100);
	cheatMenu("SUPER XP", "NUM6", superxp);
	Sleep(100);
	SetConsoleTextAttribute(hConsole, 3);
	std::cout << "\r           [+] ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << "EXIT (ESCAPE)";
	std::cout << endl;

	while (base_address && process_id)
	{
		if (GetAsyncKeyState(VK_NUMPAD1) && godmode == FALSE)
		{
			godmode = TRUE;
			refresh();
			enableGodMode(pHandle, process_id, base_address);
//			thread t(enableGodMode, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD2))
		{
			infinite_ammo = TRUE;
			refresh();
			maxAmmo(pHandle, process_id, base_address);
//			thread t(maxAmmo, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD3))
		{
			unlimited_points = TRUE;
			refresh();
			unlimitedPoints(pHandle, process_id, base_address);
//			thread t(unlimitedPoints, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD4))
		{
			rapid_fire = TRUE;
			refresh();
			rapidFire(pHandle, process_id, base_address);
//			thread t(rapidFire, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD5))
		{
			insta_kill = TRUE;
			refresh();
			instaKill(pHandle, process_id, base_address);
//			thread t(instaKill, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD6))
		{
			superxp = TRUE;
			refresh();
			superXP(pHandle, process_id, base_address);
//			thread t(superXP, pHandle, process_id, base_address);
//			t.join();
		}
		if (GetAsyncKeyState(VK_NUMPAD7))
		{
			godmode = FALSE;
			unlimited_points = FALSE;
			infinite_ammo = FALSE;
			rapid_fire = FALSE;
			insta_kill = FALSE;
			superxp = FALSE;
			refresh();
		}
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			leave = TRUE;
		}
		if (leave == TRUE)
		{
			system("CLS");
			printLogo();
			SetConsoleTextAttribute(hConsole, 15);
			cout << "Application now closing";
			for (int i = 0; i < 3; i++)
			{
				Sleep(500);
				cout << ".";
				Sleep(250);
			}
			exit(-1);
		}
	}
	return NULL;
}


