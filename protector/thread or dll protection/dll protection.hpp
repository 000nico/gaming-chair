#include <windows.h>

namespace dllProtection{
    bool processDynamicCodePolicy();
}

typedef struct _PROCESS_MITIGATION_POLICY_INFORMATION {
    PROCESS_MITIGATION_POLICY Policy;
    union {
        PROCESS_MITIGATION_ASLR_POLICY ASLRPolicy;
        PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY StrictHandleCheckPolicy;
        PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY SystemCallDisablePolicy;
        PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY ExtensionPointDisablePolicy;
        PROCESS_MITIGATION_DYNAMIC_CODE_POLICY DynamicCodePolicy;
        // ...
    };
} PROCESS_MITIGATION_POLICY_INFORMATION;