# MadFire_Source

## Command to generate project files
```
%ue4%/GenerateProjectFiles.sh -cmakefile-game -project "/path/to/Game.uproject"
```
add -vscode  for vscode (duh)

## Use the following command in bash to format all source files:
```
find . -regex '.*\.\(cpp\|hpp\|cc\|h\|cxx\)' -exec clang-format-%version% -style=file -i {} \;
```
## Script to convert source files to Unix way
```
convertFilesToUnix.sh "Source_path"
```
## MadFire docs repo url  
https://bitbucket.org/helther/madfire_docs.git

## Content shared cloud folder link  
https://drive.google.com/drive/folders/1Zw-ae0WFjMi5ui1thgywPe13kvgAhotJ?usp=sharing
