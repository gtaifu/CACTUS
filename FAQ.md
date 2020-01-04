# Frequently Asked Questions

Here are the common reported problems and their status.

#### On windows, the released executable file in demo/bin directory cannot execute in PowerShell terminal.

The released executable file in demo/bin is created with x64 Windows platform, it relies on some Microsoft C Runtime Library files.

Please check whether the following DLL files are exist in your computer.

```
MSVCP140.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
```

The cactus is also dependent on python3.7, check python37.dll as well.

You can get the missing dlls when running on a `cmd.exe` terminal.   