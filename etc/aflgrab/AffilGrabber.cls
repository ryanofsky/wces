VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "AffilGrabber"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Private WithEvents stcp As Dartsecure.SecureTcp
Attribute stcp.VB_VarHelpID = -1
Private stream As Dartsecure.DartStream

Private Sub stcp_Authenticate(ByVal RemoteCertificate As Dartsecure.ICertificate, ByVal TrustedRoot As Boolean, ByVal ValidDate As Boolean, ByVal ValidSignature As Boolean, Valid As Boolean)
  Valid = True
End Sub
Public Function retrieve(server As String, port As Long, page As String, uni As String, pw As String) As String
  Set stcp = New Dartsecure.SecureTcp
  Set stream = New DartStream
   
  Dim b64 As New Base64
  Dim auth As String
    
  If uni <> "" Or pw <> "" Then
    auth = "Authorization: Basic "
    b64.Init
    auth = auth + b64.Encode(uni + ":" + pw)
    auth = auth + vbCrLf
  Else
    auth = ""
  End If
   
  stcp.Connect server, port
  stcp.Send "GET " + page + " HTTP/1.0" + vbCrLf + auth + vbCrLf
  Do While stcp.State <> tcpClosed
    stcp.Receive stream
  Loop
  
  stream.Position = 0
  retrieve = stream.ReadString()

End Function


