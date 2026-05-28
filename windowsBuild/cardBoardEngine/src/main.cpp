// cardBoard engine demo entry point

#include "CardBoardApp.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	CardBoard::CardBoardApp& app = CardBoard::CardBoardApp::getInstance();
	if (!app.initialize())
	{
		LogManager::getInstance().log("cardBoard initialize failed.");
		return 1;
	}

	while (app.doGameLoop())
	{
	}

	CardBoard::CardBoardApp::destroyInstance();
	LogManager::destroyInstance();
	return 0;
}
