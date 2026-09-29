Set WshShell = CreateObject("WScript.Shell")
Set fso = CreateObject("Scripting.FileSystemObject")
ScriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
DashboardPy = fso.BuildPath(ScriptDir, "dashboard.py")
WshShell.Run """C:\Python313\pythonw.exe"" """ & DashboardPy & """", 0, False
