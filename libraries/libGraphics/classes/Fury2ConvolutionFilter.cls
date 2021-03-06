VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Fury2ConvolutionFilter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit
Private m_sngWeights() As Single
Private m_fpFilter As ConvolutionFilter2

Private Sub Class_Initialize()
On Error Resume Next
    ReDim m_sngWeights(0 To 2, 0 To 2)
    With m_fpFilter
        .Width = 3
        .Height = 3
        .Divisor = 1
        .XOffset = -1
        .YOffset = -1
        .WeightPointer = VarPtr(m_sngWeights(0, 0))
        SetAll 0
        Weight(-.XOffset, -.YOffset) = 1
    End With
End Sub

Public Sub Resize(ByVal Width As Long, ByVal Height As Long)
On Error Resume Next
    If (Width Mod 2) <> 1 Then Exit Sub
    If (Height Mod 2) <> 1 Then Exit Sub
    ReDim m_sngWeights(0 To Width - 1, 0 To Height - 1)
    With m_fpFilter
        .Width = Width
        .Height = Height
        .XOffset = -((Width - 1) \ 2)
        .YOffset = -((Height - 1) \ 2)
        .WeightPointer = VarPtr(m_sngWeights(0, 0))
        SetAll 0
        Weight(-.XOffset, -.YOffset) = 1
    End With
End Sub

Public Property Get Width() As Long
    Width = m_fpFilter.Width
End Property

Public Property Get Height() As Long
    Height = m_fpFilter.Height
End Property

Public Sub SetValues(Values As Variant)
On Error Resume Next
Dim l_lngX As Long, l_lngY As Long, l_lngI As Long
    For l_lngY = LBound(m_sngWeights, 2) To UBound(m_sngWeights, 2)
        For l_lngX = LBound(m_sngWeights, 1) To UBound(m_sngWeights, 1)
            m_sngWeights(l_lngX, l_lngY) = CSng(Values(l_lngI))
            l_lngI = l_lngI + 1
        Next l_lngX
    Next l_lngY
End Sub

Public Sub SetAll(ByVal Value As Single)
On Error Resume Next
Dim l_lngX As Long, l_lngY As Long
    For l_lngY = LBound(m_sngWeights, 2) To UBound(m_sngWeights, 2)
        For l_lngX = LBound(m_sngWeights, 1) To UBound(m_sngWeights, 1)
            m_sngWeights(l_lngX, l_lngY) = Value
        Next l_lngX
    Next l_lngY
End Sub

Public Sub AdjustAll(ByVal Value As Single)
On Error Resume Next
Dim l_lngX As Long, l_lngY As Long
    For l_lngY = LBound(m_sngWeights, 2) To UBound(m_sngWeights, 2)
        For l_lngX = LBound(m_sngWeights, 1) To UBound(m_sngWeights, 1)
            m_sngWeights(l_lngX, l_lngY) = m_sngWeights(l_lngX, l_lngY) + Value
        Next l_lngX
    Next l_lngY
End Sub

Public Sub MultiplyAll(ByVal Value As Single)
On Error Resume Next
Dim l_lngX As Long, l_lngY As Long
    For l_lngY = LBound(m_sngWeights, 2) To UBound(m_sngWeights, 2)
        For l_lngX = LBound(m_sngWeights, 1) To UBound(m_sngWeights, 1)
            m_sngWeights(l_lngX, l_lngY) = m_sngWeights(l_lngX, l_lngY) * Value
        Next l_lngX
    Next l_lngY
End Sub

Public Property Get Weight(ByVal X As Long, ByVal Y As Long) As Single
On Error Resume Next
    If X < 0 Or Y < 0 Or X >= Width Or Y >= Width Then
        Weight = 0
    Else
        Weight = m_sngWeights(X, Y)
    End If
End Property

Public Property Let Weight(ByVal X As Long, ByVal Y As Long, ByVal NewValue As Single)
On Error Resume Next
    If X < 0 Or Y < 0 Or X >= Width Or Y >= Width Then Exit Property
    m_sngWeights(X, Y) = NewValue
End Property

Public Property Get Divisor() As Single
On Error Resume Next
    Divisor = m_fpFilter.Divisor
End Property

Public Property Let Divisor(NewValue As Single)
On Error Resume Next
    If NewValue = 0 Then Exit Property
    m_fpFilter.Divisor = NewValue
End Property

Public Property Get Offset() As Single
On Error Resume Next
    Offset = m_fpFilter.Offset
End Property

Public Property Let Offset(NewValue As Single)
On Error Resume Next
    If NewValue = 0 Then Exit Property
    m_fpFilter.Offset = NewValue
End Property

Public Function Filter(ByVal Destination As Fury2Image, ByVal Source As Fury2Image, Optional ByVal DestinationRectangle As Fury2Rect = Nothing, Optional ByVal SourceRectangle As Fury2Rect = Nothing, Optional ByVal RenderMode As SFXRenderModes = RenderMode_Normal, Optional Color As Long = 0) As Boolean
On Error Resume Next
Dim l_rcDestination As Rectangle, l_rcSource As Rectangle
Dim l_lngRenderer As Long
    If DestinationRectangle Is Nothing Then Set DestinationRectangle = Destination.Rectangle
    If SourceRectangle Is Nothing Then Set SourceRectangle = Source.Rectangle
    l_rcDestination = DestinationRectangle.GetRectangle
    l_rcSource = SourceRectangle.GetRectangle
    l_lngRenderer = Source.GetRenderer(RenderMode)
    SoftFX.Blit_Convolve Destination.Handle, Source.Handle, VarPtr(m_fpFilter), l_rcDestination, l_rcSource.Left, l_rcSource.Top, Color, l_lngRenderer
End Function

Public Function MaskFilter(ByVal Destination As Fury2Image, ByVal Source As Fury2Image, ByVal Mask As Fury2Image, Optional ByVal DestinationRectangle As Fury2Rect = Nothing, Optional ByVal SourceRectangle As Fury2Rect = Nothing, Optional ByVal MaskRectangle As Fury2Rect = Nothing, Optional ByVal RenderMode As SFXRenderModes = RenderMode_Normal, Optional Color As Long = 0) As Boolean
On Error Resume Next
Dim l_rcDestination As Rectangle, l_rcSource As Rectangle, l_rcMask As Rectangle
Dim l_lngRenderer As Long
    If DestinationRectangle Is Nothing Then Set DestinationRectangle = Destination.Rectangle
    If SourceRectangle Is Nothing Then Set SourceRectangle = Source.Rectangle
    If MaskRectangle Is Nothing Then Set MaskRectangle = Mask.Rectangle
    l_rcDestination = DestinationRectangle.GetRectangle
    l_rcSource = SourceRectangle.GetRectangle
    l_rcMask = MaskRectangle.GetRectangle
    l_lngRenderer = Source.GetRenderer(RenderMode)
    SoftFX.Blit_Mask_Convolve Destination.Handle, Source.Handle, Mask.Handle, VarPtr(m_fpFilter), l_rcDestination, l_rcSource.Left, l_rcSource.Top, l_rcMask.Left, l_rcMask.Top, Color, l_lngRenderer
End Function

