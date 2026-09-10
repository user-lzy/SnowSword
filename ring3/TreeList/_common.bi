'/*
' * Copyright (c) 2013 Martin Mitas
' *
' * This library is free software; you can redistribute it and/or modify it
' * under the terms of the GNU Lesser General Public License as published by
' * the Free Software Foundation; either version 2.1 of the License, or
' * (at your option) any later version.
' *
' * This library is distributed in the hope that it will be useful,
' * but WITHOUT ANY WARRANTY; without even the implied warranty of
' * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' * GNU Lesser General Public License for more details.
' *
' * You should have received a copy of the GNU Lesser General Public License
' * along with this library; if not, write to the Free Software Foundation,
' * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
' */

#include Once "windows.bi"
#include Once "win\oaidl.bi"
#define MCTRL_API stdcall
#inclib "windowscodecs" 
#inclib "hsluv-c" 
#inclib "md4c-utf16" 
'#ifdef UNICODE
'     #define MCTRL_NAME_AW(xxx)    xxx##W
'#else
'     #define MCTRL_NAME_AW(xxx)    xxx##A
'#endif

'Extern "C" 

'/** @brief Equivalent of @c CLR_NONE from @c <commctrl.bi". */
#define MC_CLR_NONE             Cast(COLORREF,&HFFFFFFFF)
'/** @brief Equivalent of @c CLR_DEFAULT from @c <commctrl.bi". */
#define MC_CLR_DEFAULT          Cast(COLORREF,&HFF000000)

'/** @brief Equivalent of @c I_IMAGECALLBACK from @c <commctrl.bi". */
#define MC_I_IMAGECALLBACK       -1
'/** @brief Equivalent of @c I_IMAGENONE from @c <commctrl.bi". */
#define MC_I_IMAGENONE           -2

'/** @brief Equivalent of @c I_CHILDRENCALLBACK from @c <commctrl.bi". */
#define MC_I_CHILDRENCALLBACK    -1

'/** @brief Equivalent of @c LPSTR_TEXTCALLBACKW from @c <commctrl.bi" (Unicode variant). */
#define MC_LPSTR_TEXTCALLBACKW   Cast(LPWSTR,-1)
'/** @brief Equivalent of @c LPSTR_TEXTCALLBACKA from @c <commctrl.bi" (ANSI variant). */
#define MC_LPSTR_TEXTCALLBACKA   Cast(LPSTR,-1)

'/*@}*/
'
'
'/**
' * @name Unicode Resolution
' */
'/*@{*/
'
'/** @brief Equivalent of @c LPSTR_TEXTCALLBACK from @c <commctrl.bi".
' *  @details Unicode-resolution alias.
' *  @sa MC_LPSTR_TEXTCALLBACKW MC_LPSTR_TEXTCALLBACKA */
#ifdef UNICODE
     #define MC_LPSTR_TEXTCALLBACK   MC_LPSTR_TEXTCALLBACKW
#else
     #define MC_LPSTR_TEXTCALLBACK   MC_LPSTR_TEXTCALLBACKA
#endif


'/*@}*/


'/**
' * @name Common Types
' */
'/*@{*/
'
'/**
' * @brief Equivalent of @c NMCUSTOMDRAWINFO from @c <commctrl.bi".
' *
' * For more detailed information refer to documentation of @c NMCUSTOMDRAWINFO
' * on MSDN.
' */
Type  MC_NMCUSTOMDRAW
   hdr As NMHDR
   dwDrawStage As DWORD
   hdc As HDC
   rc As RECT
   dwItemSpec As DWORD_PTR 
   uItemState As UINT
   lItemlParam As LPARAM
End Type 

Type MC_VERSION
   dwMajor As DWORD  '/** Major version number. */
   dwMinor As DWORD '/** Minor version number. */
   dwRelease As DWORD  '/** Release version number. */
End Type 
    
Extern "C"     
    Declare Sub mcVersion MCTRL_API  Lib "mCtrl" Alias "mcVersion"(lpVersion As MC_VERSION Ptr)
End Extern
'/*@}*/
'
'
'/**
' * @name Control Specific Message Ranges
' */
'/*@{*/

#define MC_EXM_FIRST        (WM_USER+&H4000 + 0)
#define MC_EXM_LAST         (WM_USER+&H4000 + 49)

#define MC_GM_FIRST         (WM_USER+&H4000 + 50)
#define MC_GM_LAST          (WM_USER+&H4000 + 199)

#define MC_HM_FIRST         (WM_USER+&H4000 + 200)
#define MC_HM_LAST          (WM_USER+&H4000 + 299)

#define MC_MTM_FIRST        (WM_USER+&H4000 + 300)
#define MC_MTM_LAST         (WM_USER+&H4000 + 399)

#define MC_MBM_FIRST        (WM_USER+&H4000 + 400)
#define MC_MBM_LAST         (WM_USER+&H4000 + 499)

#define MC_PVM_FIRST        (WM_USER+&H4000 + 500)
#define MC_PVM_LAST         (WM_USER+&H4000 + 599)

#define MC_CHM_FIRST        (WM_USER+&H4000 + 600)
#define MC_CHM_LAST         (WM_USER+&H4000 + 699)

#define MC_TLM_FIRST        (WM_USER+&H4000 + 700)
#define MC_TLM_LAST         (WM_USER+&H4000 + 799)

#define MC_IVM_FIRST        (WM_USER+&H4000 + 800)
#define MC_IVM_LAST         (WM_USER+&H4000 + 849)

'/*@}*/
'
'
'/**
' * @name Control Specific Notification Ranges
' */
'/*@{*/

#define MC_EXN_FIRST        (&H40000000 + 0)
#define MC_EXN_LAST         (&H40000000 + 49)

#define MC_GN_FIRST         (&H40000000 + 100)
#define MC_GN_LAST          (&H40000000 + 199)

#define MC_HN_FIRST         (&H40000000 + 200)
#define MC_HN_LAST          (&H40000000 + 299)

#define MC_MTN_FIRST        (&H40000000 + 300)
#define MC_MTN_LAST         (&H40000000 + 349)

#define MC_PVN_FIRST        (&H40000000 + 400)
#define MC_PVN_LAST         (&H40000000 + 499)

#define MC_CHN_FIRST        (&H40000000 + 500)
#define MC_CHN_LAST         (&H40000000 + 599)

#define MC_TLN_FIRST        (&H40000000 + 600)
#define MC_TLN_LAST         (&H40000000 + 699)

'/*@}*/


'#ifdef __cplusplus
'}  /* extern "C" */
'#endif
'
'#endif  /* MCTRL_COMMON_H */
