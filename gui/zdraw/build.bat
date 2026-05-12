@echo off
setlocal

:: ─── Buscar MSBuild ───────────────────────────────────────────────────────────
set MSBUILD=

:: VS 2022
for %%P in (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist %%P set MSBUILD=%%P
)

:: VS 2019 fallback
if "%MSBUILD%"=="" (
    for %%P in (
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    ) do (
        if exist %%P set MSBUILD=%%P
    )
)

if "%MSBUILD%"=="" (
    echo [ERROR] No se encontro MSBuild. Instala Visual Studio 2019 o 2022 con el componente "C++ build tools".
    exit /b 1
)

echo [OK] Usando MSBuild: %MSBUILD%

:: ─── Compilar ─────────────────────────────────────────────────────────────────
set PROJECT=%~dp0zdraw\zdraw.vcxproj
set CONFIG=Release
set PLATFORM=x64

echo [..] Compilando %PROJECT%
echo      Configuracion: %CONFIG% ^| %PLATFORM%
echo.

%MSBUILD% "%PROJECT%" ^
    /p:Configuration=%CONFIG% ^
    /p:Platform=%PLATFORM% ^
    /m ^
    /nologo ^
    /v:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Compilacion fallida. Revisa los errores arriba.
    exit /b %ERRORLEVEL%
)

echo.
echo [OK] Compilacion exitosa!
echo      Output: %~dp0zdraw\bin\

endlocal