
#include "SceneMainMenu.h"

#include "Renderer.h"
#include "Sprite.h"
#include "InputSystem.h"
#include "Game.h"

#include "SoundSystem.h"

SceneMainMenu::SceneMainMenu() :    menuTitle(nullptr), menuStartPrompt(nullptr), 
                                    menuQuitPrompt(nullptr), menuOptionPrompt(nullptr), menuInstructionPrompt(nullptr),
                                    menuflashTimer(0.0f),  menuShowPrompt(true),
                                    startNode(nullptr), quitNode(nullptr), optionsNode(nullptr),
                                    instructionNode(nullptr), m_pRenderer(nullptr)
{

}

SceneMainMenu::~SceneMainMenu()
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

    menuTitle = nullptr;
    menuStartPrompt = nullptr;
    menuOptionPrompt = nullptr;
    menuInstructionPrompt = nullptr;
    menuQuitPrompt = nullptr;
}

bool SceneMainMenu::Initialise(Renderer& renderer)
{
    // store renderer
    m_pRenderer = &renderer;
    

    // create a linked list for menu navigation
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
        startNode = nullptr;
        quitNode = nullptr;
        currentSelection = nullptr;
    }

    // generate sprites
    menuTitle = renderer.createSprite("../assets/phantom_raiders_logo.png");
    menuTitle->setScale(0.7f);

    selectArrow = renderer.createSprite("../assets/arrow.png");
    selectArrow->setScale(0.5f);

    menuStartPrompt = renderer.createSprite("../assets/start.png");
    menuStartPrompt->setScale(0.5f);
    startNode = new Node;
    startNode->sprite = menuStartPrompt;
    startNode->selected = true;
    insertAtEnd(startNode);

    menuQuitPrompt = renderer.createSprite("../assets/quit.png");
    menuQuitPrompt->setScale(0.5f);
    quitNode = new Node;
    quitNode->sprite = menuQuitPrompt;
    quitNode->selected = false;
    insertAtEnd(quitNode);

    currentSelection = startNode;

    return true;
}

void SceneMainMenu::Process(float deltaTime, InputSystem& inputSystem)
{
    // flash the arrow cursor
    menuflashTimer += deltaTime;
    if (menuflashTimer >= 0.3f)
    {
        menuflashTimer = 0.0f;
        menuShowPrompt = !menuShowPrompt;
    }

    // navigate linked list nodes
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

    // player confirmed selection
    if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED)
    {
        // check what node the player currently selected
        if (currentSelection == startNode)
        {

            SoundSystem::GetInstance().PlaySound("menu_confirm");
            SoundSystem::GetInstance().ResumeMusic();
            Game::GetInstance().InitialiseScene(3, *m_pRenderer);
            Game::GetInstance().setScene(3);  // switch to SceneInGame
        }
        else if (currentSelection == quitNode)
        {
            SoundSystem::GetInstance().PlaySound("menu_confirm");
            Game::GetInstance().Quit();
        }
    }


   
}

void SceneMainMenu::Draw(Renderer& renderer)
{
    renderer.setCamera(0, 0);
    menuTitle->setX(CENTER_X);
    menuTitle->setY(200);
    menuTitle->draw(renderer);

    menuStartPrompt->setX(CENTER_X);
    menuStartPrompt->setY(400);
    menuStartPrompt->draw(renderer);

    menuQuitPrompt->setX(CENTER_X);
    menuQuitPrompt->setY(500);
    menuQuitPrompt->draw(renderer);

    if (menuShowPrompt)
    {
        if (currentSelection == startNode)
        {
            selectArrow->setX(CENTER_X - 200);
            selectArrow->setY(400);
        }
        else if (currentSelection == quitNode)
        {
            selectArrow->setX(CENTER_X - 100);
            selectArrow->setY(500);
        }
        selectArrow->draw(renderer);
    }

    
}

void SceneMainMenu::DebugDraw() {}