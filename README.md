## VTK-Volume-Visualisation-and-Flow-Visualisation

# To run the code:

Step1 : Create a folder named flowVisSample

Step2 : Create a build folder inside flowVisSample folder.

Step3 : Copy CMakeLists.txt and flowVisSample.cpp into flowVisSample folder.

Step4 : Configure and generate using CMake by choosing folder destination as flowVisSample and build folder.

Step5 : Open flowVisSample.sln inside build folder using Visual Studio.

Step5 : Copy the Solution 1 to Solution 4 into the project folder.

Step6 : Exclude the code file that doesn't use from the project.

Step6 : Change the data destination to render the data you want, eg: "../data/carotid.vtk".

Step7 : To run the code, change to Release mode and Build the solution.


** If you dont want to use the flowVisSample.cpp to create the project, you can edit the CMakeLists.txt to change it.
** Change/Delete the First line of the code in the CMakeLists.txt if your VTK are install in other location / version is different.

# Example Application

You also can directly check the result of the code through the application build in the each of the solution folder. (Ex: Solution1, Solution2...)
Make sure the data folder are outside of the application.exe, then double-click to execute the application.
