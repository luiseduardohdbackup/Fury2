VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Sound"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Description = "Fury� Sound Handle"
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Option Explicit
Private m_strFilename As String
Private m_seEngine As SoundEngine
Private m_lngHandle As Long

Public Property Get Filename() As String
On Error Resume Next
    Filename = m_strFilename
End Property

Public Sub Free()
On Error Resume Next
    m_seEngine.Sounds.Remove m_seEngine.Find(Me)
    fmodex.FMOD_Sound_Release m_lngHandle
    m_lngHandle = 0
    Set m_seEngine = Nothing
    m_strFilename = ""
End Sub

Friend Sub Init(ByVal Parent As SoundEngine, ByVal Handle As Long, ByRef Filename As String)
On Error Resume Next
    Set m_seEngine = Parent
    m_lngHandle = Handle
    m_strFilename = Filename
End Sub

Public Property Get Parent() As SoundEngine
On Error Resume Next
    Set Parent = m_seEngine
End Property

Public Property Get Handle() As Long
On Error Resume Next
    Handle = m_lngHandle
End Property

Public Property Get LoopCount() As Long
On Error Resume Next
Dim l_lngValue As Long
    fmodex.FMOD_Sound_GetLoopCount m_lngHandle, l_lngValue
    LoopCount = l_lngValue
End Property

Public Property Let LoopCount(ByVal NewCount As Long)
On Error Resume Next
    fmodex.FMOD_Sound_SetLoopCount m_lngHandle, NewCount
End Property

Public Property Get LoopMode() As SELoopModes
On Error Resume Next
Dim l_lngValue As FMOD_MODE
    fmodex.FMOD_Sound_GetMode m_lngHandle, l_lngValue
    If (l_lngValue And FMOD_LOOP_OFF) = FMOD_LOOP_OFF Then
        LoopMode = LoopMode_None
    ElseIf (l_lngValue And FMOD_LOOP_NORMAL) = FMOD_LOOP_NORMAL Then
        LoopMode = LoopMode_Normal
    ElseIf (l_lngValue And FMOD_LOOP_BIDI) = FMOD_LOOP_BIDI Then
        LoopMode = LoopMode_Bidirectional
    End If
End Property

Public Property Let LoopMode(ByVal NewMode As SELoopModes)
On Error Resume Next
Dim l_lngValue As FMOD_MODE
    fmodex.FMOD_Sound_GetMode m_lngHandle, l_lngValue
    l_lngValue = l_lngValue And (Not (FMOD_LOOP_OFF Or FMOD_LOOP_NORMAL Or FMOD_LOOP_BIDI))
    Select Case NewMode
    Case LoopMode_None
        l_lngValue = l_lngValue Or FMOD_LOOP_OFF
    Case LoopMode_Normal
        l_lngValue = l_lngValue Or FMOD_LOOP_NORMAL
    Case LoopMode_Bidirectional
        l_lngValue = l_lngValue Or FMOD_LOOP_BIDI
    Case Else
    End Select
    fmodex.FMOD_Sound_SetMode m_lngHandle, l_lngValue
End Property

Public Property Get Volume() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    Volume = l_sngVolume
End Property

Public Property Get Frequency() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    Volume = l_sngFrequency
End Property

Public Property Get Pan() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    Pan = l_sngPan
End Property

Public Property Get Priority() As Long
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    Priority = l_lngPriority
End Property

Public Property Let Volume(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    fmodex.FMOD_Sound_SetDefaults m_lngHandle, l_sngFrequency, NewValue, l_sngPan, ByVal 0, l_lngPriority
End Property

Public Property Let Frequency(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    fmodex.FMOD_Sound_SetDefaults m_lngHandle, NewValue, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
End Property

Public Property Let Pan(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    fmodex.FMOD_Sound_SetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, NewValue, ByVal 0, l_lngPriority
End Property

Public Property Let Priority(ByVal NewValue As Long)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single, l_lngPriority As Long
    fmodex.FMOD_Sound_GetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, l_lngPriority
    fmodex.FMOD_Sound_SetDefaults m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan, ByVal 0, NewValue
End Property

Public Property Get VolumeVariation() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    VolumeVariation = l_sngVolume
End Property

Public Property Get FrequencyVariation() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    FrequencyVariation = l_sngFrequency
End Property

Public Property Get PanVariation() As Single
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    PanVariation = l_sngPan
End Property

Public Property Let VolumeVariation(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    fmodex.FMOD_Sound_SetVariations m_lngHandle, l_sngFrequency, NewValue, l_sngPan
End Property

Public Property Let FrequencyVariation(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    fmodex.FMOD_Sound_SetVariations m_lngHandle, NewValue, l_sngVolume, l_sngPan
End Property

Public Property Let PanVariation(ByVal NewValue As Single)
On Error Resume Next
Dim l_sngVolume As Single, l_sngFrequency As Single, l_sngPan As Single
    fmodex.FMOD_Sound_GetVariations m_lngHandle, l_sngFrequency, l_sngVolume, l_sngPan
    fmodex.FMOD_Sound_SetVariations m_lngHandle, l_sngFrequency, l_sngVolume, NewValue
End Property

Public Property Get Length() As Double
On Error Resume Next
Dim l_lngValue As Long
    fmodex.FMOD_Sound_GetLength m_lngHandle, l_lngValue, FMOD_TIMEUNIT_MS
    Length = CDbl(l_lngValue) / 1000#
End Property

Public Property Get name() As String
On Error Resume Next
Dim l_bytName() As Byte
Dim l_strName As String
    ReDim l_bytName(0 To 512)
    fmodex.FMOD_Sound_GetName m_lngHandle, l_bytName(0), 512
    l_strName = StrConv(l_bytName, vbUnicode)
    name = Left(l_strName, InStr(l_strName, Chr(0)) - 1)
End Property

Public Function Play(Optional ByVal Paused As Boolean = False) As Channel
On Error Resume Next
Dim l_lngChannel As Long
    Call fmodex.FMOD_System_PlaySound(m_seEngine.Handle, FMOD_CHANNEL_FREE, m_lngHandle, Paused, l_lngChannel)
    Set Play = m_seEngine.CreateChannel(l_lngChannel, Me)
End Function