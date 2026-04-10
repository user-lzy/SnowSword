' udis86.bi - FreeBasic 绑定 for udis86
' 原始头文件: extern.h (udis86)
' 转换日期: 2026-04-07

#inclib "udis86"

' 以下类型定义基于 udis86 的 types.h 常见内容
' 若与实际库不符，请根据具体版本调整

' 基础整数类型映射
Type uint8_t As UByte
Type uint64_t As ULongInt
' size_t 使用平台相关的 Integer (32位下为32位，64位下为64位)

' ---------- 基础类型定义（如果未在其它地方定义） ----------
Type uint8_t   As UByte
Type uint16_t  As UShort
Type uint32_t  As UInteger
Type uint64_t  As ULongInt
Type int8_t    As Byte
Type int16_t   As Short
Type int32_t   As Integer
Type int64_t   As LongInt

Type _ud_operand As ud_operand

' ------------------------------------------------------------
' ud_mnemonic_code -- 指令助记符常量
' ------------------------------------------------------------
Enum ud_mnemonic_code
    UD_Iaaa
    UD_Iaad
    UD_Iaam
    UD_Iaas
    UD_Iadc
    UD_Iadd
    UD_Iaddpd
    UD_Iaddps
    UD_Iaddsd
    UD_Iaddss
    UD_Iaddsubpd
    UD_Iaddsubps
    UD_Iaesdec
    UD_Iaesdeclast
    UD_Iaesenc
    UD_Iaesenclast
    UD_Iaesimc
    UD_Iaeskeygenassist
    UD_Iand
    UD_Iandnpd
    UD_Iandnps
    UD_Iandpd
    UD_Iandps
    UD_Iarpl
    UD_Iblendpd
    UD_Iblendps
    UD_Iblendvpd
    UD_Iblendvps
    UD_Ibound
    UD_Ibsf
    UD_Ibsr
    UD_Ibswap
    UD_Ibt
    UD_Ibtc
    UD_Ibtr
    UD_Ibts
    UD_Icall
    UD_Icbw
    UD_Icdq
    UD_Icdqe
    UD_Iclc
    UD_Icld
    UD_Iclflush
    UD_Iclgi
    UD_Icli
    UD_Iclts
    UD_Icmc
    UD_Icmova
    UD_Icmovae
    UD_Icmovb
    UD_Icmovbe
    UD_Icmovg
    UD_Icmovge
    UD_Icmovl
    UD_Icmovle
    UD_Icmovno
    UD_Icmovnp
    UD_Icmovns
    UD_Icmovnz
    UD_Icmovo
    UD_Icmovp
    UD_Icmovs
    UD_Icmovz
    UD_Icmp
    UD_Icmppd
    UD_Icmpps
    UD_Icmpsb
    UD_Icmpsd
    UD_Icmpsq
    UD_Icmpss
    UD_Icmpsw
    UD_Icmpxchg
    UD_Icmpxchg16b
    UD_Icmpxchg8b
    UD_Icomisd
    UD_Icomiss
    UD_Icpuid
    UD_Icqo
    UD_Icrc32
    UD_Icvtdq2pd
    UD_Icvtdq2ps
    UD_Icvtpd2dq
    UD_Icvtpd2pi
    UD_Icvtpd2ps
    UD_Icvtpi2pd
    UD_Icvtpi2ps
    UD_Icvtps2dq
    UD_Icvtps2pd
    UD_Icvtps2pi
    UD_Icvtsd2si
    UD_Icvtsd2ss
    UD_Icvtsi2sd
    UD_Icvtsi2ss
    UD_Icvtss2sd
    UD_Icvtss2si
    UD_Icvttpd2dq
    UD_Icvttpd2pi
    UD_Icvttps2dq
    UD_Icvttps2pi
    UD_Icvttsd2si
    UD_Icvttss2si
    UD_Icwd
    UD_Icwde
    UD_Idaa
    UD_Idas
    UD_Idec
    UD_Idiv
    UD_Idivpd
    UD_Idivps
    UD_Idivsd
    UD_Idivss
    UD_Idppd
    UD_Idpps
    UD_Iemms
    UD_Ienter
    UD_Iextractps
    UD_If2xm1
    UD_Ifabs
    UD_Ifadd
    UD_Ifaddp
    UD_Ifbld
    UD_Ifbstp
    UD_Ifchs
    UD_Ifclex
    UD_Ifcmovb
    UD_Ifcmovbe
    UD_Ifcmove
    UD_Ifcmovnb
    UD_Ifcmovnbe
    UD_Ifcmovne
    UD_Ifcmovnu
    UD_Ifcmovu
    UD_Ifcom
    UD_Ifcom2
    UD_Ifcomi
    UD_Ifcomip
    UD_Ifcomp
    UD_Ifcomp3
    UD_Ifcomp5
    UD_Ifcompp
    UD_Ifcos
    UD_Ifdecstp
    UD_Ifdiv
    UD_Ifdivp
    UD_Ifdivr
    UD_Ifdivrp
    UD_Ifemms
    UD_Iffree
    UD_Iffreep
    UD_Ifiadd
    UD_Ificom
    UD_Ificomp
    UD_Ifidiv
    UD_Ifidivr
    UD_Ifild
    UD_Ifimul
    UD_Ifincstp
    UD_Ifist
    UD_Ifistp
    UD_Ifisttp
    UD_Ifisub
    UD_Ifisubr
    UD_Ifld
    UD_Ifld1
    UD_Ifldcw
    UD_Ifldenv
    UD_Ifldl2e
    UD_Ifldl2t
    UD_Ifldlg2
    UD_Ifldln2
    UD_Ifldpi
    UD_Ifldz
    UD_Ifmul
    UD_Ifmulp
    UD_Ifndisi
    UD_Ifneni
    UD_Ifninit
    UD_Ifnop
    UD_Ifnsave
    UD_Ifnsetpm
    UD_Ifnstcw
    UD_Ifnstenv
    UD_Ifnstsw
    UD_Ifpatan
    UD_Ifprem
    UD_Ifprem1
    UD_Ifptan
    UD_Ifrndint
    UD_Ifrstor
    UD_Ifrstpm
    UD_Ifscale
    UD_Ifsin
    UD_Ifsincos
    UD_Ifsqrt
    UD_Ifst
    UD_Ifstp
    UD_Ifstp1
    UD_Ifstp8
    UD_Ifstp9
    UD_Ifsub
    UD_Ifsubp
    UD_Ifsubr
    UD_Ifsubrp
    UD_Iftst
    UD_Ifucom
    UD_Ifucomi
    UD_Ifucomip
    UD_Ifucomp
    UD_Ifucompp
    UD_Ifxam
    UD_Ifxch
    UD_Ifxch4
    UD_Ifxch7
    UD_Ifxrstor
    UD_Ifxsave
    UD_Ifxtract
    UD_Ifyl2x
    UD_Ifyl2xp1
    UD_Igetsec
    UD_Ihaddpd
    UD_Ihaddps
    UD_Ihlt
    UD_Ihsubpd
    UD_Ihsubps
    UD_Iidiv
    UD_Iimul
    UD_Iin
    UD_Iinc
    UD_Iinsb
    UD_Iinsd
    UD_Iinsertps
    UD_Iinsw
    UD_Iint
    UD_Iint1
    UD_Iint3
    UD_Iinto
    UD_Iinvd
    UD_Iinvept
    UD_Iinvlpg
    UD_Iinvlpga
    UD_Iinvvpid
    UD_Iiretd
    UD_Iiretq
    UD_Iiretw
    UD_Ija
    UD_Ijae
    UD_Ijb
    UD_Ijbe
    UD_Ijcxz
    UD_Ijecxz
    UD_Ijg
    UD_Ijge
    UD_Ijl
    UD_Ijle
    UD_Ijmp
    UD_Ijno
    UD_Ijnp
    UD_Ijns
    UD_Ijnz
    UD_Ijo
    UD_Ijp
    UD_Ijrcxz
    UD_Ijs
    UD_Ijz
    UD_Ilahf
    UD_Ilar
    UD_Ilddqu
    UD_Ildmxcsr
    UD_Ilds
    UD_Ilea
    UD_Ileave
    UD_Iles
    UD_Ilfence
    UD_Ilfs
    UD_Ilgdt
    UD_Ilgs
    UD_Ilidt
    UD_Illdt
    UD_Ilmsw
    UD_Ilock
    UD_Ilodsb
    UD_Ilodsd
    UD_Ilodsq
    UD_Ilodsw
    UD_Iloop
    UD_Iloope
    UD_Iloopne
    UD_Ilsl
    UD_Ilss
    UD_Iltr
    UD_Imaskmovdqu
    UD_Imaskmovq
    UD_Imaxpd
    UD_Imaxps
    UD_Imaxsd
    UD_Imaxss
    UD_Imfence
    UD_Iminpd
    UD_Iminps
    UD_Iminsd
    UD_Iminss
    UD_Imonitor
    UD_Imontmul
    UD_Imov
    UD_Imovapd
    UD_Imovaps
    UD_Imovbe
    UD_Imovd
    UD_Imovddup
    UD_Imovdq2q
    UD_Imovdqa
    UD_Imovdqu
    UD_Imovhlps
    UD_Imovhpd
    UD_Imovhps
    UD_Imovlhps
    UD_Imovlpd
    UD_Imovlps
    UD_Imovmskpd
    UD_Imovmskps
    UD_Imovntdq
    UD_Imovntdqa
    UD_Imovnti
    UD_Imovntpd
    UD_Imovntps
    UD_Imovntq
    UD_Imovq
    UD_Imovq2dq
    UD_Imovsb
    UD_Imovsd
    UD_Imovshdup
    UD_Imovsldup
    UD_Imovsq
    UD_Imovss
    UD_Imovsw
    UD_Imovsx
    UD_Imovsxd
    UD_Imovupd
    UD_Imovups
    UD_Imovzx
    UD_Impsadbw
    UD_Imul
    UD_Imulpd
    UD_Imulps
    UD_Imulsd
    UD_Imulss
    UD_Imwait
    UD_Ineg
    UD_Inop
    UD_Inot
    UD_Ior
    UD_Iorpd
    UD_Iorps
    UD_Iout
    UD_Ioutsb
    UD_Ioutsd
    UD_Ioutsw
    UD_Ipabsb
    UD_Ipabsd
    UD_Ipabsw
    UD_Ipackssdw
    UD_Ipacksswb
    UD_Ipackusdw
    UD_Ipackuswb
    UD_Ipaddb
    UD_Ipaddd
    UD_Ipaddq
    UD_Ipaddsb
    UD_Ipaddsw
    UD_Ipaddusb
    UD_Ipaddusw
    UD_Ipaddw
    UD_Ipalignr
    UD_Ipand
    UD_Ipandn
    UD_Ipavgb
    UD_Ipavgusb
    UD_Ipavgw
    UD_Ipblendvb
    UD_Ipblendw
    UD_Ipclmulqdq
    UD_Ipcmpeqb
    UD_Ipcmpeqd
    UD_Ipcmpeqq
    UD_Ipcmpeqw
    UD_Ipcmpestri
    UD_Ipcmpestrm
    UD_Ipcmpgtb
    UD_Ipcmpgtd
    UD_Ipcmpgtq
    UD_Ipcmpgtw
    UD_Ipcmpistri
    UD_Ipcmpistrm
    UD_Ipextrb
    UD_Ipextrd
    UD_Ipextrq
    UD_Ipextrw
    UD_Ipf2id
    UD_Ipf2iw
    UD_Ipfacc
    UD_Ipfadd
    UD_Ipfcmpeq
    UD_Ipfcmpge
    UD_Ipfcmpgt
    UD_Ipfmax
    UD_Ipfmin
    UD_Ipfmul
    UD_Ipfnacc
    UD_Ipfpnacc
    UD_Ipfrcp
    UD_Ipfrcpit1
    UD_Ipfrcpit2
    UD_Ipfrsqit1
    UD_Ipfrsqrt
    UD_Ipfsub
    UD_Ipfsubr
    UD_Iphaddd
    UD_Iphaddsw
    UD_Iphaddw
    UD_Iphminposuw
    UD_Iphsubd
    UD_Iphsubsw
    UD_Iphsubw
    UD_Ipi2fd
    UD_Ipi2fw
    UD_Ipinsrb
    UD_Ipinsrd
    UD_Ipinsrq
    UD_Ipinsrw
    UD_Ipmaddubsw
    UD_Ipmaddwd
    UD_Ipmaxsb
    UD_Ipmaxsd
    UD_Ipmaxsw
    UD_Ipmaxub
    UD_Ipmaxud
    UD_Ipmaxuw
    UD_Ipminsb
    UD_Ipminsd
    UD_Ipminsw
    UD_Ipminub
    UD_Ipminud
    UD_Ipminuw
    UD_Ipmovmskb
    UD_Ipmovsxbd
    UD_Ipmovsxbq
    UD_Ipmovsxbw
    UD_Ipmovsxdq
    UD_Ipmovsxwd
    UD_Ipmovsxwq
    UD_Ipmovzxbd
    UD_Ipmovzxbq
    UD_Ipmovzxbw
    UD_Ipmovzxdq
    UD_Ipmovzxwd
    UD_Ipmovzxwq
    UD_Ipmuldq
    UD_Ipmulhrsw
    UD_Ipmulhrw
    UD_Ipmulhuw
    UD_Ipmulhw
    UD_Ipmulld
    UD_Ipmullw
    UD_Ipmuludq
    UD_Ipop
    UD_Ipopa
    UD_Ipopad
    UD_Ipopcnt
    UD_Ipopfd
    UD_Ipopfq
    UD_Ipopfw
    UD_Ipor
    UD_Iprefetch
    UD_Iprefetchnta
    UD_Iprefetcht0
    UD_Iprefetcht1
    UD_Iprefetcht2
    UD_Ipsadbw
    UD_Ipshufb
    UD_Ipshufd
    UD_Ipshufhw
    UD_Ipshuflw
    UD_Ipshufw
    UD_Ipsignb
    UD_Ipsignd
    UD_Ipsignw
    UD_Ipslld
    UD_Ipslldq
    UD_Ipsllq
    UD_Ipsllw
    UD_Ipsrad
    UD_Ipsraw
    UD_Ipsrld
    UD_Ipsrldq
    UD_Ipsrlq
    UD_Ipsrlw
    UD_Ipsubb
    UD_Ipsubd
    UD_Ipsubq
    UD_Ipsubsb
    UD_Ipsubsw
    UD_Ipsubusb
    UD_Ipsubusw
    UD_Ipsubw
    UD_Ipswapd
    UD_Iptest
    UD_Ipunpckhbw
    UD_Ipunpckhdq
    UD_Ipunpckhqdq
    UD_Ipunpckhwd
    UD_Ipunpcklbw
    UD_Ipunpckldq
    UD_Ipunpcklqdq
    UD_Ipunpcklwd
    UD_Ipush
    UD_Ipusha
    UD_Ipushad
    UD_Ipushfd
    UD_Ipushfq
    UD_Ipushfw
    UD_Ipxor
    UD_Ircl
    UD_Ircpps
    UD_Ircpss
    UD_Ircr
    UD_Irdmsr
    UD_Irdpmc
    UD_Irdrand
    UD_Irdtsc
    UD_Irdtscp
    UD_Irep
    UD_Irepne
    UD_Iret
    UD_Iretf
    UD_Irol
    UD_Iror
    UD_Iroundpd
    UD_Iroundps
    UD_Iroundsd
    UD_Iroundss
    UD_Irsm
    UD_Irsqrtps
    UD_Irsqrtss
    UD_Isahf
    UD_Isalc
    UD_Isar
    UD_Isbb
    UD_Iscasb
    UD_Iscasd
    UD_Iscasq
    UD_Iscasw
    UD_Iseta
    UD_Isetae
    UD_Isetb
    UD_Isetbe
    UD_Isetg
    UD_Isetge
    UD_Isetl
    UD_Isetle
    UD_Isetno
    UD_Isetnp
    UD_Isetns
    UD_Isetnz
    UD_Iseto
    UD_Isetp
    UD_Isets
    UD_Isetz
    UD_Isfence
    UD_Isgdt
    UD_Ishl
    UD_Ishld
    UD_Ishr
    UD_Ishrd
    UD_Ishufpd
    UD_Ishufps
    UD_Isidt
    UD_Iskinit
    UD_Isldt
    UD_Ismsw
    UD_Isqrtpd
    UD_Isqrtps
    UD_Isqrtsd
    UD_Isqrtss
    UD_Istc
    UD_Istd
    UD_Istgi
    UD_Isti
    UD_Istmxcsr
    UD_Istosb
    UD_Istosd
    UD_Istosq
    UD_Istosw
    UD_Istr
    UD_Isub
    UD_Isubpd
    UD_Isubps
    UD_Isubsd
    UD_Isubss
    UD_Iswapgs
    UD_Isyscall
    UD_Isysenter
    UD_Isysexit
    UD_Isysret
    UD_Itest
    UD_Iucomisd
    UD_Iucomiss
    UD_Iud2
    UD_Iunpckhpd
    UD_Iunpckhps
    UD_Iunpcklpd
    UD_Iunpcklps
    UD_Ivaddpd
    UD_Ivaddps
    UD_Ivaddsd
    UD_Ivaddss
    UD_Ivaddsubpd
    UD_Ivaddsubps
    UD_Ivaesdec
    UD_Ivaesdeclast
    UD_Ivaesenc
    UD_Ivaesenclast
    UD_Ivaesimc
    UD_Ivaeskeygenassist
    UD_Ivandnpd
    UD_Ivandnps
    UD_Ivandpd
    UD_Ivandps
    UD_Ivblendpd
    UD_Ivblendps
    UD_Ivblendvpd
    UD_Ivblendvps
    UD_Ivbroadcastsd
    UD_Ivbroadcastss
    UD_Ivcmppd
    UD_Ivcmpps
    UD_Ivcmpsd
    UD_Ivcmpss
    UD_Ivcomisd
    UD_Ivcomiss
    UD_Ivcvtdq2pd
    UD_Ivcvtdq2ps
    UD_Ivcvtpd2dq
    UD_Ivcvtpd2ps
    UD_Ivcvtps2dq
    UD_Ivcvtps2pd
    UD_Ivcvtsd2si
    UD_Ivcvtsd2ss
    UD_Ivcvtsi2sd
    UD_Ivcvtsi2ss
    UD_Ivcvtss2sd
    UD_Ivcvtss2si
    UD_Ivcvttpd2dq
    UD_Ivcvttps2dq
    UD_Ivcvttsd2si
    UD_Ivcvttss2si
    UD_Ivdivpd
    UD_Ivdivps
    UD_Ivdivsd
    UD_Ivdivss
    UD_Ivdppd
    UD_Ivdpps
    UD_Iverr
    UD_Iverw
    UD_Ivextractf128
    UD_Ivextractps
    UD_Ivhaddpd
    UD_Ivhaddps
    UD_Ivhsubpd
    UD_Ivhsubps
    UD_Ivinsertf128
    UD_Ivinsertps
    UD_Ivlddqu
    UD_Ivmaskmovdqu
    UD_Ivmaskmovpd
    UD_Ivmaskmovps
    UD_Ivmaxpd
    UD_Ivmaxps
    UD_Ivmaxsd
    UD_Ivmaxss
    UD_Ivmcall
    UD_Ivmclear
    UD_Ivminpd
    UD_Ivminps
    UD_Ivminsd
    UD_Ivminss
    UD_Ivmlaunch
    UD_Ivmload
    UD_Ivmmcall
    UD_Ivmovapd
    UD_Ivmovaps
    UD_Ivmovd
    UD_Ivmovddup
    UD_Ivmovdqa
    UD_Ivmovdqu
    UD_Ivmovhlps
    UD_Ivmovhpd
    UD_Ivmovhps
    UD_Ivmovlhps
    UD_Ivmovlpd
    UD_Ivmovlps
    UD_Ivmovmskpd
    UD_Ivmovmskps
    UD_Ivmovntdq
    UD_Ivmovntdqa
    UD_Ivmovntpd
    UD_Ivmovntps
    UD_Ivmovq
    UD_Ivmovsd
    UD_Ivmovshdup
    UD_Ivmovsldup
    UD_Ivmovss
    UD_Ivmovupd
    UD_Ivmovups
    UD_Ivmpsadbw
    UD_Ivmptrld
    UD_Ivmptrst
    UD_Ivmread
    UD_Ivmresume
    UD_Ivmrun
    UD_Ivmsave
    UD_Ivmulpd
    UD_Ivmulps
    UD_Ivmulsd
    UD_Ivmulss
    UD_Ivmwrite
    UD_Ivmxoff
    UD_Ivmxon
    UD_Ivorpd
    UD_Ivorps
    UD_Ivpabsb
    UD_Ivpabsd
    UD_Ivpabsw
    UD_Ivpackssdw
    UD_Ivpacksswb
    UD_Ivpackusdw
    UD_Ivpackuswb
    UD_Ivpaddb
    UD_Ivpaddd
    UD_Ivpaddq
    UD_Ivpaddsb
    UD_Ivpaddsw
    UD_Ivpaddusb
    UD_Ivpaddusw
    UD_Ivpaddw
    UD_Ivpalignr
    UD_Ivpand
    UD_Ivpandn
    UD_Ivpavgb
    UD_Ivpavgw
    UD_Ivpblendvb
    UD_Ivpblendw
    UD_Ivpclmulqdq
    UD_Ivpcmpeqb
    UD_Ivpcmpeqd
    UD_Ivpcmpeqq
    UD_Ivpcmpeqw
    UD_Ivpcmpestri
    UD_Ivpcmpestrm
    UD_Ivpcmpgtb
    UD_Ivpcmpgtd
    UD_Ivpcmpgtq
    UD_Ivpcmpgtw
    UD_Ivpcmpistri
    UD_Ivpcmpistrm
    UD_Ivperm2f128
    UD_Ivpermilpd
    UD_Ivpermilps
    UD_Ivpextrb
    UD_Ivpextrd
    UD_Ivpextrq
    UD_Ivpextrw
    UD_Ivphaddd
    UD_Ivphaddsw
    UD_Ivphaddw
    UD_Ivphminposuw
    UD_Ivphsubd
    UD_Ivphsubsw
    UD_Ivphsubw
    UD_Ivpinsrb
    UD_Ivpinsrd
    UD_Ivpinsrq
    UD_Ivpinsrw
    UD_Ivpmaddubsw
    UD_Ivpmaddwd
    UD_Ivpmaxsb
    UD_Ivpmaxsd
    UD_Ivpmaxsw
    UD_Ivpmaxub
    UD_Ivpmaxud
    UD_Ivpmaxuw
    UD_Ivpminsb
    UD_Ivpminsd
    UD_Ivpminsw
    UD_Ivpminub
    UD_Ivpminud
    UD_Ivpminuw
    UD_Ivpmovmskb
    UD_Ivpmovsxbd
    UD_Ivpmovsxbq
    UD_Ivpmovsxbw
    UD_Ivpmovsxwd
    UD_Ivpmovsxwq
    UD_Ivpmovzxbd
    UD_Ivpmovzxbq
    UD_Ivpmovzxbw
    UD_Ivpmovzxdq
    UD_Ivpmovzxwd
    UD_Ivpmovzxwq
    UD_Ivpmuldq
    UD_Ivpmulhrsw
    UD_Ivpmulhuw
    UD_Ivpmulhw
    UD_Ivpmulld
    UD_Ivpmullw
    UD_Ivpor
    UD_Ivpsadbw
    UD_Ivpshufb
    UD_Ivpshufd
    UD_Ivpshufhw
    UD_Ivpshuflw
    UD_Ivpsignb
    UD_Ivpsignd
    UD_Ivpsignw
    UD_Ivpslld
    UD_Ivpslldq
    UD_Ivpsllq
    UD_Ivpsllw
    UD_Ivpsrad
    UD_Ivpsraw
    UD_Ivpsrld
    UD_Ivpsrldq
    UD_Ivpsrlq
    UD_Ivpsrlw
    UD_Ivpsubb
    UD_Ivpsubd
    UD_Ivpsubq
    UD_Ivpsubsb
    UD_Ivpsubsw
    UD_Ivpsubusb
    UD_Ivpsubusw
    UD_Ivpsubw
    UD_Ivptest
    UD_Ivpunpckhbw
    UD_Ivpunpckhdq
    UD_Ivpunpckhqdq
    UD_Ivpunpckhwd
    UD_Ivpunpcklbw
    UD_Ivpunpckldq
    UD_Ivpunpcklqdq
    UD_Ivpunpcklwd
    UD_Ivpxor
    UD_Ivrcpps
    UD_Ivrcpss
    UD_Ivroundpd
    UD_Ivroundps
    UD_Ivroundsd
    UD_Ivroundss
    UD_Ivrsqrtps
    UD_Ivrsqrtss
    UD_Ivshufpd
    UD_Ivshufps
    UD_Ivsqrtpd
    UD_Ivsqrtps
    UD_Ivsqrtsd
    UD_Ivsqrtss
    UD_Ivstmxcsr
    UD_Ivsubpd
    UD_Ivsubps
    UD_Ivsubsd
    UD_Ivsubss
    UD_Ivtestpd
    UD_Ivtestps
    UD_Ivucomisd
    UD_Ivucomiss
    UD_Ivunpckhpd
    UD_Ivunpckhps
    UD_Ivunpcklpd
    UD_Ivunpcklps
    UD_Ivxorpd
    UD_Ivxorps
    UD_Ivzeroall
    UD_Ivzeroupper
    UD_Iwait
    UD_Iwbinvd
    UD_Iwrmsr
    UD_Ixadd
    UD_Ixchg
    UD_Ixcryptcbc
    UD_Ixcryptcfb
    UD_Ixcryptctr
    UD_Ixcryptecb
    UD_Ixcryptofb
    UD_Ixgetbv
    UD_Ixlatb
    UD_Ixor
    UD_Ixorpd
    UD_Ixorps
    UD_Ixrstor
    UD_Ixsave
    UD_Ixsetbv
    UD_Ixsha1
    UD_Ixsha256
    UD_Ixstore
    UD_Iinvalid
    UD_I3dnow
    UD_Inone
    UD_Idb
    UD_Ipause
    UD_MAX_MNEMONIC_CODE
End Enum

' ---------- 联合体 ud_lval ----------
Type ud_lval
    Union
        sbyte As int8_t
        UByte As uint8_t
        sword As int16_t
        UWORD As uint16_t
        sdword As int32_t
        udword As uint32_t
        sqword As int64_t
        uqword As uint64_t
        ' 修正：将匿名结构体拆分为两个成员
        ptr_seg As uint16_t
        ptr_off As uint32_t
    End Union
End Type

' ---------- 枚举 ud_type ----------
Enum ud_type
    UD_NONE

    ' 8 位通用寄存器
    UD_R_AL
    UD_R_CL
    UD_R_DL
    UD_R_BL
    UD_R_AH
    UD_R_CH
    UD_R_DH
    UD_R_BH
    UD_R_SPL
    UD_R_BPL
    UD_R_SIL
    UD_R_DIL
    UD_R_R8B
    UD_R_R9B
    UD_R_R10B
    UD_R_R11B
    UD_R_R12B
    UD_R_R13B
    UD_R_R14B
    UD_R_R15B

    ' 16 位通用寄存器
    UD_R_AX
    UD_R_CX
    UD_R_DX
    UD_R_BX
    UD_R_SP
    UD_R_BP
    UD_R_SI
    UD_R_DI
    UD_R_R8W
    UD_R_R9W
    UD_R_R10W
    UD_R_R11W
    UD_R_R12W
    UD_R_R13W
    UD_R_R14W
    UD_R_R15W

    ' 32 位通用寄存器
    UD_R_EAX
    UD_R_ECX
    UD_R_EDX
    UD_R_EBX
    UD_R_ESP
    UD_R_EBP
    UD_R_ESI
    UD_R_EDI
    UD_R_R8D
    UD_R_R9D
    UD_R_R10D
    UD_R_R11D
    UD_R_R12D
    UD_R_R13D
    UD_R_R14D
    UD_R_R15D

    ' 64 位通用寄存器
    UD_R_RAX
    UD_R_RCX
    UD_R_RDX
    UD_R_RBX
    UD_R_RSP
    UD_R_RBP
    UD_R_RSI
    UD_R_RDI
    UD_R_R8
    UD_R_R9
    UD_R_R10
    UD_R_R11
    UD_R_R12
    UD_R_R13
    UD_R_R14
    UD_R_R15

    ' 段寄存器
    UD_R_ES
    UD_R_CS
    UD_R_SS
    UD_R_DS
    UD_R_FS
    UD_R_GS

    ' 控制寄存器
    UD_R_CR0
    UD_R_CR1
    UD_R_CR2
    UD_R_CR3
    UD_R_CR4
    UD_R_CR5
    UD_R_CR6
    UD_R_CR7
    UD_R_CR8
    UD_R_CR9
    UD_R_CR10
    UD_R_CR11
    UD_R_CR12
    UD_R_CR13
    UD_R_CR14
    UD_R_CR15

    ' 调试寄存器
    UD_R_DR0
    UD_R_DR1
    UD_R_DR2
    UD_R_DR3
    UD_R_DR4
    UD_R_DR5
    UD_R_DR6
    UD_R_DR7
    UD_R_DR8
    UD_R_DR9
    UD_R_DR10
    UD_R_DR11
    UD_R_DR12
    UD_R_DR13
    UD_R_DR14
    UD_R_DR15

    ' MMX 寄存器
    UD_R_MM0
    UD_R_MM1
    UD_R_MM2
    UD_R_MM3
    UD_R_MM4
    UD_R_MM5
    UD_R_MM6
    UD_R_MM7

    ' x87 FPU 寄存器
    UD_R_ST0
    UD_R_ST1
    UD_R_ST2
    UD_R_ST3
    UD_R_ST4
    UD_R_ST5
    UD_R_ST6
    UD_R_ST7

    ' XMM 寄存器
    UD_R_XMM0
    UD_R_XMM1
    UD_R_XMM2
    UD_R_XMM3
    UD_R_XMM4
    UD_R_XMM5
    UD_R_XMM6
    UD_R_XMM7
    UD_R_XMM8
    UD_R_XMM9
    UD_R_XMM10
    UD_R_XMM11
    UD_R_XMM12
    UD_R_XMM13
    UD_R_XMM14
    UD_R_XMM15

    ' YMM 寄存器
    UD_R_YMM0
    UD_R_YMM1
    UD_R_YMM2
    UD_R_YMM3
    UD_R_YMM4
    UD_R_YMM5
    UD_R_YMM6
    UD_R_YMM7
    UD_R_YMM8
    UD_R_YMM9
    UD_R_YMM10
    UD_R_YMM11
    UD_R_YMM12
    UD_R_YMM13
    UD_R_YMM14
    UD_R_YMM15

    UD_R_RIP

    ' 操作数类型
    UD_OP_REG
    UD_OP_MEM
    UD_OP_PTR
    UD_OP_IMM
    UD_OP_JIMM
    UD_OP_CONST
End Enum

' ---------- 结构体 ud_operand ----------
Type ud_operand
    typ As ud_type          ' 操作数类型
    size As uint16_t        ' 操作数大小（位）
    base As ud_type         ' 基址寄存器
    index As ud_type        ' 索引寄存器
    scale As uint8_t        ' 比例因子
    offset As uint8_t       ' 偏移
    lval As ud_lval         ' 立即数值或地址
    _legacy As uint64_t     ' 内部使用（将在1.8中移除）
    _oprcode As uint8_t     ' 内部使用
End Type

Type ud
    ' 输入钩子
    inp_hook As Function(ByVal As ud Ptr) As Integer

    #ifndef __UD_STANDALONE__
    inp_file As PVOID ' FILE Ptr
    #endif

    inp_buf As Const uint8_t Ptr
    inp_buf_size As size_t
    inp_buf_index As size_t
    inp_curr As uint8_t
    inp_ctr As size_t
    inp_sess(63) As uint8_t   ' 固定大小 64
    inp_end As Integer
    inp_peek As Integer

    translator As Sub(ByVal As ud Ptr)
    insn_offset As uint64_t
    insn_hexcode(63) As Byte   ' char[64]

    ' 汇编输出缓冲区
    asm_buf As ZString Ptr
    asm_buf_size As size_t
    asm_buf_fill As size_t
    asm_buf_int(127) As Byte   ' char[128]

    ' 符号解析器回调
    sym_resolver As Function(ByVal As ud Ptr, ByVal As uint64_t, ByVal As int64_t Ptr) As Const ZString Ptr

    dis_mode As uint8_t
    pc As uint64_t
    vendor As uint8_t
    mnemonic As ud_mnemonic_code
    operand(3) As ud_operand   ' 最多 4 个操作数
    error As uint8_t
    _rex As uint8_t
    pfx_rex As uint8_t
    pfx_seg As uint8_t
    pfx_opr As uint8_t
    pfx_adr As uint8_t
    pfx_lock As uint8_t
    pfx_str As uint8_t
    pfx_rep As uint8_t
    pfx_repe As uint8_t
    pfx_repne As uint8_t
    opr_mode As uint8_t
    adr_mode As uint8_t
    br_far As uint8_t
    br_near As uint8_t
    have_modrm As uint8_t
    modrm As uint8_t
    modrm_offset As uint8_t
    vex_op As uint8_t
    vex_b1 As uint8_t
    vex_b2 As uint8_t
    primary_opcode As uint8_t
    user_opaque_data As Any Ptr
    ' itab_entry 和 le 指针需要声明为内部类型，这里用 Any Ptr 代替
    itab_entry As Any Ptr
    le As Any Ptr
End Type

' 函数指针类型定义
Type ud_input_hook_cb As Function(ByVal u As ud Ptr) As Integer
Type ud_syntax_cb As Sub cdecl(ByVal u As ud Ptr)
Type ud_sym_resolver_cb As Function(ByVal u As ud Ptr, ByVal addr As uint64_t, ByRef offset As Integer) As Const ZString Ptr

' ============================= PUBLIC API =================================

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
Declare Sub ud_set_asm_buffer Cdecl Lib "udis86" Alias "ud_set_asm_buffer" (ByVal u As ud Ptr, ByVal buf As ZString Ptr, ByVal size As size_t)

' 设置符号解析回调 (用于地址->符号名转换)
Declare Sub ud_set_sym_resolver cdecl Lib "udis86" Alias "ud_set_sym_resolver" (ByVal u As ud Ptr, ByVal resolver As ud_sym_resolver_cb)

' ==========================================================================

' 主程序
Sub Main()
    ' 待反汇编的机器码：c3 (ret) 和 4c 8b dc (mov r11, rsp)
    Dim As UByte code(3) = { &hC3, &h4C, &h8B, &hDC }
    
    ' 创建 ud 对象
    Dim u As ud
    
    ' 初始化 udis86
    ud_init @u
    
    ' 设置为 64 位模式
    ud_set_mode(@u, 64)
    
    ' 设置 Intel 语法（默认）
    ud_set_syntax(@u, UD_SYN_INTEL)
    
    ' 将字节数组设置为输入源
    ud_set_input_buffer(@u, @code(0), 4)
    
    ' 可选：设置程序计数器（默认0）
    ud_set_pc(@u, &H7FF600000000)
    
    ' 反汇编循环
    While ud_disassemble(@u)
        Dim asmText As Const ZString Ptr = ud_insn_asm(@u)
        Dim hexText As Const ZString Ptr = ud_insn_hex(@u)
        Dim offset As UInt64 = ud_insn_off(@u)
        
        ' 标准格式：000000000000000c  0c3          ret
        Print Hex$(offset, 16); "  ";
        Print Left(*hexText + Space(20), 20); *asmText
    Wend
End Sub