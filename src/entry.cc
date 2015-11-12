#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <framework/platform.hh>
#include <GL/gl_core_3_3.h>

#include "adsrenderer.hh"
#include "entity.hh"
#include "imageloader.hh"
#include "model.hh"
#include "objimporter.hh"
#include "objparser.hh"
#include "sleepservice.hh"
#include "systemtimer.hh"
#include "ticker.hh"
#include "types.hh"

using std::cout;
using std::endl;

const float PI = 3.1415926;

class SimpleObjectInputComponent
{
public:
    SimpleObjectInputComponent(Framework::ReadingKeyboardState *keyboardState,
                               Framework::ReadingMouseState *mouseState)
        : _keyboardState(keyboardState), _mouseState(mouseState)
    {
        _velocity = 0.02;
    }

    void update(glm::vec3 *position)
    {
        if (_keyboardState->GetKeyState(System::KeyCode::KeyUpArrow) == Framework::KeyState::Pressed)
        {
            *position = glm::vec3((*position)[0], (*position)[1] + _velocity, (*position)[2]);
        }
        if (_keyboardState->GetKeyState(System::KeyCode::KeyDownArrow) == Framework::KeyState::Pressed)
        {
            *position = glm::vec3((*position)[0], (*position)[1] - _velocity, (*position)[2]);
        }
        if (_keyboardState->GetKeyState(System::KeyCode::KeyLeftArrow) == Framework::KeyState::Pressed)
        {
            *position = glm::vec3((*position)[0] - _velocity, (*position)[1], (*position)[2]);
        }
        if (_keyboardState->GetKeyState(System::KeyCode::KeyRightArrow) == Framework::KeyState::Pressed)
        {
            *position = glm::vec3((*position)[0] + _velocity, (*position)[1], (*position)[2]);
        }

        if (_mouseState->GetMouseButtonState(System::MouseButton::Button1) == Framework::KeyState::Pressed)
        {
            cout << "Button 1 pressed" << endl;
        }
    }

private:
    Framework::ReadingKeyboardState *_keyboardState;
    Framework::ReadingMouseState *_mouseState;
    float _velocity;
};

class SimpleObjectGraphicsComponent
{
public:
    SimpleObjectGraphicsComponent(IRenderer *renderer)
        : _renderer(renderer)
    {
        ObjParser::ObjFileParser parser("assets/", "textured-things.obj");
        ObjParser::IParseResult *parseResult = parser.Parse();
        ObjImporter importer(parseResult);
        _model = renderer->CreateModelFromImporter(importer);
    }

    void update(const glm::vec3 &position)
    {
        _renderer->SetModelMatrix(glm::translate(glm::mat4(1.0), position));
        _renderer->Render(_model);
    }

private:
    IRenderer *_renderer;
    Model *_model;
};

class SimpleObject : public Entity
{
public:
    SimpleObject(SimpleObjectGraphicsComponent *graphicsComponent,
                 SimpleObjectInputComponent *inputComponent)
        : _graphicsComponent(graphicsComponent), _inputComponent(inputComponent)
    {
        _position = glm::vec3(0.0, -1.0, 0.0);
    }

    void update()
    {
        _inputComponent->update(&_position);
        //_graphicsComponent->update(_position);
    }

public:
    SimpleObjectGraphicsComponent *_graphicsComponent;
    SimpleObjectInputComponent *_inputComponent;

    glm::vec3 _position;
};

SimpleObject *CreateSimpleObject(IRenderer *renderer,
                                 Framework::ReadingKeyboardState *keyboardState,
                                 Framework::ReadingMouseState *mouseState)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);
    SimpleObjectInputComponent *inputComponent = new SimpleObjectInputComponent(keyboardState, mouseState);

    return new SimpleObject(graphicsComponent, inputComponent);
}

static Framework::ApplicationState applicationState = {
    .windowName = "Rendering Engine"
};

GetApplicationState_FunctionSignature(GetApplicationState)
{
    return &applicationState;
}

ApplicationThreadEntry_FunctionSignature(ApplicationThreadEntry)
{
    windowController->CreateContext();

    SystemTimer systemTimer(applicationContext->GetSystemUtility());
    SleepService sleepService(applicationContext->GetSystemUtility());
    Ticker ticker = Ticker(&systemTimer, &sleepService);

    ADSRenderer *adsRenderer = new ADSRenderer();

    adsRenderer->SetViewMatrix(glm::lookAt(glm::vec3( -5.0, 3.0, 10.0),
                                           glm::vec3( 0.0, 0.0, 0.0),
                                           glm::vec3( 0.0, 1.0, 0.0)));
    adsRenderer->SetProjectionMatrix(glm::ortho(-6.0f, 6.0f,
                                                -4.0f, 4.0f,
                                                1.0f, 100.0f));
    LightInfo lightInfo;
    lightInfo.position = glm::vec4(-2.0, 5.0, 4.0, 1.0);
    lightInfo.La = glm::vec3(0.0, 0.0, 0.0);
    lightInfo.Ld = glm::vec3(1.0, 1.0, 1.0);
    lightInfo.Ls = glm::vec3(0.0, 0.0, 0.0);
    adsRenderer->SetLight(lightInfo);

    IRenderer *renderer = (IRenderer *)adsRenderer;

    Framework::ReadingKeyboardState *keyboardState = windowController->GetKeyStateReader();
    Framework::ReadingMouseState *mouseState = windowController->GetMouseReader();
    Framework::ReadingWindowState *windowState = windowController->GetWindowReader();

    unsigned int windowWidth, windowHeight, newWindowWidth, newWindowHeight;
    windowState->GetSize(&windowWidth, &windowHeight);

    Entity *simpleEntity = (Entity *)CreateSimpleObject(renderer, keyboardState, mouseState);

    IndexValue tilemapId = adsRenderer->RegisterTileMap(LoadImageFromPNG("assets/minecraft-tiles.png"), 64, 64);


    ticker.Start(17);

    while (!applicationContext->IsClosing())
    {
        windowState->GetSize(&newWindowWidth, &newWindowHeight);
        if (newWindowWidth != windowWidth || newWindowHeight != windowHeight)
        {
            windowWidth = newWindowWidth;
            windowHeight = newWindowHeight;
            adsRenderer->SetProjectionMatrix(glm::ortho(-6.0f, 6.0f,
                                                        -4.0f, 4.0f,
                                                        1.0f, 100.0f));
        }

        int scrollDelta = mouseState->GetScrollDelta();
        if (scrollDelta != 0)
        {
            cout << "scroll: " << scrollDelta << endl;
        }

        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        adsRenderer->SetModelMatrix(glm::mat4(1.0));
        adsRenderer->RenderTile(tilemapId, 1, 0);
        adsRenderer->SetModelMatrix(glm::rotate(glm::mat4(1.0f), -90.0f * PI / 180, glm::vec3(0.0f, 1.0f, 0.0f)));
        adsRenderer->RenderTile(tilemapId, 2, 0);
        simpleEntity->update();
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
