VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Base64"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Option Explicit

Public icChopMask  As Integer   ' Constant 8-bit mask (Faster than using string constants)
Public icBitShift  As Integer   ' Constant shift mask (Faster than using string constants)
Public icStartMask As Integer   ' Initial mask value  (Faster than using string constants)
Public iRollOver   As Integer   ' Decoded Roll over value
Public iHighMask   As Integer   ' Mask high bits of each char
Public iShift      As Integer   ' Multiplier shift value
Public iLowShift   As Integer   ' Mask low bits of each char
Public szAlphabet  As String    ' Decode/Encode Lookup Table


Public szTemp      As String    ' Working string

Public Sub Init()

' Initialize 2nd encoding pass lookup dictionary
    szAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

' Initialize Constants
    icChopMask = 255
    icBitShift = 4
    icStartMask = &H10

' Initialize Masks
    iShift = icBitShift
    iLowShift = 0
    iRollOver = 0
    iHighMask = -1
End Sub

Public Function Decode(b64 As String) As String

' Create variables
    Dim iPtr       As Integer
    Dim iChar      As Integer
    Dim iCounter As Integer

' Check if empty decoded string.
' If Empty, return NUL and Generate error 254
    If Len(b64) = 0 Then
        Decode = ""
        Exit Function
    End If

' Initialize working string
    szTemp = ""

' Begin Decoding
    For iCounter = 1 To Len(b64)

' Get next alphabet
        iChar = Asc(Mid$(b64, iCounter, 1))

' Get Decoded value
        iPtr = InStr(szAlphabet, Chr$(iChar)) - 1

' Check if character is valid
        If iPtr >= 0 Then

' Char is valid, process it
            If iShift = icBitShift Then

' 1st char in block of 4, keep high part of character
                iRollOver = (iPtr * iShift) And icChopMask

' Reset masks for next character
                iHighMask = &H30
                iLowShift = icStartMask
                iShift = icStartMask

            Else

' Start saving decoded character
                szTemp = szTemp + Chr$(iRollOver Or ((iPtr And iHighMask) / iLowShift))

' Calculate next mask and shift values
                iRollOver = (iPtr * iShift) And icChopMask
                iShift = iShift * icBitShift
                iHighMask = (iHighMask \ icBitShift) Or &H30
                iLowShift = iLowShift / icBitShift

                If iShift > 256 Then
                    iShift = icBitShift
                    iLowShift = 0
                End If
            End If
        End If
    Next

' Concat last character if required
    If (iShift > icBitShift And iShift < 256) Then

' Character remaining in    iRollOver
'        If iEndOfText Then

' Last string to decode in file
 '           szTemp = szTemp + Chr$(iRollOver)
 '       End If
    End If

' Exit wth decoded string
    Decode = szTemp
End Function

Public Function Encode(data As String) As String

' Create variables
    Dim icLowFill  As Integer
    Dim iChar      As Integer
    Dim iLowMask   As Integer
    Dim iPtr       As Integer
    Dim iCounter As Integer


' Check if empty decoded string.
' If Empty, return NUL and Generate error 254
    If Len(data) = 0 Then
        Encode = ""
        Exit Function
    End If

' Initialize lookup dictionary and constants
    szAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

    icBitShift = 4
    icChopMask = 255
    icLowFill = 3


' Initialize Masks
    szTemp = ""

    iHighMask = &HFC
    iLowMask = &H3
    iShift = &H10
    iRollOver = 0

' Begin Encoding process
    For iCounter = 1 To Len(data)

' Fetch ascii character in decoded string
        iChar = Asc(Mid$(data, iCounter, 1))

' Calculate Alphabet lookup pointer
        iPtr = ((iChar And iHighMask) \ (iLowMask + 1)) Or iRollOver

' Roll bit patterns
        iRollOver = (iChar And iLowMask) * iShift

' Concatenate encoded character to working encoded string
        szTemp = szTemp + Mid$(szAlphabet, iPtr + 1, 1)

' Adjust masks
        iHighMask = (iHighMask * icBitShift) And icChopMask
        iLowMask = iLowMask * icBitShift + icLowFill
        iShift = iShift \ icBitShift

' If last character in block, concat last RollOver and
'   reset masks
        If iHighMask = 0 Then
            szTemp = szTemp + Mid$(szAlphabet, iRollOver + 1, 1)
            iRollOver = 0
            iHighMask = &HFC
            iLowMask = &H3
            iShift = &H10
        End If
    Next iCounter

' If RollOver remains, concat it to the working string
    If iShift < &H10 Then
        szTemp = szTemp + Mid$(szAlphabet, iRollOver + 1, 1)
    End If

' Pad encoded string with required '=' pad characters
   iPtr = (Len(szTemp) Mod 4)
   If iPtr Then szTemp = szTemp + String$(4 - iPtr, "=")

' Return encoded string
    Encode = szTemp
End Function
