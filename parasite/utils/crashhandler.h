#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#pragma comment(lib, "dbghelp.lib")

namespace CrashHandler {

    static const char* ExceptionName(DWORD code) {
        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:         return "ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:               return "BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:    return "DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_OVERFLOW:             return "FLT_OVERFLOW";
        case EXCEPTION_FLT_UNDERFLOW:            return "FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:      return "ILLEGAL_INSTRUCTION";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:             return "INT_OVERFLOW";
        case EXCEPTION_PRIV_INSTRUCTION:         return "PRIV_INSTRUCTION";
        case EXCEPTION_STACK_OVERFLOW:           return "STACK_OVERFLOW";
        default:                                 return "UNKNOWN";
        }
    }

    static LONG WINAPI Handler(EXCEPTION_POINTERS* ep) {
        EXCEPTION_RECORD* er = ep->ExceptionRecord;
        CONTEXT* ctx = ep->ContextRecord;

        HMODULE hModule = NULL;
        GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCTSTR)er->ExceptionAddress, &hModule);

        uintptr_t base = (uintptr_t)hModule;
        uintptr_t rva = (uintptr_t)er->ExceptionAddress - base;

#pragma warning(suppress: 4996)
        FILE* f = fopen("crash.log", "w");
        if (!f) f = stderr;

        fprintf(f, "=== CRASH ===\n");
        fprintf(f, "Exception : %s (0x%08X)\n", ExceptionName(er->ExceptionCode), er->ExceptionCode);
        fprintf(f, "Address   : 0x%p\n", er->ExceptionAddress);
        fprintf(f, "RVA       : 0x%llX\n", (unsigned long long)rva);
        fprintf(f, "Module    : 0x%p\n\n", (void*)hModule);

#ifdef _WIN64
        fprintf(f, "=== REGISTERS ===\n");
        fprintf(f, "RAX = %016llX  RBX = %016llX\n", ctx->Rax, ctx->Rbx);
        fprintf(f, "RCX = %016llX  RDX = %016llX\n", ctx->Rcx, ctx->Rdx);
        fprintf(f, "RSI = %016llX  RDI = %016llX\n", ctx->Rsi, ctx->Rdi);
        fprintf(f, "RSP = %016llX  RBP = %016llX\n", ctx->Rsp, ctx->Rbp);
        fprintf(f, "R8  = %016llX  R9  = %016llX\n", ctx->R8, ctx->R9);
        fprintf(f, "R10 = %016llX  R11 = %016llX\n", ctx->R10, ctx->R11);
        fprintf(f, "R12 = %016llX  R13 = %016llX\n", ctx->R12, ctx->R13);
        fprintf(f, "R14 = %016llX  R15 = %016llX\n", ctx->R14, ctx->R15);
        fprintf(f, "RIP = %016llX\n", ctx->Rip);
        fprintf(f, "EFL = %016llX\n\n", (unsigned long long)ctx->EFlags);
#else
        fprintf(f, "=== REGISTERS ===\n");
        fprintf(f, "EAX = %08X  EBX = %08X\n", ctx->Eax, ctx->Ebx);
        fprintf(f, "ECX = %08X  EDX = %08X\n", ctx->Ecx, ctx->Edx);
        fprintf(f, "ESI = %08X  EDI = %08X\n", ctx->Esi, ctx->Edi);
        fprintf(f, "ESP = %08X  EBP = %08X\n", ctx->Esp, ctx->Ebp);
        fprintf(f, "EIP = %08X\n", ctx->Eip);
        fprintf(f, "EFL = %08X\n\n", ctx->EFlags);
#endif

        fprintf(f, "=== STACK TRACE ===\n");
        SymInitialize(GetCurrentProcess(), NULL, TRUE);

        STACKFRAME64 sf = {};
        DWORD machineType;

#ifdef _WIN64
        machineType = IMAGE_FILE_MACHINE_AMD64;
        sf.AddrPC.Offset = ctx->Rip;
        sf.AddrFrame.Offset = ctx->Rbp;
        sf.AddrStack.Offset = ctx->Rsp;
#else
        machineType = IMAGE_FILE_MACHINE_I386;
        sf.AddrPC.Offset = ctx->Eip;
        sf.AddrFrame.Offset = ctx->Ebp;
        sf.AddrStack.Offset = ctx->Esp;
#endif
        sf.AddrPC.Mode = AddrModeFlat;
        sf.AddrFrame.Mode = AddrModeFlat;
        sf.AddrStack.Mode = AddrModeFlat;

        char symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
        SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = MAX_SYM_NAME;

        CONTEXT ctxCopy = *ctx;
        for (int i = 0; i < 32; i++) {
            if (!StackWalk64(machineType, GetCurrentProcess(), GetCurrentThread(),
                &sf, &ctxCopy, NULL, SymFunctionTableAccess64,
                SymGetModuleBase64, NULL))
                break;
            if (sf.AddrPC.Offset == 0) break;

            HMODULE hFrameMod = NULL;
            GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCTSTR)sf.AddrPC.Offset, &hFrameMod);

            char modName[MAX_PATH] = "<unknown>";
            if (hFrameMod)
                GetModuleFileNameA(hFrameMod, modName, MAX_PATH);

            char* modBaseName = strrchr(modName, '\\');
            if (modBaseName) modBaseName++; else modBaseName = modName;

            uintptr_t frameRVA = (uintptr_t)sf.AddrPC.Offset - (uintptr_t)hFrameMod;

            DWORD64 disp = 0;
            if (SymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &disp, sym))
                fprintf(f, "  [%2d] %s!%s+0x%llX  RVA:0x%llX\n", i, modBaseName, sym->Name, (unsigned long long)disp, (unsigned long long)frameRVA);
            else
                fprintf(f, "  [%2d] %s!RVA:0x%llX\n", i, modBaseName, (unsigned long long)frameRVA);
        }

        if (f != stderr) fclose(f);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    inline void Enable() {
        SetUnhandledExceptionFilter(Handler);
    }

}