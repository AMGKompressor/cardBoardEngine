#pragma once

#include "scene.h"
#include "Player_1.h"
#include "Map_1.h"
#include <cstdint>

class Renderer;
class Sprite;
class Player;
class Map;
class InputSystem;
class Item;

class SceneCardBoard : public Scene
{
public:
    SceneCardBoard();
    virtual ~SceneCardBoard();

    virtual bool Initialise(Renderer& renderer);
    virtual void Process(float deltaTime, InputSystem& inputSystem);
    virtual void Draw(Renderer& renderer);
    virtual void DebugDraw();

protected:

private:
    SceneCardBoard(const SceneCardBoard& sceneSplashAut);
    SceneCardBoard& operator=(const SceneCardBoard& sceneSplashAut);

    void updateCamera();

public:

protected:
    Sprite* m_pSceneCardBoard;
    Renderer* m_pRenderer;
    Player* m_pPlayer;
    Map* m_pMap;
    InputSystem* m_pInputSystem;
    Item* m_pItem;

    float mCameraX;
    float mCameraY;
    bool mLooping;
    std::int64_t mLastTime;

private:

};