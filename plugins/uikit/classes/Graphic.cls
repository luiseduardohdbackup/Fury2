VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Graphic"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit
Implements iWidget
Public Visible As Boolean
Public Parent As Object
Public EventHandler As Object
Public Name As String
Public Rectangle As Fury2Rect
Public Image As Object
Public BlitMode As SFXBlitModes
Private m_wdgMe As iWidget
Private m_engEngine As Fury2Engine

Public Property Get Engine() As Fury2Engine
    Set Engine = m_engEngine
End Property

Friend Sub SetEngine(NewEngine As Fury2Engine)
    Set m_engEngine = NewEngine
End Sub

Public Property Get iWidget_CanGetFocus() As Boolean
    iWidget_CanGetFocus = False
End Property

Public Function Class_ToString() As String
    Class_ToString = "Graphic " + Name
End Function

Private Sub DispatchEvent(EventName As String, ParamArray Parameters() As Variant)
On Error Resume Next
    mdlGlobal.Engine.ContextLevelAdd Name & "_" & EventName
    If EventHandler Is Nothing Then
    Else
        Select Case UBound(Parameters)
        Case 0
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0)
        Case 1
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1)
        Case 2
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2)
        Case 3
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2), Parameters(3)
        Case 4
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2), Parameters(3), Parameters(4)
        Case 5
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2), Parameters(3), Parameters(4), Parameters(5)
        Case 6
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2), Parameters(3), Parameters(4), Parameters(5), Parameters(6)
        Case 7
            CallByName EventHandler, Name & "_" & EventName, VbMethod, Parameters(0), Parameters(1), Parameters(2), Parameters(3), Parameters(4), Parameters(5), Parameters(6), Parameters(7)
        Case Else
            CallByName EventHandler, Name & "_" & EventName, VbMethod
        End Select
    End If
    mdlGlobal.Engine.ContextLevelRemove
End Sub

Public Property Get Widget() As iWidget
On Error Resume Next
    Set Widget = m_wdgMe
End Property

Private Sub Class_Initialize()
On Error Resume Next
    GraphicCount = GraphicCount + 1
    Name = "Graphic" & GraphicCount
    Set Rectangle = New Fury2Rect
    Visible = True
    BlitMode = BlitMode_Default
    Set m_wdgMe = Me
End Sub

Private Sub iWidget_LostCapture(ByVal Parent As iContainer)
On Error Resume Next
End Sub

Private Sub iWidget_GotFocus(ByVal Parent As iContainer, ByVal FromWidget As iWidget)
On Error Resume Next
End Sub

Private Function iWidget_HitTest(ByVal X As Long, ByVal Y As Long) As Boolean
On Error Resume Next
    If Me.Visible = False Then Exit Function
    iWidget_HitTest = Me.Rectangle.PointInside(X, Y)
End Function

Private Sub iWidget_KeyDown(ByVal Parent As iContainer, KeyCode As Long, ByVal Shift As Long)
On Error Resume Next
    DispatchEvent "KeyDown", KeyCode, Shift
End Sub

Private Sub iWidget_KeyPress(ByVal Parent As iContainer, Character As Long)
On Error Resume Next
    DispatchEvent "KeyPress", Character
End Sub

Private Sub iWidget_KeyUp(ByVal Parent As iContainer, KeyCode As Long, ByVal Shift As Long)
On Error Resume Next
    DispatchEvent "KeyUp", KeyCode, Shift
End Sub

Private Sub iWidget_LostFocus(ByVal Parent As iContainer, ByVal ToWidget As iWidget)
On Error Resume Next
End Sub

Private Sub iWidget_MouseDown(ByVal Parent As iContainer, ByVal Button As Long, ByVal Shift As Long, X As Long, Y As Long)
On Error Resume Next
    If Visible = False Then Exit Sub
    Parent.SetCapture Me
    With Rectangle
        DispatchEvent "MouseDown", Button, Shift, X - .Left, Y - .Top
    End With
End Sub

Private Sub iWidget_MouseEnter(ByVal Parent As iContainer)
On Error Resume Next
    DispatchEvent "MouseEnter"
End Sub

Private Sub iWidget_MouseLeave(ByVal Parent As iContainer)
On Error Resume Next
    DispatchEvent "MouseLeave"
End Sub

Private Sub iWidget_MouseMove(ByVal Parent As iContainer, ByVal Button As Long, ByVal Shift As Long, X As Long, Y As Long)
On Error Resume Next
    If Visible = False Then Exit Sub
    With Rectangle
        DispatchEvent "MouseMove", Button, Shift, X - .Left, Y - .Top
    End With
End Sub

Private Sub iWidget_MouseUp(ByVal Parent As iContainer, ByVal Button As Long, ByVal Shift As Long, X As Long, Y As Long)
On Error Resume Next
    If Visible = False Then Exit Sub
    Parent.ReleaseCapture Me
    With Rectangle
        DispatchEvent "MouseUp", Button, Shift, X - .Left, Y - .Top
    End With
End Sub

Private Sub iWidget_Redraw(ByVal Parent As iContainer, ByVal Surface As libGraphics.Fury2Image, ByVal XOffset As Long, ByVal YOffset As Long, ByVal Opacity As Single)
On Error Resume Next
Dim l_rctNew As Fury2Rect
Dim l_imgImage As Fury2Image
Dim l_gphImage As Fury2Graphic
    If Visible = False Then Exit Sub
    With Surface
        DispatchEvent "Redraw"
        Set l_rctNew = Rectangle.Copy.Translate(XOffset, YOffset)
        .PushClipRectangle l_rctNew
        If TypeOf Image Is Fury2Image Then
            Set l_imgImage = Image
            l_rctNew.Translate (Rectangle.Width - l_imgImage.Width) / 2, (Rectangle.Height - l_imgImage.Height) / 2
            If BlitMode = BlitMode_Default Then
                If Image.AlphaChannel Then
                    .Blit l_rctNew, , l_imgImage, Opacity, BlitMode_SourceAlpha
                Else
                    .Blit l_rctNew, , l_imgImage, Opacity, BlitMode_Matte
                End If
            Else
                .Blit l_rctNew, , l_imgImage, Opacity, BlitMode
            End If
        ElseIf TypeOf Image Is Fury2Graphic Then
            Set l_gphImage = Image
            l_gphImage.Draw Surface, l_rctNew.Left + (l_rctNew.Width / 2), l_rctNew.Top + (l_rctNew.Height / 2), Opacity
        End If
        .PopClipRectangle
    End With
End Sub
