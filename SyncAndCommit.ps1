# SyncAndCommit.ps1 - publish a verified SnowSword build
# The development repository is the source of truth. This script is the only
# supported path for copying a verified source/build snapshot to the public repo.

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# ===================== User configuration =====================
$RepoDir = "D:\GitHub Project\SnowSword\"
$R3_Source_Root = "D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\"
$R3_Output_Exe = "D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release64\SnowSword.exe"
$R0_Source_Root = "C:\Users\21607\source\repos\SnowSword\"
$R0_Output_Sys = "C:\Users\21607\source\repos\SnowSword\x64\Debug\SnowSword.sys"
$GitUserName = "user-lzy"
$GitUserEmail = "2160752730@qq.com"
$RemoteURL = "https://github.com/user-lzy/SnowSword.git"
$TargetBranch = "main"
# =============================================================

function Fail([string]$Message) {
    Write-Host "发布已中止：$Message" -ForegroundColor Red
    exit 1
}

function Require-Directory([string]$Path, [string]$Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) { Fail "$Description不存在：$Path" }
}

function Require-File([string]$Path, [string]$Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { Fail "$Description不存在：$Path" }
}

function Invoke-Robocopy([string]$Source, [string]$Destination, [string[]]$ExcludedDirectories) {
    & robocopy $Source $Destination /E /XD $ExcludedDirectories /NJH /NJS /NP
    if ($LASTEXITCODE -gt 7) { Fail "同步失败：$Source -> $Destination，robocopy exit code=$LASTEXITCODE" }
}

function Confirm-Action([string]$Prompt) {
    $answer = Read-Host "$Prompt (Y/N，默认:N)"
    return $answer -match "^[Yy]$"
}

try {
    Clear-Host
    Write-Host "=========================================================" -ForegroundColor Cyan
    Write-Host "       SnowSword 已验证版本发布工具" -ForegroundColor White -BackgroundColor DarkBlue
    Write-Host "=========================================================" -ForegroundColor Cyan

    Require-Directory $RepoDir "公开仓库"
    Require-Directory $R3_Source_Root "R3 源码目录"
    Require-Directory $R0_Source_Root "R0 源码目录"
    Require-File $R3_Output_Exe "R3 编译产物"
    Require-File $R0_Output_Sys "R0 编译产物"

    Set-Location -LiteralPath $RepoDir
    Require-Directory (Join-Path $RepoDir ".git") "Git 元数据目录"
    $branch = (git branch --show-current).Trim()
    if ($branch -ne $TargetBranch) { Fail "公开仓库当前分支为 '$branch'，要求在 '$TargetBranch' 发布" }
    $remote = (git remote get-url origin 2>$null).Trim()
    if ($remote -ne $RemoteURL) { Fail "origin 地址不匹配：$remote" }
    if ((git status --porcelain)) { Fail "公开仓库存在未提交改动，请先清理后再发布" }

    if (-not (Confirm-Action "确认发布到 $RepoDir，并清理 ring3、ring0、bin 的旧内容？")) {
        Write-Host "已取消发布" -ForegroundColor Yellow
        exit 0
    }

    Write-Host "[1/6] 清理旧发布内容..." -ForegroundColor Yellow
    Remove-Item -LiteralPath (Join-Path $RepoDir "ring3") -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $RepoDir "ring0") -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $RepoDir "bin") -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path (Join-Path $RepoDir "ring3"), (Join-Path $RepoDir "ring0") | Out-Null

    $excluded = @("release64", "Release", "Debug", "x64", ".git", ".vercel", ".vs", "bin")
    Write-Host "[2/6] 同步 R3 源码..." -ForegroundColor Yellow
    Invoke-Robocopy $R3_Source_Root (Join-Path $RepoDir "ring3") $excluded
    Write-Host "[3/6] 同步 R0 源码..." -ForegroundColor Yellow
    Invoke-Robocopy $R0_Source_Root (Join-Path $RepoDir "ring0") $excluded

    Write-Host "[4/6] 复制已验证编译产物..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Force -Path (Join-Path $RepoDir "bin\ring3"), (Join-Path $RepoDir "bin\ring0") | Out-Null
    Copy-Item -LiteralPath $R3_Output_Exe -Destination (Join-Path $RepoDir "bin\ring3\SnowSword.exe") -Force
    Copy-Item -LiteralPath $R0_Output_Sys -Destination (Join-Path $RepoDir "bin\ring0\SnowSword.sys") -Force

    Write-Host "[5/6] 检查待提交文件..." -ForegroundColor Yellow
    git add -A
    $staged = @(git diff --cached --name-only)
    $forbidden = @($staged | Where-Object {
        $_ -match '(^|/)(\.git|\.vercel|release64|Release|Debug|x64)(/|$)' -or
        $_ -match '\.(pdb|obj|lib|dll|log|tmp|bak)$'
    })
    if ($forbidden.Count -gt 0) {
        git reset --quiet
        Fail "检测到不允许发布的文件：$($forbidden -join ', ')"
    }
    if ($staged.Count -eq 0) { Write-Host "没有需要发布的变更" -ForegroundColor Yellow; exit 0 }
    git status --short

    $customMsg = Read-Host "请输入发布备注（留空使用默认备注）"
    $date = Get-Date -Format "yyyy-MM-dd"
    $commitMsg = if ([string]::IsNullOrWhiteSpace($customMsg)) { "chore: publish verified build | $date" } else { "$customMsg | $date" }
    if (-not (Confirm-Action "确认创建公开仓库提交：$commitMsg")) {
        git reset --quiet
        Write-Host "已取消发布，已保留文件但撤销暂存" -ForegroundColor Yellow
        exit 0
    }

    git -c user.name=$GitUserName -c user.email=$GitUserEmail commit -m $commitMsg
    if ($LASTEXITCODE -ne 0) { Fail "Git commit 失败" }
    $tag = "verified-{0}-{1}" -f (Get-Date -Format "yyyyMMdd"), (Get-Date -Format "HHmmss")
    git tag -a $tag -m "Verified SnowSword release $tag"
    if ($LASTEXITCODE -ne 0) { Fail "创建发布标签失败：$tag" }

    Write-Host "[6/6] 推送提交和标签..." -ForegroundColor Yellow
    git push origin ("HEAD:{0}" -f $TargetBranch)
    if ($LASTEXITCODE -ne 0) { Fail "推送提交失败；标签 $tag 已保留在本地" }
    git push origin $tag
    if ($LASTEXITCODE -ne 0) { Fail "推送标签失败：$tag；提交已推送，请稍后单独推送标签" }
    Write-Host "发布成功：$tag" -ForegroundColor Green
    git show --stat --oneline --decorate HEAD
}
catch {
    Write-Host "发布已中止：$($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
