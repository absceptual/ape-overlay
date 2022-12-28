#pragma once
#include <cassert>
#include <type_traits>
#include <memory>
#include <string>
#include <vector>

#include <Windows.h>
#include <TlHelp32.h>
#include <winnt.h> 
#include <winternl.h> 

#define UMDF_USING_NTSTATUS 
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

namespace process
{
	using offset_t = std::uintptr_t;
	inline HANDLE m_handle{ };
	inline std::uint32_t m_pid{ };
	inline uintptr_t m_base{ };

	bool attach(const wchar_t* name);
	std::uint32_t get_id(const wchar_t* name);
	uintptr_t get_module_address(const wchar_t* module);

	// Returns a buffer containing the requested type or a uintptr_t after traversing an offset chain
	template <typename type>
	auto read(uintptr_t address, size_t size = sizeof(std::remove_pointer<type>::type))
	{
		assert(m_handle && "Handle not set! (did you attach to process?)");

		using raw_type = std::remove_pointer<type>::type;
		raw_type buffer{ };
		ReadProcessMemory(m_handle, LPCVOID(address), &buffer, size, nullptr);

		return buffer;
	}

	template <typename type>
	auto write(uintptr_t address, typename std::remove_pointer<type>::type data, size_t size = sizeof(std::remove_pointer<type>::type))
	{
		assert(m_handle && "Handle not set! (did you attach to process?)");

		auto buffer = std::make_unique<char[]>(size);
		memcpy(buffer.get(), &data, size);

		return WriteProcessMemory(m_handle, LPVOID(address), buffer.get(), size, nullptr);
	}

	auto read_chain(uintptr_t address, std::vector<offset_t> offsets) -> uintptr_t;
	auto read_string(uintptr_t address, bool pointer, size_t size = MAX_PATH) -> std::string;
	auto read_wstring(uintptr_t address, bool pointer, size_t size = MAX_PATH * 2) -> std::wstring;
	auto read_ustring(uintptr_t address) -> UNICODE_STRING;	
}
