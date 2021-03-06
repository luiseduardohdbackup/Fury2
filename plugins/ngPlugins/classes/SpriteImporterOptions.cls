VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "SpriteImporterOptions"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
'
'    ngPlugins (Fury� Game Creation System Next-Generation Editor Standard Plugin Set)
'    Copyright (C) 2003 Kevin Gadd
'
'    This library is free software; you can redistribute it and/or
'    modify it under the terms of the GNU Lesser General Public
'    License as published by the Free Software Foundation; either
'    version 2.1 of the License, or (at your option) any later version.
'
'    This library is distributed in the hope that it will be useful,
'    but WITHOUT ANY WARRANTY; without even the implied warranty of
'    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
'    Lesser General Public License for more details.
'
'    You should have received a copy of the GNU Lesser General Public
'    License along with this library; if not, write to the Free Software
'    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'

Option Explicit
Implements IVirtualFileSaveable
Implements IInspectable
Private mvarMarginLeft As Long
Private mvarMarginTop As Long
Private mvarMarginRight As Long
Private mvarMarginBottom As Long
Private mvarCellWidth As Long
Private mvarCellHeight As Long
Private mvarAlignmentX As Single
Private mvarAlignmentY As Single
Private mvarFrameDelay As Long
Private mvarImage As String
Private mvarMatteColor As Long
Private mvarDuplicateFirstFrame As Boolean
Private mvarMultiRowPoses As Boolean
Private mvarGridWidth As Long
Private mvarGridHeight As Long
Private mvarMaxPoses As Long
Private mvarMaxFrames As Long
Private mvarImageWidth As Long
Private mvarImageHeight As Long
Private mvarEngine As Fury2Engine

Friend Sub SetEngine(ByRef Engine As Fury2Engine)
On Error Resume Next
    Set mvarEngine = Engine
    Image = Image
End Sub

Public Property Let MaxPoses(ByVal vData As Long)
Attribute MaxPoses.VB_Description = "The maximum number of poses to import. If the image contains more poses they will be omitted."
    mvarMaxPoses = vData
End Property

Public Property Get MaxPoses() As Long
    MaxPoses = mvarMaxPoses
End Property

Public Property Let MaxFrames(ByVal vData As Long)
Attribute MaxFrames.VB_Description = "The maximum number of frames to import in a single pose. If a pose contains more frames, they will be omitted."
    mvarMaxFrames = vData
End Property

Public Property Get MaxFrames() As Long
    MaxFrames = mvarMaxFrames
End Property

Public Property Let GridHeight(ByVal vData As Long)
Attribute GridHeight.VB_Description = "The height of the gridlines between each cell."
    mvarGridHeight = vData
End Property

Public Property Get GridHeight() As Long
    GridHeight = mvarGridHeight
End Property

Public Property Let GridWidth(ByVal vData As Long)
Attribute GridWidth.VB_Description = "The width of the gridlines between each cell."
    mvarGridWidth = vData
End Property

Public Property Get GridWidth() As Long
    GridWidth = mvarGridWidth
End Property

Public Property Let MarginBottom(ByVal vData As Long)
Attribute MarginBottom.VB_Description = "The number of margin pixels on the bottom of the image."
    mvarMarginBottom = vData
End Property

Public Property Get MarginBottom() As Long
    MarginBottom = mvarMarginBottom
End Property

Public Property Let MarginRight(ByVal vData As Long)
Attribute MarginRight.VB_Description = "The number of margin pixels on the right of the image."
    mvarMarginRight = vData
End Property

Public Property Get MarginRight() As Long
    MarginRight = mvarMarginRight
End Property

Public Property Let MarginTop(ByVal vData As Long)
Attribute MarginTop.VB_Description = "The number of margin pixels on the top of the image."
    mvarMarginTop = vData
End Property

Public Property Get MarginTop() As Long
    MarginTop = mvarMarginTop
End Property

Public Property Let MarginLeft(ByVal vData As Long)
Attribute MarginLeft.VB_Description = "The number of margin pixels on the left of the image."
    mvarMarginLeft = vData
End Property

Public Property Get MarginLeft() As Long
    MarginLeft = mvarMarginLeft
End Property

Public Property Let DuplicateFirstFrame(ByVal vData As Boolean)
Attribute DuplicateFirstFrame.VB_Description = "Imports the first frame of each imported pose as both the stopped frame and the first animation frame."
    mvarDuplicateFirstFrame = vData
End Property

Public Property Get DuplicateFirstFrame() As Boolean
    DuplicateFirstFrame = mvarDuplicateFirstFrame
End Property

Public Property Let MultiRowPoses(ByVal vData As Boolean)
Attribute MultiRowPoses.VB_Description = "If True, the importer will insert as many frames as possible into a pose until either running out of frames or hitting MaxFrames. Otherwise, each row of the image is a seperate pose."
    mvarMultiRowPoses = vData
End Property

Public Property Get MultiRowPoses() As Boolean
    MultiRowPoses = mvarMultiRowPoses
End Property

Public Property Let MatteColor(ByVal vData As Long)
Attribute MatteColor.VB_Description = "The color that is shown as transparent in non-alpha-channel blit modes.{Color}"
    mvarMatteColor = vData
End Property

Public Property Get MatteColor() As Long
    MatteColor = mvarMatteColor
End Property

Public Property Let Image(ByVal vData As String)
Attribute Image.VB_Description = "The filename of the image.{ImagePath}"
On Error Resume Next
Dim l_imgImage As Fury2Image
    mvarImage = vData
    Set l_imgImage = mvarEngine.LoadImage(mvarImage, True)
    If (l_imgImage Is Nothing) Then
        mvarImageWidth = 0
        mvarImageHeight = 0
    Else
        mvarImageWidth = l_imgImage.Width
        mvarImageHeight = l_imgImage.Height
    End If
End Property

Public Property Get Image() As String
    Image = mvarImage
End Property

Public Property Get ImageSize() As String
Attribute ImageSize.VB_Description = "The size of the image."
On Error Resume Next
    ImageSize = mvarImageWidth & "x" & mvarImageHeight
End Property

Public Property Let CellHeight(ByVal vData As Long)
Attribute CellHeight.VB_Description = "The height of each cell in pixels, not including the grid."
    mvarCellHeight = vData
End Property

Public Property Get CellHeight() As Long
    CellHeight = mvarCellHeight
End Property

Public Property Let CellWidth(ByVal vData As Long)
Attribute CellWidth.VB_Description = "The width of each cell in pixels, not including the grid."
    mvarCellWidth = vData
End Property

Public Property Get CellWidth() As Long
    CellWidth = mvarCellWidth
End Property

Public Property Let CellsX(ByVal NewValue As Single)
Attribute CellsX.VB_Description = "The number of columns in the image."
On Error Resume Next
    mvarCellWidth = ((mvarImageWidth - mvarMarginLeft - mvarMarginRight) / NewValue) - mvarGridWidth
End Property

Public Property Get CellsX() As Single
On Error Resume Next
    CellsX = (mvarImageWidth - mvarMarginLeft - mvarMarginRight) / (mvarCellWidth + mvarGridWidth)
End Property

Public Property Let CellsY(ByVal NewValue As Single)
Attribute CellsY.VB_Description = "The number of rows in the image."
On Error Resume Next
    mvarCellHeight = ((mvarImageHeight - mvarMarginTop - mvarMarginBottom) / NewValue) - mvarGridHeight
End Property

Public Property Get CellsY() As Single
On Error Resume Next
    CellsY = (mvarImageHeight - mvarMarginTop - mvarMarginBottom) / (mvarCellHeight + mvarGridHeight)
End Property

Public Property Let AlignmentX(ByVal vData As Single)
Attribute AlignmentX.VB_Description = "The default horizontal alignment value for new frames."
    mvarAlignmentX = vData
End Property

Public Property Get AlignmentX() As Single
    AlignmentX = mvarAlignmentX
End Property

Public Property Let AlignmentY(ByVal vData As Single)
Attribute AlignmentY.VB_Description = "The default vertical alignment value for new frames."
    mvarAlignmentY = vData
End Property

Public Property Get AlignmentY() As Single
    AlignmentY = mvarAlignmentY
End Property

Public Property Let FrameDelay(ByVal vData As Long)
Attribute FrameDelay.VB_Description = "The default animation delay for new frames."
    mvarFrameDelay = vData
End Property

Public Property Get FrameDelay() As Long
    FrameDelay = mvarFrameDelay
End Property

Private Sub Class_Initialize()
    CellWidth = 16
    CellHeight = 32
    FrameDelay = 5
    AlignmentX = 8
    AlignmentY = 32
    MatteColor = 0
    MaxPoses = 999
    MaxFrames = 999
End Sub

Private Sub IVirtualFileSaveable_Deserialize(File As libGraphics.VirtualFile)
On Error Resume Next
    With File
        .Load mvarCellWidth
        .Load mvarCellHeight
        .Load mvarFrameDelay
        .Load mvarAlignmentX
        .Load mvarAlignmentY
        .Load mvarMatteColor
        .Load mvarImage
        .Load mvarDuplicateFirstFrame
        .Load mvarGridWidth
        .Load mvarGridHeight
        .Load mvarMarginLeft
        .Load mvarMarginTop
        .Load mvarMarginRight
        .Load mvarMarginBottom
        .Load mvarMaxPoses
        .Load mvarMaxFrames
        .Load mvarMultiRowPoses
        Image = Image
    End With
End Sub

Private Sub IVirtualFileSaveable_Serialize(File As libGraphics.VirtualFile)
On Error Resume Next
    With File
        .Save mvarCellWidth
        .Save mvarCellHeight
        .Save mvarFrameDelay
        .Save mvarAlignmentX
        .Save mvarAlignmentY
        .Save mvarMatteColor
        .Save mvarImage
        .Save mvarDuplicateFirstFrame
        .Save mvarGridWidth
        .Save mvarGridHeight
        .Save mvarMarginLeft
        .Save mvarMarginTop
        .Save mvarMarginRight
        .Save mvarMarginBottom
        .Save mvarMaxPoses
        .Save mvarMaxFrames
        .Save mvarMultiRowPoses
    End With
End Sub


