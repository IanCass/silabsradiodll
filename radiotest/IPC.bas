Attribute VB_Name = "IPC"
     Type COPYDATASTRUCT
              dwData As Long
              cbData As Long
              lpData As Long
      End Type

      Public Const GWL_WNDPROC = (-4)
      Public Const WM_COPYDATA = &H4A
      Global lpPrevWndProc As Long
      Global gHW As Long

      'Copies a block of memory from one location to another.

      Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" _
         (hpvDest As Any, hpvSource As Any, ByVal cbCopy As Long)

      Declare Function CallWindowProc Lib "user32" Alias _
         "CallWindowProcA" (ByVal lpPrevWndFunc As Long, ByVal hwnd As _
         Long, ByVal Msg As Long, ByVal wParam As Long, ByVal lParam As _
         Long) As Long

      Declare Function SetWindowLong Lib "user32" Alias "SetWindowLongA" _
         (ByVal hwnd As Long, ByVal nIndex As Long, ByVal dwNewLong As _
         Long) As Long

      Public Sub Hook()
          lpPrevWndProc = SetWindowLong(gHW, GWL_WNDPROC, _
          AddressOf WindowProc)
          Debug.Print lpPrevWndProc
      End Sub

      Public Sub Unhook()
          Dim temp As Long
          temp = SetWindowLong(gHW, GWL_WNDPROC, lpPrevWndProc)
      End Sub

      Function WindowProc(ByVal hw As Long, ByVal uMsg As Long, _
         ByVal wParam As Long, ByVal lParam As Long) As Long
          If uMsg = WM_COPYDATA Then
              Call mySub(lParam)
          End If
          WindowProc = CallWindowProc(lpPrevWndProc, hw, uMsg, wParam, _
             lParam)
      End Function

      Sub mySub(lParam As Long)
          Dim cds As COPYDATASTRUCT
          Dim buf(1 To 255) As Byte
          Dim message As String
          Dim ret As Long

          Call CopyMemory(cds, ByVal lParam, Len(cds))

          Select Case cds.dwData
           Case 1
              Debug.Print "got a 1"
           Case 2
              Debug.Print "got a 2"

          ' Case 3
              Call CopyMemory(buf(1), ByVal cds.lpData, cds.cbData)

              message = StrConv(buf, vbUnicode)
              message = Left$(message$, InStr(1, message, Chr$(0)) - 1)
              Debug.Print message
              
              If (message = "TASTART") Then
                frmMain.Traffic.Visible = True
              End If
              
              If (message = "TASTOP") Then
                frmMain.Traffic.Visible = False
              End If
              
                
                
              
              'frmMain.VSPortAx1.WriteStr "?"
              'frmMain.VSPortAx1.WriteStr message
              'frmMain.VSPortAx1.WriteStr "?" & Chr(10) & Chr(13)

              'frmMain.VSPortAx1.Write (buf, cds.cbdata)
              'frmMain.VSPortAx1.WriteStr ("?" & Chr(10) & Chr(13))
              'Form1.Print a$
          End Select
      End Sub
                

