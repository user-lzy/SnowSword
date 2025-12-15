# sync_and_upload_fixed.ps1 - 修复版同步上传脚本
# 使用方法：右键点击此脚本，选择“使用PowerShell运行”

# ====== 用户配置区域 ======
$SourceRing3 = "D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword32\"
$SourceRing0 = "C:\Users\21607\source\repos\SnowSword\"
$RepoDir = "D:\GitHub Project\SnowSword\"
$GitUserName = "user-lzy"  # 替换
$GitUserEmail = "2160752730@qq.com"  # 替换
$RemoteURL = "https://github.com/user-lzy/SnowSword.git"  # 替换

# ====== 主脚本开始 ======
Clear-Host
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " SnowSword 项目一键同步与上传工具" -ForegroundColor White -BackgroundColor DarkBlue
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# 步骤1: 检查并初始化Git仓库
Write-Host "[1/5] 检查Git仓库配置..." -ForegroundColor Yellow
Set-Location $RepoDir

if (-not (Test-Path ".git")) {
    Write-Host "  初始化新仓库..." -ForegroundColor Green
    git init
    git config user.name $GitUserName
    git config user.email $GitUserEmail
    
    $remoteCheck = git remote -v
    if (-not $remoteCheck) {
        git remote add origin $RemoteURL
	git branch -M main
    }
} else {
    git config user.name $GitUserName
    git config user.email $GitUserEmail
}

# 步骤2: 清理目标目录（删除旧内容）
Write-Host "[2/5] 清理目标目录..." -ForegroundColor Yellow
if (Test-Path "$RepoDir\ring3") {
    Remove-Item "$RepoDir\ring3\*" -Recurse -Force -ErrorAction SilentlyContinue
}
if (Test-Path "$RepoDir\ring0") {
    Remove-Item "$RepoDir\ring0\*" -Recurse -Force -ErrorAction SilentlyContinue
}

# 步骤3: 同步文件（修复robocopy参数）
Write-Host "[3/5] 同步项目文件..." -ForegroundColor Yellow

# 创建必要的目录
New-Item -ItemType Directory -Force -Path "$RepoDir\ring3" | Out-Null
New-Item -ItemType Directory -Force -Path "$RepoDir\ring0" | Out-Null

# 修复的robocopy命令 - 每个参数单独传递
Write-Host "  -> 同步 ring3 (SnowSword32)" -ForegroundColor Gray
robocopy "$SourceRing3" "$RepoDir\ring3" /XF *.txt /MIR /NJH /NJS /NDL /NP /R:2 /W:5

Write-Host "  -> 同步 ring0 (SnowSword)" -ForegroundColor Gray
# 注意：参数分开写，不是合并成一个字符串
robocopy "$SourceRing0" "$RepoDir\ring0" /XD .vs .git /XF *.suo *.user /MIR /NJH /NJS /NDL /NP /R:2 /W:5

# 步骤4: 清理ring3目录中的.git文件夹（关键修复）
Write-Host "[4/5] 清理Git冲突文件..." -ForegroundColor Yellow
$gitDirs = Get-ChildItem -Path "$RepoDir\ring3" -Directory -Filter ".git" -Recurse -Force -ErrorAction SilentlyContinue
foreach ($gitDir in $gitDirs) {
    Write-Host "  删除: $($gitDir.FullName)" -ForegroundColor Gray
    Remove-Item $gitDir.FullName -Recurse -Force -ErrorAction SilentlyContinue
}

# 同样清理ring0中的.git文件夹
$gitDirs0 = Get-ChildItem -Path "$RepoDir\ring0" -Directory -Filter ".git" -Recurse -Force -ErrorAction SilentlyContinue
foreach ($gitDir in $gitDirs0) {
    Write-Host "  删除: $($gitDir.FullName)" -ForegroundColor Gray
    Remove-Item $gitDir.FullName -Recurse -Force -ErrorAction SilentlyContinue
}

# 检查是否有文件被同步
$changes = git status --porcelain
if (-not $changes) {
    Write-Host "  没有检测到文件更改。" -ForegroundColor Yellow
} else {
    # 步骤5: 提交更改（交互式）
    Write-Host "[5/5] 提交更改到Git..." -ForegroundColor Yellow
    Write-Host "`n本次更改的文件：" -ForegroundColor Cyan
    git status -s

    # 交互式提交信息
    Write-Host "`n请选择提交信息类型：" -ForegroundColor Cyan
    Write-Host "  1. 使用默认信息（自动时间戳）" -ForegroundColor Gray
    Write-Host "  2. 输入单行自定义信息" -ForegroundColor Gray
    Write-Host "  3. 输入多行自定义信息" -ForegroundColor Gray

    $choice = Read-Host "`n请输入选择 (1/2/3，默认:1)"
    $commitMessage = ""
    $commitTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    switch ($choice) {
        "2" {
            Write-Host "`n请输入提交信息：" -ForegroundColor Yellow
            $customMsg = Read-Host "> "
            if ($customMsg.Trim() -ne "") {
                $commitMessage = "$customMsg`n`n[自动同步于 $commitTime]"
            } else {
                $commitMessage = "自动更新: $commitTime`n`n同步内容:`n- ring3: SnowSword32项目`n- ring0: SnowSword内核项目"
            }
        }
        "3" {
            Write-Host "`n请输入提交信息（输入空行结束）：" -ForegroundColor Yellow
            Write-Host "（按Enter开始新行，连续两个Enter结束）" -ForegroundColor Gray
            $lines = @()
            do {
                $line = Read-Host "> "
                if ($line.Trim() -ne "") {
                    $lines += $line
                }
            } while ($line.Trim() -ne "")
            
            if ($lines.Count -gt 0) {
                $commitMessage = ($lines -join "`n") + "`n`n[自动同步于 $commitTime]"
            } else {
                $commitMessage = "自动更新: $commitTime`n`n同步内容:`n- ring3: SnowSword32项目`n- ring0: SnowSword内核项目"
            }
        }
        default {
            $commitMessage = "自动更新: $commitTime`n`n同步内容:`n- ring3: SnowSword32项目`n- ring0: SnowSword内核项目"
        }
    }

    Write-Host "`n即将使用以下信息提交：" -ForegroundColor Cyan
    Write-Host "----------------------------------------" -ForegroundColor Gray
    Write-Host $commitMessage -ForegroundColor White
    Write-Host "----------------------------------------" -ForegroundColor Gray

    $confirm = Read-Host "`n是否确认提交？(Y/N，默认:Y)"
    if ($confirm -ne "N" -and $confirm -ne "n") {
        Write-Host "  添加文件到暂存区..." -ForegroundColor Gray
        git add .
        
        Write-Host "  创建提交..." -ForegroundColor Gray
        git commit -m $commitMessage
	git remote -v
        
        # 推送到GitHub
        Write-Host "  推送到GitHub..." -ForegroundColor Gray
        try {
            git push -u origin main
            Write-Host "  推送成功！" -ForegroundColor Green
        } catch {
            git push -u origin master
        }
    }
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "操作完成！" -ForegroundColor Green
Write-Host ""
Write-Host "按 Enter 键退出..." -ForegroundColor Gray
Read-Host