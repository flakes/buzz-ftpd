@echo off

REM Requires Visual Studio 2010
REM Boost must have been extracted to deps\boost, then built using 'bootstrap' and '.\bjam'
REM OpenSSL must have been built with --prefix=blah\buzz-ftpd\deps\openssl

if %cd%\==%~dp0 GOTO BADCWD

call "%VS100COMNTOOLS%\vsvars32.bat"

set BOOST_ROOT=%~dp0\deps\boost
set BOOST_LIBRARYDIR=%BOOST_ROOT%\stage\lib

set OPENSSL_ROOT_DIR=%~dp0\deps\openssl

cmake %~dp0

goto DONE

:BADCWD

echo Do an out-of-source build please!

:DONE
