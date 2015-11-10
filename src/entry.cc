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

    void update()
    {
        _renderer->Render(_model);
    }

private:
    IRenderer *_renderer;
    Model *_model;
};

class SimpleObject : public Entity
{
public:
    SimpleObject(SimpleObjectGraphicsComponent *graphicsComponent)
        : _graphicsComponent(graphicsComponent)
    {
    }

    void update()
    {
        _graphicsComponent->update();
    }

public:
    SimpleObjectGraphicsComponent *_graphicsComponent;
};

SimpleObject *CreateSimpleObject(IRenderer *renderer)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);

    return new SimpleObject(graphicsComponent);
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

    adsRenderer->SetModelMatrix(glm::translate(glm::mat4(1.0),
                                               glm::vec3(0.0, 0.0, 0.0)));
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

    Entity *simpleEntity = (Entity *)CreateSimpleObject(renderer);

    ticker.Start(17);

    Framework::ReadingKeyboardState *keyboardState = windowController->GetKeyStateReader();
    
    while (!applicationContext->IsClosing())
    {
        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }

        glClear(GL_DEPTH_BUFFER_BIT);
        simpleEntity->update();
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
