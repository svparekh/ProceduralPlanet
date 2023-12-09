FreeGLUT template code for CS334


NOTE: This template relies on (somewhat) recent additions to
the C++ standard, specifically the std::filesystem module that
was introduced in C++17. Thus, you will need a modern compiler
that implements this standard (VS2019 or later, or GCC 8.0 or
later). There are workarounds if you use an older compiler (ask
on Piazza if this affects you).



LINUX =========================

1. Make sure you have all dependencies installed.

	Debian-based systems (e.g. Ubuntu):
	$ sudo apt install build-essential libglm-dev freeglut3-dev

	Arch-based systems (e.g. Manjaro):
	$ sudo pacman -Sy base-devel glm freeglut

2. Compile
	$ make

3. Run
	$ ./base_freeglut




WINDOWS =======================

Dependencies are included with the project.

1. Open base_freeglut.sln in Visual Studio
2. Build & run

