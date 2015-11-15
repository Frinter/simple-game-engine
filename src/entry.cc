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
#include <glm/gtx/rotate_vector.hpp>

#include <framework/platform.hh>
#include <GL/gl_core_3_3.h>

#include "adsrenderer.hh"
#include "entity.hh"
#include "imageloader.hh"
#include "model.hh"
#include "objimporter/objimporter.hh"
#include "objimporter/objparser.hh"
#include "utility/sleepservice.hh"
#include "utility/systemtimer.hh"
#include "utility/ticker.hh"
#include "tilerenderer.hh"
#include "types.hh"
#include "voxels.hh"
#include "voxelsector.hh"

using std::cout;
using std::endl;

const float PI = 3.1415926;
const float TAU = 2 * PI;

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
        _position = glm::vec3(0.0, 0.0, -6.0);
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

SimpleObject *CreateSimpleObject(IRenderer *renderer,
                                 Framework::ReadingKeyboardState *keyboardState,
                                 Framework::ReadingMouseState *mouseState)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);
    SimpleObjectInputComponent *inputComponent = new SimpleObjectInputComponent(keyboardState, mouseState);

    return new SimpleObject(graphicsComponent, inputComponent);
}

class MouseTracker : public Entity
{
public:
    MouseTracker(Framework::IWindowController *windowController)
        : _windowController(windowController)
    {
        _mouseState = _windowController->GetMouseReader();
        _previousMouseButtonState = Framework::KeyState::Unpressed;
    }

    void GetDeltaPosition(int *deltaX, int *deltaY)
    {
        *deltaX = _mouseDeltaX;
        *deltaY = _mouseDeltaY;
    }

    void update()
    {
        Framework::KeyState mouseButtonState = _mouseState->GetMouseButtonState(System::MouseButton::Button2);
        unsigned int mouseX = _mouseState->GetMouseX();
        unsigned int mouseY = _mouseState->GetMouseY();

        if (mouseButtonState == Framework::KeyState::Pressed)
        {
            if (_previousMouseButtonState == Framework::KeyState::Unpressed)
            {
                _mouseLockX = mouseX;
                _mouseLockY = mouseY;
            }

            if (mouseX != _mouseLockX || mouseY != _mouseLockY)
            {
                _windowController->SetMousePosition(_mouseLockX, _mouseLockY);

                _mouseDeltaX = mouseX - _mouseLockX;
                _mouseDeltaY = mouseY - _mouseLockY;

                mouseX = _mouseState->GetMouseX();
                mouseY = _mouseState->GetMouseY();
            }
            else
            {
                _mouseDeltaX = 0;
                _mouseDeltaY = 0;
            }
        }
        else
        {
            _mouseDeltaX = mouseX - _mousePreviousX;
            _mouseDeltaY = mouseY - _mousePreviousY;

            _mousePreviousX = mouseX;
            _mousePreviousY = mouseY;
        }

        _previousMouseButtonState = mouseButtonState;
    }

private:
    Framework::IWindowController *_windowController;
    Framework::ReadingMouseState *_mouseState;

    Framework::KeyState _previousMouseButtonState;
    unsigned int _mouseLockX, _mouseLockY;
    unsigned int _mousePreviousX, _mousePreviousY;
    int _mouseDeltaX, _mouseDeltaY;
};

class Camera : public Entity
{
public:
    Camera(MouseTracker *mouseTracker, Framework::ReadingMouseState *mouseState,
        ADSRenderer *renderer)
        : _mouseTracker(mouseTracker), _mouseState(mouseState),
          _renderer(renderer)
    {
        _position = glm::vec3( 5.0f, 1.5f, 5.0f);
        _rotation = 0;
        _zoom = 5.0f;
        _orthoParamX = 4.0f;
        _orthoParamY = 3.0f;

        setProjectionMatrix();
        setPosition();
    }

    void update()
    {
        if (_mouseState->GetMouseButtonState(System::MouseButton::Button2) == Framework::KeyState::Pressed)
        {
            int mouseDeltaX, mouseDeltaY;
            _mouseTracker->GetDeltaPosition(&mouseDeltaX, &mouseDeltaY);

            _rotation += ((float)mouseDeltaX / 10) * (PI / 180);
            if (_rotation > TAU)
                _rotation -= TAU;
            if (_rotation < 0)
                _rotation += TAU;
        }

        int scrollDelta = _mouseState->GetScrollDelta();
        if (scrollDelta != 0)
        {
            _zoom += -0.1f * scrollDelta;
            setProjectionMatrix();
        }

        setPosition();
    }

private:
    MouseTracker *_mouseTracker;
    Framework::ReadingMouseState *_mouseState;
    ADSRenderer *_renderer;

    glm::vec3 _position;
    float _rotation;
    float _zoom;
    float _orthoParamX;
    float _orthoParamY;

private:
    void setPosition()
    {
        glm::vec3 adjustedPosition = _zoom * _position;
        glm::vec3 rotatedPosition = glm::rotate(adjustedPosition,
                                                _rotation,
                                                glm::vec3( 0.0, 1.0, 0.0));
        _renderer->SetViewMatrix(glm::lookAt(rotatedPosition,
                                             glm::vec3( 0.0, 0.0, 0.0),
                                             glm::vec3( 0.0, 1.0, 0.0)));
    }

    void setProjectionMatrix()
    {
        float xParam = _zoom * _orthoParamX;
        float yParam = _zoom * _orthoParamY;
        _renderer->SetProjectionMatrix(glm::ortho(-xParam, xParam,
                                                  -yParam, yParam,
                                                  1.0f, 100.0f));
    }
};

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

    LightInfo lightInfo;
    lightInfo.position = glm::vec4(-2.0, 5.0, 4.0, 1.0);
    lightInfo.La = glm::vec3(0.5, 0.5, 0.5);
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

    TileRenderer tileRenderer(adsRenderer, LoadImageFromPNG("assets/colors.png"), 16, 16);
    VoxelRepository voxelRepository;
    voxelRepository.AddVoxel(GenerateVoxel(0,0));
    voxelRepository.AddVoxel(GenerateVoxel(0,1));
    voxelRepository.AddVoxel(GenerateVoxel(1,0));
    voxelRepository.AddVoxel(GenerateVoxel(1,1));

    MouseTracker *mouseTracker = new MouseTracker(windowController);
    Camera *camera = new Camera(mouseTracker, mouseState, adsRenderer);
    ticker.Start(17);

    std::vector<Entity*> entities;

    entities.push_back(simpleEntity);

    VoxelSector *sector = CreateVoxelSector(adsRenderer, &tileRenderer,
                                            &voxelRepository, Position(0, 0, 0));
    sector->SetVoxel(Position(0, 0, 0), 0);
    sector->SetVoxel(Position(1, 0, 0), 1);
    sector->SetVoxel(Position(2, 0, 0), 2);
    sector->SetVoxel(Position(3, 0, 0), 1);
    sector->SetVoxel(Position(4, 0, 0), 3);

    entities.push_back(sector);

    sector = CreateVoxelSector(adsRenderer, &tileRenderer,
                               &voxelRepository, Position(-1, 0, 0));
    sector->SetVoxel(Position(15, 0, 0), 0);
    sector->SetVoxel(Position(13, 0, 0), 1);
    sector->SetVoxel(Position(12, 0, 0), 2);
    sector->SetVoxel(Position(11, 0, 0), 1);
    sector->SetVoxel(Position(10, 0, 0), 3);

    entities.push_back(sector);

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

        if (keyboardState->GetKeyState(System::KeyCode::KeyQ) == Framework::KeyState::Pressed)
        {
            applicationContext->Close();
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        mouseTracker->update();
        camera->update();

        for (int i = 0; i < entities.size(); ++i)
        {
            entities[i]->update();
        }

        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
