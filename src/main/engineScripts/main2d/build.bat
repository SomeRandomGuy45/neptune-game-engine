IF EXIST "main2d.project" del main2d.project
powershell Compress-Archive -Path main2d -DestinationPath main2d.zip
powershell Rename-Item main2d.zip main2d.project