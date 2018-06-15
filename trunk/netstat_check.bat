@echo off
netstat -an|findstr 5556
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "facedt connect fail"
pause
:success
echo "facedt connect success"

netstat -an | findstr 5558
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "facetell connect fail"
pause
:success
echo "facetell connect success"

netstat -an | findstr 5507
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "java connect fail"
pause
:success
echo "java connect success"

netstat -an | findstr 6000
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "rtsp connect fail"
pause
:success
echo "rtsp connect success"

netstat -an | findstr 6013
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "search connect fail"
pause
:success
echo "search connect success"

netstat -an | findstr 6015
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "data update connect fail"
pause
:success
echo "data update connect success"

netstat -an | findstr 6004
set res=%errorlevel%
if %res%==0 goto success else goto fail
:fail
echo "shape connect fail"
pause
:success
echo "shape connect success"





