#pragma once
#include "GraphicsEngine.h"

class Game
{
public:
	static Game& getInstance()
	{
		static Game instance;
		return instance;
	}
	void operator=(Game&) = delete;

	void Run();
private:
	Game() = default; 
	
private:
	GraphicsEngine& mGraphicsEngine = GraphicsEngine::getInstance();
};
