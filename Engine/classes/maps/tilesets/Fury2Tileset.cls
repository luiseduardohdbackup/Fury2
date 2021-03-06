VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Fury2Tileset"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Description = "Fury� Tileset Object"
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
'
'    Engine (Fury� Game Creation System Runtime Engine)
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

Implements IInspectable
Implements Fury2Object
Implements IVirtualFileSaveable

Public Handle As Long
Attribute Handle.VB_VarDescription = "*Internal value"
Public Embed As Boolean
Private m_lngRowWidth As Long
Private m_booImageBased As Boolean
Private m_lyrParent As Fury2MapLayer
Private m_booOwnsTileset As Boolean
Private mvarTilesPerRow As Long
Private mvarTilesPerColumn As Long
Private mvarTileCount As Long
Private mvarFilename As String
Private mvarBuffer As Fury2Image
Private mvarTileWidth As Long
Private mvarTileHeight As Long
Private mvarMatteColor As Long
Private mvarHasAlpha As Boolean
Private mvarTransparentTile As Long
Private m_engEngine As Fury2Engine
Private m_colTileData As Fury2Collection

Public Function Add(ByVal NewTile As Fury2Image) As Long
Attribute Add.VB_Description = "Adds a copy of an image to the tileset as a tile."
On Error Resume Next
    If NewTile Is Nothing Then Exit Function
    If NewTile.Width <> TileWidth Then Exit Function
    If NewTile.Height <> TileHeight Then Exit Function
    If Handle <> 0 Then
        SoftFX.AddTile Handle, NewTile.Handle
        m_colTileData.Add Array()
        mvarTileCount = GetTileCount(Handle)
        Add = mvarTileCount
        Set mvarBuffer = Nothing
    End If
End Function

Public Sub AddTiles(ByVal Source As Fury2Image)
On Error Resume Next
Dim l_imgTiles() As Fury2Image
Dim l_lngIndex As Long
    If Handle <> 0 Then
        If Source Is Nothing Then Exit Sub
        If TileCount = 0 Then
            m_lngRowWidth = Source.Width \ TileWidth
        End If
        l_imgTiles = Source.Split(TileWidth, TileHeight)
        For l_lngIndex = LBound(l_imgTiles) To UBound(l_imgTiles)
            Add l_imgTiles(l_lngIndex)
        Next l_lngIndex
        Set mvarBuffer = Nothing
    End If
End Sub

Public Sub AllocateTileset(Optional ForceNew As Boolean = False)
Attribute AllocateTileset.VB_MemberFlags = "40"
On Error Resume Next
Dim l_lngLayers As Long
Dim l_lngIndex As Long
    If Handle <> 0 Then DeallocateTileset
    If Engine Is Nothing Then
    Else
        If Engine.EditMode Then ForceNew = True
    End If
    If ImageBased Then
        If mvarBuffer Is Nothing Then
            Load
        End If
        If mvarBuffer.Handle = 0 Then
            Exit Sub
        End If
        If Handle = 0 Then
            Handle = 0
            Handle = CLng(mvarBuffer.Tag(0))
            If mvarBuffer.Tag(2) = MatteColor Then
            Else
                Handle = 0
            End If
            Err.Clear
            If (Handle = 0) Or (ForceNew) Then
                Handle = SoftFX.AllocateTileset(mvarBuffer.Handle, mvarTileWidth, mvarTileHeight)
                If Handle = 0 Then
                Else
                    RefTileset Handle
                    mvarBuffer.Tag = Array(Handle, 1, MatteColor)
                End If
'                m_colTileData.Clear
'                For l_lngIndex = 1 To SoftFX.GetTileCount(Handle)
'                    m_colTileData.Add Array()
'                Next l_lngIndex
            Else
                mvarBuffer.Tag = Array(mvarBuffer.Tag(0), mvarBuffer.Tag(1) + 1, mvarBuffer.Tag(2))
                Handle = CLng(mvarBuffer.Tag(0))
                RefTileset Handle
'                m_colTileData.Clear
'                For l_lngIndex = 1 To SoftFX.GetTileCount(Handle)
'                    m_colTileData.Add Array()
'                Next l_lngIndex
            End If
        End If
    Else
        Handle = SoftFX.AllocateEmptyTileset(0, mvarTileWidth, mvarTileHeight)
        RefTileset Handle
    End If
End Sub

Public Property Set Buffer(vData As Fury2Image)
Attribute Buffer.VB_Description = "*The image containing the tileset's tiles."
On Error Resume Next
    Set mvarBuffer = vData
    AllocateTileset
End Property

Public Property Get Buffer() As Fury2Image
On Error Resume Next
    Set Buffer = mvarBuffer
End Property

Private Sub Class_Initialize()
On Error Resume Next
    mvarTileWidth = 16
    mvarTileHeight = 16
    mvarTransparentTile = -1
    mvarMatteColor = c_lngNullColor
    AllocateTileset
    m_booImageBased = False
End Sub

Private Sub Class_Terminate()
On Error Resume Next
    DeallocateTileset
End Sub

Public Sub Clear()
On Error Resume Next
Dim l_lngWidth As Long, l_lngHeight As Long
    l_lngWidth = TileWidth
    l_lngHeight = TileHeight
    RemoveAll
    DeallocateTileset
    Handle = SoftFX.AllocateEmptyTileset(0, l_lngWidth, l_lngHeight)
End Sub

Public Sub DeallocateTileset()
Attribute DeallocateTileset.VB_MemberFlags = "40"
On Error Resume Next
    If (Handle <> 0) Then
        SoftFX.DerefTileset Handle
        If mvarBuffer Is Nothing Then
        Else
            If CLng(mvarBuffer.Tag(1)) <= 1 Then
                mvarBuffer.Tag = Array(0, 0)
            Else
                mvarBuffer.Tag = Array(mvarBuffer.Tag(0), mvarBuffer.Tag(1) - 1)
            End If
        End If
        Handle = 0
    End If
End Sub

Public Function Duplicate() As Fury2Tileset
Attribute Duplicate.VB_Description = "Creates a copy of the tileset."
On Error Resume Next
Dim m_tstNew As Fury2Tileset
Dim l_lngTiles As Long
    Set m_tstNew = New Fury2Tileset
    With m_tstNew
        .SetEngine Engine
        .HasAlpha = mvarHasAlpha
        .MatteColor = MatteColor
        .TransparentTile = mvarTransparentTile
        .Embed = Embed
        .Filename = Filename
        .TileWidth = TileWidth
        .TileHeight = TileHeight
        If Embed Then
            .AllocateTileset
            For l_lngTiles = 0 To TileCount - 1
                AddTile .Handle, GetTile(Handle, l_lngTiles)
            Next l_lngTiles
        ElseIf ImageBased Then
            .Load
        Else
            .Handle = Handle
            RefTileset Handle
        End If
        Set .TileDataCollection = m_colTileData.Duplicate
    End With
    Set Duplicate = m_tstNew
    Set m_tstNew = Nothing
End Function

Public Property Get Engine() As Fury2Engine
    Set Engine = m_engEngine
End Property

Public Property Let Filename(ByVal vData As String)
Attribute Filename.VB_Description = "The name of the image the tileset's tiles will be loaded from.{ImagePath}"
On Error Resume Next
    If mvarFilename = vData Then Exit Property
    mvarFilename = vData
    Reload
End Property

Public Property Get Filename() As String
    Filename = mvarFilename
End Property

Public Function FindTile(ByVal Image As Fury2Image) As Long
On Error Resume Next
Dim l_lngIndex As Long
Dim l_lngTile As Long
Dim l_lngResult As Long
Dim l_lngCount As Long
    l_lngCount = GetTileCount(Handle)
    For l_lngIndex = 1 To l_lngCount
        l_lngTile = SoftFX.GetTile(Handle, l_lngIndex - 1)
        l_lngResult = SoftFX.Blit_Compare(Image.Handle, l_lngTile, Image.Rectangle.GetRectangle, 0, 0)
        If l_lngResult Then
            FindTile = l_lngIndex
            Exit For
        End If
    Next l_lngIndex
End Function

Sub Free()
On Error Resume Next
    DeallocateTileset
    Set m_lyrParent = Nothing
    Set m_engEngine = Nothing
    Set mvarBuffer = Nothing
    If Not (m_colTileData Is Nothing) Then m_colTileData.Clear
    Set m_colTileData = Nothing
End Sub

Sub InternalFree()
On Error Resume Next
    Handle = 0
    Set m_lyrParent = Nothing
    Set m_engEngine = Nothing
    Set mvarBuffer = Nothing
    m_colTileData.Clear
    Set m_colTileData = Nothing
End Sub

Public Function GenerateBuffer() As Fury2Image
On Error Resume Next
    Set GenerateBuffer = GetBuffer.Duplicate()
End Function

Public Function GetBuffer() As Fury2Image
On Error Resume Next
Dim l_imgTile As Fury2Image, l_rctDest As Fury2Rect
Dim l_lngX As Long, l_lngY As Long, l_lngIndex As Long
    If ImageBased Then
        Set GetBuffer = mvarBuffer
    Else
        If (mvarBuffer Is Nothing) Then
            m_lngRowWidth = ClipValue(m_lngRowWidth, 1, 512)
            Set GetBuffer = F2Image(TileWidth * m_lngRowWidth, TileHeight * Ceil(TileCount / m_lngRowWidth))
            GetBuffer.AlphaChannel = True
            Set l_imgTile = New Fury2Image
            l_imgTile.Deallocate
            Set l_rctDest = F2Rect(0, 0, TileWidth, TileHeight)
            For l_lngIndex = 0 To TileCount - 1
                l_rctDest.RelLeft = l_lngX
                l_rctDest.RelTop = l_lngY
                l_imgTile.SetHandle GetTile(Handle, l_lngIndex)
                GetBuffer.Blit l_rctDest, , l_imgTile
                l_lngX = l_lngX + TileWidth
                If ((l_lngX + TileWidth) > GetBuffer.Width) Then
                    l_lngX = 0
                    l_lngY = l_lngY + TileHeight
                End If
            Next l_lngIndex
            Set mvarBuffer = GetBuffer
        Else
            Set GetBuffer = mvarBuffer
        End If
    End If
End Function

Public Property Let HasAlpha(ByVal vData As Boolean)
Attribute HasAlpha.VB_Description = "This flag determines whether or not the tileset's alpha channel is enabled. (Deprecated)"
Attribute HasAlpha.VB_MemberFlags = "40"
    mvarHasAlpha = vData
End Property

' Deprecated
Public Property Get HasAlpha() As Boolean
    HasAlpha = True
End Property

Public Property Get Height() As Long
Attribute Height.VB_Description = "The height of the tileset's source image."
On Error Resume Next
    If ImageBased Then
    Else
        Height = mvarBuffer.Height
    End If
End Property

Public Property Get ImageBased() As Boolean
On Error Resume Next
    ImageBased = m_booImageBased
End Property

Private Sub IVirtualFileSaveable_Deserialize(File As VirtualFile)
On Error Resume Next
Dim l_strFilename As String
    mvarTransparentTile = -1
    m_booImageBased = True
    Embed = False
    Set mvarBuffer = Nothing
    With File
        .Progress_Begin 0.1
        .ReadSegment_Begin
            .Load mvarFilename
            .Load mvarTileWidth
            .Load mvarTileHeight
            .Load mvarMatteColor
            .Load mvarHasAlpha
            .Load mvarTileCount
            .Load mvarTilesPerRow
            .Load mvarTilesPerColumn
            If Not .EOS Then
                .Load mvarTransparentTile
            End If
            If Not .EOS Then
                .Load m_booImageBased
                .Load Embed
            End If
            .ReadSegment_Begin
                .Load m_lngRowWidth
            .ReadSegment_End
        .ReadSegment_End
        .Progress_End
        .Progress_Begin 0.9
        If Embed Then
            ReadTiles File
        Else
            Load
        End If
        .Progress_End
    End With
    Err.Clear
End Sub

Private Sub IVirtualFileSaveable_Serialize(File As VirtualFile)
On Error Resume Next
    With File
        .WriteSegment_Begin
            .Save mvarFilename
            .Save mvarTileWidth
            .Save mvarTileHeight
            .Save mvarMatteColor
            .Save mvarHasAlpha
            .Save mvarTileCount
            .Save mvarTilesPerRow
            .Save mvarTilesPerColumn
            .Save mvarTransparentTile
            .Save m_booImageBased
            .Save Embed
            .WriteSegment_Begin
                .Save m_lngRowWidth
            .WriteSegment_End
        .WriteSegment_End
        If Embed Then
            WriteTiles File
        End If
    End With
End Sub

Sub Load(Optional DetectMask As Boolean = False)
On Error Resume Next
Dim m_glbGlobals As Fury2Globals
Dim m_strDir As String
Dim l_lngIndex As Long
Dim l_booNew As Boolean
Dim l_strFilename As String
Static m_lngRecurse As Long
    If Trim(mvarFilename) = "" Then
        AllocateTileset
        m_booImageBased = False
        Exit Sub
    End If
    l_strFilename = LCase(Trim(Engine.Filesystem.AutoSelectExtension(Filename, "f2tileset")))
    If InStr(l_strFilename, ".f2tileset") Then
        m_booImageBased = False
        LoadTileset l_strFilename
        Exit Sub
    End If
    Engine.ContextLevelAdd "Tileset.Load(""" + CStr(mvarFilename) + """)"
    m_lngRecurse = m_lngRecurse + 1
    Set mvarBuffer = Nothing
    m_booImageBased = True
    If Engine.EditMode Then
        Set mvarBuffer = Engine.LoadImage(l_strFilename)
    Else
        l_booNew = False
        l_lngIndex = Engine.iLoadImage(l_strFilename, , l_booNew)
        Set mvarBuffer = Engine.Images(l_lngIndex)
    End If
    If mvarBuffer Is Nothing Then
        If Not Engine.EditMode Then Engine.CriticalError "Fury2Tileset.Load", "Sanity Check", "Tileset """ + mvarFilename + """ could not be loaded."
        m_lngRecurse = m_lngRecurse - 1
        Exit Sub
    End If
    If mvarBuffer.Width <= 0 Or mvarBuffer.Height <= 0 Then
        If Not Engine.EditMode Then Engine.CriticalError "Fury2Tileset.Load", "Sanity Check", "Tileset """ + mvarFilename + """ could not be loaded."
        m_lngRecurse = m_lngRecurse - 1
        Exit Sub
    End If
    m_lngRowWidth = mvarBuffer.Width \ TileWidth
    mvarBuffer.MatteColor = mvarMatteColor
    With mvarBuffer
        RecalculateSizes
        If DetectMask Then
            .MatteColor = .GetPixel(.Width - 1, .Height - 1)
            mvarMatteColor = .MatteColor
        End If
    End With
    AllocateTileset
    m_lngRecurse = m_lngRecurse - 1
    Engine.ContextLevelRemove
End Sub

Sub LoadTileset(ByVal Filename)
On Error Resume Next
Dim l_vfFile As VirtualFile
Dim l_tstTileset As Fury2Tileset
    If VarType(Filename) = vbString Then
        Set l_tstTileset = Engine.LoadTileset(Filename)
    Else
        Set l_vfFile = Filename
        If ValidateFileHeader(l_vfFile) Then
            Set l_tstTileset = New Fury2Tileset
            l_tstTileset.SetEngine Engine
            l_vfFile.Load l_tstTileset
        End If
    End If
    Set m_colTileData = l_tstTileset.TileDataCollection
    DeallocateTileset
    Handle = l_tstTileset.Handle
    RefTileset Handle
    RowWidth = l_tstTileset.RowWidth
    TransparentTile = l_tstTileset.TransparentTile
    Set l_tstTileset = Nothing
End Sub

Public Property Get MaskColor() As Long
Attribute MaskColor.VB_Description = "*"
Attribute MaskColor.VB_MemberFlags = "40"
On Error Resume Next
    MaskColor = mvarMatteColor
End Property

Public Property Let MaskColor(ByVal vData As Long)
On Error Resume Next
    mvarBuffer.MatteColor = vData
    mvarMatteColor = vData
End Property

Public Property Get MatteColor() As Long
Attribute MatteColor.VB_Description = "The color in the tileset which will be transparent.{Color}"
On Error Resume Next
    MatteColor = mvarMatteColor
End Property

Public Property Let MatteColor(ByVal vData As Long)
On Error Resume Next
    mvarBuffer.MatteColor = vData
    mvarMatteColor = vData
End Property

Friend Sub ReadTiles(ByVal File As VirtualFile)
On Error Resume Next
Dim l_lngCount As Long
Dim l_lngWidth As Long
Dim l_lngHeight As Long
Dim l_lngIndex As Long
Dim l_imgImage As Fury2Image
    Engine.ContextLevelAdd "Tileset.ReadTiles()"
    DeallocateTileset
    With File
        .ReadSegment_Begin
            .Progress_Begin 0.05
            .ReadSegment_Begin
                .Load l_lngCount
                .Load l_lngWidth
                .Load l_lngHeight
            .ReadSegment_End
            .Progress_End
            .Progress_Begin 0.1
            Handle = AllocateEmptyTileset(l_lngCount, l_lngWidth, l_lngHeight)
            .Progress_End
            .Progress_Begin 0.75
            .ReadSegment_Begin
                Set l_imgImage = New Fury2Image
                For l_lngIndex = 1 To l_lngCount
                    .Load l_imgImage
                    ReplaceTile Handle, l_lngIndex - 1, l_imgImage.Handle
                    .Progress_Set l_lngIndex / l_lngCount
                Next l_lngIndex
            .ReadSegment_End
            .Progress_End
            .Progress_Begin 0.1
            Dim l_varData As Variant
            .ReadSegment_Begin
                m_colTileData.Clear
                For l_lngIndex = 1 To l_lngCount
                    .ReadSegment_Begin
                        If (.EOS Or .EOF) Then
                            m_colTileData.Add Array()
                        Else
                            l_varData = Array()
                            l_varData = .LoadVariant
                            m_colTileData.Add l_varData
                        End If
                    .ReadSegment_End
                    .Progress_Set l_lngIndex / l_lngCount
                Next l_lngIndex
            .ReadSegment_End
            .Progress_End
        .ReadSegment_End
    End With
    Engine.ContextLevelRemove
End Sub

Public Sub RecalculateSizes()
Attribute RecalculateSizes.VB_Description = "Updates the TilesPerRow, TilesPerColumn, and TileCount values."
On Error Resume Next
    If ImageBased Then
        If mvarBuffer Is Nothing Then Exit Sub
        With mvarBuffer
            If mvarTileWidth >= .Width Then
                mvarTilesPerRow = 1
            Else
                mvarTilesPerRow = .Width \ mvarTileWidth
            End If
            If mvarTileHeight >= .Height Then
                mvarTilesPerColumn = 1
            Else
                mvarTilesPerColumn = .Height \ mvarTileHeight
            End If
            If mvarTileHeight >= .Height Then
                If mvarTileWidth >= .Width Then
                    mvarTileCount = 1
                Else
                    mvarTileCount = (.Width \ mvarTileWidth)
                End If
            Else
                If mvarTileWidth >= .Width Then
                    mvarTileCount = 1
                Else
                    mvarTileCount = (.Width \ mvarTileWidth) * (.Height \ mvarTileHeight)
                End If
            End If
        End With
    Else
        mvarTilesPerColumn = 1
        mvarTilesPerRow = 1
    End If
End Sub

Sub Reload()
Attribute Reload.VB_Description = "Reloads the tileset."
On Error Resume Next
    If Embed Then
    Else
        DeallocateTileset
        Set mvarBuffer = Nothing
        Load
    End If
End Sub

Public Sub Remove(ByVal Index As Long)
Attribute Remove.VB_Description = "Removes a tile from the tileset."
On Error Resume Next
    If Handle <> 0 Then
        m_colTileData.Remove Index
        SoftFX.RemoveTile Handle, Index - 1
        Set mvarBuffer = Nothing
    End If
End Sub

Public Sub RemoveAll()
On Error Resume Next
    If Handle <> 0 Then
        Do While SoftFX.GetTileCount(Handle) > 0
            SoftFX.RemoveTile Handle, 0
        Loop
        m_colTileData.Clear
        Set mvarBuffer = Nothing
    End If
End Sub

Public Sub Render(Surface As Fury2Image, Optional Area As Fury2Rect = Nothing, Optional Alpha As Single = 1, Optional XOffset As Long = 0, Optional YOffset As Long = 0)
On Error Resume Next
Dim l_lngX As Long, l_lngY As Long
Dim l_lngTile As Long, l_rctDest As Fury2Rect, l_rctSource As Fury2Rect
Dim l_imgHandle As Fury2Image
    If Handle = 0 Then Exit Sub
    If Surface Is Nothing Then Exit Sub
    If Area Is Nothing Then Set Area = Surface.Rectangle
    l_lngX = Area.left - XOffset
    l_lngY = Area.top - YOffset
    Set l_rctDest = New Fury2Rect
    Set l_rctSource = F2Rect(0, 0, mvarTileWidth, mvarTileHeight, False)
    For l_lngTile = 0 To mvarTileCount - 1
        If (l_lngY + mvarTileHeight) >= Area.top Then
            If (l_lngY < Area.bottom) Then
                l_rctDest.SetValues l_lngX, l_lngY, l_lngX + mvarTileWidth, l_lngY + mvarTileHeight
                l_imgHandle.SetHandle SoftFX.GetTile(Handle, l_lngTile)
                Surface.Blit l_rctDest, l_rctSource, l_imgHandle, Alpha, BlitMode_Normal
            End If
        End If
        l_lngX = l_lngX + mvarTileWidth
        If (l_lngX + mvarTileWidth) > Area.right Then
            l_lngX = Area.left - XOffset
            l_lngY = l_lngY + mvarTileHeight
        End If
        If l_lngY > Area.bottom Then
            Exit For
        End If
    Next l_lngTile
End Sub

Public Sub ResizeTileset(ByVal NewWidth As Long, ByVal NewHeight As Long)
On Error Resume Next
    DeallocateTileset
    Handle = AllocateEmptyTileset(0, NewWidth, NewHeight)
    Set mvarBuffer = Nothing
End Sub

Public Property Get RowWidth() As Long
    RowWidth = m_lngRowWidth
End Property

Public Property Let RowWidth(ByVal NewValue As Long)
    m_lngRowWidth = NewValue
End Property

Sub SaveTileset(ByVal Filename)
On Error Resume Next
Dim l_vfFile As VirtualFile
    If VarType(Filename) = vbString Then
        Set l_vfFile = F2File()
    Else
        Set l_vfFile = Filename
    End If
    l_vfFile.Save g_strHeader
    l_vfFile.Save Me
    If VarType(Filename) = vbString Then
        Engine.Filesystem.CreateFile(Filename).Save l_vfFile
    End If
End Sub

Friend Sub SetData(Width As Long, Height As Long, Filename As String, Buffer As Fury2Image)
On Error Resume Next
    mvarTileWidth = Width
    mvarTileHeight = Height
    mvarFilename = Filename
    Set mvarBuffer = Buffer
    RecalculateSizes
End Sub

Friend Sub SetEngine(Engine As Fury2Engine)
On Error Resume Next
    Set m_engEngine = Engine
    Set m_colTileData = m_engEngine.F2Collection()
End Sub

Friend Sub SetParent(Parent As Fury2MapLayer)
On Error Resume Next
    Set m_lyrParent = Parent
End Sub

Public Property Get Tile(ByVal Index As Long) As Fury2Image
On Error Resume Next
    If Handle <> 0 Then
        Set Tile = New Fury2Image
        Tile.Deallocate
        Tile.SetHandle SoftFX.GetTile(Handle, Index - 1)
    End If
End Property

Public Property Get TileCount() As Long
Attribute TileCount.VB_Description = "The number of tiles in the tileset."
On Error Resume Next
    If ImageBased Then
        TileCount = mvarTileCount
    Else
        TileCount = GetTileCount(Handle)
    End If
End Property

Public Function TileHasAttribute(ByVal Index As Long, ByRef TheAttribute As Variant) As Boolean
On Error Resume Next
Dim l_varData As Variant
Dim l_lngIndex As Long
Dim l_lngCount As Long
    l_varData = m_colTileData(Index + 1)
    Err.Clear
    l_lngCount = UBound(l_varData) - LBound(l_varData) + 1
    If Err = 0 Then
        For l_lngIndex = LBound(l_varData) To UBound(l_varData)
            If l_varData(l_lngIndex) = TheAttribute Then
                TileHasAttribute = True
                Exit Function
            End If
        Next l_lngIndex
    Else
        Exit Function
    End If
End Function

Public Property Get TileData(ByVal Index As Long) As Variant
On Error Resume Next
Dim l_varData As Variant
    l_varData = m_colTileData(Index + 1)
    TileData = l_varData
End Property

Public Property Let TileData(ByVal Index As Long, ByVal NewData As Variant)
On Error Resume Next
    m_colTileData.Add NewData, , Index + 1
    m_colTileData.Remove Index + 2
End Property

Friend Property Get TileDataCollection() As Fury2Collection
    Set TileDataCollection = m_colTileData
End Property

Friend Property Set TileDataCollection(ByRef NewData As Fury2Collection)
    Set m_colTileData = NewData
End Property

Public Property Let TileHeight(ByVal vData As Long)
Attribute TileHeight.VB_Description = "The height of a tile."
On Error Resume Next
    mvarTileHeight = ClipValue(vData, 2, 10240)
    If ImageBased Then
        RecalculateSizes
        DeallocateTileset
        AllocateTileset True
    Else
        ResizeTileset TileWidth, mvarTileHeight
    End If
End Property

Public Property Get TileHeight() As Long
On Error Resume Next
    If ImageBased Then
        TileHeight = mvarTileHeight
    Else
        If Handle <> 0 Then TileHeight = GetTileHeight(Handle)
    End If
End Property

Public Property Get TilesPerColumn() As Long
Attribute TilesPerColumn.VB_Description = "The number of rows in the tileset."
    TilesPerColumn = mvarTilesPerColumn
End Property

Public Property Get TilesPerRow() As Long
Attribute TilesPerRow.VB_Description = "The number of columns in the tileset."
    TilesPerRow = mvarTilesPerRow
End Property

Public Property Let TileWidth(ByVal vData As Long)
Attribute TileWidth.VB_Description = "The width of a tile."
On Error Resume Next
    mvarTileWidth = ClipValue(vData, 2, 10240)
    If ImageBased Then
        RecalculateSizes
        DeallocateTileset
        AllocateTileset True
    Else
        ResizeTileset mvarTileWidth, TileHeight
    End If
End Property

Public Property Get TileWidth() As Long
On Error Resume Next
    If ImageBased Then
        TileWidth = mvarTileWidth
    Else
        If Handle <> 0 Then TileWidth = GetTileWidth(Handle)
    End If
End Property

Public Property Let TransparentTile(ByVal vData As Long)
Attribute TransparentTile.VB_Description = "The tile index in this tileset which will be invisible."
    mvarTransparentTile = vData
End Property

Public Property Get TransparentTile() As Long
    TransparentTile = mvarTransparentTile
End Property

Public Property Get Width() As Long
Attribute Width.VB_Description = "The width of the tileset's source image."
On Error Resume Next
    If ImageBased Then
    Else
        Width = mvarBuffer.Width
    End If
End Property

Friend Sub WriteTiles(ByVal File As VirtualFile)
On Error Resume Next
Dim l_lngCount As Long
Dim l_lngWidth As Long
Dim l_lngHeight As Long
Dim l_lngIndex As Long
    Engine.ContextLevelAdd "Tileset.WriteTiles()"
    With File
        .WriteSegment_Begin
            l_lngCount = GetTileCount(Handle)
            l_lngWidth = GetTileWidth(Handle)
            l_lngHeight = GetTileHeight(Handle)
            .WriteSegment_Begin
                .Save l_lngCount
                .Save l_lngWidth
                .Save l_lngHeight
            .WriteSegment_End
            .WriteSegment_Begin
                For l_lngIndex = 1 To l_lngCount
                    .Save Tile(l_lngIndex)
                Next l_lngIndex
            .WriteSegment_End
            Dim l_varData As Variant
            .WriteSegment_Begin
                For l_lngIndex = 1 To l_lngCount
                    .WriteSegment_Begin
                        .SaveVariant m_colTileData(l_lngIndex)
                    .WriteSegment_End
                Next l_lngIndex
            .WriteSegment_End
        .WriteSegment_End
    End With
    Engine.ContextLevelRemove
End Sub

