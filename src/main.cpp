#include "Game.h"
#include <iostream>

int main()
{
	Game game = Game::getInstance();
	try
	{
		game.Run();
	}
	catch (std::runtime_error e)
	{
		std::cerr << e.what();
	}
}