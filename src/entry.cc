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
#include "model.hh"
#include "objimporter.hh"
#include "objparser.hh"
#include "sleepservice.hh"
#include "systemtimer.hh"
#include "ticker.hh"
#include "types.hh"

using std::cout;
using std::endl;

class SimpleObjectInputComponent
{
public:
    SimpleObjectInputComponent(Framework::ReadingKeyboardState *keyboardState)
        : _keyboardState(keyboardState)
    {
        _velocity = 0.01;
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
    }

private:
    Framework::ReadingKeyboardState *_keyboardState;
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
        _graphicsComponent->update(_position);
    }

public:
    SimpleObjectGraphicsComponent *_graphicsComponent;
    SimpleObjectInputComponent *_inputComponent;

    glm::vec3 _position;
};

SimpleObject *CreateSimpleObject(IRenderer *renderer, Framework::ReadingKeyboardState *keyboardState)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);
    SimpleObjectInputComponent *inputComponent = new SimpleObjectInputComponent(keyboardState);

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

    adsRenderer->SetViewMatrix(glm::lookAt(glm::vec3(-2.0, 3.0, 3.0),
                                           glm::vec3( 0.0, 0.0, 0.0),
                                           glm::vec3( 0.0, 1.0, 0.0)));
    LightInfo lightInfo;
    lightInfo.position = glm::vec4(-2.0, 5.0, 4.0, 1.0);
    lightInfo.La = glm::vec3(0.0, 0.0, 0.0);
    lightInfo.Ld = glm::vec3(1.0, 1.0, 1.0);
    lightInfo.Ls = glm::vec3(0.0, 0.0, 0.0);
    adsRenderer->SetLight(lightInfo);

    IRenderer *renderer = (IRenderer *)adsRenderer;

    Framework::ReadingKeyboardState *keyboardState = windowController->GetKeyStateReader();

    Entity *simpleEntity = (Entity *)CreateSimpleObject(renderer, keyboardState);

    ticker.Start(17);

    while (!applicationContext->IsClosing())
    {
        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        simpleEntity->update();
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
