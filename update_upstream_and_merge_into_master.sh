#!/usr/bin/env bash

git checkout upstream
git pull https://github.com/pixlra/calyp.git master
git push origin upstream

git checkout master
git pull origin master
git merge upstream
git push origin master