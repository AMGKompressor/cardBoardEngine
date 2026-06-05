#pragma once

#include "UI.h"
#include "scene.h"
#include "Player_1.h"
#include "Map_1.h"
#include "Inventory.h"
#include "LootConfig.h"
#include <cstdint>
#include <vector>

class Renderer;
class Sprite;
class Player;
class Map;
class InputSystem;
class Item;
class UI;
class PlayerConfig;
class EnemyManager;
class Minimap;

class SceneCardBoard : public Scene
{
public:
    SceneCardBoard();
    virtual ~SceneCardBoard();

    virtual bool Initialise(Renderer& renderer);
    virtual void Process(float deltaTime, InputSystem& inputSystem);
    virtual void Draw(Renderer& renderer);
    virtual void DrawHudOverlay() override;
    virtual void DebugDraw();

protected:

private:
    SceneCardBoard(const SceneCardBoard& sceneSplashAut);
    SceneCardBoard& operator=(const SceneCardBoard& sceneSplashAut);

    void updateCamera();
    void updateLowFlashlightSound(float deltaTime);
    bool spawnBatteries(Renderer& renderer);
    void clearBatteries();
    bool spawnWorldLoot(Renderer& renderer);
    void clearWorldLoot();
    void buildWorldItemList(std::vector<Item*>& outItems) const;
    bool tryPickupNearbyLoot();
    bool tryPickupNearbyBattery();
    bool tryDropSelectedLoot();
    bool tryExtract();
    void drawExtractionZone() const;
    LootTier lootTierForSpawnIndex(int index) const;

public:

protected:
    Sprite* m_pSceneCardBoard;
    Renderer* m_pRenderer;
    Player* m_pPlayer;
    Map* m_pMap;
    InputSystem* m_pInputSystem;
    std::vector<Item*> m_batteries;
    std::vector<Item*> m_worldLoot;
    Inventory m_inventory;
    int m_moneyCollected = 0;
    int m_extractionGoal = kExtractionGoalDollars;
    EnemyManager* m_pEnemies;
    UI* m_pUI;
    Minimap* m_pMinimap;
    int mCollectedItems;
    PlayerConfig* m_pPlayerConfig;

    float mCameraX;
    float mCameraY;
    bool mLooping;
    std::int64_t mLastTime;
    float mLowFlashlightSoundTimer = 0.0f;
    bool mLowFlashlightSoundLoaded = false;

private:

};
