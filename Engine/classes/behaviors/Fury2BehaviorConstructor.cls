VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Fury2BehaviorConstructor"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Description = "Fury� Behavior Constructor Object"
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

Public Function Translate(Optional EndRect = Nothing, Optional Speed = 1 / 60, Optional EndEvent = "") As Fury2TranslateBehavior
On Error Resume Next
    Set Translate = New Fury2TranslateBehavior
    With Translate
        Set .EndRectangle = EndRect
        .Speed = CSng(Speed)
        .EndEvent = CStr(EndEvent)
    End With
End Function

Public Function Fade(Optional EndAlpha = 0, Optional Speed = 1 / 60, Optional EndEvent = "") As Fury2FadeBehavior
On Error Resume Next
    Set Fade = New Fury2FadeBehavior
    With Fade
        .EndAlpha = CSng(EndAlpha)
        .FadeSpeed = CSng(Speed)
        .EndEvent = CStr(EndEvent)
    End With
End Function

Public Function Script(Optional ScriptText = "") As Fury2ScriptBehavior
On Error Resume Next
    Set Script = New Fury2ScriptBehavior
    With Script
        .ScriptSource = CStr(ScriptText)
    End With
End Function


