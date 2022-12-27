#include "process.h"


inline std::uint32_t process::get_id(const wchar_t* name)
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