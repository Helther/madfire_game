# MadFire_Source

## Command to generate project files
```
%ue4%/GenerateProjectFiles.sh -cmakefile-game -project "/path/to/Game.uproject"
```
add -vscode for vscode (duh)

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
## Madfire content repo url
http://helther.asuscomm.com:8000/git/Content.git

## Cloning the project
Because MadFire tracks it's content as submodule, there are additional commands needed to run, when working with repository.

### Cloning
```
$ git clone $URL
$ git submodule update --init 

OR

$ git clone --recurse-submodules $URL
```

### Pulling
```
$ git pull
$ git submodule update --init
```

### Pushing
We need to push main repo as well, as submodule
```
$ git push --recurse-submodules=on-demand
```
