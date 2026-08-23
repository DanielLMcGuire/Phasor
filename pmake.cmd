@ECHO OFF
IF NOT EXIST src\modules\pmake\pmake.phsb scripts\phasorbuild src\modules\pmake build
phasor src\modules\pmake\pmake.phsb -- %*