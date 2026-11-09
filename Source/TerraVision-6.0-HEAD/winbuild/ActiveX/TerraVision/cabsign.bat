@ECHO OFF
REM This batch file signs a .ocx and creates and signs a .cab file.

ECHO *********** About to sign TerraVision.ocx ***********
signcode.exe -spc tvactivex.spc -v tvactivex.pvk -n TerraVision -i http://www.tvgeo.com -t http://timestamp.verisign.com/scripts/timstamp.dll release/TerraVision.ocx

ECHO *********** About to create .cab file ***********
makecab /V3 /F TerraVision.ddf

ECHO *********** About to sign archive using signcode ***********
signcode.exe -spc tvactivex.spc -v tvactivex.pvk -n TerraVision -i http://www.tvgeo.com -t http://timestamp.verisign.com/scripts/timstamp.dll TerraVision.cab

ECHO *********** Test the signed .cab file ***********
chktrust TerraVision.cab

