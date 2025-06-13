#!/usr/bin/env nu

let cc = "clang"
let flags = [-Wall -Wextra -g -O0 -Wno-missing-field-initializers]

def main [expr --debug --assemble] {
    clang ...$flags -o rekt ./rekt.c

    if $debug {
        echo $expr
        | ./rekt 
        | bat --language=asm
    }

    if $assemble {
        mkdir /tmp/rekt ./build
        let tmp_path = $"/tmp/rekt/(random chars --length 4).out" 
        echo $expr
        | ./rekt
        | as -o $tmp_path
        ld $tmp_path -o ./build/a.out
    }
}
