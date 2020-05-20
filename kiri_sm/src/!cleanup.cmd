@echo off
rem ------------------------------------------------------------------------
rem
rem Simple script for deleting unnecessary Delphi 7 compiled data. :)
rem
rem Author: Dmitri Poguliayev. For Delphi projects. Use at your own risk. ;)
rem
rem ------------------------------------------------------------------------
title Deleting compilation garbage...
del *.dcu
del *.ddp
del *.~*
title Deleting compilation garbare... - DONE