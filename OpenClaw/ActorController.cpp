#include "ActorController.h"
#include "Engine/Scene/SceneNodes.h"
#include "Engine/Events/EventMgr.h"
#include "Engine/Events/Events.h"
#include "ClawEvents.h"
#include "Engine/GameApp/BaseGameApp.h"

ActorController::ActorController(shared_ptr<SceneNode> controlledObject, float speed)
{
    m_pControlledObject = controlledObject;
    m_Speed = speed;
    m_pKeyStates = NULL;
    m_MouseLeftButtonDown = m_MouseRightButtonDown = false;
}

void ActorController::HandleAction(ActionType actionType)
{
    switch(actionType)
    {
        case ActionType_Fire:
        {
            shared_ptr<EventData_Actor_Fire> pClimbEvent(new EventData_Actor_Fire(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pClimbEvent);
            break;
        }
        case ActionType_Attack:
        {
            shared_ptr<EventData_Actor_Attack> pClimbEvent(new EventData_Actor_Attack(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pClimbEvent);
            break;
        }
        case ActionType_Change_Ammo_Type:
        {
            shared_ptr<EventData_Request_Change_Ammo_Type> pEvent(new EventData_Request_Change_Ammo_Type(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pEvent);
            break;
        }
    }
}

void ActorController::OnUpdate(uint32 msDiff)
{
    int count;
    SDL_PumpEvents();
    m_pKeyStates = SDL_GetKeyboardState(&count);

    float moveX = 0.0f;
    float moveY = 0.0f;

    float climbY = 0.0f;

    // We need two conditions because I want behaviour such as when both right and left
    // buttons are pressed, I dont want actor to move, e.g. to have the move effect nullyfied

    if (g_pApp->GetGlobalOptions()->useAlternateControls)
    {
        if (m_InputKeys[SDLK_d] || m_ControllerAxis[0] > 0 || m_ControllerKeys[14])
        {
            moveX += m_Speed * (float)msDiff;
        }
        if (m_InputKeys[SDLK_a] || m_ControllerAxis[0] < 0 || m_ControllerKeys[12])
        {
            moveX -= m_Speed * (float)msDiff;
        }

        // CLimbing
        if (m_InputKeys[SDLK_s] || m_ControllerAxis[1] > 0 || m_ControllerKeys[15])
        {
            climbY += 5.0;
        }
        if (m_InputKeys[SDLK_w] || m_ControllerAxis[1] < 0 || m_ControllerKeys[13])
        {
            climbY -= 5.0;
        }

        // Jumping
        if (m_InputKeys[SDLK_SPACE] || m_InputKeys[SDLK_w] || m_ControllerKeys[1])
        {
            moveY -= m_Speed * (float)msDiff;
        }
    }
    else
    {
        if (m_InputKeys[SDLK_RIGHT] || m_ControllerAxis[0] > 0 || m_ControllerKeys[14])
        {
            moveX += m_Speed * (float)msDiff;
        }
        if (m_InputKeys[SDLK_LEFT] || m_ControllerAxis[0] < 0 || m_ControllerKeys[12])
        {
            moveX -= m_Speed * (float)msDiff;
        }

        // CLimbing
        if (m_InputKeys[SDLK_DOWN] || m_ControllerAxis[1] > 0 || m_ControllerKeys[15])
        {
            climbY += 5.0;
        }
        if (m_InputKeys[SDLK_UP] || m_ControllerAxis[1] < 0 || m_ControllerKeys[13])
        {
            climbY -= 5.0;
        }

        // Jumping
        if (m_InputKeys[SDLK_SPACE] || m_ControllerKeys[1])
        {
            moveY -= m_Speed * (float)msDiff;
        }
    }

    if (fabs(climbY) > FLT_EPSILON)
    {
        shared_ptr<EventData_Start_Climb> pClimbEvent(new EventData_Start_Climb(m_pControlledObject->VGetProperties()->GetActorId(), Point(0, climbY)));
        IEventMgr::Get()->VTriggerEvent(pClimbEvent);
    }
    if (fabs(moveX) > FLT_EPSILON || fabs(moveY) > FLT_EPSILON)
    {
        shared_ptr<EventData_Actor_Start_Move> pMoveEvent(new EventData_Actor_Start_Move(m_pControlledObject->VGetProperties()->GetActorId(), Point(moveX, moveY)));
        IEventMgr::Get()->VTriggerEvent(pMoveEvent);
    }
}

bool ActorController::VOnKeyDown(SDL_Keycode key)
{
    if (g_pApp->GetGlobalOptions()->useAlternateControls)
    {
        if (key == SDLK_e)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LSHIFT)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
    }
    else
    {
        if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LALT)
        {
            HandleAction(ActionType_Fire);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LCTRL)
        {
            HandleAction(ActionType_Attack);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LSHIFT)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
    }

    m_InputKeys[key] = true;

    return false;
}

bool ActorController::VOnKeyUp(SDL_Keycode key)
{
    if (g_pApp->GetGlobalOptions()->useAlternateControls)
    {

    }
    else
    {
        if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LALT)
        {
            shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);

            return true;
        }
    }

    m_InputKeys[key] = false;
    return false;
}

bool ActorController::VOnPointerMove(SDL_MouseMotionEvent& mouseEvent)
{
    return false;
}

bool ActorController::VOnPointerButtonDown(SDL_MouseButtonEvent& mouseEvent)
{
    if (mouseEvent.button == SDL_BUTTON_LEFT)
    {
        m_MouseLeftButtonDown = true;

        if (g_pApp->GetGlobalOptions()->useAlternateControls)
        {
            HandleAction(ActionType_Fire);
        }

        return true;
    }
    else if (mouseEvent.button == SDL_BUTTON_RIGHT)
    {
        m_MouseRightButtonDown = true;

        if (g_pApp->GetGlobalOptions()->useAlternateControls)
        {
            HandleAction(ActionType_Attack);
        }

        return true;
    }

    return false;
}

bool ActorController::VOnPointerButtonUp(SDL_MouseButtonEvent& mouseEvent)
{
    if (mouseEvent.button == SDL_BUTTON_LEFT)
    {
        shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
        IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);

        m_MouseLeftButtonDown = false;
        return true;
    }
    else if (mouseEvent.button == SDL_BUTTON_RIGHT)
    {
        m_MouseRightButtonDown = false;
        return true;
    }

    return false;
}



bool ActorController::VOnJoystickButtonDown(Uint8 button)
{
    // ToDo: Make those buttons configurable
    switch (button)
    {
        case 2:
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        case 0:
            HandleAction(ActionType_Fire);
            return true;
        case 3:
            HandleAction(ActionType_Attack);
            return true;
    }

    m_ControllerKeys[button] = true;

    return false;
}

bool ActorController::VOnJoystickButtonUp(Uint8 button)
{
    m_ControllerKeys[button] = false;

    if (button == 0)
    {
        shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
        IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);
    }

    return false;
}

bool ActorController::VOnJoystickAxisMotion(Uint8 axis, Sint16 value)
{
    m_ControllerAxis[axis] = value < 0 ? -1 : value > 0 ? 1 : 0;

    return false;
}
