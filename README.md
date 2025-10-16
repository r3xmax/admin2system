# admin2system
This proof of concept demonstrates that an attacker can still steal the `NT AUTHORITY\SYSTEM` token and impersonate the system account in the current thread context without enabling the `SeDebugPrivilege`.

## Background Explanation
Members of the `BUILTIN\Administrators` group possess elevated privileges that allow them to manipulate the security tokens of running processes, including those operating under the `NT AUTHORITY\SYSTEM` account, the most powerful identity in the Windows operating system.<br><br>
The primary target process of this PoC is `winlogon.exe`. We'll inspect it with `System Informer` (or `Process Hacker`) and review the process token's DACL, focusing on permissions granted to `BUILTIN\Administrators`.  

<img width="633" height="466" alt="image" src="https://github.com/user-attachments/assets/9b1281ec-08e0-4aa9-8081-35aee638611f" />
<br><br>

As shown in the image above, the `BUILTIN\Administrators` group holds both `Duplicate` and `Query` permissions on the process token. These permissions are **essential** for access and impersonating the token within the context of the current thread.  

Reading this, you’re probably thinking, “Sure, but critical processes like winlogon.exe, lsass.exe, and others are protected by PPL.” That’s true, my friend. But here’s the best part: **there are plenty of other processes running as SYSTEM that aren’t protected**.  

Let's look at another process that is typically not protected by PPL on modern Windows 11: `smss.exe`.  

<img width="659" height="445" alt="image" src="https://github.com/user-attachments/assets/bc3eeec7-0723-4d51-acde-159f634ae531" />

## Token Access
Let's go in more detailed about what steps are needed to perform token impersonation:
1. First, open the target process using the WinAPI function `OpenProcess`. This step is required to obtain a valid handle to the running process. Since the desired access is specified as `PROCESS_QUERY_LIMITED_INFORMATION`, the `SeDebugPrivilege` is **NOT** required to steal the token.
2. Next comes the critical step: stealing the token using `OpenProcessToken`. It's essential to request the access rights `TOKEN_DUPLICATE | TOKEN_QUERY`. If the `TOKEN_DUPLICATE` right is missing, any attempt to impersonate the token will result in an error code **5: Access Denied**.
3. Impersonate the current thread in the context of the `NT AUTHORITY\SYSTEM` token using the WinAPI function `ImpersonateLoggedOnUser`.

One limitation of an impersonation token is that it cannot be used to spawn new processes. Its scope is restricted to the current thread only. However, executing C++ code within that thread under the context of `NT AUTHORITY\SYSTEM` remains extremely powerful.

To launch a new process as `NT AUTHORITY\SYSTEM` using `CreateProcessAsUser` or `CreateProcessWithTokenW`, the stolen token must be assigned as a **primary token**. This operation requires specific privileges: `SeDebugPrivilege`, `SeAssignPrimaryTokenPrivilege`, and `SeIncreaseQuotaPrivilege`. For more information, check [Access Rights for Access-Token Objects – Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/secauthz/access-rights-for-access-token-objects)

## PoC

<img width="627" height="159" alt="image" src="https://github.com/user-attachments/assets/d22d8a1d-575a-4731-bc67-76d6046831e0" />

## Disclaimer: Educational-only purpose
This PoC is provided solely for learning purposes and to share knowledge within the Red Team community. The code and techniques described herein must only be used in authorized audits and in controlled environments you own or are explicitly permitted to use.
I am not responsible for any misuse, damage, loss, or legal consequences resulting from unauthorized use of this PoC, its tools, or its code.
By using this code you agree to use it only for lawful and authorized purposes, and you accept full responsibility for any misuse, damage, or legal consequences that result from your use.
