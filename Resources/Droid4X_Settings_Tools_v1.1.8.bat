@echo off && setlocal ENABLEDELAYEDEXPANSION
color 0a
CHCP 936>nul 2>nul||set path=%systemroot%\system32;%path%
chcp 936>nul 
title ������ģ������Droid4X�������޸Ĺ��� v1.1.8
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
echo.���ڲ��� Oracle VM VirtualBox ·��...
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
echo. ���ڲ��� Droid4X ��װ·��...
if exist "%windir%\sysWOW64" (
	set D4Reg="HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Droid4X"
) else (
	set D4Reg="HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Droid4X"
)
for /f "tokens=2,*" %%i in ('"reg query %D4Reg% /v DisplayIcon 2>nul|findstr /i "Droid4X" 2>nul"') do (
	rem ��ȡD4·��
	set D4Path=%%~dpj
)
echo.
if not defined D4Path (
	echo. �޷��ҵ���ĺ�����ģ���������ڰ�װģ�������ٽ��в�����
	PAUSE
	exit
)

if not defined VboxPath (
	echo. �޷��ҵ�����ϵͳ�е� Oracle VM VirtualBox ��������������޸Ĺ��߽��޷���������
	echo.
	echo. ������Ϊ�����°�װ Oracle VM VirtualBox
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
rem ��ʼ������
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
	echo. Droid4X ģ�������������У��㲻�ܽ����κε��޸�, ֻ�ܲ鿴 Droid4X ��״̬��Ϣ
	echo.
	echo -----===== ��ѡ��Ҫִ�еĲ��� =====-----
	echo. 1. �޸� adb ���Ӷ˿�
	echo. 2. ���½���ģ���� ADB ����
	echo. 3. adb shell ����ģ����
	echo. 4. ���¹��� SD ��
	echo. 
	echo. 6. �鿴�����ӵİ�׿�豸��Ϣ
	echo. 7. ���� VirtualBox Ŀ¼
	echo. 8. ���� ������ģ���� Ŀ¼
	echo. 9. �������ݰ� ^& Hosts�ļ�
	echo. 10. �鿴 Droid4X ������Ϣ
	echo. 0. ������־ ^& ���µ�ַ
	echo.
	echo. ���Ҫ�޸�����, ��ر�ģ�����������б�����
echo -----==================================-----
set /p checks="��������ѡ��Ҫִ�еĲ�����"
	if /i "!checks!"=="0" goto :log
	if /i "!checks!"=="1" (
		echo ��ǰ�˿�Ϊ��127.0.0.1:%port%
		set /p port="�޸Ķ˿�Ϊ��������530xx��ͷ����53001��ʼ��������"
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
	echo ��� Oracle VirtualBox ·���ǣ�%VboxPath%
) else (
	echo �޷��ҵ�����ϵͳ�е� Oracle VirtualBox
)
echo.
echo -----===== ��ѡ��Ҫִ�еĲ��� =====-----
echo. 1. �޸ķֱ��ʼ�DPI
echo. 2. �޸ķֱ���
echo. 3. �޸�DPI
echo. 4. �޸������ڴ�
echo. 5. �޸�ģ����CPU��������
echo. --------------------------------------
echo. 6. ��ʾ\�ر��������
echo. 7. �� VirtualBox ������
echo. 9. �޸� Oracle VM VirtualBox
echo. 10. �鿴 Droid4X ������Ϣ
echo. 11. ���� VirtualBox Ŀ¼
echo. 12. ���� ������ģ���� Ŀ¼
echo. 0. ������־ ^& ���·���
echo. --------------------------------------
echo.
echo. ��������������й����г��������޷�ʵ���޸�Ч�������ԡ�����Ա������С�
echo. by. ��ƷСè��QQ��391616001��		Droid4X�ٷ�QȺ��369149979
echo -----==============================-----
set /p menu="��������ѡ��Ҫִ�еĲ�����"
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
echo ------------- �����ֱ���Ԥ�� -------------
echo.  1.  480 x  320 DPI 120 (����)
echo.  2.  640 x  360 DPI 160 (����)
echo.  3.  800 x  480 DPI 240 (����)
echo.  4.  960 x  540 DPI 280 (����)
echo.  5. 1280 x  720 DPI 320 (����)
echo.  6. 1440 x  900 DPI 320 (����)
echo.  7. 1920 x 1080 DPI 480 (����)
echo. 
echo ------------- �����ֱ���Ԥ�� -------------
echo.  8.  320 x  480 DPI 120 (����)
echo.  9.  360 x  640 DPI 160 (����)
echo. 10.  480 x  800 DPI 240 (����)
echo. 11.  540 x  960 DPI 280 (����)
echo. 12.  720 x 1280 DPI 320 (����)
echo. 13.  900 x 1440 DPI 320 (����)
echo. 14. 1080 x 1920 DPI 480 (����)
echo ------------------------------------------
echo.  0. ����� �Զ���ֱ��� �� DPI �����
echo.
echo. Q. �ص����ܲ˵�
echo.
echo. Droid4X ��ǰ��ʹ�õķֱ���Ϊ��%DefaultGraph%
echo.
set /p input="��������ѡ��Ҫִ�еĲ�����"
echo ----------------------------------------
if /i "%input%"=="0" (
	echo.
	echo.===== �Զ���ע������ =====
	echo. �Զ���ֱ���ǰ�����ȳ���˽�÷ֱ�������Ӧ��DPI����
	echo.
	echo. ģ�����ڲ��ķֱ��ʲ�����ʾ���ֱ���Լ������������밴����ʾ���ֱ�����ʾ��
	echo. ����Զ���ֱ��ʡ���ʾ���ֱ��ʣ����DPI����Ӧ�����������ʾ�쳣��
	echo.
	set /p width="�������ȣ�"
	set /p height="������߶ȣ�"
	goto :ReInput
)
rem ����
if /i "%input%"=="1" call :Set_Pixels_DPI 480x320 120
if /i "%input%"=="2" call :Set_Pixels_DPI 640x360 160
if /i "%input%"=="3" call :Set_Pixels_DPI 800x480 240
if /i "%input%"=="4" call :Set_Pixels_DPI 960x540 280
if /i "%input%"=="5" call :Set_Pixels_DPI 1280x720 320
if /i "%input%"=="6" call :Set_Pixels_DPI 1440x900 320
if /i "%input%"=="7" call :Set_Pixels_DPI 1920x1080 480
rem ����
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
				set /p height="�߶�ֻ���������֣����������룺"
				goto :ReInput
			)
		) else (
			if /i "%input%"=="0" (
				set /p height="��û������߶ȣ����������룺"
				goto :ReInput
			)
		)
	) || (
		set /p width="���ֻ���������֣����������룺"
		goto :ReInput
	)
) else (
	if /i "%input%"=="0" (
		set /p width="��û�������ȣ����������룺"
		goto :ReInput
	)
)
echo.
echo ���޸ĵķֱ���Ϊ��%pixels%
echo.
echo ----------------------------------------
echo.
echo. Droid4X ��ǰ��ʹ�õķֱ���Ϊ��%DefaultDPI%
echo.
:DPI
echo. Droid4X ��ǰ��ʹ�õ�DPIΪ��%DefaultDPI%
echo.
echo. DPIΪ��Ļ�����ܶȣ�����㲻��ʲô��DPI���벻Ҫ�޸Ĵ���ֵ��
echo.����ֱ���^��1024*768��������DPIΪ��240��
echo.
if not defined DPI set /p DPI="�������޸�DPI����ֵ��"
if DEFINED DPI (
	echo %DPI%|findstr "^[0-9]*$">nul || (
		set /p DPI="DPIֻ���������֣����������룺"
		goto :DPI
	)
) else (
	goto :DPI
)
echo ���޸ĵ�DPIΪ��%DPI%
echo.
echo ----------------------------------------
echo.
:ok
CLS
echo.
echo ----------------------------------------
echo Droid4X ��ǰ��ʹ�õķֱ���Ϊ��%DefaultGraph%
echo.
echo ���޸ĵķֱ���Ϊ��%pixels%
echo ----------------------------------------
echo.
echo ----------------------------------------
echo Droid4X ��ǰ��ʹ�õ�DPIΪ��%DefaultDPI%
echo.
echo ���޸ĵ�DPIΪ��%DPI%
echo ----------------------------------------
echo.
set /p ok="��ȷ��������Ϣ�Ƿ���ȷ������[Y-��ȷ/N-�ص��˵�]��"
if DEFINED ok (
	echo %ok%|findstr /i "Y" >nul 2>nul && (
		VBoxManage guestproperty set droid4x vbox_graph_mode %pixels%
		VBoxManage guestproperty set droid4x vbox_dpi %DPI%
		set DefaultDPI=%DPI%
		set DefaultGraph=%pixels%
		echo.
		echo �޸���ɣ����������ص��˵�
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
echo. ��ǰ Droid4X �������ڴ棺%Defaultmemory%
echo.
echo. �� 1G=1024M��4G�ڴ����ϵ��û��Ƽ���2048M��4G���½��飺640M��1024M��
echo. �� ����1G�ڴ棬�п��ܵ�����Ϸ�޷����У����߳��ָ������˻��߱�����
echo. �� �����ڴ���ֱ�Ӵ�ϵͳ�ڴ滮�ֵģ���ˡ������ڴ�^<ϵͳ�ڴ桿��
echo. �� Ĭ��1G�ڴ��������и��ִ���3D��Ϸ������������������䣬���3.5G��
echo.
echo. �� ���������Ӧ�����֣����Զ������Ӧ�������ڴ�
echo.
echo.   ��1��1024M��  ��2��2048M��  ��3��3072M��  ��3.75��3840M��
echo.
if not defined memory set /p memory="�����������ڴ���ֵ(���赥λ)��"
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
		set /p memory="��(�t_�s) ����ô����~~��Ҫ�����������ֻ��512M����������ɣ�"
		goto :Memory
	) else if %memory% GEQ 3000 (
		set /p memory="(�ѣ���) ����~����ô��Ҳ���˷ѵģ����� 1024 �� 2048 ���ˣ���������ɣ�"
		goto :Memory
	)
	echo %memory%|findstr "^[0-9]*$">nul && (
		VBoxManage modifyvm droid4x --memory %memory%
		echo.
		set Defaultmemory=%memory%
		echo. �޸���ɣ���ǰ�����ڴ�Ϊ��%memory%M�����������ص��˵�
		pause>nul
		cls
		goto :menu
	) || (
		set /p memory="�����ڴ���ֵֻ���������֣����������룺"
		goto :Memory
	)
)
cls
goto :Memory

:navBar
cls && set navBar=
echo.
echo. ----------========== ��������������� ==========----------
VBoxManage guestproperty get droid4x droid4x_force_navbar|findstr /i "No 0">nul &&(
echo �������״̬���ѹر� ) || (echo �������״̬���ѿ���)
echo.
echo. 0. �ر��������
echo. 1. �����������
echo. M. �ص����ܲ˵�
set /p navBar="��������ѡ��Ҫִ�еĲ�����"
if "%navBar%"=="0" VBoxManage guestproperty set droid4x droid4x_force_navbar 0
if "%navBar%"=="1" VBoxManage guestproperty set droid4x droid4x_force_navbar 1
if /i "%navBar%"=="m" goto :menu
cls
goto :navBar

:CPU
cls && set modCPU=
echo.
echo. ----------========== CPU������������ ==========----------
if not defined DefaulCPUCores CALL :info
echo.
if EXIST %windir%\SYSTEM32\wbem\wmic.exe (
	echo. CPU ʵ�ʺ����߳����Ϊ��%DefaulCPUCores%
	echo. ������ģ������������Ϊ��%DefaulLogical%
	echo.
) else (
	if EXIST "%~dp0wmic.exe" (
		call %~dp0wmic.exe exit
		goto :CPU
	) else (
		echo. ��ĵ�����ȱ�� WMIC ����������������Ҫ�ĳ���Ȼ���뱾�������ͬһĿ¼��
		echo.
		echo. ���ص�ַ��http://www.droid4x.cn/bbs/forum.php?mod=viewthread^&tid=13054
		echo.
		call :CopyUrl
		PAUSE && goto :menu
	)
)
echo. ����������ֱ��Ӱ��ģ�����������ٶȣ�������������������벻Ҫ���Ĵ�ֵ��

echo. �����������Ϊ2�����Ϊ CPU ʵ�ʺ�������
echo.
set /p "modCPU=������Ҫ�޸ĵĺ���������M-�ɷ������˵�����"
if /i "%modCPU%"=="m" goto :menu
if not defined modCPU goto :cpu
echo %modCPU%|findstr /i "[0-9]*">nul 2>nul&& (
	if %modCPU% LEQ 1 goto :cpu
	if %modCPU% GTR %DefaulCPUCores% goto :cpu
	"%VboxPath%VBoxManage" modifyvm droid4x --cpus %modCPU%
	for /f "tokens=3,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"CPUs"') do set DefaulLogical=%%j
	echo �޸ĳɹ�����ǰģ�����ĺ�������Ϊ��%DefaulCPUCores%
	pause
)
goto :CPU
pause

:Report
cls
call :Info
echo.
echo. ----------========== Droid4X ������Ϣ ==========----------
echo.
echo. �����ڴ棺%Defaultmemory%
echo. 
echo. �ֱ��ʣ�%DefaultGraph%
echo.
echo. DPI��%DefaultDPI%
echo.
echo. CPU ʵ�ʺ����߳����Ϊ��%DefaulCPUCores%
echo. ������ģ������������Ϊ��%DefaulLogical%
echo.
echo. Droid4X Ŀ¼��%D4Path%
echo.
echo. VirtualBox Ŀ¼��%VboxPath%
echo.
echo. �������״̬��%navBarStart%
echo.
echo. ----------======================================----------
echo.
echo. ���������ص��˵�
pause>nul
goto :menu

:Devices
cls
echo.
echo. ----------========== �����Ӱ�׿�豸��Ϣ ==========----------
echo.
"%D4Path%adb.exe" devices -l
echo. ----------========================================----------
goto :menuNocls

:InstallData
cls
echo. 
echo. ���� adb push �������ݰ���Ч�ʺ��ٶ����⣬�������Ƴ��ù��ܡ�
echo. �������Ҫ�������ݰ�������ο��ý̳̽��С�
echo.
echo. http://www.droid4x.cn/bbs/forum.php?mod=viewthread^&tid=5869
call :CopyUrl
echo. 
echo. ���������ص��˵�
pause>nul && goto :menu

:ReInstallVBOX
cls
echo.
echo. ----------========== �޸� VirtualBox ����==========----------
echo.
echo. �����������޸�ģ������װ�쳣���Լ�ʹ����������׿ģ���������޷�����������
echo.
echo. �޸� VirtualBox �������ġ�������ģ����������������κ�Ӱ�졣
echo.
echo. ----------=====================================----------
echo.
set InstallVBOX=
set deleteVBOX=
set /p InstallVBOX="Ҫ�޸� VirtualBox ������ [Y-ȷ��\����-����]��"
if /i "%InstallVBOX%"=="Y" (
		echo. ����ж�� VirtualBox
		msiexec /x "{D90E08B8-E7BB-4D29-8249-8670D4CC24BD}" /q
		msiexec /x "{B5121457-0126-4E62-BCBF-6DC7C73D9E4A}" /q
		reg delete "HKLM\SOFTWARE\Oracle\VirtualBox" /f
		echo.
		echo. �������ȳ����������ģ�����������Ȼ�޷��������볢��ɾ�� .VirtualBox �������ݡ�
		echo.
		echo. .VirtualBox �������ݰ�����ע�����������������������ĳ��û���ɾ������֮������������ע����ص��������
		echo.
		set /p deleteVBOX="�Ƿ�Ҫɾ�� .VirtualBox �������� [Y-ȷ��\����-����]��"
		echo. 
		if /i "!deleteVBOX!"=="Y" (
			rd /q /s "%USERPROFILE%\.VirtualBox"
			echo. ɾ����ϣ����ٴγ����������ģ������
			echo. �����Ȼ�޷������������°�װģ������
			if EXIST "%D4Path%VirtualBox VMs\droid4x\droid4x.vbox" (
				echo. ����Ϊ������ע�� Droid4X �����
				"%VboxPath%vboxmanage.exe" registervm "%D4Path%VirtualBox VMs\droid4x\droid4x.vbox"
			) else (
				echo �޷��ҵ���� Droid4X ����������������·�������ֶ�ע�� droid4x.vbox
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
echo -----===== Droid4X �����쳣��� =====-----
echo.
echo. ���¹���Ϊ�ͷ��ͼ�����Աʹ��, ��ͨ�û�����ָ����ʹ��.
echo.
echo. 1. �鿴 %port% �˿�
echo. 2. �鿴 127.0.0.1 ���ж˿�
echo. 3. logcat ��־��¼
echo. 4. �鿴 APK ����
echo. 0. �ص��˵�
echo.
echo. ----------=====================----------
echo.
set checkPort=
set /p checkPort="��������ѡ��Ҫִ�еĲ�����"
if /i "%checkPort%"=="1" netstat -ano|findstr %port% && pause>nul
if /i "%checkPort%"=="2" netstat -ano|findstr 127.0.0.1* && pause>nul
if /i "%checkPort%"=="3" (
		del /q /f %Desktop%\Droid4X_RunLog.log
		echo. ��������־��¼����
		echo. ��־��¼λ��������, �ļ���Ϊ: Droid4X_RunLog.log
		echo. �����Ҫ��ֹ��־��¼, �밴�� Ctrl + C , ���ڵ�������ʾ��ѡ�� N.
		%adb% shell logcat -s ActivityManager>>%Desktop%\Droid4X_RunLog.log
		goto :checkPort
)
if /i "%checkPort%"=="4" goto :package
if /i "%checkPort%"=="0" goto :menu
goto :checkPort

:package
set InstallData=
set /p InstallData="������APK�ļ����������У�"
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
echo. ----------========== ������־ ==========----------
echo. v1.0.0
echo. 1. [��һ��] �޸� �ֱ��� �� DPI ����
echo. 
echo. v1.0.1
echo. 1. [����] ���������ڴ��޸Ĺ���
echo.
echo. v1.0.2
echo. 1. [����] �������ù������ݳ�ʼ��
echo. 2. [����] ����������Ϣ��ʾ���ִ���
echo.
echo. v1.0.3 [2014-11-13]
echo. 1. [����] ������Ϣ�鿴
echo. 2. [����] ������־
echo. 3. [�޸�] ��������ڴ��޸���������� 512M ��Ϊ�˱�֤ Droid4X �����ȶ���
echo. 4. [�޸�] �޸Ĳ�����Ϣ��ʾ��ʽ
echo.
echo. v1.1.0 [2014-11-25]
echo. 1. [����] ���� Droid4X ʱ�ɲ鿴������Ϣ��������־�������ӵİ�װ�豸��Ϣ
echo. 2. [����] ���� Droid4X ʱ�ɰ�װ APK ���ܣ��ù��ܽ����ڿͷ�Э������ APK ��װ�쳣��ʧ��
echo. 3. [����] ��װ VirtualBOX ����
echo. 4. [�Ż�] �Ż���Ϣ����ģ�飬��������Ч��
echo. 5. [�Ż�] �Ż����ߴ��룬����BUG
echo.
echo. v1.1.1 [2014-11-26]
echo. 1. [����] �ṩ����Ԥ��ֱ���
echo. 2. [����] �޸����ú�鿴������Ϣ��ˢ������
echo. 3. [����] �޸��ڴ�ʱ��������˳�������
echo.
echo. v1.1.2 [2014-11-27]
echo. 1. [����] Droid4X �����쳣��� ����(����/�ͷ�ʹ��)
echo.
echo. v1.1.3 [2015-01-08]
echo. 1. �����ӡ�������ģ���� ADB �������ӹ��ܣ�����������������Ҳ���ģ�������⣩
echo. 2. �����ӡ�Ĭ�Ϸֱ��ʣ�360*640 DPI 160 ������
echo. 3. ��������Ĭ�Ϸֱ��ʣ�480*320 DPI 120 �� DPI ����
echo. 4. ���Ż���VirtualBox �޸�����
echo. 5. ���Ż���������ʾ�Լ���Ϣ����ģ��
echo.
echo. v1.1.4 [2015.01.12]
echo. 1. ��������ɾ�� .VirtualBox ��������֮��û������ע��Droid4X����������⡣
echo.
echo. v1.1.5 [2015.04.30]
echo. 1. �����ӡ����ݰ� �� Hosts ���빦�ܣ�ز�ۣ��뿴�̳̣�
echo. 2. �����ӡ��������Ŀ¼�������Ŀ¼������ adb ��ع���
echo. 3. �����ӡ���ʾ\�ر���������Ĺ���
echo. 4. ���Ż�������ģ�����
echo.
echo. v1.1.6 [2015.06.15]
echo. 1. ��������δ����ģ����ʱ���������Ŀ¼��ģ����Ŀ¼����������
echo. 2. �����ӡ�ģ�������������޸Ĺ��ܣ���֧�� 0.8.2 ���ϰ汾
echo. 3. �����������ֲ˵����
echo.
echo. v1.1.7 [2015.07.31]
echo. 1. ���������ڴ��޸ĵ�һЩ��ʾ�ͽ���
echo. 2. ��������CHCP ������ʾ���⣨��������Ϊϵͳ���⣬�˴���Ϊ��ʱ����ϵͳ�������⣩
echo. 
echo. v1.1.8 [2015.08.02]
echo. 1. ���������ڴ��޸���Ч��������һЩ����ѡ��ķ�ʽ
echo. 2. �����ӡ�UAC��ȨVBS�ű��������Ȩ���޷�ִ���޸�����
echo.
echo. ���µ�ַ��http://www.colafile.com/u/%%E6%%9E%%81%%E5%%93%%81%%E5%%B0%%8F%%E7%%8C%%AB
set /p updata="�����Ҫ���£�������[Y-�򿪸���ҳ��/���ջص����˵�]��"
if /i "%updata%"=="Y" (start "" "http://www.colafile.com/u/��ƷСè")
cls
goto :menu


Rem ========== Callback ==========

:info
echo ���ڻ�ȡ Droid4X �Ļ�����Ϣ...
for /f "tokens=1,*" %%i in ('VBoxManage.exe guestproperty get droid4x vbox_graph_mode') do (
	rem ��ȡ�ֱ���
	set DefaultGraph=%%j
) & for /f "tokens=1,*" %%i in ('VBoxManage.exe guestproperty get droid4x vbox_dpi') do (
	rem ��ȡDPI
	set DefaultDPI=%%j
) & for /f "tokens=2,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"memory size:"') do (
	rem ��ȡ�����ڴ�
	set Defaultmemory=%%j
) & for /f "tokens=3,*" %%i in ('VBoxManage.exe showvminfo droid4x^|findstr /ic:"CPUs"') do (
	rem ��ȡ�����߳�����
	set DefaulLogical=%%j
) & if EXIST %windir%\SYSTEM32\wbem\wmic.exe (
	for /f "skip=1" %%i in ('wmic cpu get NumberOfLogicalProcessors^|findstr /r .') do (
		rem Ӳ������������
		set DefaulCPUCores=%%i
	)
) & VBoxManage guestproperty get droid4x droid4x_force_navbar|findstr /i "No 0">nul &&(
	set navBarStart=�ѹر�) || (set navBarStart=�ѿ���)
exit /b

:Set_Pixels_DPI
echo Y
set pixels=%1-16
set DPI=%2
goto :ok

:Format
for /f "tokens=2,*" %%i in ('"reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v Desktop 2>nul"') do (
	rem ��ȡ����·��
	set Desktop="%%j"
)
exit /b

:ConnectDroid4x
echo �������� ������ģ����
"%D4Path%\adb.exe" connect 127.0.0.1:%port%
"%D4Path%\adb.exe" -s 127.0.0.1:%port% shell mount -o,remount rw /system
echo.
exit/b

:RemountSD
call :ConnectDroid4x
"%D4Path%\adb.exe" -s 127.0.0.1:%port%  remount
echo �Ѿ����¹���SD��������ģ�������ٴγ��Դ�SD��Ŀ¼
pause
exit/b

:CopyUrl
echo. (������ַ�취���Ҽ�����ǡ�ѡ����ַ���Ҽ������Ƴɹ�)
exit/b