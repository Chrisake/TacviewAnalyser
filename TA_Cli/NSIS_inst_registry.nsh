WriteRegStr HKCR ".acmi\OpenWithProgids" "TacviewAnalyser.acmi" ""
WriteRegStr HKCR "Applications\TacViewAnalyser.exe\shell\open\command" "" '"$INSTDIR\bin\TacViewAnalyser.exe" "%1"'
WriteRegStr HKCR "Applications\TacViewAnalyser.exe" "FriendlyAppName" "Tacview Analyser"