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

class Command
{
public:
    virtual ~Command() {}
    virtual void Execute() = 0;
};

class Moveable
{
public:
    virtual ~Moveable() {}
    virtual void Move(float x, float y, float z) = 0;
    virtual void Rotate(float radians) = 0;
};

class SimpleObject : public Entity, public Moveable
{
public:
    SimpleObject(SimpleObjectGraphicsComponent *graphicsComponent)
        : _graphicsComponent(graphicsComponent)
    {
        _position = glm::vec3(0.0, 0.0, -6.0);
    }

    void update()
    {
        _graphicsComponent->update(_position);
    }

    void Move(float x, float y, float z)
    {
        _position = glm::vec3((_position)[0] + x, (_position)[1] + y, (_position)[2] + z);
    }

    void Rotate(float radians)
    {
    }

public:
    SimpleObjectGraphicsComponent *_graphicsComponent;

    glm::vec3 _position;
};

SimpleObject *CreateSimpleObject(IRenderer *renderer)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);

    return new SimpleObject(graphicsComponent);
}

class MoveCommand : public Command
{
public:
    MoveCommand(Moveable *moveable, float x, float y, float z)
        : _moveable(moveable), _x(x), _y(y), _z(z)
    {
    }

    void Execute()
    {
        _moveable->Move(_x, _y, _z);
    }

private:
    Moveable *_moveable;
    float _x;
    float _y;
    float _z;
};

class Camera : public Moveable
{
public:
    Camera(ADSRenderer *renderer)
        : _renderer(renderer)
    {
        _position = glm::vec3( 5.0f, 1.5f, 5.0f);
        _rotation = 0;
        _zoom = 5.0f;
        _orthoParamX = 4.0f;
        _orthoParamY = 3.0f;

        setProjectionMatrix();
        setPosition();
    }

    void Move(float x, float y, float z)
    {
    }

    void Rotate(float radians)
    {
        _rotation += radians;
        if (_rotation > TAU)
            _rotation -= TAU;
        if (_rotation < 0)
            _rotation += TAU;

        setPosition();
    }

    void Zoom(float delta)
    {
        _zoom += delta;
        setProjectionMatrix();
        setPosition();
    }

private:
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

class MouseTracker
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

    void update(Camera *camera)
    {
        Framework::KeyState mouseButtonState = _mouseState->GetMouseButtonState(System::MouseButton::Button2);
        unsigned int mouseX = _mouseState->GetMouseX();
        unsigned int mouseY = _mouseState->GetMouseY();

        int scrollDelta = _mouseState->GetScrollDelta();
        if (scrollDelta != 0)
            camera->Zoom(-0.1f * scrollDelta);

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

                camera->Rotate(((float)_mouseDeltaX / 10) * (PI / 180));
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

class ButtonState
{
public:
    ButtonState(System::KeyCode key, Framework::KeyState state)
        : _key(key), _state(state)
    {
    }

    System::KeyCode GetKey() const
    {
        return _key;
    }

    Framework::KeyState GetState() const
    {
        return _state;
    }

private:
    System::KeyCode _key;
    Framework::KeyState _state;
};

class InputHandler
{
private:
    class StateHandler
    {
    public:
        StateHandler(const ButtonState &_state, Command *_command)
            : state(_state), command(_command)
        {
        }

        ButtonState state;
        Command *command;
    };

public:
    InputHandler(Framework::ReadingKeyboardState *keyboardState,
                 Framework::ReadingMouseState *mouseState,
                 Framework::ReadingWindowState *windowState)
        : _keyboardState(keyboardState),
          _mouseState(mouseState),
          _windowState(windowState)
    {
    }

    void Update()
    {
        for (int i = 0; i < _handlers.size(); ++i)
        {
            StateHandler handler = _handlers[i];

            if (_keyboardState->GetKeyState(handler.state.GetKey()) == handler.state.GetState())
                handler.command->Execute();
        }
    }

    void SetHandler(const ButtonState &state, Command *command)
    {
        _handlers.push_back(StateHandler(state, command));
    }

private:
    Framework::ReadingKeyboardState *_keyboardState;
    Framework::ReadingMouseState *_mouseState;
    Framework::ReadingWindowState *_windowState;

    std::vector<StateHandler> _handlers;
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

    SimpleObject *simpleObject = CreateSimpleObject(renderer);

    TileRenderer tileRenderer(adsRenderer, LoadImageFromPNG("assets/colors.png"), 16, 16);
    VoxelRepository voxelRepository;
    voxelRepository.AddVoxel(GenerateVoxel(1,0,0));
    voxelRepository.AddVoxel(GenerateVoxel(2,0,1));
    voxelRepository.AddVoxel(GenerateVoxel(3,1,0));
    voxelRepository.AddVoxel(GenerateVoxel(4,1,1));

    MouseTracker *mouseTracker = new MouseTracker(windowController);
    Camera *camera = new Camera(adsRenderer);
    ticker.Start(17);

    InputHandler inputHandler(keyboardState, mouseState, windowState);
    MoveCommand moveUp(simpleObject, 0.0f, 0.1f, 0.0f);
    MoveCommand moveDown(simpleObject, 0.0f, -0.1f, 0.0f);
    inputHandler.SetHandler(ButtonState(System::KeyCode::KeyUpArrow,
                                        Framework::KeyState::Pressed),
                            &moveUp);
    inputHandler.SetHandler(ButtonState(System::KeyCode::KeyDownArrow,
                                        Framework::KeyState::Pressed),
                            &moveDown);

    std::vector<Entity*> entities;

    entities.push_back((Entity*)simpleObject);

    VoxelSector *sector = CreateVoxelSector(adsRenderer, &tileRenderer,
                                            &voxelRepository, Position(0, 0, 0));
    //sector->SetVoxel(Position(0, 0, 0), 0);
    //sector->SetVoxel(Position(1, 0, 0), 1);
    //sector->SetVoxel(Position(2, 0, 0), 2);
    //sector->SetVoxel(Position(3, 0, 0), 1);
    //sector->SetVoxel(Position(4, 0, 0), 3);

    entities.push_back(sector);
    //sector->Export("test.map");
    sector->Import("test.map");

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
        inputHandler.Update();
        mouseTracker->update(camera);
        for (int i = 0; i < entities.size(); ++i)
        {
            entities[i]->update();
        }

        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
