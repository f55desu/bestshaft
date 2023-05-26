@echo off
cd %~1
set path=%~2;path

echo ...Start tetgen 1>&2 
tetgen.exe -zpqmVCCkO9/7a%~3 model.tri.mesh.smesh >tetgen.log
if %errorlevel% neq 0 (echo Tetgen Exit Code is %errorlevel% & type tetgen.log & exit) 1>&2

echo ...Start tet2inp 1>&2
tet2inp.exe -n "%~4" -f "%~5" -e "%~6" -c "%~7" -v "%~8" >tet2inp.log
if %errorlevel% neq 0 (echo Tet2inp Exit Code is %errorlevel% & type tet2inp.log & exit) 1>&2

echo ...Start CalculiX 1>&2 
ccx.exe -i "%~9" >calculix.log
if %errorlevel% neq 0 (echo CalculiX Exit Code is %errorlevel% & type calculix.log & exit) 1>&2

echo ...Start vtk generator 1>&2 
%~2\cpython-3.10\PCbuild\amd64\python.exe %~2\ccx2paraview.py %~9.frd vtk 1>&2 2>frd2vtk.log
if %errorlevel% neq 0 (echo ccx2paraview.py Exit Code is %errorlevel% & type frd2vtk.log & exit) 1>&2

shift
shift
echo ...Start StressCalculator 1>&2
StressCalculator.exe -d "%~8" > "%~9"
if %errorlevel% neq 0 (echo StressCalculator Exit Code is %errorlevel% & type vonmises_result.log & exit) 1>&2

echo Done. 1>&2