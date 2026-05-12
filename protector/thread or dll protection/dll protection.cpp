#include "dll protection.hpp"

bool dllProtection::processDynamicCodePolicy() {
    PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY policy = {};
    policy.MicrosoftSignedOnly = 1;

    return SetProcessMitigationPolicy(
        ProcessSignaturePolicy,
        &policy,
        sizeof(policy)
    );
}