VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmMain 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Silab USB Radio Tester"
   ClientHeight    =   7215
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   11895
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   7215
   ScaleWidth      =   11895
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox Check2 
      Caption         =   "Mute"
      Height          =   615
      Left            =   6120
      TabIndex        =   31
      Top             =   2280
      Width           =   1335
   End
   Begin VB.Frame Frame7 
      Caption         =   "Alternate Frequencies"
      Enabled         =   0   'False
      Height          =   2535
      Left            =   3795
      TabIndex        =   27
      Top             =   330
      Width           =   2130
      Begin VB.CommandButton cmdTune 
         Caption         =   "Tune it"
         Enabled         =   0   'False
         Height          =   420
         Index           =   4
         Left            =   90
         TabIndex        =   29
         Top             =   2025
         Width           =   1875
      End
      Begin VB.ListBox lstAF 
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1380
         ItemData        =   "frmMain.frx":0000
         Left            =   120
         List            =   "frmMain.frx":0002
         TabIndex        =   28
         Top             =   240
         Width           =   1845
      End
   End
   Begin VB.Frame Frame6 
      Caption         =   "EON"
      Enabled         =   0   'False
      Height          =   1710
      Left            =   7320
      TabIndex        =   26
      Top             =   4200
      Width           =   3705
   End
   Begin VB.Frame Frame4 
      Caption         =   "FM Radio"
      Height          =   2640
      Left            =   315
      TabIndex        =   16
      Top             =   210
      Width           =   3315
      Begin VB.CommandButton cmdOnOff 
         Caption         =   "Turn ON"
         Height          =   510
         Left            =   2295
         TabIndex        =   30
         Tag             =   "0"
         Top             =   585
         Width           =   735
      End
      Begin VB.CommandButton cmdTune 
         Caption         =   "< Tune"
         Height          =   420
         Index           =   0
         Left            =   90
         TabIndex        =   20
         Top             =   2070
         Width           =   700
      End
      Begin VB.CommandButton cmdTune 
         Caption         =   "< Freq."
         Height          =   420
         Index           =   1
         Left            =   855
         TabIndex        =   19
         Top             =   2070
         Width           =   700
      End
      Begin VB.CommandButton cmdTune 
         Caption         =   "Freq. >"
         Height          =   420
         Index           =   2
         Left            =   1665
         TabIndex        =   18
         Top             =   2070
         Width           =   700
      End
      Begin VB.CommandButton cmdTune 
         Caption         =   "Tune >"
         Height          =   420
         Index           =   3
         Left            =   2445
         TabIndex        =   17
         Top             =   2070
         Width           =   700
      End
      Begin MSComctlLib.ProgressBar bpSignal 
         Height          =   480
         Left            =   75
         TabIndex        =   21
         Top             =   1485
         Width           =   2400
         _ExtentX        =   4233
         _ExtentY        =   847
         _Version        =   393216
         Appearance      =   1
         Max             =   64
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "Signal:"
         Height          =   195
         Index           =   0
         Left            =   105
         TabIndex        =   25
         Top             =   1245
         Width           =   480
      End
      Begin VB.Label lblFreq 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "---,---"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   24
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   630
         Left            =   165
         TabIndex        =   24
         Top             =   510
         Width           =   1725
      End
      Begin VB.Label lblSignal 
         AutoSize        =   -1  'True
         BorderStyle     =   1  'Fixed Single
         Caption         =   "100"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   2580
         TabIndex        =   23
         Top             =   1530
         Width           =   600
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "Frequence:"
         Height          =   195
         Index           =   1
         Left            =   180
         TabIndex        =   22
         Top             =   270
         Width           =   810
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "Regional Information / Decoder Information"
      Height          =   1095
      Left            =   315
      TabIndex        =   9
      Top             =   2955
      Width           =   6750
      Begin VB.Label lblCoverage 
         BorderStyle     =   1  'Fixed Single
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   390
         Left            =   1980
         TabIndex        =   15
         Top             =   540
         Width           =   3390
      End
      Begin VB.Label lblCountry 
         BorderStyle     =   1  'Fixed Single
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00808080&
         Height          =   390
         Left            =   285
         TabIndex        =   14
         Top             =   540
         Width           =   1530
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "Country:"
         Enabled         =   0   'False
         Height          =   195
         Index           =   11
         Left            =   300
         TabIndex        =   13
         Top             =   330
         Width           =   585
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "Coverage:"
         Enabled         =   0   'False
         Height          =   195
         Index           =   10
         Left            =   2010
         TabIndex        =   12
         Top             =   270
         Width           =   735
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "Interpretation:"
         Height          =   195
         Index           =   9
         Left            =   5520
         TabIndex        =   11
         Top             =   330
         Width           =   975
      End
      Begin VB.Label lblStereo 
         AutoSize        =   -1  'True
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Stereo"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00808080&
         Height          =   390
         Left            =   5520
         TabIndex        =   10
         Top             =   540
         Width           =   975
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Radio Text:"
      Height          =   1065
      Left            =   285
      TabIndex        =   7
      Top             =   5970
      Width           =   11415
      Begin VB.Label lblRadioTextA 
         BorderStyle     =   1  'Fixed Single
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   165
         TabIndex        =   8
         Top             =   390
         Width           =   10980
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Service Details:"
      Height          =   1695
      Left            =   360
      TabIndex        =   0
      Top             =   4200
      Width           =   6735
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "PS:"
         Height          =   195
         Index           =   5
         Left            =   120
         TabIndex        =   35
         Top             =   480
         Width           =   255
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "PI:"
         Height          =   195
         Index           =   6
         Left            =   135
         TabIndex        =   34
         Top             =   1170
         Width           =   195
      End
      Begin VB.Label lblPS 
         BorderStyle     =   1  'Fixed Single
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   480
         TabIndex        =   33
         Top             =   480
         Width           =   2085
      End
      Begin VB.Label lblPI 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   480
         TabIndex        =   32
         Top             =   1080
         Width           =   1650
      End
      Begin VB.Label lbl 
         AutoSize        =   -1  'True
         Caption         =   "PTY:"
         Height          =   195
         Index           =   8
         Left            =   2280
         TabIndex        =   6
         Top             =   1080
         Width           =   360
      End
      Begin VB.Label lblPTYCode 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   2760
         TabIndex        =   5
         Top             =   1065
         Width           =   420
      End
      Begin VB.Label lblPTYDescr 
         BorderStyle     =   1  'Fixed Single
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   15.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   420
         Left            =   3360
         TabIndex        =   4
         Top             =   1080
         Width           =   3105
      End
      Begin VB.Label lblTP 
         BorderStyle     =   1  'Fixed Single
         Caption         =   "TP"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00808080&
         Height          =   390
         Left            =   3360
         TabIndex        =   3
         Top             =   375
         Width           =   450
      End
      Begin VB.Label lblTA 
         BorderStyle     =   1  'Fixed Single
         Caption         =   "TA"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00808080&
         Height          =   390
         Left            =   3960
         TabIndex        =   2
         Top             =   360
         Width           =   450
      End
      Begin VB.Label lblMS 
         BorderStyle     =   1  'Fixed Single
         Caption         =   "MS"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   14.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00808080&
         Height          =   390
         Left            =   4560
         TabIndex        =   1
         Top             =   360
         Width           =   510
      End
   End
   Begin VB.Timer tmrUpdate 
      Enabled         =   0   'False
      Interval        =   100
      Left            =   11355
      Top             =   150
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim lGloRRegs(15)   As Long
Dim lGloFreq        As Long
Dim bGloRadioON     As Boolean
Dim bGloFreqChanged As Boolean
Dim bRet            As Boolean

Private Sub Check2_Click()
If (Check2.value = 1) Then
SetMute (True)
Else
SetMute (False)
End If
End Sub

Private Sub cmdOnOff_Click()

    If Not bGloRadioON Then
        bRet = HWInit()
        tmrUpdate.Enabled = True
        cmdOnOff.Caption = "Turn OFF"
        bGloRadioON = True
        bGloFreqChanged = True
        tmrUpdate_Timer
      Else
        bRet = HWDeInit()
        tmrUpdate.Enabled = False
        cmdOnOff.Caption = "Turn ON"
        bGloRadioON = False

        bGloFreqChanged = True
        tmrUpdate_Timer
    End If
        
End Sub

Private Sub cmdTune_Click(Index As Integer)
    Dim iCnt As Integer

    Select Case Index
        Case 0 ' < Tune
            lGloFreq = SeekStation(0)
        Case 1 ' < Freq
            If lGloFreq > 87500 Then
                lGloFreq = lGloFreq - 100
                bRet = FMTune(lGloFreq)
            End If
        Case 2 ' Freq >
            If lGloFreq < 108000 Then
                lGloFreq = lGloFreq + 100
                bRet = FMTune(lGloFreq)
            End If
        Case 3 ' Tune >
            lGloFreq = SeekStation(1)
        Case 4 'Change AF
            If lstAF.Text <> "" Then
                lGloFreq = lstAF.Text * 1000
                bRet = FMTune(lGloFreq)
            End If
    End Select
    
    
    bGloFreqChanged = True
   
    'Force Update
    tmrUpdate_Timer
    
        
End Sub


Public Function memset(ByRef byArray() As Byte, iAsc As Integer) As Boolean

    Dim lUbound As Long
    Dim lCount  As Long
    
    memset = False
    lUbound = UBound(byArray)
    
    For lCount = 0 To lUbound
        byArray(lCount) = CByte(iAsc)
    Next lCount
    
    memset = True

End Function

Private Sub Command1_Click()
FMTune (98800)
End Sub

Private Sub Form_Load()
    bGloRadioON = False
End Sub

Private Sub Form_Unload(Cancel As Integer)
    
    bRet = HWDeInit()

End Sub


Private Sub Label1_Click()

End Sub

Private Sub tmrUpdate_Timer()

    Dim iCnt As Integer
    Dim sTemp As String
    Static lLast1SecUpdate As Long
    Static lLast4SecUpdate As Long
        
    '---Always Upadated
    'PS
    'lblPS = RDSData.sPSName
    lblPS = WVB_GetRDSPS()
    
    'RadioText
    lblRadioTextA = WVB_GetRDSText()
    'lblRadioTextA = RDSData.sRadioTextA
    'lblRadioTextB = RDSData.sRadioTextB
    
    'PTY
    'lblPTYCode = RDSData.iPTYCode
    lblPTYCode = WVB_GetRDSPTY()
    lblPTYDescr = WVB_GetRDSPTYString()
    'lblPTYDescr = DecodePTY(WVB_GetRDSPTY())
   
    '---(every second or when frequence change)
    If (Timer - lLast1SecUpdate > 1) Or bGloFreqChanged Then
    '
    '    'TP
        lblTP.ForeColor = IIf(WVB_GetRDSTP, &HC000&, &H808080)
    '
    '    'TA
        lblTA.ForeColor = IIf(WVB_GetRDSTA, &HC000&, &H808080)
    '
    '    'MS
        lblMS.ForeColor = IIf(WVB_GetRDSMS, &HC000&, &H808080)
    '
    '    'DI
    '    lblDI.ForeColor = IIf(RDSData.bDI, &HC000&, &H808080)#
        lblPI = WVB_GetRDSPI
    '
        lLast1SecUpdate = Timer
    End If
  
    '---(every 4 seconds or when frequence change)
    If (Timer - lLast4SecUpdate > 4) Or bGloFreqChanged Then
    '
    '    'Alternate Frequencies
    '    Dim iTemp As Integer
    '    iTemp = lstAF.ListIndex
    '    lstAF.Clear
    '    For iCnt = 1 To UBound(RDSData.lAltFreq)
    '        If RDSData.lAltFreq(iCnt) <> 0 Then
    '            lstAF.AddItem Round(RDSData.lAltFreq(iCnt) / 1000, 2)
    '        End If
    '    Next iCnt
    '    If lstAF.ListCount >= iTemp + 1 Then
    '        lstAF.ListIndex = iTemp
    '    End If
    '
    '    'Group Detected
    '    For iCnt = 0 To 15
    '        picGroupA(iCnt).BackColor = IIf(RDSData.byGroupA(iCnt), &HFF00&, &H8000&)
    '        picGroupB(iCnt).BackColor = IIf(RDSData.byGroupB(iCnt), &HFF00&, &H8000&)
    '    Next iCnt

    '    'RSSI
        lblSignal = GetSignal()
        bpSignal.value = GetSignal()
    '
        'Stereo
        lblStereo.ForeColor = IIf(IsStereo(), &HC000&, &H808080)
    '
    '    'Time/UTC
    '    lblDate = RDSData.sDate
    '    lblTime = RDSData.sTime
    '    lblUTC = RDSData.sUTC
    '
    '    'PI: Country/Coverage
    '    lblCountry = RDSData.sCountryDesc
    '    lblCoverage = RDSData.sCoverDesc

        
    'RDSData.sCoverDesc = ""

        
        
        lLast4SecUpdate = Timer
    End If
    
    '---Only Once
    If bGloFreqChanged Then
        
        'Frequence
        lGloFreq = GetCurrStation()
        If lGloFreq = 0 Then
            lblFreq = "---,---"
          Else
            lblFreq = Round(lGloFreq / 1000, 2)
        End If
        
        'Date
    '    lblDate = RDSData.sDate
        
        bGloFreqChanged = False
        
    End If

    
End Sub
Public Function Hex2Bin(sHex As String, iReg As Integer) As String
    
    Dim iCnt As Integer
    Dim sRet As String
    
    For iCnt = 1 To Len(sHex)
        Select Case UCase(Mid(sHex, iCnt, 1))
            Case "0": sRet = sRet & "0000"
            Case "1": sRet = sRet & "0001"
            Case "2": sRet = sRet & "0010"
            Case "3": sRet = sRet & "0011"
            Case "4": sRet = sRet & "0100"
            Case "5": sRet = sRet & "0101"
            Case "6": sRet = sRet & "0110"
            Case "7": sRet = sRet & "0111"
            Case "8": sRet = sRet & "1000"
            Case "9": sRet = sRet & "1001"
            Case "A": sRet = sRet & "1010"
            Case "B": sRet = sRet & "1011"
            Case "C": sRet = sRet & "1100"
            Case "D": sRet = sRet & "1101"
            Case "E": sRet = sRet & "1110"
            Case "F": sRet = sRet & "1111"
        End Select
    Next iCnt
    
    'Split in the proper way according to the register meaning
    Select Case iReg
        Case 13:   sRet = Mid(sRet, 1, 4) & "-" & Mid(sRet, 5, 1) & "-" & Mid(sRet, 6, 1) & "-" & Mid(sRet, 7, 5) & "-" & Mid(sRet, 12)
        Case Else: sRet = Mid(sRet, 1, 4) & "-" & Mid(sRet, 5, 4) & "-" & Mid(sRet, 9, 4) & "-" & Mid(sRet, 13, 4)
    End Select
    
    Hex2Bin = sRet
End Function

Public Function DecodePTY(iCode As Integer)

    Select Case iCode
        Case 0: DecodePTY = ""
        Case 1: DecodePTY = "News"
        Case 2: DecodePTY = "Current Affairs"
        Case 3: DecodePTY = "Information"
        Case 4: DecodePTY = "Sport"
        Case 5: DecodePTY = "Education"
        Case 6: DecodePTY = "Drama"
        Case 7: DecodePTY = "Culture"
        Case 8: DecodePTY = "Science"
        Case 9: DecodePTY = "Varied"
        Case 10: DecodePTY = "Pop Music"
        Case 11: DecodePTY = "Rock Music"
        Case 12: DecodePTY = "Easy Listening"
        Case 13: DecodePTY = "Light Classic Music"
        Case 14: DecodePTY = "Serious Classic Music"
        Case 15: DecodePTY = "Other Music"
        Case 16: DecodePTY = "Weather & Metr"
        Case 17: DecodePTY = "Finance"
        Case 18: DecodePTY = "Children's Progs"
        Case 19: DecodePTY = "Social Affairs"
        Case 20: DecodePTY = "Religion"
        Case 21: DecodePTY = "Phone In"
        Case 22: DecodePTY = "Travel & Touring"
        Case 23: DecodePTY = "Leisure & Hobby"
        Case 24: DecodePTY = "Jazz Music"
        Case 25: DecodePTY = "Country Music"
        Case 26: DecodePTY = "National Music"
        Case 27: DecodePTY = "Oldies Music"
        Case 28: DecodePTY = "Folk Music"
        Case 29: DecodePTY = "Documentary"
        Case 30: DecodePTY = "Alarm Test"
        Case 31: DecodePTY = "Alarm - Alarm !"
    End Select
    
End Function
Public Function DecodePICountryCode(iCode As Integer)

    Select Case iCode
        Case 0: DecodePICountryCode = ""
        Case 1: DecodePICountryCode = "News"
        Case 2: DecodePICountryCode = "Current Affairs"
        Case 3: DecodePICountryCode = "Information"
        Case 4: DecodePICountryCode = "Sport"
        Case 5: DecodePICountryCode = "Education"
        Case 6: DecodePICountryCode = "Drama"
        Case 7: DecodePICountryCode = "Culture"
        Case 8: DecodePICountryCode = "Science"
        Case 9: DecodePICountryCode = "Varied"
        Case 10: DecodePICountryCode = "Pop Music"
        Case 11: DecodePICountryCode = "Rock Music"
        Case 12: DecodePICountryCode = "Easy Listening"
        Case 13: DecodePICountryCode = "Light Classic Music"
        Case 14: DecodePICountryCode = "Serious Classic Music"
        Case 15: DecodePICountryCode = "Other Music"
        Case 16: DecodePICountryCode = "Weather & Metr"
        Case 17: DecodePICountryCode = "Finance"
        Case 18: DecodePICountryCode = "Children's Progs"
        Case 19: DecodePICountryCode = "Social Affairs"
        Case 20: DecodePICountryCode = "Religion"
        Case 21: DecodePICountryCode = "Phone In"
        Case 22: DecodePICountryCode = "Travel & Touring"
        Case 23: DecodePICountryCode = "Leisure & Hobby"
        Case 24: DecodePICountryCode = "Jazz Music"
        Case 25: DecodePICountryCode = "Country Music"
        Case 26: DecodePICountryCode = "National Music"
        Case 27: DecodePICountryCode = "Oldies Music"
        Case 28: DecodePICountryCode = "Folk Music"
        Case 29: DecodePICountryCode = "Documentary"
        Case 30: DecodePICountryCode = "Alarm Test"
        Case 31: DecodePICountryCode = "Alarm - Alarm !"
    End Select
    
End Function


