@echo off

mkdir q:\treetop\build
pushd q:\treetop\build
cl -Zi q:\treetop\code\treetop.cpp user32.lib gdi32.lib
popd
