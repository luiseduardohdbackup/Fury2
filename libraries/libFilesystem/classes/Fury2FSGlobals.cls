VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Fury2FSGlobals"
Attribute VB_GlobalNameSpace = True
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Public Type FileEnumerationEntry
    Filename As String
    Parent As Fury2FSModule
    Size As Long
    CreatedDate As Date
    ModifiedDate As Date
End Type

Public Type FolderEnumerationEntry
    Path As String
    Parent As Fury2FSModule
End Type

