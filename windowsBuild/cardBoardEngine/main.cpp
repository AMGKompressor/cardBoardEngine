// cardBoard engine demo entry point

#include "CardBoardApp.h"
#include "Game.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{

	Game& gameInstance = Game::GetInstance();
	if (!gameInstance.initialise())
	{
		LogManager::getInstance().log("cardBoard initialize failed.");
		return 1;
	}

	while (gameInstance.DoGameLoop())
	{
	}

	Game::DestroyInstance();
	LogManager::destroyInstance();
	return 0;
}
