@ECHO OFF
if "%~1"=="list" (
    scripts\phasorbuild src\modules\makemsi list
) else (
    scripts\phasorbuild src\modules\makemsi task %1
)