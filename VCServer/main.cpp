#include "Defines.h"
#include "BaseTypes.h"

#include "VCSystem.h"

#include <windows.h>

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
int main(int argc, char*argv[])
{
	new TRC::VCS::CVCSystem;

	TRC::VCS::CVCSystem::GetSingleton().Init();

	TRC::VCS::CVCSystem::GetSingleton().Run(argc, argv);

	TRC::VCS::CVCSystem::GetSingleton().DeInit();

	delete TRC::VCS::CVCSystem::GetSingletonPtr();

	return 0;
}