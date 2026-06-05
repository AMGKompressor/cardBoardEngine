#include "ScenePause.h"

#include "Renderer.h"
#include "Sprite.h"
#include "InputSystem.h"
#include "Game.h"

#include "SoundSystem.h"

#include <iostream>

ScenePause::ScenePause() : mPauseText(nullptr), mPauseResumePrompt(nullptr), mPauseQuitPrompt(nullptr)
{

}

ScenePause::~ScenePause() 
{
    if (head != nullptr)
    {
        Node* current = head;
        Node* next = nullptr;
        do {
            next = current->next;
            delete current;
            current = next;
        } while (current != head);
        head = nullptr;
    }
}

bool ScenePause::Initialise(Renderer& renderer)
{

    // create linked list for navigation
    if (head != nullptr)
    {
        Node* current = head;
        Node* next = nullptr;
        do {
            next = current->next;
            delete current;
            current = next;
        } while (current != head);

        head = nullptr;
        resumeNode = nullptr;
        quitNode = nullptr;
        currentSelection = nullptr;
    }

    // generate text sprites
    /*renderer.CreateStaticText("PAUSED", 72);
    mPauseText = renderer.createSprite("PAUSED");*/

    selectArrow = renderer.createSprite("../assets/arrow.png");
    selectArrow->setScale(0.5f);
    
    mPauseResumePrompt = renderer.createSprite("../assets/start.png");
    mPauseResumePrompt->setScale(0.5f);
    resumeNode = new Node;
    resumeNode->sprite = mPauseResumePrompt;
    resumeNode->selected = true;
    insertAtEnd(resumeNode);

    mPauseQuitPrompt = renderer.createSprite("../assets/quit.png");
    mPauseQuitPrompt->setScale(0.5f);
    quitNode = new Node;
    quitNode->sprite = mPauseQuitPrompt;
    quitNode->selected = false;
    insertAtEnd(quitNode);

    currentSelection = resumeNode;

    return true;
}

void ScenePause::Process(float deltaTime, InputSystem& inputSystem)
{
    // flash arrow cursor
    menuFlashTimer += deltaTime;
    if (menuFlashTimer >= 0.3f)
    {
        menuFlashTimer = 0.0f;
        menuShowPrompt = !menuShowPrompt;
    }

    // navigate linked list
    if (inputSystem.GetKeyState(SDL_SCANCODE_DOWN) == BS_PRESSED)
    {
        currentSelection = moveDown(currentSelection);
        SoundSystem::GetInstance().PlaySound("menu_select");

    }
    else if (inputSystem.GetKeyState(SDL_SCANCODE_UP) == BS_PRESSED)
    {
        currentSelection = moveUp(currentSelection);
        SoundSystem::GetInstance().PlaySound("menu_select");
    }

    // player conirms selection
    if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED)
    {
        // checks current selection
        if (currentSelection == resumeNode)
        {
            SoundSystem::GetInstance().PlaySound("menu_confirm");
            SoundSystem::GetInstance().ResumeMusic();
            Game::GetInstance().setScene(3);
        }
        else if (currentSelection == quitNode)
        {
            SoundSystem::GetInstance().PlaySound("menu_confirm");
            Game::GetInstance().setScene(1);
        }
    }
}

void ScenePause::Draw(Renderer& renderer)
{
    renderer.setCamera(0, 0);
    // flash arrow cursor
    if (menuShowPrompt)
    {
        if (currentSelection == resumeNode)
        {
            selectArrow->setX(CENTER_X - 200);
            selectArrow->setY(renderer.getHeight() / 2);
        }
        else if (currentSelection == quitNode)
        {
            selectArrow->setX(CENTER_X - 200);
            selectArrow->setY(renderer.getHeight() / 2 + 100);
        }
        selectArrow->draw(renderer);
    }
    
    //mPauseText->setX(CENTER_X);
    //mPauseText->setY(350);
    //mPauseText->draw(renderer);

    mPauseResumePrompt->setX(CENTER_X);
    mPauseResumePrompt->setY(renderer.getHeight() / 2);
    mPauseResumePrompt->draw(renderer);
   
    

    mPauseQuitPrompt->setX(CENTER_X);
    mPauseQuitPrompt->setY(renderer.getHeight() / 2 + 100);
    mPauseQuitPrompt->draw(renderer);
}

void ScenePause::DebugDraw() {}