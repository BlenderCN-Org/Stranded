@echo off && setlocal ENABLEDELAYEDEXPANSION
color 0a
CHCP 936>nul 2>nul||set path=%systemroot%\system32;%path%
chcp 936>nul 
title 海马玩模拟器（Droid4X）设置修改工具 v1.1.8
:BatchGotAdmin
:-------------------------------------
cls
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params = %*:"=""
    echo UAC.ShellExecute "cmd.exe", "/c %~s0 %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del /q /f "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------
:begin
echo.
echo.正在查找 Oracle VM VirtualBox 路径...
echo.
if exist "%ProgramFiles%\Oracle\VirtualBox\VBoxManage.exe" (
	set VboxPath=%ProgramFiles%\Oracle\VirtualBox\
) else (
    for /f "tokens=2,*" %%i in ('"reg query "HKLM\SOFTWARE\Oracle\VirtualBox" /v InstallDir 2>nul|findstr /i "InstallDir" 2>nul"') do (
		if exist "%%~jVBoxManage.exe" (
        	set VboxPath=%%~j
		)
    )
)
echo. 正在查找 Droid4X 安装路径...
if exist "%windir%\sysWOW64" (
	set D4Reg="HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Droid4X"
) else (
	set D4Reg="HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Droid4X"
)
for /f "tokens=2,*" %%i in ('"reg query %D4Reg% /v DisplayIcon 2>nul|findstr /i "Droid4X" 2>nul"') do (
	rem 获取D4路径
	set D4Path=%%~dpj
)
echo.
if not defined D4Path (
	echo. 无法找到你的海马玩模拟器，请在安装模拟器后再进行操作。
	PAUSE
	exit
)

if not defined VboxPath (
	echo. 无法找到电脑系统中的 Oracle VM VirtualBox 虚拟机，本设置修改工具将无法正常运作
	echo.
	echo. 本程序将为你重新安装 Oracle VM VirtualBox
	echo.
	pause
	call :ReInstallVBOX
)

cd /d "%VboxPath%"
call :Info
call :Format

set port=53001
:menu
cls
:menuNocls
echo.
rem 初始化变量
set memory=
set DPI=
set pixels=
set updata=
set menu=
set Q=
set checks=
set adb="%D4Path%\adb.exe" -s 127.0.0.1:%port%
TASKLIST /FI "USERNAME ne NT AUTHORITY\SYSTEM" /FI "STATUS eq running"|findstr /i "Droid4X.exe VboxHeadless.exe" >nul 2>nul && (
	echo.
	echo. Droid4X 模拟器正在运行中，你不能进行任何的修改, 只能查看 Droid4X 的状态信息
	echo.
	echo -----===== 请选择要执行的操作 =====-----
	echo. 1. 修改 adb 连接端口
	echo. 2. 重新建立模拟器 ADB 链接
	echo. 3. adb shell 进入模拟器
	echo. 4. 重新挂载 SD 卡
	echo. 
	echo. 6. 查看已连接的安卓设备信息
	echo. 7. 进入 VirtualBox 目录
	echo. 8. 进入 海马玩模拟器 目录
	echo. 9. 导入数据包 ^& Hosts文件
	echo. 10. 查看 Droid4X 设置信息
	echo. 0. 更新日志 ^& 更新地址
	echo.
	echo. 如果要修改设置, 请关闭模拟器后再运行本程序
echo -----==================================-----
set /p checks="请输入编号选择要执行的操作："
	if /i "!checks!"=="0" goto :log
	if /i "!checks!"=="1" (
		echo 当前端口为：127.0.0.1:%port%
		set /p port="修改端口为（必须以530xx开头，从53001开始递增）："
	)
	if /i "!checks!"=="2" call :ConnectDroid4x
	if /i "!checks!"=="3" call :ConnectDroid4x && %adb% shell
	if /i "!checks!"=="4" call :RemountSD
	if /i "!checks!"=="6" goto :Devices
	if /i "!checks!"=="7" cd /d "%VboxPath%" && cmd
	if /i "!checks!"=="8" cd /d "%D4Path%" && cmd
	if /i "!checks!"=="9" goto :InstallData
	if /i "!checks!"=="10" goto :Report
	if /i "!checks!"=="*" goto :checkPort
	if /i "!checks!"=="-" goto :setting
	cls
	goto :menu
)

:Setting
cls
echo.
if defined VboxPath (
	echo 你的 Oracle VirtualBox 路径是：%VboxPath%
) else (
	echo 无法找到电脑系统中的 Oracle VirtualBox
)
echo.
echo -----===== 请选择要执行的操作 =====-----
echo. 1. 修改分辨率及DPI
echo. 2. 修改分辨率
echo. 3. 修改DPI
echo. 4. 修改运行内存
echo. 5. 修改模拟器CPU核心数量
echo. --------------------------------------
echo. 6. 显示\关闭虚拟软键
echo. 7. 打开 VirtualBox 管理器
echo. 9. 修复 Oracle VM VirtualBox
echo. 10. 查看 Droid4X 设置信息
echo. 11. 进入 VirtualBox 目录
echo. 12. 进入 海马玩模拟器 目录
echo. 0. 更新日志 ^& 更新方法
echo. --------------------------------------
echo.
echo. 如果本程序在运行过程中出错，或者无法实现修改效果，请以“管理员身份运行”
echo. by. 极品小猫（QQ：391616001）		Droid4X官方Q群：369149979
echo -----==============================-----
set /p menu="请输入编号选择要执行的操作："
if "%menu%"=="1" goto :Graph
if "%menu%"=="2" (set DPI=%DefaultDPI%) && goto :Graph
if "%menu%"=="3" (set pixels=%DefaultGraph%) && goto :DPI
if "%menu%"=="4" goto :Memory
if "%menu%"=="5" goto :CPU
if "%menu%"=="6" goto :navBar
if "%menu%"=="7" start "" "%VboxPath%VirtualBox.exe"
if "%menu%"=="9" goto :ReInstallVBOX
if "%menu%"=="10" goto :Report
if "%menu%"=="11" cd /d "%VboxPath%" && cmd
if "%menu%"=="12" cd /d "%D4Path%" && cmd
if "%menu%"=="*" goto :checkPort
if "%menu%"=="0" goto :log
cls
goto :menu

:Graph
cls
echo.
echo ------------- 横屏分辨率预设 -------------
echo.  1.  480 x  320 DPI 120 (横屏)
echo.  2.  640 x  360 DPI 160 (横屏)
echo.  3.  800 x  480 DPI 240 (横屏)
echo.  4.  960 x  540 DPI 280 (横屏)
echo.  5. 1280 x  720 DPI 320 (横屏)
echo.  6. 1440 x  900 DPI 320 (横屏)
echo.  7. 1920 x 1080 DPI 480 (横屏)
echo. 
echo ------------- 竖屏分辨率预设 -------------
echo.  8.  320 x  480 DPI 120 (竖屏)
echo.  9.  360 x  640 DPI 160 (竖屏)
echo. 10.  480 x  800 DPI 240 (竖屏)
echo. 11.  540 x  960 DPI 280 (竖屏)
echo. 12.  720 x 1280 DPI 320 (竖屏)
echo. 13.  900 x 1440 DPI 320 (竖屏)
echo. 14. 1080 x 1920 DPI 480 (竖屏)
echo ------------------------------------------
echo.  0. ★★★★ 自定义分辨率 及 DPI ★★★★
echo.
echo. Q. 回到功能菜单
echo.
echo. Droid4X 当前所使用的分辨率为：%DefaultGraph%
echo.
set /p input="请输入编号选择要执行的操作："
echo ----------------------------------------
if /i "%input%"=="0" (
	echo.
	echo.===== 自定义注意事项 =====
	echo. 自定义分辨率前，请先充分了解该分辨率所适应的DPI！！
	echo.
	echo. 模拟器内部的分辨率不受显示器分辨率约束，窗口则必须按照显示器分辨率显示。
	echo. 如果自定义分辨率≥显示器分辨率，如果DPI不适应，容易造成显示异常。
	echo.
	set /p width="请输入宽度："
	set /p height="请输入高度："
	goto :ReInput
)
rem 横屏
if /i "%input%"=="1" call :Set_Pixels_DPI 480x320 120
if /i "%input%"=="2" call :Set_Pixels_DPI 640x360 160
if /i "%input%"=="3" call :Set_Pixels_DPI 800x480 240
if /i "%input%"=="4" call :Set_Pixels_DPI 960x540 280
if /i "%input%"=="5" call :Set_Pixels_DPI 1280x720 320
if /i "%input%"=="6" call :Set_Pixels_DPI 1440x900 320
if /i "%input%"=="7" call :Set_Pixels_DPI 1920x1080 480
rem 竖屏
if /i "%input%"=="8" call :Set_Pixels_DPI 320x480 120
if /i "%input%"=="9" call :Set_Pixels_DPI 360x640 160
if /i "%input%"=="10" call :Set_Pixels_DPI 480x800 240
if /i "%input%"=="11" call :Set_Pixels_DPI 540x960 280
if /i "%input%"=="12" call :Set_Pixels_DPI 720x1280 320
if /i "%input%"=="13" call :Set_Pixels_DPI 900x1440 320
if /i "%input%"=="14" call :Set_Pixels_DPI 1080x1920 480
if /i "%input%"=="Q" cls && goto :menu
if not defined menu goto :Graph
cls
goto :Graph

:ReInput
echo.
if DEFINED width (
	echo %width%|findstr "^[0-9]*$">nul && (
		if DEFINED height (
			echo %height%|findstr "^[0-9]*$">nul && (
				set pixels=%width%x%height%-16
			) || (
				set /p height="高度只能输入数字，请重新输入："
				goto :ReInput
			)
		) else (
			if /i "%input%"=="0" (
				set /p height="你没有输入高度，请重新输入："
				goto :ReInput
			)
		)
	) || (
		set /p width="宽度只能输入数字，请重新输入："
		goto :ReInput
	)
) else (
	if /i "%input%"=="0" (
		set /p width="你没有输入宽度，请重新输入："
		goto :ReInput
	)
)
echo.
echo 你修改的分辨率为：%pixels%
echo.
echo ----------------------------------------
echo.
echo. Droid4X 当前所使用的分辨率为：%DefaultDPI%
echo.
:DPI
echo. Droid4X 当前所使用的DPI为：%DefaultDPI%
echo.
echo. DPI为屏幕像素密度，如果你不懂什么是DPI，请不要修改此数值。
echo.如果分辨率^＜1024*768，请设置DPI为：240。
echo.
if not defined DPI set /p DPI="请输入修改DPI的数值："
if DEFINED DPI (
	echo %DPI%|findstr "^[0-9]*$">nul || (
		set /p DPI="DPI只能输入数字，请重新输入："
		goto :DPI
	)
) else (
	goto :DPI
)
echo 你修改的DPI为：%DPI%
echo.
echo ----------------------------------------
echo.
:ok
CLS
echo.
echo ----------------------------------------
echo Droid4X 当前所使用的分辨率为：%DefaultGraph%
echo.
echo 你修改的分辨率为：%pixels%
echo ----------------------------------------
echo.
echo ----------------------------------------
echo Droid4X 当前所使用的DPI为：%DefaultDPI%
echo.
echo 你修改的DPI为：%DPI%
echo ----------------------------------------
echo.
set /p ok="请确认以上信息是否正确，输入[Y-正确/N-回到菜单]："
if DEFINED ok (
	echo %ok%|findstr /i "Y" >nul 2>nul && (
		VBoxManage guestproperty set droid4x vbox_graph_mode %pixels%
		VBoxManage guestproperty set droid4x vbox_dpi %DPI%
		set DefaultDPI=%DPI%
		set DefaultGraph=%pixels%
		echo.
		echo 修改完成，按任意键后回到菜单
		pause>nul
		cls
		goto :menu
	) || echo %ok%|findstr /i "N" >nul 2>nul && (
		cls
		goto :menu
	) || (
		cls
		goto :ok
	)
) else (
	cls
	goto :ok
)

:Memory
cls
echo. 当前 Droid4X 的运行内存：%Defaultmemory%
echo.
echo. ★ 1G=1024M，4G内存以上的用户推荐：2048M，4G以下建议：640M～1024M。
echo. ★ 少于1G内存，有可能导致游戏无法运行，或者出现各种闪退或者崩溃。
echo. ★ 运行内存是直接从系统内存划分的，因此【运行内存^<系统内存】。
echo. ★ 默认1G内存足以运行各种大型3D游戏，请根据条件适量分配，最大：3.5G。
echo.
echo. ★ 输入下面对应的数字，可自动换算对应的运行内存
echo.
echo.   【1＝1024M】  【2＝2048M】  【3＝3072M】  【3.75＝3840M】
echo.
if not defined memory set /p memory="请输入运行内存数值(无需单位)："
echo.
if not defined memory (
	goto :Memory
)
if /i "%memory%"=="1" set memory=1024
if /i "%memory%"=="2" set memory=2048
if /i "%memory%"=="3" set memory=3072
if /i "%memory%"=="3.75" set memory=3840

if defined memory (
	if %memory% lss 512 (
		set /p memory="ψ(╰_╯) 别那么吝啬~~想要运行流畅最低只能512M，重新输入吧："
		goto :Memory
	) else if %memory% GEQ 3000 (
		set /p memory="(⊙０⊙) 土豪~给那么多也是浪费的，建议 1024 ～ 2048 够了，重新输入吧："
		goto :Memory
	)
	echo %memory%|findstr "^[0-9]*$">nul && (
		VBoxManage modifyvm droid4x --memory %memory%
		echo.
		set Defaultmemory=%memory%
		echo. 修改完成，当前运行内存为：%memory%M，按任意键后回到菜单
		pause>nul
		cls
		goto :menu
	) || (
		set /p memory="运行内存数值只能输入数字，请重新输入："
		goto :Memory
	)
)
cls
goto :Memory

:navBar
cls && set navBar=
echo.
echo. ----------========== 虚拟软键开关设置 ==========----------
VBoxManage guestproperty get droid4x droid4x_force_navbar|findstr /i "No 0">nul &&(
echo 虚拟软键状态：已关闭 ) || (echo 虚拟软键状态：已开启)
echo.
echo. 0. 关闭虚拟软键
echo. 1. 开启虚拟软键
echo. M. 回到功能菜单
set /p navBar="请输入编号选择要执行的操作："
if "%navBar%"=="0" VBoxManage guestproperty set droid4x droid4x_force_navbar 0
if "%navBar%"=="1" VBoxManage guestproperty set droid4x droid4x_force_navbar 1
if /i "%navBar%"=="m" goto :menu
cls
goto :navBar

:CPU
cls && set modCPU=
echo.
echo. ----------========== CPU核心数量设置 ==========----------
if not defined DefaulCPUCores CALL :info
echo.
if EXIST %windir%\SYSTEM32\wbem\wmic.exe (
	echo. CPU 实际核心线程最大为：%DefaulCPUCores%
	echo. 海马玩模拟器核心数量为：%DefaulLogical%
	echo.
) else (
	if EXIST "%~dp0wmic.exe" (
		call %~dp0wmic.exe exit
		goto :CPU
	) else (
		echo. 你的电脑中缺乏 WMIC 检测程序，请下载所需要的程序，然后与本程序放在同一目录下
		echo.
		echo. 下载地址：http://www.droid4x.cn/bbs/forum.php?mod=viewthread^&tid=13054
		echo.
		call :CopyUrl
		PAUSE && goto :menu
	)
)
echo. 核心数量将直接影响模拟器的运行速度，如果不是有特殊需求，请不要更改此值。

echo. 核心数量最低为2，最高为 CPU 实际核心数量
echo.
set /p "modCPU=请输入要修改的核心数量【M-可返回主菜单】："
if /i "%modCPU%"=="m" goto :menu
if not defined modCPU goto :cpu
echo %modCPU%|findstr /i "[0-9]*">nul 2>nul&& (
	if %modCPU% LEQ 1 goto :cpu
	if %modCPU% GTR %DefaulCPUCores% goto :cpu
	"%VboxPath%VBoxManage" modifyvm droid4x --cpus %modCPU%
	for /f "tokens=3,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"CPUs"') do set DefaulLogical=%%j
	echo 修改成功，当前模拟器的核心数量为：%DefaulCPUCores%
	pause
)
goto :CPU
pause

:Report
cls
call :Info
echo.
echo. ----------========== Droid4X 设置信息 ==========----------
echo.
echo. 运行内存：%Defaultmemory%
echo. 
echo. 分辨率：%DefaultGraph%
echo.
echo. DPI：%DefaultDPI%
echo.
echo. CPU 实际核心线程最大为：%DefaulCPUCores%
echo. 海马玩模拟器核心数量为：%DefaulLogical%
echo.
echo. Droid4X 目录：%D4Path%
echo.
echo. VirtualBox 目录：%VboxPath%
echo.
echo. 虚拟软键状态：%navBarStart%
echo.
echo. ----------======================================----------
echo.
echo. 按任意键后回到菜单
pause>nul
goto :menu

:Devices
cls
echo.
echo. ----------========== 已连接安卓设备信息 ==========----------
echo.
"%D4Path%adb.exe" devices -l
echo. ----------========================================----------
goto :menuNocls

:InstallData
cls
echo. 
echo. 由于 adb push 导入数据包的效率和速度问题，最后决定移除该功能。
echo. 如果你需要导入数据包，建议参考该教程进行。
echo.
echo. http://www.droid4x.cn/bbs/forum.php?mod=viewthread^&tid=5869
call :CopyUrl
echo. 
echo. 按任意键后回到菜单
pause>nul && goto :menu

:ReInstallVBOX
cls
echo.
echo. ----------========== 修复 VirtualBox 故障==========----------
echo.
echo. 本功能用于修复模拟器安装异常，以及使用了其它安卓模拟器导致无法正常启动。
echo.
echo. 修复 VirtualBox 不会对你的【海马玩模拟器】的数据造成任何影响。
echo.
echo. ----------=====================================----------
echo.
set InstallVBOX=
set deleteVBOX=
set /p InstallVBOX="要修复 VirtualBox 请输入 [Y-确认\留空-跳过]："
if /i "%InstallVBOX%"=="Y" (
		echo. 正在卸载 VirtualBox
		msiexec /x "{D90E08B8-E7BB-4D29-8249-8670D4CC24BD}" /q
		msiexec /x "{B5121457-0126-4E62-BCBF-6DC7C73D9E4A}" /q
		reg delete "HKLM\SOFTWARE\Oracle\VirtualBox" /f
		echo.
		echo. 现在请先尝试启动你的模拟器，如果依然无法启动，请尝试删除 .VirtualBox 配置数据。
		echo.
		echo. .VirtualBox 配置数据包括已注册的虚拟机，如果你是虚拟机的常用户，删除数据之后请自行重新注册相关的虚拟机。
		echo.
		set /p deleteVBOX="是否要删除 .VirtualBox 配置数据 [Y-确认\留空-跳过]："
		echo. 
		if /i "!deleteVBOX!"=="Y" (
			rd /q /s "%USERPROFILE%\.VirtualBox"
			echo. 删除完毕，请再次尝试启动你的模拟器。
			echo. 如果仍然无法启动，请重新安装模拟器。
			if EXIST "%D4Path%VirtualBox VMs\droid4x\droid4x.vbox" (
				echo. 正在为你重新注册 Droid4X 虚拟机
				"%VboxPath%vboxmanage.exe" registervm "%D4Path%VirtualBox VMs\droid4x\droid4x.vbox"
			) else (
				echo 无法找到你的 Droid4X 虚拟机，如果你变更了路径，请手动注册 droid4x.vbox
			)
		)
		pause
	)
)
CLS
goto :begin

:checkPort
cls
echo.
echo -----===== Droid4X 启动异常检查 =====-----
echo.
echo. 以下功能为客服和技术人员使用, 普通用户请在指导下使用.
echo.
echo. 1. 查看 %port% 端口
echo. 2. 查看 127.0.0.1 所有端口
echo. 3. logcat 日志记录
echo. 4. 查看 APK 包名
echo. 0. 回到菜单
echo.
echo. ----------=====================----------
echo.
set checkPort=
set /p checkPort="请输入编号选择要执行的操作："
if /i "%checkPort%"=="1" netstat -ano|findstr %port% && pause>nul
if /i "%checkPort%"=="2" netstat -ano|findstr 127.0.0.1* && pause>nul
if /i "%checkPort%"=="3" (
		del /q /f %Desktop%\Droid4X_RunLog.log
		echo. 已启动日志记录功能
		echo. 日志记录位置在桌面, 文件名为: Droid4X_RunLog.log
		echo. 如果需要终止日志记录, 请按下 Ctrl + C , 并在弹出的提示中选择 N.
		%adb% shell logcat -s ActivityManager>>%Desktop%\Droid4X_RunLog.log
		goto :checkPort
)
if /i "%checkPort%"=="4" goto :package
if /i "%checkPort%"=="0" goto :menu
goto :checkPort

:package
set InstallData=
set /p InstallData="请拖入APK文件至本窗口中："
echo.
if /i "%InstallData%"=="M" GOTO :checkPort
if defined InstallData (
	echo %InstallData%
	%D4Path%\aapt.exe d badging %InstallData% | grep 'package:'
)
pause
goto :package

:log
CLS
echo.
echo. ----------========== 更新日志 ==========----------
echo. v1.0.0
echo. 1. [第一版] 修改 分辨率 及 DPI 功能
echo. 
echo. v1.0.1
echo. 1. [增加] 加入运行内存修改功能
echo.
echo. v1.0.2
echo. 1. [修正] 修正设置工具数据初始化
echo. 2. [修正] 修正部分信息提示文字错误
echo.
echo. v1.0.3 [2014-11-13]
echo. 1. [增加] 设置信息查看
echo. 2. [增加] 更新日志
echo. 3. [修改] 提高运行内存修改最低下限至 512M （为了保证 Droid4X 运行稳定）
echo. 4. [修改] 修改部分信息提示方式
echo.
echo. v1.1.0 [2014-11-25]
echo. 1. [增加] 运行 Droid4X 时可查看设置信息、更新日志、已连接的安装设备信息
echo. 2. [增加] 运行 Droid4X 时可安装 APK 功能，该功能仅用于客服协助调试 APK 安装异常和失败
echo. 3. [增加] 重装 VirtualBOX 功能
echo. 4. [优化] 优化信息加载模块，提升加载效率
echo. 5. [优化] 优化工具代码，修正BUG
echo.
echo. v1.1.1 [2014-11-26]
echo. 1. [增加] 提供更多预设分辨率
echo. 2. [修正] 修改设置后查看设置信息无刷新问题
echo. 3. [修正] 修改内存时无输入会退出的问题
echo.
echo. v1.1.2 [2014-11-27]
echo. 1. [增加] Droid4X 启动异常检查 功能(技术/客服使用)
echo.
echo. v1.1.3 [2015-01-08]
echo. 1. 【增加】海马玩模拟器 ADB 重新连接功能（解决按键精灵助手找不到模拟器问题）
echo. 2. 【增加】默认分辨率：360*640 DPI 160 横竖屏
echo. 3. 【修正】默认分辨率：480*320 DPI 120 的 DPI 错误
echo. 4. 【优化】VirtualBox 修复功能
echo. 5. 【优化】各类提示以及信息加载模块
echo.
echo. v1.1.4 [2015.01.12]
echo. 1. 【修正】删除 .VirtualBox 配置数据之后没有重新注册Droid4X虚拟机的问题。
echo.
echo. v1.1.5 [2015.04.30]
echo. 1. 【增加】数据包 与 Hosts 导入功能（夭折，请看教程）
echo. 2. 【增加】进入软件目录、虚拟机目录、及与 adb 相关功能
echo. 3. 【增加】显示\关闭虚拟软键的功能
echo. 4. 【优化】部分模块代码
echo.
echo. v1.1.6 [2015.06.15]
echo. 1. 【修正】未启动模拟器时进入虚拟机目录及模拟器目录的闪退问题
echo. 2. 【增加】模拟器核心数量修改功能，仅支持 0.8.2 以上版本
echo. 3. 【调整】部分菜单编号
echo.
echo. v1.1.7 [2015.07.31]
echo. 1. 【修正】内存修改的一些提示和建议
echo. 2. 【修正】CHCP 错误提示问题（该问题仍为系统问题，此处仅为临时修正系统环境问题）
echo. 
echo. v1.1.8 [2015.08.02]
echo. 1. 【修正】内存修改无效，并增加一些快速选择的方式
echo. 2. 【增加】UAC提权VBS脚本，避免低权限无法执行修改命令
echo.
echo. 更新地址：http://www.colafile.com/u/%%E6%%9E%%81%%E5%%93%%81%%E5%%B0%%8F%%E7%%8C%%AB
set /p updata="如果需要更新，请输入[Y-打开更新页面/留空回到主菜单]："
if /i "%updata%"=="Y" (start "" "http://www.colafile.com/u/极品小猫")
cls
goto :menu


Rem ========== Callback ==========

:info
echo 正在获取 Droid4X 的基本信息...
for /f "tokens=1,*" %%i in ('VBoxManage.exe guestproperty get droid4x vbox_graph_mode') do (
	rem 获取分辨率
	set DefaultGraph=%%j
) & for /f "tokens=1,*" %%i in ('VBoxManage.exe guestproperty get droid4x vbox_dpi') do (
	rem 获取DPI
	set DefaultDPI=%%j
) & for /f "tokens=2,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"memory size:"') do (
	rem 获取运行内存
	set Defaultmemory=%%j
) & for /f "tokens=3,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"CPUs"') do (
	rem 获取核心线程数量
	set DefaulLogical=%%j
) & if EXIST %windir%\SYSTEM32\wbem\wmic.exe (
	for /f "skip=1" %%i in ('wmic cpu get NumberOfLogicalProcessors^|findstr /r .') do (
		rem 硬件最大核心数量
		set DefaulCPUCores=%%i
	)
) & VBoxManage guestproperty get droid4x droid4x_force_navbar|findstr /i "No 0">nul &&(
	set navBarStart=已关闭) || (set navBarStart=已开启)
exit /b

:Set_Pixels_DPI
echo Y
set pixels=%1-16
set DPI=%2
goto :ok

:Format
for /f "tokens=2,*" %%i in ('"reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v Desktop 2>nul"') do (
	rem 获取桌面路径
	set Desktop="%%j"
)
exit /b

:ConnectDroid4x
echo 重新连接 海马玩模拟器
"%D4Path%\adb.exe" connect 127.0.0.1:%port%
"%D4Path%\adb.exe" -s 127.0.0.1:%port% shell mount -o,remount rw /system
echo.
exit/b

:RemountSD
call :ConnectDroid4x
"%D4Path%\adb.exe" -s 127.0.0.1:%port%  remount
echo 已经重新挂载SD卡，请在模拟器中再次尝试打开SD卡目录
pause
exit/b

:CopyUrl
echo. (复制网址办法：右键→标记→选择网址→右键→复制成功)
exit/b