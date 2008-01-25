Attribute VB_Name = "CommLib"
Option Explicit

'---Dichiarazioni di Windows a 32 bit
Private Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Long, ByVal lpFileName As String) As Long
Private Declare Function GetPrivateProfileInt Lib "kernel32" Alias "GetPrivateProfileIntA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal nDefault As Long, ByVal lpFileName As String) As Long
Private Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpString As Any, ByVal lpFileName As String) As Long
Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Integer) As Integer
Private Declare Function FindWindow Lib "user32" Alias "FindWindowA" (ByVal lpClassName As String, ByVal lpWindowName As String) As Long
Private Declare Function GetFileAttributes Lib "kernel32" Alias "GetFileAttributesA" (ByVal lpFileName As String) As Long
Public Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
Public Declare Function GetTempPath Lib "kernel32" Alias "GetTempPathA" (ByVal nBufferLength As Long, ByVal lpBuffer As String) As Long
Public Declare Function GetTempFileName Lib "kernel32" Alias "GetTempFileNameA" (ByVal lpszPath As String, ByVal lpPrefixString As String, ByVal wUnique As Long, ByVal lpTempFileName As String) As Long
Private Declare Function RegQueryValueExString Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, ByVal lpData As String, lpcbData As Long) As Long
Private Declare Function RegQueryValueExLong Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, lpData As Long, lpcbData As Long) As Long
Private Declare Function RegQueryValueExNULL Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, ByVal lpData As Long, lpcbData As Long) As Long
Private Declare Function RegSetValueExString Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, ByVal dwType As Long, ByVal lpValue As String, ByVal cbData As Long) As Long
Private Declare Function RegSetValueExLong Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, ByVal dwType As Long, lpValue As Long, ByVal cbData As Long) As Long
Public Declare Function RegCloseKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
Private Declare Function RegDeleteKey Lib "advapi32.dll" Alias "RegDeleteKeyA" (ByVal hKey As Long, ByVal lpSubKey As String) As Long
Private Declare Function RegDeleteValue Lib "advapi32.dll" Alias "RegDeleteValueA" (ByVal hKey As Long, ByVal lpValueName As String) As Long
Private Declare Function RegCreateKeyEx Lib "advapi32.dll" Alias "RegCreateKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal Reserved As Long, ByVal lpClass As String, ByVal dwOptions As Long, ByVal samDesired As Long, lpSecurityAttributes As Any, phkResult As Long, lpdwDisposition As Long) As Long
Public Declare Function RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal Reserved As Long, ByVal samDesired As Long, phkResult As Long) As Long
Public Declare Function RegEnumValue Lib "advapi32.dll" Alias "RegEnumValueA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpValueName As String, lpcbValueName As Long, ByVal lpReserved As Long, lpType As Long, lpData As Byte, lpcbData As Long) As Long
Private Declare Function RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, lpData As Any, lpcbData As Long) As Long
Private Declare Function RegSetValueEx Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, ByVal dwType As Long, lpData As Any, ByVal cbData As Long) As Long

Const MAX_PATH = 260
Const MAXDWORD = &HFFFF
Const FILE_ATTRIBUTE_ARCHIVE = &H20
Const FILE_ATTRIBUTE_DIRECTORY = &H10
Const FILE_ATTRIBUTE_HIDDEN = &H2
Const FILE_ATTRIBUTE_NORMAL = &H80
Const FILE_ATTRIBUTE_READONLY = &H1
Const FILE_ATTRIBUTE_SYSTEM = &H4
Const FILE_ATTRIBUTE_TEMPORARY = &H100

Public Const HKEY_LOCAL_MACHINE = &H80000002
Public Const REG_SZ As Long = 1
Public Const REG_DWORD As Long = 4

Public Const ERROR_NONE = 0
Public Const ERROR_BADDB = 1
Public Const ERROR_BADKEY = 2
Public Const ERROR_CANTOPEN = 3
Public Const ERROR_CANTREAD = 4
Public Const ERROR_CANTWRITE = 5
Public Const ERROR_OUTOFMEMORY = 6
Public Const ERROR_ARENA_TRASHED = 7
Public Const ERROR_ACCESS_DENIED = 8
Public Const ERROR_INVALID_PARAMETERS = 87
Public Const ERROR_NO_MORE_ITEMS = 259

Const HKEY_CURRENT_USER = &H80000001
Const REG_OPTION_BACKUP_RESTORE = 4     ' open for backup or restore
Const REG_OPTION_VOLATILE = 1           ' Key is not preserved when system is rebooted
Const REG_OPTION_NON_VOLATILE = 0       ' Key is preserved when system is rebooted
Const STANDARD_RIGHTS_ALL = &H1F0000
Const SYNCHRONIZE = &H100000
Const READ_CONTROL = &H20000
Const STANDARD_RIGHTS_READ = (READ_CONTROL)
Const STANDARD_RIGHTS_WRITE = (READ_CONTROL)
Const KEY_CREATE_LINK = &H20
Const KEY_CREATE_SUB_KEY = &H4
Const KEY_ENUMERATE_SUB_KEYS = &H8
Const KEY_NOTIFY = &H10
Const KEY_QUERY_VALUE = &H1
Const KEY_SET_VALUE = &H2
Const KEY_READ = ((STANDARD_RIGHTS_READ Or KEY_QUERY_VALUE Or KEY_ENUMERATE_SUB_KEYS Or KEY_NOTIFY) And (Not SYNCHRONIZE))
Const KEY_WRITE = ((STANDARD_RIGHTS_WRITE Or KEY_SET_VALUE Or KEY_CREATE_SUB_KEY) And (Not SYNCHRONIZE))
Const KEY_EXECUTE = (KEY_READ)
Public Const KEY_ALL_ACCESS = ((STANDARD_RIGHTS_ALL Or KEY_QUERY_VALUE Or KEY_SET_VALUE Or KEY_CREATE_SUB_KEY Or KEY_ENUMERATE_SUB_KEYS Or KEY_NOTIFY Or KEY_CREATE_LINK) And (Not SYNCHRONIZE))

Const KCRYPTKEY = "S" & "U" & "P" & "E" & "R" & "C" & "O" & "D" & "E" & "U" & "M" & "B" & "R" & "E" & "A" & "K" & "A" & "B" & "L" & "E"

Global sCfgApplIni   As String
Global sCfgLibIni    As String
Global sCfgSysIni    As String
Global sCfgSFIni     As String
Global sCfgPathLog   As String
Global iCfgLogLevel  As Integer
Global sCfgWorkDir   As String
Global bCfgDebug     As Boolean
Global sPathLog      As String
Function GetINILong(sFile As String, sGruppo As String, sVoce As String) As Long
    GetINILong = GetPrivateProfileInt(sGruppo, sVoce, 0, sFile)
End Function
Function GetINIString(sFile As String, sGruppo As String, sVoce As String) As String
    
Dim TempStr As String * 1024

  If GetPrivateProfileString(sGruppo, sVoce, "", TempStr, 1024, sFile) Then
    GetINIString = Left(TempStr, InStr(TempStr, Chr(0)) - 1)
  Else
    GetINIString = ""
  End If

End Function
Sub NoMouse()
    
  Screen.MousePointer = 11
    
End Sub
Function PutINILong(sFile As String, sGruppo As String, sVoce As String, lValore As Long) As Boolean

  If WritePrivateProfileString(sGruppo, sVoce, CStr(lValore), sFile) Then
    PutINILong = True
  Else
    PutINILong = False
  End If

End Function
Function PutIniString(sFile As String, sGruppo As String, sVoce As String, sValore As String) As Boolean
    
  If WritePrivateProfileString(sGruppo, sVoce, sValore, sFile) Then
    PutIniString = True
  Else
    PutIniString = False
  End If

End Function
Sub SiMouse()
   Screen.MousePointer = 0
End Sub

Function cVal(vVal As Variant) As Currency
    
  cVal = Val(Format(vVal, "General Number"))

End Function

Function ThousSep(ByVal txt As TextBox, Optional vTipo As Variant) As String
    
'***************************************************************
'*      Descrizione : Formatta automaticamente  con i punti    *
'*                    delle migliaia i TextBox                 *
'***************************************************************

Dim iPos    As Integer
Dim iLen    As Integer
Dim sFormat As String
Dim iCount

  If IsMissing(vTipo) Then
    vTipo = "N"
  End If
  
  Select Case vTipo
    Case "N":  sFormat = "#,##0"
    Case "F":  sFormat = "#,##0"
    Case "V":  sFormat = "L #,##0"
    Case "D":  sFormat = "dd-mm-yyyy"
    Case Else: sFormat = vTipo
  End Select
  
  iPos = txt.SelStart
  iLen = Len(txt)
  Select Case vTipo
    Case "N", "F"
      If IsNumeric(txt) Then
        txt = Format(txt, sFormat)
      Else
        txt = ""
      End If
    Case "V"
      iCount = 1
      While Not IsNumeric(Mid(txt, iCount))
        iCount = iCount + 1
      Wend
      If IsNumeric(Mid(txt, iCount)) Then
        txt = Format(Mid(txt, iCount), sFormat)
      Else
        txt = ""
      End If
    Case Else
      txt = Format(txt, sFormat)
  End Select
  
  iPos = iPos + Len(txt) - iLen
  If iPos >= 0 Then
    txt.SelStart = iPos
  End If
  
  ThousSep = txt
    
End Function
Public Function ConvApici(sStr As Variant)

Dim iPos As Integer
  
  iPos = InStr(sStr, "'")
  While iPos
    sStr = Left(sStr, iPos) & Mid(sStr, iPos)
    iPos = InStr(iPos + 2, sStr, "'")
  Wend
  ConvApici = sStr

End Function
Public Function CopyFile(ByVal sSourceDir As String, ByVal sSourceFile As String, ByVal sDestDir As String, ByVal sDestFile As String, ByVal bFlagMove As Boolean, ByVal lStart As Long, ByVal lLenght As Long, sErrString As String) As Boolean
    
    On Error GoTo ApplErr
    
    Dim sSourcePF   As String
    Dim sDestPF     As String
    Dim sTempPF     As String
    Dim lSourceSize As Long
    Dim lDestSize   As Long
    Dim iLoop1      As Integer
    Dim iLoop2      As Integer
    Dim lFhnd       As Long
    Dim bFileData() As Byte
    
    CopyFile = False
    
    '---Compongo il path+file source
    If Right(sSourceDir, 1) = "\" Then
        sSourcePF = sSourceDir & sSourceFile
        sTempPF = sSourceDir & "Temp"
      Else
        sSourcePF = sSourceDir & "\" & sSourceFile
        sTempPF = sSourceDir & "\Temp"
    End If
    
    '---Compongo il path+file dest
    If Right(sDestDir, 1) = "\" Then
        sDestPF = sDestDir & sDestFile
      Else
        sDestPF = sDestDir & "\" & sDestFile
    End If
    
    '---Controllo l'esistenza della directory Source
    If Dir(sSourceDir, vbDirectory) = "" Then
         sErrString = "Source Directory doesn't exist " & sSourceDir
         Exit Function
    End If
    
    '---Controllo l'esistenza del file Source
    If Dir(sSourcePF) = "" Then
         sErrString = "Source File doesn't exist " & sSourcePF
         Exit Function
    End If
    
    '---Solo una porzione del file?
    If lLenght <> 0 Then
        lFhnd = FreeFile
        ReDim bFileData(1 To lLenght) As Byte
        Open sSourcePF For Binary Access Read As lFhnd
        Get lFhnd, lStart + 1, bFileData
        Close lFhnd
        Open sTempPF For Binary Access Write As lFhnd
        Put lFhnd, , bFileData
        Close lFhnd
        sSourcePF = sTempPF
    End If
    
    '---Controllo l'esistenza della directory Dest
    If Dir(sDestDir, vbDirectory) = "" Then
         sErrString = "Destination Directory doesn't exist " & sDestDir
         Exit Function
    End If
    
    '---Copio il file da source a Dest
    FileCopy sSourcePF, sDestPF
    
    '---Controllo l'esistenza del file Dest (10 test con attesa di 500 ms)
    For iLoop1 = 1 To 10
        If Dir(sDestPF) = "" Then
            If iLoop1 = 10 Then
                sErrString = "Destination File doesn't exist after copy operation" & sDestPF
                Exit Function
              Else
                Sleep 500
            End If
          Else
            Exit For
        End If
    Next iLoop1
    
    '---Controllo che coincidi la dimensione del due file
    For iLoop2 = 1 To 10
        lSourceSize = FileLen(sSourcePF)
        lDestSize = FileLen(sDestPF)
        If lSourceSize <> lDestSize Then
            If iLoop2 = 10 Then
                sErrString = "File size doesn't match. Source:" & lSourceSize & " Dest:" & lDestSize
                Exit Function
              Else
                Sleep 500
            End If
          Else
            Exit For
        End If
    Next iLoop2
    
    '---Se richiesto elimino il file sorgente
    If bFlagMove Then
        Kill sSourcePF
    End If

    sErrString = "File copied in: " & sDestPF & ". Size: " & lDestSize & ". Retry: " & iLoop1 & "/" & iLoop2
    CopyFile = True
    
Exit Function

ApplErr:
    sErrString = "Generic application error during ""CopyFile"". Error=" & Err.Number & ". Description=" & Err.Description
End Function

Sub PKill(sFileName As String)
    
    On Error Resume Next
  
    If Dir(sFileName) <> "" Then
        Kill sFileName
    End If

End Sub
'Procedura di registrazione del file di Log
Sub LogFile(ByVal sStringa As String, iLogLevel As Integer, sLogPath As String)

    Static iIndent As Integer
    Dim sDir As String
    Dim lFile As Long
    Dim iPos As Integer
    
    On Error Resume Next
    
    'No Log for Level 0
    If iLogLevel = 0 Then
        Exit Sub
    End If
    
    'No Log for Log Higher then setted
    Select Case Left(sStringa, 4)
        Case "LOG1": If iLogLevel < 1 Then Exit Sub
        Case "LOG2": If iLogLevel < 2 Then Exit Sub
        Case "LOG3": If iLogLevel < 3 Then Exit Sub
    End Select

    If Left(sStringa, 4) = "ERRO" Then
        MsgBox Mid(sStringa, 8), vbCritical, "Road Runner Mobile"
    End If
    
    iPos = InStr(sStringa, "[-]")
    If iPos <> 0 Then
        If iIndent > 0 Then
            iIndent = iIndent - 1
        End If
    End If
    
    If iIndent <> 0 Then
        iPos = InStr(sStringa, " - ")
        sStringa = Left(sStringa, iPos + 2) & String(iIndent * 3, " ") & Mid(sStringa, iPos + 3)
    End If
    
    sStringa = Format(Now(), "hh:mm:ss") & " " & sStringa
    lFile = FreeFile
    Open sLogPath & "\" & Format(Now(), "yyyymmdd") + ".TXT" For Append As #lFile
    Print #lFile, sStringa
    Close #lFile
0    DoEvents
    
    iPos = InStr(sStringa, "[+]")
    If iPos <> 0 Then
        iIndent = iIndent + 1
    End If
    
End Sub
Function GetSubString(ByRef sStr As String, sSep As String) As String
    
    Dim iStrPos As Integer

    iStrPos = InStr(sStr, sSep)
    
    If iStrPos <> 0 Then
        GetSubString = Left(sStr, iStrPos - 1)
        sStr = Mid(sStr, iStrPos + 1)
      Else
        If sStr <> "" Then
            GetSubString = sStr
            sStr = ""
          Else
            GetSubString = ""
        End If
    End If

End Function
Function CreaDir(ByVal sStringa As String, sErrString As String) As Boolean

    Dim iPos         As Integer
    Dim sPathFile    As String
    Dim iPartSkipped As Integer
    Dim iPartToSkip  As Integer
    Dim iErrNum      As Integer
    
    On Error Resume Next
    CreaDir = False
    
    iPartSkipped = 1
    If Left(sStringa, 2) = "\\" Then
        iPartToSkip = 2 '(es \\Srv\share\   pippo\pluto\...)
        iPos = 2
      Else
        iPartToSkip = 0 '(es. C:\ pippo\pluto\...)
        iPos = 3
    End If
    
    While InStr(iPos + 1, sStringa, "\") > 0
        iPos = InStr(iPos + 1, sStringa, "\")
        If iPartSkipped > iPartToSkip Then
            If Dir(Left(sStringa, iPos - 1), vbDirectory) = "" Then
                Err = 0
                MkDir Left(sStringa, iPos - 1)
                iErrNum = Err.Number
                If iErrNum <> 0 Then
                    sErrString = "ERRO - La directory non è creabile: " & Left(sStringa, iPos - 1)
                    Exit Function
                End If
            End If
        End If
        iPartSkipped = iPartSkipped + 1
    Wend
    
    CreaDir = True

End Function
'Procedura di registrazione del file di Log
Function LockFile(ByVal sFile As String) As Boolean

Dim lFile As Long

LockFile = True
On Error GoTo LockError

If FileLen(sFile) > 0 Then
  lFile = FreeFile
  Open sFile For Input Lock Write As #lFile
  Close #lFile
  LockFile = False
End If

Exit Function

LockError:
  LockFile = True
  Err = 0
End Function

Function EstraiFile(ByVal sStringa As String) As String

On Error Resume Next
    
  EstraiFile = ""
  EstraiFile = Dir(sStringa)

End Function

Function EstraiDir(ByVal sStringa As String) As String

Dim iPos As Integer
Dim sPathFile As String

On Error Resume Next
    
  iPos = 0
  While InStr(iPos + 1, sStringa, "\") > 0
      iPos = InStr(iPos + 1, sStringa, "\")
  Wend
  EstraiDir = Left(sStringa, iPos - 1)

End Function
Function LockProgram() As Boolean

Dim lFile As Long
Const sFile = "Lock.lck"

LockProgram = False

On Error GoTo ErrorLock
  
  If Dir(App.Path + "\" + sFile, vbNormal) <> "" Then
    GoTo ErrorLock
  Else
    lFile = FreeFile
    Open App.Path + "\" + sFile For Output As #lFile
    Close #lFile
    LockProgram = True
  End If

Exit Function

ErrorLock:

End Function

Function UnLockProgram(sErrString As String) As Boolean
Dim lFile As Long
Const sFile = "Lock.lck"
UnLockProgram = False
On Error GoTo ErrorUnlock
If Dir(App.Path + "\" + sFile, vbNormal) <> "" Then
    Kill App.Path + "\" + sFile
    UnLockProgram = True
End If
Exit Function
ErrorUnlock:
  sErrString = "** Errore di Unlock del Programma"
End Function
Function KCrypt(ByVal Strg As String) As String
    
   
Dim a%, i%, b%, j%
Dim H$, J1$
  
  a = 1
  For i = 1 To Len(Strg$)
    b = 0
    For j = a To Len(KCRYPTKEY)
     b = b + Asc(Mid$(KCRYPTKEY, j, 1))
    Next j
    Do
      If b > 255 Then
    b = b - 256
      Else
    Exit Do
      End If
    Loop
     a = a + 1
     If a > Len(KCRYPTKEY) - 1 Then a = 1
     Mid$(Strg$, i, 1) = Chr$(Asc(Mid$(Strg$, i, 1)) Xor b)
  Next i

    H$ = ""
    For i = 1 To Len(Strg$)
       J1$ = Hex$(Asc(Mid$(Strg$, i, 1)))
       If Len(J1$) = 1 Then J1$ = "0" + J1$
       H$ = H$ + J1$
    Next
   
   Strg$ = Format$(Len(H$), "00") + H$

    a = 1
  For i = 1 To Len(Strg$)
    b = 0
    For j = a To Len(KCRYPTKEY)
     b = b + Asc(Mid$(KCRYPTKEY, j, 1))
    Next j
    Do
      If b > 255 Then
    b = b - 256
      Else
    Exit Do
      End If
    Loop
     a = a + 1
     If a > Len(KCRYPTKEY) - 1 Then a = 1
     Mid$(Strg$, i, 1) = Chr$(Asc(Mid$(Strg$, i, 1)) Xor b)
  Next i
  
  H$ = ""
    For i = 1 To Len(Strg$)
       J1$ = Hex$(Asc(Mid$(Strg$, i, 1)))
       If Len(J1$) = 1 Then J1$ = "0" + J1$
       H$ = H$ + J1$
    Next
  
  Strg$ = Format$(Len(H$), "00") + H$

KCrypt = Strg$
    
End Function
Function KDecrypt(ByVal Strg As String) As String
  
    Dim a%, i%, b%, j%
    Dim H$, J1$

    H$ = Mid$(Strg$, 3, Val(Left$(Strg$, 2)))
    Strg$ = ""
    For i = 1 To Len(H$) Step 2
       J1$ = Mid$(H$, i, 2)
       Strg$ = Strg$ + Chr$(Val("&H" + J1$))
    Next

  a = 1
  For i = 1 To Len(Strg$)
    b = 0
    For j = a To Len(KCRYPTKEY)
     b = b + Asc(Mid$(KCRYPTKEY, j, 1))
    Next j
    Do
      If b > 255 Then
    b = b - 256
      Else
    Exit Do
      End If
    Loop
     a = a + 1
     If a > Len(KCRYPTKEY) - 1 Then a = 1
     Mid$(Strg$, i, 1) = Chr$(Asc(Mid$(Strg$, i, 1)) Xor b)
  Next i

H$ = Mid$(Strg$, 3, Abs(Val(Left$(Strg$, 2))))

Strg$ = ""
For i = 1 To Len(H$) Step 2
   J1$ = Mid$(H$, i, 2)
   Strg$ = Strg$ + Chr$(Val("&H" + J1$))
Next
  a = 1
  For i = 1 To Len(Strg$)
    b = 0
    For j = a To Len(KCRYPTKEY)
     b = b + Asc(Mid$(KCRYPTKEY, j, 1))
    Next j
    Do
      If b > 255 Then
    b = b - 256
      Else
    Exit Do
      End If
    Loop
     a = a + 1
     If a > Len(KCRYPTKEY) - 1 Then a = 1
     Mid$(Strg$, i, 1) = Chr$(Asc(Mid$(Strg$, i, 1)) Xor b)
  Next i
KDecrypt = Strg$

End Function

Function ChkInt(stringa As String) As Boolean

Dim i As Integer
  
  ChkInt = False
  For i = 1 To Len(stringa)
    If Mid(stringa, i, 1) < "0" And Mid(stringa, i, 1) > "9" Then
      Exit Function
    End If
  Next i

ChkInt = True

End Function

Sub PSospendi(ByVal sFile As String)

Dim sDir As String
Dim sNewfile As String
Dim iPunto As Integer

On Error Resume Next
  
  'Copia il file in una directory di sos
  'Verifico esistenza directory "SOS"
  sDir = EstraiDir(sFile) + "\SOS"
  If Dir(sDir, vbDirectory) = "" Then
    MkDir sDir
  End If
  'Carico il nome del file
  iPunto = InStr(EstraiFile(sFile), ".") - 1
  sNewfile = sDir + "\" + Left(EstraiFile(sFile), iPunto) + Format(Now(), "aaaammdd") + Mid(EstraiFile(sFile), iPunto + 1)
  If Dir(sNewfile) <> "" Then
    'Controllo se il file già esiste lo cancello
    PKill sNewfile
  End If
  'Sposto il File
  Name sFile As sNewfile

End Sub

Sub PBackup(ByVal sFile As String)

Dim sDir As String
Dim sNewfile As String
Dim iPunto As Integer

On Error Resume Next
  
  'Copia il file in una directory di BKP
  'Verifico esistenza directory "BKP"
  sDir = EstraiDir(sFile) + "\BKP"
  If Dir(sDir, vbDirectory) = "" Then
    MkDir sDir
  End If
  'Carico il nome del file
  iPunto = InStr(EstraiFile(sFile), ".") - 1
  sNewfile = sDir + "\" + Left(EstraiFile(sFile), iPunto) + Format(Now(), "yyyymmdd") + Mid(EstraiFile(sFile), iPunto + 1)
  If Dir(sNewfile) <> "" Then
    'Controllo se il file già esiste lo cancello
    PKill sNewfile
  End If
  'Sposto il File
  Name sFile As sNewfile

End Sub

Function GetDirSize(sDir As String) As Long
If sDir = "" Or Dir(sDir, vbDirectory) = "" Then Exit Function
On Error GoTo ErrDir

  Dim fso As Object
  Set fso = CreateObject("Scripting.FileSystemObject")
  GetDirSize = fso.GetFolder(sDir).Size
  Set fso = Nothing
Exit Function
ErrDir:
End Function

Function GetDirSizeC(sDir As String, lCluster As Long) As Long
If sDir = "" Or Dir(sDir, vbDirectory) = "" Then Exit Function
Dim Numfiles As Integer
Dim NumDirs As Integer
On Error GoTo ErrDir

  GetDirSizeC = FindFilesAPI(sDir, "*.*", Numfiles, NumDirs, lCluster)

Exit Function
ErrDir:
End Function

Function GetPhysicalFileSize(sFile As String, lCluster As Long) As Long
    If FileLen(sFile) Mod lCluster = 0 Then
        GetPhysicalFileSize = FileLen(sFile)
    Else
        GetPhysicalFileSize = lCluster * (Int(FileLen(sFile) / lCluster) + 1)
    End If
End Function

Function FindFilesAPI(Path As String, SearchStr As String, FileCount As Integer, DirCount As Integer, lCluster As Long)
    'KPD-Team 1999
    'E-Mail: KPDTeam@Allapi.net

    Dim FileName As String ' Walking filename variable...
    Dim DirName As String ' SubDirectory Name
    Dim dirNames() As String ' Buffer for directory name entries
    Dim nDir As Integer ' Number of directories in this path
    Dim i As Integer ' For-loop counter...
    Dim hSearch As String ' Search Handle
    Dim Cont As Integer
    If Right(Path, 1) <> "\" Then Path = Path & "\"
    ' Search for subdirectories.
    nDir = 0
    ReDim dirNames(nDir)
    Cont = True
    hSearch = Dir(Path & "*", vbDirectory)
    If hSearch <> "" Then
        Do While hSearch <> ""
        DirName = hSearch
        ' Ignore the current and encompassing directories.
        If (DirName <> ".") And (DirName <> "..") Then
            ' Check for directory with bitwise comparison.
            If GetFileAttributes(Path & DirName) And FILE_ATTRIBUTE_DIRECTORY Then
                dirNames(nDir) = DirName
                DirCount = DirCount + 1
                nDir = nDir + 1
                ReDim Preserve dirNames(nDir)
            End If
        End If
        hSearch = Dir() 'Get next subdirectory.
        Loop
    End If
    ' Walk through this directory and sum file sizes.
    hSearch = Dir(Path & "*")
    If hSearch <> "" Then
        Do While hSearch <> ""
            FileName = hSearch
            If (FileName <> ".") And (FileName <> "..") Then
                FindFilesAPI = FindFilesAPI + GetPhysicalFileSize(Path & FileName, lCluster)
                FileCount = FileCount + 1
            End If
            hSearch = Dir() 'Get next subdirectory.
        Loop
    End If
    ' If there are sub-directories...
    If nDir > 0 Then
        ' Recursively walk into them...
        For i = 0 To nDir - 1
            FindFilesAPI = FindFilesAPI + FindFilesAPI(Path & dirNames(i) & "\", SearchStr, FileCount, DirCount, lCluster)
        Next i
    End If
End Function
Sub DebugMsg(sStr As String)
    If bCfgDebug Then
        MsgBox sStr, vbInformation, App.Title
    End If
End Sub
Sub ErrMsgBox(sStr As String)
    MsgBox sStr, vbCritical, App.Title
End Sub
Sub BigSleep(lMin As Long)
    Dim lNow As Long
    
    lNow = Timer
    While Timer < lNow + (lMin * 60)
        DoEvents
    Wend
    
End Sub
Public Function TrimC(ByVal Str As String, Char As String) As String
    
    While Left(Str, Len(Char)) = Char
        Str = Mid(Str, Len(Char) + 1)
    Wend
    While Right(Str, Len(Char)) = Char
        Str = Left(Str, Len(Str) - Len(Char))
    Wend
    TrimC = Str

End Function
Public Function DeleteKeyValue(hKey As Long, strPath As String, strValue As String) As Boolean
    
    Dim ret As Long
    Dim hNewKey As Long
    'Create a new key
    ret = RegCreateKeyEx(hKey, strPath, 0&, _
              vbNullString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, _
              ByVal 0&, hNewKey, ret)
    'RegCreateKey hKey, strPath, Ret
    If hNewKey <> 0 Then
    'Delete the key's value
      ret = RegDeleteValue(hNewKey, strValue)
      If ret = 0 Then DeleteKeyValue = True
    End If
    'close the key
    RegCloseKey hNewKey
    
End Function

Public Function CreateNewKey(hKeyRoot As Long, strNewKeyName As String) As Long
    
    Dim hNewKey As Long         'handle to the new key
    Dim rc As Long         'result of the RegCreateKeyEx function

    rc = RegCreateKeyEx(hKeyRoot, strNewKeyName, 0&, _
              vbNullString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, _
              ByVal 0&, hNewKey, rc)
    RegCloseKey (hNewKey)
    CreateNewKey = rc
    
End Function

Public Function RegSetKeyValue(ByVal hKey As Long, _
                                strPath As String, _
                                strKeyName As String, _
                                lTypeKey As Long, _
                                vKeyValue As Variant) As Long
    Dim lValue As Long
    Dim strValue As String
    Dim rc As Long
    
    RegOpenKeyEx hKey, strPath, 0, KEY_ALL_ACCESS, rc
    If rc = 0 Then
        RegSetKeyValue = ERROR_CANTOPEN
        Exit Function
    End If
    
    Select Case lTypeKey
        Case REG_SZ
            strValue = vKeyValue & Chr$(0)
            RegSetKeyValue = RegSetValueExString(rc, strKeyName, 0&, _
                                           lTypeKey, strValue, Len(strValue))
        Case REG_DWORD
            lValue = vKeyValue
            RegSetKeyValue = RegSetValueExLong(rc, strKeyName, 0&, _
                                            lTypeKey, lValue, 4)
    End Select
        
End Function

Public Function RegGetKeyValue(ByVal hKey As Long, _
                                ByVal strPath As String, _
                                ByVal strKeyName As String, _
                                vKeyValue As Variant) As Long
    Dim cch As Long
    Dim rc As Long
    Dim hTmpKey As Long
    Dim lType As Long
    Dim lValue As Long
    Dim strValue As String

    On Error GoTo QueryValueExError

    RegOpenKeyEx hKey, strPath, 0, KEY_ALL_ACCESS, hTmpKey
    If hTmpKey = 0 Then
        If CreateNewKey(hKey, strPath) = 0 Then
            Error ERROR_CANTOPEN
            Exit Function
        End If
    End If
    
    ' Determine the size and type of data to be read
    rc = RegQueryValueExNULL(hTmpKey, strKeyName, 0&, lType, 0&, cch)
    If rc <> ERROR_NONE Then Error 5

    Select Case lType
        ' For strings
        Case REG_SZ:
            strValue = String(cch, 0)
            rc = RegQueryValueExString(hTmpKey, strKeyName, 0&, lType, strValue, cch)
            If rc = ERROR_NONE Then
                vKeyValue = Left$(strValue, cch - 1)
            Else
                vKeyValue = Empty
            End If
        ' For DWORDS
        Case REG_DWORD:
            rc = RegQueryValueExLong(hTmpKey, strKeyName, 0&, lType, lValue, cch)
            If rc = ERROR_NONE Then vKeyValue = lValue
        Case Else
        'all other data types not supported
            rc = -1
    End Select

QueryValueExExit:
       RegGetKeyValue = rc
       Exit Function

QueryValueExError:
       Resume QueryValueExExit
       
End Function

Public Function GetWindowDir() As String
    Dim strSave As String
    'Create a buffer string
    strSave = String(255, Chr$(0))
    'Get the windows directory
    GetWindowDir = Left(strSave, GetWindowsDirectory(strSave, Len(strSave)))
End Function
Public Sub KGlobalKrypt(ByVal sSerNum As String, ByVal sSiteAddress As String, ByVal sLicName As String, ByVal sLicExpr As String, sLicCode As String)

    
    Dim sSerNum1       As String * 20
    Dim sSiteAddress1  As String * 20
    Dim sLicName1      As String * 20
    Dim sLicExpr1      As String * 20
    Dim sTempCode1     As String
    Dim sTempCode2     As String
    Dim sSequence      As String
    Dim iCount1        As Integer
    Dim iCount2        As Integer
    Dim iSeqPos        As Integer
    
    '---Generate sequence
    'Dim sSeq(1 To 344) As String
    'Dim iPos As Integer
    'Dim iPos2 As Integer
    'Dim sTemp As String
    'Dim lFHnd  As Long
    'Randomize (Timer)
    'sTemp = ""
    'For iPos = 1 To 344
    '    sSeq(iPos) = Format(iPos, "000")
    'Next iPos
    'For iPos = 1 To 344
    '    Do
    '        iPos2 = 1 + Int(Rnd * 344)
    '        If sSeq(iPos2) <> "" Then
    '            sTemp = sTemp & sSeq(iPos2)
    '            sSeq(iPos2) = ""
    '            Exit Do
    '        End If
    '    Loop
    'Next iPos
    'lFHnd = FreeFile
    'Open "C:\code.txt" For Output As lFHnd
    'Print #lFHnd, sTemp
    'Close lFHnd
    'Debug.Print sTemp
    '--------------------Get the sequence from the Debug windows of C:\code.txt and paste below-------------
    
    sSequence = "301048236160195297126043254053115327104200323248170260143184197074076288266192210231177291159073082125054304321187044021135147318196142338230127060162243216229027156314337042013064255181282343247172092280024103188217319335072286227012238342030298341137091032212157075233262016258140100"
    sSequence = sSequence & "302240062292050106019213047326320063261251085283237334202096324101259204133086120232267145020214205123256028161277102005182132118272173306039166038121309331080313250329079051094281124245155269290183264002293198119244201089270083339149055077175041169139279207056049307017057315224090068"
    sSequence = sSequence & "276211344278059225078235273111031108071226084340130203252110158129141105296151099289221081234299268066065087178148295163153171332112223165029215022070036265136069167154228241189180025222067010015199322007310312004242185263186249009219018303330208257325146023194014045006034098174164001"
    sSequence = sSequence & "003333271193305052294134107093336046191206246220285008150275138253168097308300026117190311284328109033116152058088037144040317274095114239061011316113179287035122218176128131209"

    sSerNum1 = sSerNum
    sSiteAddress1 = sSiteAddress
    sLicName1 = sLicName
    sLicExpr1 = sLicExpr
    
    '---Generating total code 86 chars per block
    sTempCode1 = KCrypt(sSerNum1) & KCrypt(sSiteAddress1) & KCrypt(sLicName1) & KCrypt(sLicExpr1)
    
    '---Using the "sequence" for each block
    sTempCode2 = ""
    For iCount1 = 1 To 344
        iSeqPos = CInt(Mid(sSequence, (iCount1 * 3) - 2, 3))
        sTempCode2 = sTempCode2 & Mid(sTempCode1, iSeqPos, 1)
    Next iCount1
    
    sLicCode = sTempCode2

End Sub
Public Function KGlobalDeKrypt(ByVal sLicCode As String, sSerNum As String, sSiteAddress As String, sLicName As String, sLicExpr As String) As String

    Dim sTempCode1    As String
    Dim sTempCode2    As String
    Dim sSequence     As String
    Dim iCount1       As Integer
    Dim iCount2       As Integer
    Dim iSeqPos       As Integer
    
    sSequence = "301048236160195297126043254053115327104200323248170260143184197074076288266192210231177291159073082125054304321187044021135147318196142338230127060162243216229027156314337042013064255181282343247172092280024103188217319335072286227012238342030298341137091032212157075233262016258140100"
    sSequence = sSequence & "302240062292050106019213047326320063261251085283237334202096324101259204133086120232267145020214205123256028161277102005182132118272173306039166038121309331080313250329079051094281124245155269290183264002293198119244201089270083339149055077175041169139279207056049307017057315224090068"
    sSequence = sSequence & "276211344278059225078235273111031108071226084340130203252110158129141105296151099289221081234299268066065087178148295163153171332112223165029215022070036265136069167154228241189180025222067010015199322007310312004242185263186249009219018303330208257325146023194014045006034098174164001"
    sSequence = sSequence & "003333271193305052294134107093336046191206246220285008150275138253168097308300026117190311284328109033116152058088037144040317274095114239061011316113179287035122218176128131209"

    sTempCode1 = sLicCode
    
    '---Using the "sequence" for each block
    sTempCode2 = String(344, " ")
    For iCount1 = 1 To 344
            iSeqPos = CInt(Mid(sSequence, (iCount1 * 3) - 2, 3))
            sTempCode2 = Left(sTempCode2, iSeqPos - 1) & Mid(sTempCode1, iCount1, 1) & Mid(sTempCode2, iSeqPos + 1)
    Next iCount1
    sSerNum = KDecrypt(Mid(sTempCode2, 1, 86))
    sSiteAddress = KDecrypt(Mid(sTempCode2, 87, 86))
    sLicName = KDecrypt(Mid(sTempCode2, 173, 86))
    sLicExpr = KDecrypt(Mid(sTempCode2, 259, 86))
    
End Function
Function PUBound(array1 As Variant)

    On Error Resume Next
    PUBound = -1
    PUBound = UBound(array1)

End Function
Function PLeft(ByRef sInStr As String, ByVal sEndSep As String, bTrim As Boolean) As String
    Dim sTemp As String
    Dim lPos As Long
    
    lPos = InStr(1, sInStr, sEndSep)
    If lPos <> 0 Then
        sTemp = Left(sInStr, lPos - 1)
        If bTrim Then
            sInStr = Mid(sInStr, lPos + Len(sEndSep))
        End If
        PLeft = sTemp
    End If

End Function
