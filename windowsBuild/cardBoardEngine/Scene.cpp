#include "Scene.h"

#include "renderer.h"
#include "Sprite.h"
#include "Game.h"

#include <string>

Scene::Scene() : currentSelection(nullptr), head(nullptr), selectArrow(nullptr)
{
    CENTER_X = Game::GetInstance().m_pRenderer->getWidth() / 2;
}

Scene::~Scene()
{

}

Node* Scene::insertAtEnd(Node* newNode)
{
    newNode->selected = false;
    newNode->next = nullptr;
    newNode->prev = nullptr;

    if (head == nullptr)
    {
        newNode->next = newNode;
        newNode->prev = newNode;
        head = newNode;
    }
    else
    {
        Node* last = head->prev;
        newNode->next = head;
        newNode->prev = last;
        last->next = newNode;
        head->prev = newNode;
    }
    return head;
}

bool Scene::searchList(Node* start, bool search)
{
    Node* temp = start;

    int counter = 0;
    bool found = false;

    if (temp == NULL)
    {
        return false;
    }
    else
    {
        while (temp->next != start && counter <= 99)
        {
            counter++;
            if (temp->selected == search)
            {
                counter--;
                found = true;
                break;
            }
            temp = temp->next;
            found = false;
        }
    }
    return found;
}

Node* Scene::moveDown(Node* current)
{
    current->selected = false;
    current = current->next;
    current->selected = true;

    currentSelection = current;

    return current;
}

Node* Scene::moveUp(Node* current)
{
    current->selected = false;
    current = current->prev;
    current->selected = true;

    currentSelection = current;

    return current;
}