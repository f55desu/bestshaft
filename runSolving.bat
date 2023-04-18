@echo off
cd %~1
set path=%~2;path
echo ...Start tetgen 1>&2 
tetgen.exe -a%~3 model.tri.mesh.poly >tetgen.log
if %errorlevel% neq 0 (echo Tetgen Exit Code is %errorlevel% & type tetgen.log & exit) 1>&2
echo ...Start CalculiX 1>&2 
ccx.exe -i ccx.input >calculix.log
if %errorlevel% neq 0 (echo CalculiX Exit Code is %errorlevel% & type calculix.log & exit) 1>&2
echo ...Start vtk generator 1>&2 
python.exe %~2\ccx2paraview.py ccx.input.frd vtk 1>&2 2>frd2vtk.log
if %errorlevel% neq 0 (echo ccx2paraview.py Exit Code is %errorlevel% & type frd2vtk.log & exit) 1>&2
echo Done. 1>&2 
