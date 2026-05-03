#!/usr/bin/env bash

if [-f "main2d.project"] then
    rm -rf main2d.project
fi

zip -r main2d.zip main2d
mv main2d.zip main2d.project