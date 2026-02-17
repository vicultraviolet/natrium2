#include "Pch.hpp"
#include "Natrium2/Main.hpp"
#undef main

#include "Natrium2/Core/Logger.hpp"

#if defined(NA2_WINDOWED_APP)
#include <windows.h>
#endif // NA2_WINDOWED_APP

extern int Na2::Main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	try
	{
		return Na2::Main(argc, argv);
	} catch (const std::exception& e)
	{
		Na2::g_Logger.printf(Na2::Fatal, "Unhandled exception reached main! {}", e.what());
		return -1;
	}
	return 0;
}

#if defined(NA2_WINDOWED_APP)
int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE HPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	return main(__argc, __argv);
}
#endif // NA2_WINDOWED_APP
