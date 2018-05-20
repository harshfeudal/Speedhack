#pragma once

#include <Windows.h>
#include <detours.h>

namespace Speedhack
{
	float speed = 1.0f;

	typedef DWORD(WINAPI *_tGetTickCount)(void);
	_tGetTickCount _GTC;
	DWORD _GTC_BaseTime;

	typedef ULONGLONG(WINAPI *_tGetTickCount64)(void);
	_tGetTickCount64 _GTC64;
	DWORD _GTC64_BaseTime;

	typedef BOOL(WINAPI *_tQueryPerformanceCounter)(LARGE_INTEGER*);
	_tQueryPerformanceCounter _QPC;
	LARGE_INTEGER _QPC_BaseTime;

	typedef DWORD(WINAPI *_tSleepEx)(DWORD dwMilliseconds, BOOL bAlertable);
	_tSleepEx _SE;



	DWORD WINAPI _hGetTickCount()
	{
		return _GTC_BaseTime + ((_GTC() - _GTC_BaseTime) * speed);
	}

	DWORD WINAPI _hGetTickCount64()
	{
		return _GTC64_BaseTime + ((_GTC64() - _GTC64_BaseTime) * speed);
	}

	DWORD WINAPI _hQueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
	{
		LARGE_INTEGER x;
		_QPC(&x);
		lpPerformanceCount->QuadPart = _QPC_BaseTime.QuadPart + ((x.QuadPart - _QPC_BaseTime.QuadPart) * speed);
		return TRUE;
	}

	DWORD WINAPI _hSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
	{
		return _SE(dwMilliseconds / speed, bAlertable);
	}



	void Setup()
	{
		_GTC = (_tGetTickCount)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "GetTickCount");
		_GTC_BaseTime = _GTC();

		_GTC64 = (_tGetTickCount64)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "GetTickCount64");
		_GTC64_BaseTime = _GTC64();

		_QPC = (_tQueryPerformanceCounter)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "QueryPerformanceCounter");
		_QPC(&_QPC_BaseTime);

		_SE = (_tSleepEx)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SleepEx");

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)_GTC, _hGetTickCount);
		DetourAttach(&(PVOID&)_GTC64, _hGetTickCount64);
		DetourAttach(&(PVOID&)_QPC, _hQueryPerformanceCounter);
		DetourAttach(&(PVOID&)_SE, _hSleepEx);

		DetourTransactionCommit();
	}

	void Detach()
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID&)_GTC, _hGetTickCount);
		DetourDetach(&(PVOID&)_GTC64, _hGetTickCount64);
		DetourDetach(&(PVOID&)_QPC, _hQueryPerformanceCounter);
		DetourDetach(&(PVOID&)_SE, _hSleepEx);

		DetourTransactionCommit();
	}

	void SetSpeed(float relSpeed)
	{
		_GTC_BaseTime = _hGetTickCount();
		_GTC64_BaseTime = _hGetTickCount64();
		_hQueryPerformanceCounter(&_QPC_BaseTime);

		speed = relSpeed;
	}
}
