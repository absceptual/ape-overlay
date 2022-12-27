#pragma once
#include <type_traits>
#include <string>

#include <Windows.h>
#include <winnt.h> 
#include <winternl.h> 

#define UMDF_USING_NTSTATUS 
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

namespace process
{
	inline HANDLE m_handle{ };
	
	std::uint32_t get_id(const wchar_t* name);

	template <typename type>
	auto read(uintptr_t address, size_t size = sizeof(std::remove_pointer<type>))
	{
		using raw_type = std::remove_pointer<type>;
	}

	template <const char*>
	auto read(uintptr_t address, size_t size = MAX_PATH) -> std::string
	{

	}

	template <const wchar_t*>
	auto read(uintptr_t address, size_t size = MAX_PATH) -> std::wstring
	{

	}

	template <UNICODE_STRING>
	auto read(uintptr_t address) -> UNICODE_STRING
	{

	}

	template <typename type>
	auto write(uintptr_t address, size_t size)
	{

	}
}
