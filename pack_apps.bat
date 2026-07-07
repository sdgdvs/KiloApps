@echo off
echo Packaging all KApps into KApps.zip...
powershell -Command "Compress-Archive -Path 'KiloOS\public\exe\*.exe' -DestinationPath 'KiloOS\public\exe\KApps.zip' -Force"
echo Done!
