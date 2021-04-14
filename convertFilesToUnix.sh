#!/bin/bash
# find source files that are non utf encoded or have CRLF le 
# inside a given directory and sub directories, and fix them

rootPath="$1"

processFile()
{
    result="notFound";
    for file in "$@";
    do
        encoding=$( file -b --mime-encoding "$file" );
        if [[ "$encoding" != "utf-8" ]]; then
            echo -ne '\xEF\xBB\xBF' > $file.utf8; # insert BOM chars first
            iconv -f "$encoding" -t utf-8 $file >> $file.utf8;
            mv "$file.utf8" $file;
            result="Replaced encoding.";
        fi
        crlfStr="CRLF";
        leStr=$( file "$file" );
        if echo "$leStr" | grep -q "$crlfStr"; then
            vim "$file" -c "set ff=unix" -c ":wq";
            if [[ $result != "Replaced encoding." ]]; then
                result="";
            fi
            result="$result Replaced line endings.";
        fi
        if [[ "$result" != "notFound" && "$result" != "found" ]]; then
            printf "File: $file\n\tHas been successfully processed! $result \n";
            result="found";
        fi
    done;
    if [[ "$result" == "notFound" ]]; then
        printf "No files were processed";
    fi
}

export -f processFile
find "$rootPath" -regex '.*\.\(cpp\|hpp\|cc\|h\|cxx\)' -exec bash -c 'processFile "$@"' bash {} +;
