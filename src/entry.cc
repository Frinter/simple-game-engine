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
#include "types.hh"

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

SimpleObject *CreateSimpleObject(IRenderer *renderer,
                                 Framework::ReadingKeyboardState *keyboardState,
                                 Framework::ReadingMouseState *mouseState)
{
    SimpleObjectGraphicsComponent *graphicsComponent = new SimpleObjectGraphicsComponent(renderer);
    SimpleObjectInputComponent *inputComponent = new SimpleObjectInputComponent(keyboardState, mouseState);

    return new SimpleObject(graphicsComponent, inputComponent);
}

enum class Direction
{
    Up,
    Down,
    Left,
    Right,
    Forward,
    Backward
};

class TileRenderer
{
public:
    TileRenderer(ADSRenderer *renderer, RawImageInfo *tilemapImage, float tileWidth, float tileHeight)
        : _renderer(renderer), _tilemapImage(tilemapImage), _tileWidth(tileWidth), _tileHeight(tileHeight)
    {
        MaterialInfo tilemapMaterialInfo;
        tilemapMaterialInfo.Ka = glm::vec3(0.0f, 0.0f, 0.0f);
        tilemapMaterialInfo.Kd = glm::vec3(1.0f, 1.0f, 1.0f);
        tilemapMaterialInfo.Ks = glm::vec3(0.0f, 0.0f, 0.0f);
        tilemapMaterialInfo.shininess = 1.0f;
        tilemapMaterialInfo.Kd_imageInfo = LoadImageFromPNG("assets/minecraft-tiles.png");

        _materialId = _renderer->RegisterMaterial(tilemapMaterialInfo);
    }

    void Render(IndexValue tileX, IndexValue tileY, const glm::vec4 &location, Direction direction)
    {
        std::vector<IndexValue> indices;
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> UVs = GetUVsForTile(tileX, tileY);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(4);
        indices.push_back(5);

        switch(direction)
        {
        case Direction::Up:
            addToVector(vertices, location);
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));

            addToVector(vertices, location);
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f, 1.0f, 0.0f));
            break;

        case Direction::Down:
            addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 1.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, -1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f, -1.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));

            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
            addToVector(normals, glm::vec3(0.0f,-1.0f, 0.0f));
            break;

        case Direction::Left:
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));

            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3(-1.0f, 0.0f, 0.0f));
            break;

        case Direction::Right:
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));

            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            break;

        case Direction::Forward:
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            break;

        case Direction::Backward:
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            break;
        }

        _renderer->UseMaterial(_materialId);
        _renderer->Render(indices, vertices, normals, UVs);
    }

private:
    ADSRenderer *_renderer;
    IndexValue _materialId;
    float _tileWidth;
    float _tileHeight;
    RawImageInfo *_tilemapImage;

private:
    void addToVector(std::vector<float> &list, const glm::vec4 &vec)
    {
        list.push_back(vec[0]);
        list.push_back(vec[1]);
        list.push_back(vec[2]);
        list.push_back(vec[3]);
    }

    void addToVector(std::vector<float> &list, const glm::vec3 &vec)
    {
        list.push_back(vec[0]);
        list.push_back(vec[1]);
        list.push_back(vec[2]);
    }

    std::vector<float> GetUVsForTile(IndexValue x, IndexValue y)
    {
        std::vector<float> UVs;

        unsigned int top, bottom, left, right;
        top = y * _tileHeight;
        bottom = (y + 1) * _tileHeight - 1;
        left = x * _tileWidth;
        right = (x + 1) * _tileWidth - 1;

        float UVtop, UVbottom, UVleft, UVright;
        UVleft = (float) left / _tilemapImage->width;
        UVright = (float) right / _tilemapImage->width;
        UVtop = (float) top / _tilemapImage->height;
        UVbottom = (float) bottom / _tilemapImage->height;

        UVs.push_back(UVright);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVbottom);

        UVs.push_back(UVright);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVbottom);
        UVs.push_back(UVright);
        UVs.push_back(UVbottom);

        return UVs;
    }
};

class VoxelRenderer
{
public:
    VoxelRenderer(TileRenderer *tileRenderer)
        : _tileRenderer(tileRenderer)
    {
    }

    void Render(IndexValue tileX, IndexValue tileY, const glm::vec3 &position)
    {
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Up);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Down);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Left);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Right);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Forward);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Backward);
    }

private:
    TileRenderer *_tileRenderer;
};

typedef struct Voxel
{
    IndexValue tileX;
    IndexValue tileY;
} Voxel;

class VoxelSectorGraphicsComponent
{
public:
    VoxelSectorGraphicsComponent(ADSRenderer *renderer, TileRenderer *tileRenderer)
        : _renderer(renderer), _tileRenderer(tileRenderer)
    {
    }
    
    void update()
    {
        _renderer->SetModelMatrix(glm::mat4(1.0));
        renderVoxel(1, 0, glm::vec3(3.0f, 0.0f, 0.0f));
        renderVoxel(1, 0, glm::vec3(3.0f, 1.0f, 0.0f));
        renderVoxel(1, 0, glm::vec3(3.0f, 2.0f, 0.0f));
        renderVoxel(1, 0, glm::vec3(2.0f, 0.0f, 0.0f));
        renderVoxel(1, 0, glm::vec3(2.0f, 1.0f, 0.0f));
        renderVoxel(1, 0, glm::vec3(2.0f, 2.0f, 0.0f));
        renderVoxel(2, 0, glm::vec3(2.0f, 2.0f, 1.0f));
        renderVoxel(2, 0, glm::vec3(2.0f, 1.0f, 1.0f));
    }

private:
    ADSRenderer *_renderer;
    TileRenderer *_tileRenderer;

private:
    void renderVoxel(IndexValue tileX, IndexValue tileY, const glm::vec3 &position)
    {
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Up);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Down);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Left);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Right);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Forward);
        _tileRenderer->Render(tileX, tileY, glm::vec4(position, 1.0), Direction::Backward);
    }
};

class VoxelSector : public Entity
{
public:
    VoxelSector(VoxelSectorGraphicsComponent *graphicsComponent)
        : _graphicsComponent(graphicsComponent)
    {
    }
    
    void update()
    {
        _graphicsComponent->update();
    }
    
private:
    VoxelSectorGraphicsComponent *_graphicsComponent;
    
};

VoxelSector *CreateVoxelSector(ADSRenderer *renderer, TileRenderer *tileRenderer)
{
    VoxelSectorGraphicsComponent *graphicsComponent = new VoxelSectorGraphicsComponent(renderer, tileRenderer);
    VoxelSector *sector = new VoxelSector(graphicsComponent);

    return sector;
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
        Framework::KeyState mouseButtonState = _mouseState->GetMouseButtonState(System::MouseButton::Button1);
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
        _position = glm::vec3( -3.0, 3.0, 4.0);
        _rotation = 0;
        _renderer->SetViewMatrix(glm::lookAt(_position,
                                             glm::vec3( 0.0, 0.0, 0.0),
                                             glm::vec3( 0.0, 1.0, 0.0)));
        _renderer->SetProjectionMatrix(glm::ortho(-6.0f, 6.0f,
                                                  -4.0f, 4.0f,
                                                  1.0f, 100.0f));
    }

    void update()
    {
        if (_mouseState->GetMouseButtonState(System::MouseButton::Button1) == Framework::KeyState::Pressed)
        {
            int mouseDeltaX, mouseDeltaY;
            _mouseTracker->GetDeltaPosition(&mouseDeltaX, &mouseDeltaY);

            _rotation += (mouseDeltaX / 5) * (PI / 180);
            if (_rotation > TAU)
                _rotation -= TAU;
            if (_rotation < 0)
                _rotation += TAU;
            
            glm::vec3 rotatedPosition = glm::rotate(_position,
                                                    _rotation,
                                                    glm::vec3( 0.0, 1.0, 0.0));
            _renderer->SetViewMatrix(glm::lookAt(rotatedPosition,
                                                 glm::vec3( 0.0, 0.0, 0.0),
                                                 glm::vec3( 0.0, 1.0, 0.0)));        
        }
    }
    
private:
    MouseTracker *_mouseTracker;
    Framework::ReadingMouseState *_mouseState;
    ADSRenderer *_renderer;

    glm::vec3 _position;
    float _rotation;
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

    TileRenderer tileRenderer(adsRenderer, LoadImageFromPNG("assets/minecraft-tiles.png"), 64, 64);
    VoxelSector *sector = CreateVoxelSector(adsRenderer, &tileRenderer);

    MouseTracker *mouseTracker = new MouseTracker(windowController);
    Camera *camera = new Camera(mouseTracker, mouseState, adsRenderer);
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
        mouseTracker->update();
        camera->update();
        sector->update();
        simpleEntity->update();
        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
