Import-Module NtObjectManager

Set-NtTokenPrivilege -Privilege SeDebugPrivilege

$proc = Get-NtProcess -Name winlogon.exe
$token = Get-NtToken -Process $proc -TokenType Primary

$primaryToken = $token.DuplicateToken(
    [NtCoreLib.TokenType]::Primary,
    [NtCoreLib.Security.Token.SecurityImpersonationLevel]::Impersonation,
    [NtCoreLib.TokenAccessRights]::MaximumAllowed
)

$config = New-Win32ProcessConfig -ApplicationName "C:\Windows\System32\cmd.exe" -CommandLine NULL -CreationFlags NewConsole -Token $primaryToken
$config.Create()