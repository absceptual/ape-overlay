#include "process.h"

std::uint32_t process::get_id(const wchar_t* name)
{
    using query_t = NTSTATUS(WINAPI*)(SYSTEM_INFORMATION_CLASS, void*, ULONG, ULONG*);
    auto query_information{ reinterpret_cast<query_t>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation")) };

    // Retrieve the size needed to query our processes, and then query the processes on the system
    std::uint32_t pid{ 0 };
    ULONG size{ sizeof(SYSTEM_PROCESS_INFORMATION) };
    SYSTEM_PROCESS_INFORMATION dummy{ };

    if (query_information(SystemProcessInformation, &dummy, sizeof(SYSTEM_PROCESS_INFORMATION), &size) != STATUS_INFO_LENGTH_MISMATCH)
        return pid;

    // Allocate the buffer needed to store our system process information structures
    auto buffer = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!buffer)
        return pid;

    auto status = query_information(SystemProcessInformation, buffer, size, nullptr);
    if (NT_ERROR(status))
        return pid;

    for (auto process = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(buffer);
        process->NextEntryOffset;
        process = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(uintptr_t(process) + process->NextEntryOffset))
    {
        auto string = process->ImageName;
        if (!string.Buffer)
            continue;

        if (!_wcsicmp(string.Buffer, name))
        {
            pid = std::uint32_t(process->UniqueProcessId);
            break;
        }
    }
    return pid;
}

bool process::attach(const wchar_t* name)
{
    m_pid = process::get_id(name);
    if (!m_pid)
        return false;

    m_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
    return (m_handle != INVALID_HANDLE_VALUE);
}

uintptr_t process::get_module_address(const wchar_t* base)
{
    assert(m_handle && "Handle was not set! (did you attach to process?)");
    assert(m_pid && "PID was not set! (did you attach to process?)");

    uintptr_t address = 0;
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pid);

    MODULEENTRY32 me32{  };
    me32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &me32))
    {
        do {
            if (!_wcsicmp(me32.szModule, base))
            {
                address = uintptr_t(me32.modBaseAddr);
                break;
            }
        } while (Module32Next(snapshot, &me32));
    }

    CloseHandle(snapshot);
    return address;
}

auto process::read_chain(uintptr_t address, std::vector<offset_t> offsets) -> uintptr_t
{
    uintptr_t pointer = address;
    for (const auto& offset : offsets)
    {
        pointer += offset;
        pointer = read<uintptr_t>(address);;
    }
    return pointer;
}

auto process::read_string(uintptr_t address, bool pointer, size_t size) -> std::string
{
    std::string string{ };
    string.reserve(size);

    if (pointer)
    {
        ReadProcessMemory(m_handle, LPCVOID(address), &address, sizeof(uintptr_t), nullptr);
        ReadProcessMemory(m_handle, LPCVOID(address), string.data(), size, nullptr);
    }
    else
        ReadProcessMemory(m_handle, LPCVOID(address), string.data(), size, nullptr);

    return string;
}

auto process::read_wstring(uintptr_t address, bool pointer, size_t size) -> std::wstring
{
    std::wstring string{ };
    string.reserve(size);

    if (pointer)
    {
        ReadProcessMemory(m_handle, LPCVOID(address), &address, sizeof(uintptr_t), nullptr);
        ReadProcessMemory(m_handle, LPCVOID(address), string.data(), size, nullptr);
    }
    else
        ReadProcessMemory(m_handle, LPCVOID(address), string.data(), size, nullptr);

    return string;
}

 auto process::read_ustring(uintptr_t address) -> UNICODE_STRING
{
    UNICODE_STRING string{ };

    ReadProcessMemory(m_handle, LPCVOID(address), &string, sizeof(UNICODE_STRING), nullptr);
    ReadProcessMemory(m_handle, LPCVOID(string.Buffer), string.Buffer, string.Length, nullptr);
    return string;
}
