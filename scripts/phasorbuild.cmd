@ECHO OFF
IF NOT EXIST src\modules\phasorbuild\phasorbuild.phsb phasor src\modules\phasorbuild\main.phs src\modules\phasorbuild build
phasor src\modules\phasorbuild\phasorbuild.phsb -- %*