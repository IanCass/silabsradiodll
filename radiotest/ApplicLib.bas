Attribute VB_Name = "Module1"
Option Explicit

'Silabs USB FM Radio Reference Design API
Public Declare Function HWInit Lib "USBRadio.dll" () As Boolean
Public Declare Function HWDeInit Lib "USBRadio.dll" () As Boolean

Public Declare Function FMTune Lib "USBRadio.dll" (ByVal frequency As Long) As Boolean
Public Declare Function SeekStation Lib "USBRadio.dll" (ByVal SeekUp As Long) As Long
Public Declare Function GetRDS Lib "USBRadio.dll" () As String
Public Declare Function GetModuleName Lib "USBRadio.dll" () As String
Public Declare Function GetSignal Lib "USBRadio.dll" () As Long
Public Declare Function GetCurrStation Lib "USBRadio.dll" () As Long
Public Declare Sub SetMute Lib "USBRadio.dll" (ByVal mute As Boolean)
Public Declare Function IsStereo Lib "USBRadio.dll" () As Boolean

Public Declare Function VB_GetModuleName Lib "USBRadio.dll" (ByVal sNameBuffer As String, ByRef iNameLenght As Integer) As Boolean

Public Declare Function VB_GetRDSText Lib "USBRadio.dll" (ByVal sRDSBuffer As String, ByRef iRDSLenght As Integer) As Boolean
Public Declare Function VB_GetRDSPS Lib "USBRadio.dll" (ByVal sRDSBuffer As String, ByRef iRDSLenght As Integer) As Boolean
Public Declare Function VB_GetRDSPI Lib "USBRadio.dll" (ByRef sRDSPI As Integer) As Boolean
Public Declare Function VB_GetRDSPTY Lib "USBRadio.dll" (ByRef sRDSPTY As Integer) As Boolean
Public Declare Function VB_GetRDSPTYString Lib "USBRadio.dll" (ByVal sRDSBuffer As String, ByRef iRDSLenght As Integer) As Boolean
Public Declare Function VB_GetRDSTP Lib "USBRadio.dll" (ByRef sRDSTP As Boolean) As Boolean
Public Declare Function VB_GetRDSTA Lib "USBRadio.dll" (ByRef sRDSTA As Boolean) As Boolean
Public Declare Function VB_GetRDSMS Lib "USBRadio.dll" (ByRef sRDSMS As Boolean) As Boolean

Public Declare Function VB_GetRadioRegisters Lib "USBRadio.dll" (ByVal sRegBuffer As String, ByRef iRegLenght As Integer) As Boolean
Public Declare Function VB_GetRDSRegisters Lib "USBRadio.dll" (ByVal sRegBuffer As String, ByRef iRegLenght As Integer) As Boolean

Public Declare Sub VB_RegisterTMCCallback Lib "USBRadio.dll" (ByVal pFunc As Long)

Public Declare Function VB_GetAFList Lib "USBRadio.dll" (ByRef ary As Single, ByRef size As Long) As Boolean
Public Declare Function VB_GetRDSPICountry Lib "USBRadio.dll" (ByVal sRDSBuffer As String, ByRef iRDSLenght As Integer) As Boolean
Public Declare Function VB_GetRDSPIRegion Lib "USBRadio.dll" (ByVal sRDSBuffer As String, ByRef iRDSLenght As Integer) As Boolean


Public Declare Function RegisterTAStart Lib "USBRadio.dll" (ByVal windowName As String, ByVal dwData As Integer, ByVal lpData As String) As Boolean
Public Declare Function RegisterTAStop Lib "USBRadio.dll" (ByVal windowName As String, ByVal dwData As Integer, ByVal lpData As String) As Boolean


Global Const IntOffset = 65536
Global Const MaxInt = 32767

Public Function WVB_GetAFList() As Single()
    Dim aflist() As Single
    Dim arysize As Long
    Dim LoopArr As Single
    
    ' Create the array
    arysize = 25
    ReDim aflist(arysize - 1) As Single

    If (VB_GetAFList(aflist(0), arysize)) Then
            ReDim Preserve aflist(arysize - 1)
            For LoopArr = 0 To UBound(aflist)
                Debug.Print "AFLIST = " & aflist(LoopArr)
            Next LoopArr
    End If
    WVB_GetAFList = aflist
End Function


Public Function WVB_GetModuleName() As String
    
    Dim sBuffer    As String * 256
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetModuleName(sBuffer, iBufferLen)) Then
        WVB_GetModuleName = Left(sBuffer, iBufferLen)
    End If
    
End Function


      Public Function StrToHex(ByVal Text As String) As String

          Dim tmpArr() As Byte, strArr() As Byte

          Dim a As Long, b As Long, UpperBits As Byte, LowerBits As Byte

          If LenB(Text) = 0 Then Exit Function

          strArr = Text

          ReDim tmpArr(LenB(Text) + LenB(Text) - 1)

          For a = 0 To UBound(strArr) Step 2

              UpperBits = (strArr(a) And &HF0) \ &H10

              LowerBits = strArr(a) And &HF

              b = a + a

              If UpperBits > 10 Then

                  tmpArr(b) = (48 Or UpperBits) + 7

              Else

                  tmpArr(b) = 48 Or UpperBits

              End If

              b = b + 2

              If LowerBits > 10 Then

                  tmpArr(b) = (48 Or LowerBits) + 7

              Else

                  tmpArr(b) = 48 Or LowerBits

              End If

          Next a
          StrToHex = tmpArr

      End Function

Public Function WVB_GetRDSPI() As String
    
    Dim sPI As Integer
    
    
    If (VB_GetRDSPI(sPI)) Then
        WVB_GetRDSPI = SInt2UInt(sPI)
    End If
    
End Function



Public Function WVB_GetRDSTP() As Boolean
    
    Dim sRet    As Boolean
    
    If (VB_GetRDSTP(sRet)) Then
        WVB_GetRDSTP = sRet
    End If
    
End Function

Public Function WVB_GetRDSTA() As Boolean
    
    Dim sRet    As Boolean
    
    If (VB_GetRDSTA(sRet)) Then
        WVB_GetRDSTA = sRet
    End If
    
End Function

Public Function WVB_GetRDSMS() As Boolean
    
    Dim sRet    As Boolean
    
    If (VB_GetRDSMS(sRet)) Then
        WVB_GetRDSMS = sRet
    End If
    
End Function

Public Function WVB_GetRDSPTY() As String
    
    Dim sPTY As Integer
    
    
    If (VB_GetRDSPTY(sPTY)) Then
            WVB_GetRDSPTY = sPTY
    End If
    
End Function

Public Function WVB_GetRDSPS() As String
    
    Dim sBuffer    As String * 8
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetRDSPS(sBuffer, iBufferLen)) Then
        WVB_GetRDSPS = Left(sBuffer, iBufferLen)
    End If
    
End Function

Public Function WVB_GetRDSPIRegion() As String
    
    Dim sBuffer    As String * 256
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetRDSPIRegion(sBuffer, iBufferLen)) Then
        Debug.Print "REGION = " & Left(sBuffer, iBufferLen)
           WVB_GetRDSPIRegion = Left(sBuffer, iBufferLen)
    End If
    
End Function

Public Function WVB_GetRDSPICountry() As String
    
    Dim sBuffer    As String * 256
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetRDSPICountry(sBuffer, iBufferLen)) Then
            Debug.Print "COUNTRY = " & Left(sBuffer, iBufferLen)
            WVB_GetRDSPICountry = Left(sBuffer, iBufferLen)
    End If
    
End Function

Public Function WVB_GetRDSText() As String
    
    Dim sBuffer    As String * 256
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetRDSText(sBuffer, iBufferLen)) Then
        WVB_GetRDSText = Left(sBuffer, iBufferLen)
    End If
    
End Function

Public Function WVB_GetRDSPTYString() As String
    
    Dim sBuffer    As String * 256
    Dim iBufferLen As Integer
    Dim lRet       As Long
    
    If (VB_GetRDSPTYString(sBuffer, iBufferLen)) Then
        WVB_GetRDSPTYString = Left(sBuffer, iBufferLen)
    End If
    
End Function

Public Function UInt2SInt(ByVal value As Variant) As Integer

    If value >= IntOffset Then value = IntOffset - 1
    If value < (0 - MaxInt) - 1 Then value = (0 - MaxInt) - 1
    If value <= MaxInt Then
    UInt2SInt = value
    Else
    UInt2SInt = value - IntOffset
    End If

End Function

Public Function SInt2UInt(ByVal value As Variant) As Long

    If value > MaxInt Then value = MaxInt
    If value < 0 Then
    SInt2UInt = value + IntOffset
    Else
    SInt2UInt = value
    End If

End Function


'Sub TMCCallback(ByVal a As Integer, ByVal b As Integer, ByVal c As Integer, ByVal d As Integer, ByVal e As Integer, ByVal f As Integer, ByVal g As Integer, ByVal H As Integer)
'    On Error GoTo localhandler
'    Debug.Print "?" & Chr(a) & Chr(b) & Chr(c) & Chr(d) & Chr(e) & Chr(f) & Chr(g) & Chr(H) & "?"
'    Exit Sub
'
'    frmMain.VSPortAx1.WriteStr "?" & Chr(a) & Chr(b) & Chr(c) & Chr(d) & Chr(e) & Chr(f) & Chr(g) & Chr(H) & "?"
'
'localhandler:
'    Debug.Print "ERROR SENDING TO SOCKET"
'
'End Sub





