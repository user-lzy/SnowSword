'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #define LVM_SETITEMHEIGHT &H101B '已经搬到定义文件处
' Dim Shared ListViewData() As String '已经搬到定义文件处

Public Function IsListViewItemSelected(ListView As Class_ListView, ByVal ItemIndex As Integer) As BOOLEAN
    Return SendMessage(ListView.hWnd, LVM_GETITEMSTATE, Cast(wParam, ItemIndex), Cast(lParam, LVIS_SELECTED)) And LVIS_SELECTED
End Function

'Public Sub RemoveListViewItem(ListView As Class_ListView, ByVal ItemIndex As Integer)
'    ReDim Pre
'End Sub

Public Sub RefreshListViewItem(ListView As Class_ListView)
    ReDim ListViewData(ListView.ItemCount - 1, ListView.ColumnCount - 1) As String
    For i As Integer = 0 To ListView.ItemCount - 1
        For j As Integer = 0 To ListView.ColumnCount - 1
            ListViewData(i, j) = ListView.GetItemText(i, j)
        Next
    Next
End Sub

Public Function FilterListViewItem(ListView As Class_ListView, ByVal SubStr As String) As Integer
    Dim ItemCount As Integer, NewIndex As Integer
    LockWindowUpdate ListView.hWnd
    ListView.DeleteAllItems
    For i As Integer = 0 To UBound(ListViewData, 1)
        For j As Integer = 0 To UBound(ListViewData, 2)
            If SubStr = "" Then Goto jmp1
            If InStr(ListViewData(i, j), SubStr) <> 0 Then
                'SetListViewItemHeight ListView, i, 0
                Goto jmp1
            End If
        Next
        Goto jmp
        jmp1:
        NewIndex = ListView.AddItem(ListViewData(i, 0))
        For j As Integer = 1 To UBound(ListViewData, 2)
            ListView.SetItemText NewIndex, j, ListViewData(i, j)
        Next
        ItemCount += 1
        jmp :
    Next
    LockWindowUpdate NULL
    Return ItemCount
End Function

Sub SetListViewItemBackColor(ListView As Class_ListView, ByVal ItemIndex As Integer, ByVal ItemColor As COLORREF)
    '设置背景色
    SendMessage ListView.hWnd, LVM_SETTEXTBKCOLOR, ItemIndex, ItemColor
End Sub

Function GetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long) As BOOLEAN
    Dim state As UINT = GetMenuState(Menu.hMenu, iditem, MF_BYCOMMAND)
    Return (state And MF_CHECKED)
End Function

Sub SetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long, ByVal state As BOOLEAN)
    CheckMenuItem Menu.HMENU, iditem, MF_BYCOMMAND Or IIf(state, MF_CHECKED, MF_UNCHECKED)
End Sub