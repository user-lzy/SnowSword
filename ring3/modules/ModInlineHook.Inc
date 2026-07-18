'' ---- 新增：跳转链节点 ----
'Type InlineChainNode
'    FromVA    As ULongInt
'    ToVA      As ULongInt
'    JumpType  As String          ' 如 "jmp rel", "mov+jmp", "call [mem]", ...
'End Type
'
'' ---- 新增：链长度上限 ----
'#define MAX_INLINE_CHAIN 8
'
'' ---- 新增：分类状态枚举（可选） ----
'Enum INLINE_STATUS
'    STATUS_UNKNOWN = 0
'    STATUS_INLINE_HOOK
'    STATUS_WRAPPER
'    STATUS_FORWARD_STUB
'End Enum
'
'' ---- 扩展 InlineHookInfo ----
'Type InlineHookInfo
'    hookType    As String        ' 保留，现由 Classifier 填充（分类名）
'    proxyFunc   As ULongInt      ' 保留，最终跳转目标地址
'    Status      As INLINE_STATUS ' 新增，便于程序判断
'    HopCount    As Integer       ' 跳转次数
'    Chains(MAX_INLINE_CHAIN - 1) As InlineChainNode
'End Type
'
' 初始化 ud 对象
Declare Sub ud_init cdecl Lib "udis86" Alias "ud_init"(ByVal u As ud Ptr)
' 设置模式 (16/32/64)
Declare Sub ud_set_mode Cdecl Lib "udis86" Alias "ud_set_mode" (ByVal u As ud Ptr, ByVal mode As uint8_t)

' 设置程序计数器 (PC)
Declare Sub ud_set_pc Cdecl Lib "udis86" Alias "ud_set_pc" (ByVal u As ud Ptr, ByVal pc As uint64_t)

' 设置输入钩子 (自定义输入函数)
Declare Sub ud_set_input_hook cdecl Lib "udis86" Alias "ud_set_input_hook" (ByVal u As ud Ptr, ByVal hook As ud_input_hook_cb)

' 设置输入缓冲区
Declare Sub ud_set_input_buffer Cdecl Lib "udis86" Alias "ud_set_input_buffer" (ByVal u As ud Ptr, ByVal buffer As Const uint8_t Ptr, ByVal size As size_t)

' 从文件输入 (若不定义 __UD_STANDALONE__ 则可用)
#ifndef __UD_STANDALONE__
'Declare Sub ud_set_input_file Cdecl Lib "udis86" Alias "ud_set_input_file" (ByVal u As ud Ptr, ByVal f As FILE Ptr)
#endif

' 设置供应商 (Intel 或 AMD)
Declare Sub ud_set_vendor Cdecl Lib "udis86" Alias "ud_set_vendor" (ByVal u As ud Ptr, ByVal vendor As UInteger)

' 设置输出语法 (Intel/ATT)
Declare Sub ud_set_syntax cdecl Lib "udis86" Alias "ud_set_syntax" (ByVal u As ud Ptr, ByVal syntax As ud_syntax_cb)

' 跳过指定字节数
Declare Sub ud_input_skip Cdecl Lib "udis86" Alias "ud_input_skip" (ByVal u As ud Ptr, ByVal n As size_t)

' 检查输入是否结束
Declare Function ud_input_end Cdecl Lib "udis86" Alias "ud_input_end" (ByVal u As Const ud Ptr) As Integer

' 解码指令 (返回指令长度)
Declare Function ud_decode Cdecl Lib "udis86" Alias "ud_decode" (ByVal u As ud Ptr) As UInteger

' 反汇编指令 (返回指令长度，与 ud_decode 类似)
Declare Function ud_disassemble Cdecl Lib "udis86" Alias "ud_disassemble" (ByVal u As ud Ptr) As UInteger

' Intel 语法格式化
Declare Sub ud_translate_intel cdecl Lib "udis86" Alias "ud_translate_intel" (ByVal u As ud Ptr)

' ATT 语法格式化
Declare Sub ud_translate_att Cdecl Lib "udis86" Alias "ud_translate_att" (ByVal u As ud Ptr)

' 获取当前指令的汇编文本
Declare Function ud_insn_asm Cdecl Lib "udis86" Alias "ud_insn_asm" (ByVal u As Const ud Ptr) As Const ZString Ptr

' 获取当前指令的原始字节指针
Declare Function ud_insn_ptr Cdecl Lib "udis86" Alias "ud_insn_ptr" (ByVal u As Const ud Ptr) As Const uint8_t Ptr

' 获取当前指令的地址偏移
Declare Function ud_insn_off Cdecl Lib "udis86" Alias "ud_insn_off" (ByVal u As Const ud Ptr) As uint64_t

' 获取当前指令的十六进制字符串
Declare Function ud_insn_hex cdecl Lib "udis86" Alias "ud_insn_hex" (ByVal u As ud Ptr) As Const ZString Ptr

' 获取当前指令的长度
Declare Function ud_insn_len Cdecl Lib "udis86" Alias "ud_insn_len" (ByVal u As Const ud Ptr) As UInteger

' 获取指定索引的操作数
Declare Function ud_insn_opr Cdecl Lib "udis86" Alias "ud_insn_opr" (ByVal u As Const ud Ptr, ByVal n As UInteger) As Const ud_operand Ptr

' 判断操作数是否为段寄存器
Declare Function ud_opr_is_sreg Cdecl Lib "udis86" Alias "ud_opr_is_sreg" (ByVal opr As Const ud_operand Ptr) As Integer

' 判断操作数是否为通用寄存器
Declare Function ud_opr_is_gpr Cdecl Lib "udis86" Alias "ud_opr_is_gpr" (ByVal opr As Const ud_operand Ptr) As Integer

' 获取当前指令的助记符代码
Declare Function ud_insn_mnemonic Cdecl Lib "udis86" Alias "ud_insn_mnemonic" (ByVal u As Const ud Ptr) As ud_mnemonic_code

' 根据助记符代码获取字符串
Declare Function ud_lookup_mnemonic cdecl Lib "udis86" Alias "ud_lookup_mnemonic" (ByVal c As ud_mnemonic_code) As Const ZString Ptr

' 设置用户数据指针
Declare Sub ud_set_user_opaque_data Cdecl Lib "udis86" Alias "ud_set_user_opaque_data" (ByVal u As ud Ptr, ByVal opaque As Any Ptr)

' 获取用户数据指针
Declare Function ud_get_user_opaque_data Cdecl Lib "udis86" Alias "ud_get_user_opaque_data" (ByVal u As Const ud Ptr) As Any Ptr

' 设置汇编输出缓冲区 (自定义格式化)
Declare Sub ud_set_asm_buffer cdecl Lib "udis86" Alias "ud_set_asm_buffer" (ByVal u As ud Ptr, ByVal buf As ZString Ptr, ByVal SIZE As size_t)

' 设置符号解析回调 (用于地址->符号名转换)
Declare Sub ud_set_sym_resolver cdecl Lib "udis86" Alias "ud_set_sym_resolver"(ByVal u As ud Ptr, ByVal resolver As ud_sym_resolver_cb)
'
'' ---- Resolver：纯解析，不判断 Hook ----
'Function ResolveControlTransfer(img As RemotePeImageEx, funcVA As Any Ptr, _
'                                ByRef info As InlineHookInfo) As Boolean
'    Const MAX_HOPS = MAX_INLINE_CHAIN
'    Dim u As ud
'    Dim buf(63) As UByte
'    Dim curVA As ULongInt = Cast(ULongInt, funcVA)
'    Dim visited(0 To MAX_HOPS-1) As ULongInt
'    Dim hopCount As Integer = 0
'    Dim lastJumpType As String = ""
'
'    ud_init(@u)
'    ud_set_mode(@u, IIf(img.arch = PE_ARCH_X64, 64, 32))
'    ud_set_syntax(@u, UD_SYN_INTEL)
'
'    ' 初始化输出
'    info.HopCount = 0
'    info.proxyFunc = 0
'    ' Chain 无需清零，HopCount 控制有效长度
'
'    Do
'        ' 1. 循环检测
'        If hopCount >= MAX_HOPS Then
'            info.hopCount = hopCount
'            info.proxyFunc = curVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Finish", "HopCount=" & hopCount & " Final=0x" & WHex(info.proxyFunc)
'            Return True   ' 达到上限，视为有效链
'        End If
'
'        For i As Integer = 0 To hopCount-1
'            If visited(i) = curVA Then
'                info.HopCount = hopCount
'                info.proxyFunc = curVA
'                If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Finish", "HopCount=" & hopCount & " Final=0x" & WHex(info.proxyFunc)
'                Return True   ' 循环跳转，链终止
'            End If
'        Next
'        visited(hopCount) = curVA
'
'        ' 2. 读取指令
'        If Not ReadMem(img.hProc, Cast(Any Ptr, curVA), @buf(0), 64) Then
'            info.HopCount = hopCount
'            info.proxyFunc = curVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'            Return False   ' 读取失败，无法继续
'        End If
'
'        ud_set_pc(@u, curVA)
'        ud_set_input_buffer(@u, @buf(0), 64)
'
'        If ud_disassemble(@u) = 0 Then
'            info.HopCount = hopCount
'            info.proxyFunc = curVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'            Return False   ' 反汇编失败
'        End If
'
'        Dim insnLen As UInteger = ud_insn_len(@u)
'        Dim mnemonic As Integer = ud_insn_mnemonic(@u)
'        Dim opr0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
'        Dim oprType As Integer = IIf(opr0 <> NULL, opr0->typ, -1)
'        
'        Dim asmText As String = *ud_insn_asm(@u)
'
'        MyLog.PrintLog LOG_INFO, _
'            "Resolve", _
'            "Disasm", _
'            "VA=0x" & WHex(curVA) & _
'            " asm=" & asmText & _
'            " len=" & insnLen & _
'            " mnemonic=" & mnemonic
'
'        ' 3. 判断是否为跳转/调用指令
'        Dim isJump As Boolean = (mnemonic = UD_Ijmp)
'        Dim isCall As Boolean = (mnemonic = UD_Icall)
'        If Not (isJump Or isCall) Then
'            ' 非分支指令 → 链终止
'            info.HopCount = hopCount
'            info.proxyFunc = curVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Finish", "HopCount=" & hopCount & " Final=0x" & WHex(info.proxyFunc)
'            Return True
'        End If
'
'        Dim nextVA As ULongInt = 0
'        Dim jumpType As String = ""
'
'        ' 4. 解析目标地址
'        Select Case oprType
'        Case UD_OP_JIMM   ' 相对跳转 (rel8/rel16/rel32/rel64)
'            Dim disp As LongInt
'            Select Case opr0->size
'            Case 8  : disp = Cast(LongInt, opr0->lval.sbyte)
'            Case 16 : disp = Cast(LongInt, opr0->lval.sword)
'            Case 32 : disp = Cast(LongInt, opr0->lval.sdword)
'            Case 64 : disp = opr0->lval.sqword
'            Case Else
'                ' 回退解析
'                If insnLen >= 2 AndAlso buf(0) = &HEB Then
'                    disp = Cast(LongInt, Cast(Byte, buf(1)))
'                Else
'                    disp = Cast(LongInt, *Cast(Long Ptr, @buf(insnLen-4)))
'                End If
'            End Select
'            nextVA = Cast(ULongInt, Cast(LongInt, curVA) + Cast(LongInt, insnLen) + disp)
'            jumpType = IIf(isJump, "jmp rel", "call rel")
'
'        Case UD_OP_MEM   ' 内存间接跳转 (jmp [mem] / call [mem])
'            Dim ptrVA As ULongInt = 0
'            Dim memSize As Integer = IIf(img.arch = PE_ARCH_X64, 8, 4)
'            ' 简单处理：仅支持 [rip+disp32] 或 [disp32]
'            If insnLen >= 6 Then
'                Dim modrmOffset As Integer = 1
'                If img.arch = PE_ARCH_X64 AndAlso (buf(0) >= &H40 AndAlso buf(0) <= &H4F) Then
'                    modrmOffset = 2
'                End If
'                Dim modrm As UByte = buf(modrmOffset)
'                Dim modBits As UByte = (modrm Shr 6) And 3
'                Dim rmBits As UByte = modrm And 7
'                If modBits = 0 AndAlso rmBits = 5 Then   ' [disp32] 或 [rip+disp32]
'                    Dim disp32 As Long = *Cast(Long Ptr, @buf(insnLen-4))
'                    If img.arch = PE_ARCH_X64 Then
'                        ptrVA = curVA + insnLen + Cast(ULongInt, Cast(LongInt, disp32))
'                    Else
'                        ptrVA = Cast(ULongInt, Cast(ULong, disp32))
'                    End If
'                Else
'                    ' 其他寻址方式无法静态追踪，记录为 [mem]
'                    jumpType = IIf(isJump, "jmp [mem]", "call [mem]")
'                    info.HopCount = hopCount + 1
'                    info.proxyFunc = 0
'                    If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'                    Return False
'                End If
'            End If
'
'            If ptrVA <> 0 Then
'                Dim targetAddr As ULongInt = 0
'                If Not ReadMem(img.hProc, Cast(Any Ptr, ptrVA), @targetAddr, memSize) Then
'                    jumpType = IIf(isJump, "jmp [mem]", "call [mem]")
'                    info.hopCount = hopCount + 1
'                    info.proxyFunc = 0
'                    If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'                    Return False
'                End If
'                If img.arch = PE_ARCH_X86 Then targetAddr = Cast(ULongInt, Cast(ULong, targetAddr))
'                nextVA = targetAddr
'                jumpType = IIf(isJump, "jmp [mem]", "call [mem]")
'            End If
'
'        Case UD_OP_REG   ' 寄存器跳转 (jmp reg / call reg)
'            jumpType = IIf(isJump, "jmp reg", "call reg")
'            info.HopCount = hopCount + 1
'            info.proxyFunc = 0
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'            Return False   ' 无法追踪
'
'        Case Else
'            ' 未知操作数
'            info.HopCount = hopCount
'            info.proxyFunc = curVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Abort", "VA=0x" & WHex(curVA)
'            Return False
'        End Select
'
'        ' 5. 记录当前跳转
'        If hopCount < MAX_INLINE_CHAIN Then
'            info.Chains(hopCount).FromVA = curVA
'            info.Chains(hopCount).ToVA = nextVA
'            info.Chains(hopCount).jumpType = jumpType
'            
'            MyLog.PrintLog LOG_INFO, _
'                "Resolve", _
'                "Hop", _
'                "#" & hopCount & _
'                " " & jumpType & _
'                " From=0x" & WHex(curVA) & _
'                " To=0x" & WHex(nextVA)
'        End If
'        hopCount += 1
'        info.HopCount = hopCount
'
'        ' 6. 如果是 CALL 或无法获取 nextVA，链终止
'        If isCall OrElse nextVA = 0 Then
'            info.proxyFunc = nextVA
'            If curVA = &H7777BD90 Then MyLog.PrintLog LOG_INFO, "Resolve", "Finish", "HopCount=" & hopCount & " Final=0x" & WHex(info.proxyFunc)
'            Return True
'        End If
'
'        ' 7. 继续追踪 JMP
'        curVA = nextVA
'
'    Loop
'End Function
'
'' ---- Classifier：仅做 ARK 级分类，不涉及白名单 ----
'Function ClassifyControlTransfer(img As RemotePeImageEx, entryVA As ULongInt, ByRef info As InlineHookInfo) As String
'    Dim finalVA As ULongInt = info.proxyFunc
'    If finalVA = 0 Then
'        info.Status = STATUS_UNKNOWN
'        Return "Unknown"
'    End If
'
'    ' 获取最终地址所在模块
'    Dim modBase As ULongInt, modSize As ULongInt
'    Dim modPath As StringW = ""
'    If img.arch = PE_ARCH_X86 Then
'        modPath = GetModuleByVA32(img, finalVA, Cast(ULong, modBase), Cast(ULong, modSize))
'    Else
'        modPath = GetModuleByVA64(img, finalVA, modBase, modSize)
'    End If
'    
'    If modPath = "" Then
'        info.Status = STATUS_INLINE_HOOK
'        Return "InlineHook"
'    End If
'
'    ' 获取入口地址所在模块
'    Dim curModBase As ULongInt, curModSize As ULongInt
'    Dim curModPath As StringW = GetModuleByVA64(img, entryVA, curModBase, curModSize)
'
'    ' 比较模块路径（不区分大小写）
'    If UCase(modPath) = UCase(curModPath) Then
'        'info.Status = STATUS_FORWARD_STUB
'        'Return "ForwardStub"
'    Else
'        If InStr(UCase(modPath), "\SYSTEM32\") > 0 OrElse _
'           InStr(UCase(modPath), "\SYSWOW64\") > 0 Then
'            info.Status = STATUS_WRAPPER
'            Return "Wrapper"
'        Else
'            info.Status = STATUS_INLINE_HOOK
'            Return "InlineHook"
'        End If
'    End If
'End Function
'
'Function AnalyzeInlineHook(img As RemotePeImageEx, funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'    If Not ResolveControlTransfer(img, funcVA, info) Then
'        info.hookType = "Unknown"
'        info.Status = STATUS_UNKNOWN
'        Return False
'    End If
'
'    Dim classification As String = ClassifyControlTransfer(img, Cast(ULongInt, funcVA), info)
'    info.hookType = classification
'    Return True
'End Function
'
'Function GetInlineHookList(ByRef img As RemotePeImageEx, funcVA As Any Ptr, diskEntry As UByte Ptr, ByVal FuncPath As StringW, lvwHook As Class_ListView) As ULong
'    Dim textStart As ULongInt, textEnd As ULongInt
'    GetTextSectionRange img, textStart, textEnd
'    If funcVA < Cast(Any Ptr, textStart) OrElse funcVA >= Cast(Any Ptr, textEnd) Then
'        Return 0
'    End If
'
'    Dim cur(31) As UByte
'    If Not ReadMem(img.hProc, Cast(Any Ptr, Cast(ULongInt, funcVA)), @cur(0), 32) Then
'        MyLog.PrintLog LOG_WARN, "GetInlineHookList", "ReadMem", "1"
'        Return 0
'    End If
'
'    ' 调用重构后的 AnalyzeInlineHook
'    Dim info As InlineHookInfo
'
'    MyLog.PrintLog LOG_INFO, "InlineHook", "Analyze", _
'        FuncPath & " Entry=0x" & WHex(funcVA)
'
'    If AnalyzeInlineHook(img, funcVA, info) = False Then
'        MyLog.PrintLog LOG_INFO, "InlineHook", "Analyze", _
'            "Resolve FAILED"
'
'        Return 0
'    End If
'
'    ' 仅当分类为 InlineHook 时才添加
'    If info.hookType <> "InlineHook" Then Return 0
'
'    ' 构建显示字符串（原逻辑不变）
'    Dim curCode As StringW, oldCode As StringW
'    For i As Integer = 0 To 31
'        curCode &= WHex(cur(i), 2) & " "
'        oldCode &= WHex(diskEntry[i], 2) & " "
'    Next
'
'    Dim ProxyDescription As StringW
'    If info.proxyFunc <> 0 Then
'        ProxyDescription = GetProxyDescription(img, info.proxyFunc)
'    Else
'        ProxyDescription = "unknown"
'    End If
'
'    ' 添加 ListView 项，注意 Hook 类型直接显示分类（info.hookType）
'    AddItemColListEx lvwHook, 7,,, _
'        WStr("InlineHook"), _   ' 可直接显示 "InlineHook"
'        "0x" & WHex(funcVA), _
'        WStr(FuncPath), _
'        "0x" & WHex(info.proxyFunc), _
'        WStr(ProxyDescription), _
'        WStr(curCode), _
'        WStr(oldCode)
'
'    Return 1
'End Function

Type InlineHookInfo
    hookType    As String
    proxyFunc   As ULongInt
End Type

'Function AnalyzeInlineHook(img As RemotePeImageEx, funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'    Const MAX_HOPS = 8
'    info.hookType = ""
'    info.proxyFunc = 0
'
'    Dim u As ud
'    Dim buf(63) As UByte
'    Dim curVA As ULongInt = Cast(ULongInt, funcVA)
'    Dim visited(0 To MAX_HOPS) As ULongInt
'    Dim hopCount As Integer = 0
'
'    ud_init(@u)
'    ud_set_mode(@u, IIf(img.arch = PE_ARCH_X64, 64, 32))
'    ud_set_syntax(@u, UD_SYN_INTEL)
'
'    Do
'        hopCount += 1
'        If hopCount > MAX_HOPS Then
'            info.hookType = "LoopJump(MaxHops)"
'            info.proxyFunc = curVA
'            Return True
'        End If
'
'        For i As Integer = 1 To hopCount - 1
'            If visited(i) = curVA Then
'                info.hookType = "LoopJump"
'                info.proxyFunc = curVA
'                Return True
'            End If
'        Next
'        visited(hopCount) = curVA
'
'        If Not ReadMem(img.hProc, Cast(Any Ptr, curVA), @buf(0), 64) Then
'            Print "[AnalyzeInlineHook] ReadMem failed at 0x" & WHex(curVA)
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            Return False
'        End If
'        Dim readSize As Integer = 64
'
'        ud_set_pc(@u, curVA)
'        ud_set_input_buffer(@u, @buf(0), readSize)
'
'        If ud_disassemble(@u) = 0 Then
'            Print "[AnalyzeInlineHook] ud_disassemble failed at 0x" & WHex(curVA)
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            info.hookType = "unknown"
'            Return False
'        End If
'
'        Dim insnLen As UInteger = ud_insn_len(@u)
'        Dim mnemonic As Integer = ud_insn_mnemonic(@u)
'        Dim opr0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
'
'        ' --------------------------------------------------------
'        ' 1. 检测多指令跳板：MOV RAX, imm64 ; JMP RAX
'        '    48 B8 xx..xx FF E0  (10+2=12 字节)
'        ' --------------------------------------------------------
'        If img.arch = PE_ARCH_X64 AndAlso mnemonic = UD_Imov Then
'            If readSize >= 12 AndAlso buf(0) = &H48 AndAlso buf(1) = &HB8 Then
'                If buf(10) = &HFF AndAlso buf(11) = &HE0 Then
'                    Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))
'                    info.hookType = "mov rax+jmp rax"
'                    info.proxyFunc = imm64
'                    Return True
'                End If
'            End If
'            ' 可扩展：MOV R11, imm64 ; JMP R11  (49 BB ... 41 FF E3)
'            If readSize >= 13 AndAlso buf(0) = &H49 AndAlso buf(1) = &HBB Then
'                If buf(10) = &H41 AndAlso buf(11) = &HFF AndAlso buf(12) = &HE3 Then
'                    Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))
'                    info.hookType = "mov r11+jmp r11"
'                    info.proxyFunc = imm64
'                    Return True
'                End If
'            End If
'        End If
'
'        ' --------------------------------------------------------
'        ' 2. 检测 PUSH + RET 跳板
'        ' --------------------------------------------------------
'        If mnemonic = UD_Ipush Then
'            If readSize >= insnLen + 1 Then
'                Dim nextByte As UByte = buf(insnLen)
'                If nextByte = &HC3 OrElse nextByte = &HCB Then  ' RET / RETF
'                    Dim opr0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
'                    Dim immVal As ULongInt = 0
'                    If opr0 <> NULL AndAlso opr0->typ = UD_OP_IMM Then
'                        Select Case opr0->size
'                        Case 8
'                            immVal = Cast(ULongInt, Cast(Byte, opr0->lval.sbyte))
'                        Case 16
'                            immVal = Cast(ULongInt, Cast(Short, opr0->lval.sword))
'                        Case 32
'                            immVal = Cast(ULongInt, opr0->lval.udword)
'                        Case 64
'                            immVal = opr0->lval.uqword
'                        End Select
'                    Else
'                        ' 回退字节解析
'                        If buf(0) = &H6A Then
'                            immVal = Cast(ULongInt, Cast(Byte, buf(1)))
'                        ElseIf buf(0) = &H68 Then
'                            immVal = Cast(ULongInt, *Cast(ULong Ptr, @buf(1)))
'                        End If
'                    End If
'
'                    info.hookType = "push+ret"
'                    info.proxyFunc = immVal
'                    Return True
'                End If
'            End If
'
'            ' 不是 push+ret，终止
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            info.hookType = "unknown"
'            Return False
'        End If
'
'        ' --------------------------------------------------------
'        ' 3. RET / 非分支指令 → 链终止
'        ' --------------------------------------------------------
'        If mnemonic = UD_Iret OrElse mnemonic = UD_Iretf Then
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            info.hookType = "unknown"
'            Return False
'        End If
'
'        If mnemonic <> UD_Ijmp AndAlso mnemonic <> UD_Icall Then
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            info.hookType = "unknown"
'            Return False
'        End If
'
'        Dim isCall As Boolean = (mnemonic = UD_Icall)
'        Dim nextVA As ULongInt = 0
'        opr0 = ud_insn_opr(@u, 0)
'        Dim oprType As Integer = IIf(opr0 <> NULL, opr0->typ, -1)
'
'        ' --------------------------------------------------------
'        ' 4. 相对跳转 JMP/CALL rel8/rel16/rel32
'        ' --------------------------------------------------------
'        If oprType = UD_OP_JIMM Then
'            Dim disp As LongInt
'            Select Case opr0->size
'            Case 8
'                disp = Cast(LongInt, opr0->lval.sbyte)
'            Case 16
'                disp = Cast(LongInt, opr0->lval.sword)
'            Case 32
'                disp = Cast(LongInt, opr0->lval.sdword)
'            Case 64
'                disp = opr0->lval.sqword
'            Case Else
'                ' 回退字节解析（利用 udis86 给出的 insnLen）
'                If insnLen >= 2 AndAlso buf(0) = &HEB Then
'                    disp = Cast(LongInt, Cast(Byte, buf(insnLen - 1)))
'                Else
'                    disp = Cast(LongInt, *Cast(Long Ptr, @buf(insnLen - 4)))
'                End If
'            End Select
'
'            ' 关键：全部用有符号 LongInt 计算，再转回 ULongInt
'            nextVA = Cast(ULongInt, Cast(LongInt, curVA) + Cast(LongInt, insnLen) + disp)
'
'            If isCall Then
'                info.hookType = "call rel"
'                info.proxyFunc = nextVA
'                Return True   ' CALL 终止追踪
'            Else
'                If hopCount = 1 Then info.hookType = "jmp rel"
'                info.proxyFunc = nextVA
'                curVA = nextVA
'                Continue Do
'            End If
'
'        ' --------------------------------------------------------
'        ' 5. 内存间接跳转 JMP/CALL [mem]
'        ' --------------------------------------------------------
'        ElseIf oprType = UD_OP_MEM OrElse (opr0 = NULL AndAlso buf(0) = &HFF) Then
'            Dim ptrVA As ULongInt = 0
'            Dim memReadSize As Integer = IIf(img.arch = PE_ARCH_X64, 8, 4)
'
'            ' udis86 已给出完整指令长度（含前缀），disp32 永远在指令最后 4 字节
'            If insnLen >= 6 Then
'                Dim modrmOffset As Integer = 1
'                ' 跳过 x64 REX 前缀（40-4F）
'                If img.arch = PE_ARCH_X64 AndAlso (buf(0) >= &H40 AndAlso buf(0) <= &H4F) Then
'                    modrmOffset = 2
'                End If
'
'                Dim modrm As UByte = buf(modrmOffset)
'                Dim modBits As UByte = (modrm Shr 6) And 3
'                Dim rmBits As UByte = modrm And 7
'
'                ' Mod=00, R/M=101 → [disp32] (x86 绝对地址) 或 [RIP+disp32] (x64)
'                If modBits = 0 AndAlso rmBits = 5 Then
'                    Dim disp32 As Long = *Cast(Long Ptr, @buf(insnLen - 4))
'
'                    If img.arch = PE_ARCH_X64 Then
'                        ' x64: RIP-relative
'                        ptrVA = curVA + insnLen + Cast(ULongInt, Cast(LongInt, disp32))
'                    Else
'                        ' x86 / Wow64: disp32 是绝对虚拟地址
'                        ptrVA = Cast(ULongInt, Cast(ULong, disp32))
'                    End If
'                Else
'                    ' [reg]、[reg+reg*scale] 等其他寻址，无法静态追踪
'                    info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
'                    info.proxyFunc = 0
'                    Return True
'                End If
'            Else
'                info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
'                info.proxyFunc = 0
'                Return True
'            End If
'
'            ' 读取内存中的目标地址
'            If ptrVA <> 0 Then
'                Dim targetAddr As ULongInt = 0
'                If ReadMem(img.hProc, Cast(Any Ptr, ptrVA), @targetAddr, memReadSize) = 0 Then
'                    info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
'                    info.proxyFunc = 0
'                    Return True
'                End If
'
'                ' x86 下确保零扩展
'                If img.arch = PE_ARCH_X86 Then
'                    targetAddr = Cast(ULongInt, Cast(ULong, targetAddr))
'                End If
'
'                nextVA = targetAddr
'
'                If isCall Then
'                    info.hookType = "call [mem]"
'                    info.proxyFunc = nextVA
'                    Return True
'                Else
'                    If hopCount = 1 Then info.hookType = "jmp [mem]"
'                    info.proxyFunc = nextVA
'                    curVA = nextVA
'                    Continue Do
'                End If
'            End If
'
'        ' --------------------------------------------------------
'        ' 6. 寄存器跳转 JMP/CALL reg
'        ' --------------------------------------------------------
'        ElseIf oprType = UD_OP_REG OrElse (opr0 = NULL AndAlso buf(0) = &HFF AndAlso (buf(1) And &HC0) = &HC0) Then
'            info.hookType = IIf(isCall, "call reg", "jmp reg")
'            info.proxyFunc = 0
'            Return True
'
'        Else
'            If info.hookType <> "" Then
'                info.proxyFunc = curVA
'                Return True
'            End If
'            info.hookType = "unknown"
'            Return False
'        End If
'
'    Loop
'
'    Return False
'End Function
'
'Function IsMemoryDifferentFromDisk(curBytes() As UByte, diskBytes() As UByte, ByVal nLen As Integer) As Boolean
'    For i As Integer = 0 To nLen - 1
'        If curBytes(i) <> diskBytes(i) Then Return True
'    Next
'    Return False
'End Function
'
'' ============================================================
'' 配套改进：GetInlineHookList
'' 用 udis86 精确获取 Hook 指令长度，避免 16 字节全比较导致的误报
'' ============================================================
'Function GetInlineHookList(ByRef img As RemotePeImageEx, funcVA As Any Ptr, diskEntry As UByte Ptr, ByVal FuncPath As StringW, lvwHook As Class_ListView) As ULong
'    Dim textStart As ULongInt, textEnd As ULongInt
'    GetTextSectionRange img, textStart, textEnd
'    If funcVA < Cast(Any Ptr, textStart) OrElse funcVA >= Cast(Any Ptr, textEnd) Then
'        Return 0
'    End If
'
'    ' 读取 32 字节（仅用于后续显示差异，不参与判断）
'    Dim cur(31) As UByte
'    If Not ReadMem(img.hProc, Cast(Any Ptr, Cast(ULongInt, funcVA)), @cur(0), 32) Then
'        MyLog.PrintLog LOG_WARN, "GetInlineHookList", "ReadMem", "1"
'        Return 0
'    End If
'    If FuncPath = "ntdll.dll!KiUserCallbackDispatcher" Then
'        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, FuncPath
'        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, "funcVA:0x" & WHex(funcVA)
'        Dim curCode1 As StringW, oldCode1 As StringW
'        For i As Integer = 0 To 31
'            curCode1 &= WHex(cur(i), 2) & " "
'            oldCode1 &= WHex(diskEntry[i], 2) & " "
'        Next
'        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, curCode1
'        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, oldCode1
'    End If
'    
'    Dim bDiff As Boolean = False
'    For i As Integer = 0 To 5 - 1
'        If cur(i) <> diskEntry[i] Then
'            bDiff = True
'            Exit For
'        End If
'    Next
'    
'    If Not bDiff Then Return 0
'
'    ' 【唯一判断标准】AnalyzeInlineHook 分析指令流
'    Dim info As InlineHookInfo
'    If AnalyzeInlineHook(img, funcVA, info) = False Then Return 0
'    If info.hookType = "" OrElse info.hookType = "unknown" Then Return 0
'
'    ' 是 hook，构建显示
'    Dim curCode As StringW, oldCode As StringW
'    For i As Integer = 0 To 31
'        curCode &= WHex(cur(i), 2) & " "
'        oldCode &= WHex(diskEntry[i], 2) & " "
'    Next
'
'    Dim ProxyDescription As StringW
'    If info.proxyFunc <> 0 Then
'        ProxyDescription = GetProxyDescription(img, info.proxyFunc)
'    Else
'        ProxyDescription = "unknown"
'    End If
'
'    AddItemColListEx lvwHook, 7,,, _
'        WStr("InlineHook(" & info.hookType & ")"), _
'        "0x" & WHex(funcVA), _
'        WStr(FuncPath), _
'        "0x" & WHex(info.proxyFunc), _
'        WStr(ProxyDescription), _
'        WStr(curCode), _
'        WStr(oldCode)
'
'    Return 1
'End Function

' ============================================================
' 类型定义
' ============================================================
Type IAT_RANGE
    startVA As ULongInt
    endVA   As ULongInt
End Type

Function AnalyzeInlineHook(img As RemotePeImageEx, funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
    Const MAX_HOPS = 8
    info.hookType = ""
    info.proxyFunc = 0

    Dim u As ud
    Dim buf(63) As UByte
    Dim curVA As ULongInt = Cast(ULongInt, funcVA)
    Dim visited(0 To MAX_HOPS) As ULongInt
    Dim hopCount As Integer = 0

    ud_init(@u)
    ud_set_mode(@u, IIf(img.arch = PE_ARCH_X64, 64, 32))
    ud_set_syntax(@u, UD_SYN_INTEL)

    Do
        hopCount += 1
        If hopCount > MAX_HOPS Then
            info.hookType = "LoopJump(MaxHops)"
            info.proxyFunc = curVA
            Return True
        End If

        For i As Integer = 1 To hopCount - 1
            If visited(i) = curVA Then
                info.hookType = "LoopJump"
                info.proxyFunc = curVA
                Return True
            End If
        Next
        visited(hopCount) = curVA

        If Not ReadMem(img.hProc, Cast(Any Ptr, curVA), @buf(0), 64) Then
            Print "[AnalyzeInlineHook] ReadMem failed at 0x" & WHex(curVA)
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            Return False
        End If
        Dim readSize As Integer = 64

        ud_set_pc(@u, curVA)
        ud_set_input_buffer(@u, @buf(0), readSize)

        If ud_disassemble(@u) = 0 Then
            Print "[AnalyzeInlineHook] ud_disassemble failed at 0x" & WHex(curVA)
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            info.hookType = "unknown"
            Return False
        End If

        Dim insnLen As UInteger = ud_insn_len(@u)
        Dim mnemonic As Integer = ud_insn_mnemonic(@u)
        Dim opr0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)

        ' --------------------------------------------------------
        ' 1. 检测多指令跳板：MOV RAX, imm64 ; JMP RAX
        '    48 B8 xx..xx FF E0  (10+2=12 字节)
        ' --------------------------------------------------------
        If img.arch = PE_ARCH_X64 AndAlso mnemonic = UD_Imov Then
            If readSize >= 12 AndAlso buf(0) = &H48 AndAlso buf(1) = &HB8 Then
                If buf(10) = &HFF AndAlso buf(11) = &HE0 Then
                    Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))
                    info.hookType = "mov rax+jmp rax"
                    info.proxyFunc = imm64
                    Return True
                End If
            End If
            ' 可扩展：MOV R11, imm64 ; JMP R11  (49 BB ... 41 FF E3)
            If readSize >= 13 AndAlso buf(0) = &H49 AndAlso buf(1) = &HBB Then
                If buf(10) = &H41 AndAlso buf(11) = &HFF AndAlso buf(12) = &HE3 Then
                    Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))
                    info.hookType = "mov r11+jmp r11"
                    info.proxyFunc = imm64
                    Return True
                End If
            End If
        End If

        ' --------------------------------------------------------
        ' 2. 检测 PUSH + RET 跳板
        ' --------------------------------------------------------
        If mnemonic = UD_Ipush Then
            If readSize >= insnLen + 1 Then
                Dim nextByte As UByte = buf(insnLen)
                If nextByte = &HC3 OrElse nextByte = &HCB Then  ' RET / RETF
                    Dim opr0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
                    Dim immVal As ULongInt = 0
                    If opr0 <> NULL AndAlso opr0->typ = UD_OP_IMM Then
                        Select Case opr0->size
                        Case 8
                            immVal = Cast(ULongInt, Cast(Byte, opr0->lval.sbyte))
                        Case 16
                            immVal = Cast(ULongInt, Cast(Short, opr0->lval.sword))
                        Case 32
                            immVal = Cast(ULongInt, opr0->lval.udword)
                        Case 64
                            immVal = opr0->lval.uqword
                        End Select
                    Else
                        ' 回退字节解析
                        If buf(0) = &H6A Then
                            immVal = Cast(ULongInt, Cast(Byte, buf(1)))
                        ElseIf buf(0) = &H68 Then
                            immVal = Cast(ULongInt, *Cast(ULong Ptr, @buf(1)))
                        End If
                    End If

                    info.hookType = "push+ret"
                    info.proxyFunc = immVal
                    Return True
                End If
            End If

            ' 不是 push+ret，终止
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            info.hookType = "unknown"
            Return False
        End If

        ' --------------------------------------------------------
        ' 3. RET / 非分支指令 → 链终止
        ' --------------------------------------------------------
        If mnemonic = UD_Iret OrElse mnemonic = UD_Iretf Then
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            info.hookType = "unknown"
            Return False
        End If

        If mnemonic <> UD_Ijmp AndAlso mnemonic <> UD_Icall Then
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            info.hookType = "unknown"
            Return False
        End If

        Dim isCall As Boolean = (mnemonic = UD_Icall)
        Dim nextVA As ULongInt = 0
        opr0 = ud_insn_opr(@u, 0)
        Dim oprType As Integer = IIf(opr0 <> NULL, opr0->typ, -1)

        ' --------------------------------------------------------
        ' 4. 相对跳转 JMP/CALL rel8/rel16/rel32
        ' --------------------------------------------------------
        If oprType = UD_OP_JIMM Then
            Dim disp As LongInt
            Select Case opr0->size
            Case 8
                disp = Cast(LongInt, opr0->lval.sbyte)
            Case 16
                disp = Cast(LongInt, opr0->lval.sword)
            Case 32
                disp = Cast(LongInt, opr0->lval.sdword)
            Case 64
                disp = opr0->lval.sqword
            Case Else
                ' 回退字节解析（利用 udis86 给出的 insnLen）
                If insnLen >= 2 AndAlso buf(0) = &HEB Then
                    disp = Cast(LongInt, Cast(Byte, buf(insnLen - 1)))
                Else
                    disp = Cast(LongInt, *Cast(Long Ptr, @buf(insnLen - 4)))
                End If
            End Select

            ' 关键：全部用有符号 LongInt 计算，再转回 ULongInt
            nextVA = Cast(ULongInt, Cast(LongInt, curVA) + Cast(LongInt, insnLen) + disp)

            If isCall Then
                info.hookType = "call rel"
                info.proxyFunc = nextVA
                Return True   ' CALL 终止追踪
            Else
                If hopCount = 1 Then info.hookType = "jmp rel"
                info.proxyFunc = nextVA
                curVA = nextVA
                Continue Do
            End If

        ' --------------------------------------------------------
        ' 5. 内存间接跳转 JMP/CALL [mem]
        ' --------------------------------------------------------
        ElseIf oprType = UD_OP_MEM OrElse (opr0 = NULL AndAlso buf(0) = &HFF) Then
            Dim ptrVA As ULongInt = 0
            Dim memReadSize As Integer = IIf(img.arch = PE_ARCH_X64, 8, 4)

            ' udis86 已给出完整指令长度（含前缀），disp32 永远在指令最后 4 字节
            If insnLen >= 6 Then
                Dim modrmOffset As Integer = 1
                ' 跳过 x64 REX 前缀（40-4F）
                If img.arch = PE_ARCH_X64 AndAlso (buf(0) >= &H40 AndAlso buf(0) <= &H4F) Then
                    modrmOffset = 2
                End If

                Dim modrm As UByte = buf(modrmOffset)
                Dim modBits As UByte = (modrm Shr 6) And 3
                Dim rmBits As UByte = modrm And 7

                ' Mod=00, R/M=101 → [disp32] (x86 绝对地址) 或 [RIP+disp32] (x64)
                If modBits = 0 AndAlso rmBits = 5 Then
                    Dim disp32 As Long = *Cast(Long Ptr, @buf(insnLen - 4))

                    If img.arch = PE_ARCH_X64 Then
                        ' x64: RIP-relative
                        ptrVA = curVA + insnLen + Cast(ULongInt, Cast(LongInt, disp32))
                    Else
                        ' x86 / Wow64: disp32 是绝对虚拟地址
                        ptrVA = Cast(ULongInt, Cast(ULong, disp32))
                    End If
                Else
                    ' [reg]、[reg+reg*scale] 等其他寻址，无法静态追踪
                    info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
                    info.proxyFunc = 0
                    Return True
                End If
            Else
                info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
                info.proxyFunc = 0
                Return True
            End If

            ' 读取内存中的目标地址
            If ptrVA <> 0 Then
                Dim targetAddr As ULongInt = 0
                If ReadMem(img.hProc, Cast(Any Ptr, ptrVA), @targetAddr, memReadSize) = 0 Then
                    info.hookType = IIf(isCall, "call [mem]", "jmp [mem]")
                    info.proxyFunc = 0
                    Return True
                End If

                ' x86 下确保零扩展
                If img.arch = PE_ARCH_X86 Then
                    targetAddr = Cast(ULongInt, Cast(ULong, targetAddr))
                End If

                nextVA = targetAddr

                If isCall Then
                    info.hookType = "call [mem]"
                    info.proxyFunc = nextVA
                    Return True
                Else
                    If hopCount = 1 Then info.hookType = "jmp [mem]"
                    info.proxyFunc = nextVA
                    curVA = nextVA
                    Continue Do
                End If
            End If

        ' --------------------------------------------------------
        ' 6. 寄存器跳转 JMP/CALL reg
        ' --------------------------------------------------------
        ElseIf oprType = UD_OP_REG OrElse (opr0 = NULL AndAlso buf(0) = &HFF AndAlso (buf(1) And &HC0) = &HC0) Then
            info.hookType = IIf(isCall, "call reg", "jmp reg")
            info.proxyFunc = 0
            Return True

        Else
            If info.hookType <> "" Then
                info.proxyFunc = curVA
                Return True
            End If
            info.hookType = "unknown"
            Return False
        End If

    Loop

    Return False
End Function

' ============================================================
' 判断磁盘原始指令是否是正常的系统跳转结构
' 用于排除 IAT 跳板、API 转发、编译器生成的导入跳转等导致的误报
' 返回 True 表示这是正常的系统结构，不应视为 InlineHook
' ============================================================
Function IsDiskEntryTrampoline(diskEntry As UByte Ptr, arch As PE_ARCH) As Boolean
    If diskEntry = NULL Then Return False
    
    If arch = PE_ARCH_X86 Then
        ' FF 25 xx xx xx xx → JMP [disp32] (x86 绝对地址)
        ' 这是 32 位系统 DLL 中最典型的导出函数实现方式（IAT 跳板）
        If diskEntry[0] = &HFF AndAlso diskEntry[1] = &H25 Then
            Return True
        End If
        
        ' E9 xx xx xx xx → JMP rel32 (API 转发或内部跳转)
        If diskEntry[0] = &HE9 Then
            Return True
        End If
        
        ' EB xx → JMP rel8 (短跳转)
        If diskEntry[0] = &HEB Then
            Return True
        End If
        
        ' FF 15 xx xx xx xx → CALL [disp32] (IAT 调用跳板，某些编译器生成)
        If diskEntry[0] = &HFF AndAlso diskEntry[1] = &H15 Then
            Return True
        End If
        
    ElseIf arch = PE_ARCH_X64 Then
        ' FF 25 xx xx xx xx → JMP [RIP+disp32] (x64 IAT 跳板)
        If diskEntry[0] = &HFF AndAlso diskEntry[1] = &H25 Then
            Return True
        End If
        
        ' 48 B8 xx xx xx xx xx xx xx xx FF E0
        ' → MOV RAX, imm64 ; JMP RAX (x64 多指令跳板)
        If diskEntry[0] = &H48 AndAlso diskEntry[1] = &HB8 AndAlso _
           diskEntry[10] = &HFF AndAlso diskEntry[11] = &HE0 Then
            Return True
        End If
        
        ' 49 BB xx xx xx xx xx xx xx xx 41 FF E3
        ' → MOV R11, imm64 ; JMP R11
        If diskEntry[0] = &H49 AndAlso diskEntry[1] = &HBB AndAlso _
           diskEntry[10] = &H41 AndAlso diskEntry[11] = &HFF AndAlso diskEntry[12] = &HE3 Then
            Return True
        End If
    End If
    
    Return False
End Function

' ============================================================
' 配套改进：GetInlineHookList
' 用 udis86 精确获取 Hook 指令长度，避免 16 字节全比较导致的误报
' ============================================================
Function GetInlineHookList(ByRef img As RemotePeImageEx, funcVA As Any Ptr, diskEntry As UByte Ptr, ByVal FuncPath As StringW, lvwHook As Class_ListView) As ULong
    Dim textStart As ULongInt, textEnd As ULongInt
    GetTextSectionRange img, textStart, textEnd
    If funcVA < Cast(Any Ptr, textStart) OrElse funcVA >= Cast(Any Ptr, textEnd) Then
        Return 0
    End If

    ' 读取 32 字节（仅用于后续显示差异，不参与判断）
    Dim cur(31) As UByte
    If Not ReadMem(img.hProc, Cast(Any Ptr, Cast(ULongInt, funcVA)), @cur(0), 32) Then
        MyLog.PrintLog LOG_WARN, "GetInlineHookList", "ReadMem", "1"
        Return 0
    End If
    If FuncPath = "ntdll.dll!KiUserCallbackDispatcher" Then
        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, FuncPath
        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, "funcVA:0x" & WHex(funcVA)
        Dim curCode1 As StringW, oldCode1 As StringW
        For i As Integer = 0 To 31
            curCode1 &= WHex(cur(i), 2) & " "
            oldCode1 &= WHex(diskEntry[i], 2) & " "
        Next
        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, curCode1
        MyLog.PrintLog LOG_INFO, "GetInlineHookList",, oldCode1
    End If
    
    Dim bDiff As Boolean = False
    For i As Integer = 0 To 5 - 1
        If cur(i) <> diskEntry[i] Then
            bDiff = True
            Exit For
        End If
    Next
    
    If Not bDiff Then Return 0

    ' 【唯一判断标准】AnalyzeInlineHook 分析指令流
    Dim info As InlineHookInfo
    If AnalyzeInlineHook(img, funcVA, info) = False Then Return 0
    If info.hookType = "" OrElse info.hookType = "unknown" Then Return 0
    ' 直接排除磁盘上就是系统跳转桩的情况
    If (Left(info.hookType, 3) = "jmp" OrElse Left(info.hookType, 4) = "call") Then
        ' 32 位: FF 25 (JMP [mem]), E9 (JMP rel), EB (JMP rel8), FF 15 (CALL [mem])
        ' 64 位: FF 25 (RIP-relative JMP), 48 B8...FF E0 (MOV RAX,JMP RAX)
        If IsDiskEntryTrampoline(diskEntry, img.arch) Then
            Return 0
        End If
    End If

    ' 是 hook，构建显示
    Dim curCode As StringW, oldCode As StringW
    For i As Integer = 0 To 31
        curCode &= WHex(cur(i), 2) & " "
        oldCode &= WHex(diskEntry[i], 2) & " "
    Next

    Dim ProxyDescription As StringW
    If info.proxyFunc <> 0 Then
        ProxyDescription = GetProxyDescription(img, info.proxyFunc)
    Else
        ProxyDescription = "unknown"
    End If

    AddItemColListEx lvwHook, 7,,, _
        WStr("InlineHook(" & info.hookType & ")"), _
        "0x" & WHex(funcVA), _
        WStr(FuncPath), _
        "0x" & WHex(info.proxyFunc), _
        WStr(ProxyDescription), _
        WStr(curCode), _
        WStr(oldCode)

    Return 1
End Function