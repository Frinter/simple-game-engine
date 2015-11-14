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
#include "voxels.hh"

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
        tilemapMaterialInfo.Ka = glm::vec3(1.0f, 1.0f, 1.0f);
        tilemapMaterialInfo.Kd = glm::vec3(1.0f, 1.0f, 1.0f);
        tilemapMaterialInfo.Ks = glm::vec3(0.0f, 0.0f, 0.0f);
        tilemapMaterialInfo.shininess = 1.0f;
        tilemapMaterialInfo.Kd_imageInfo = tilemapImage;

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
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));

            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            addToVector(normals, glm::vec3( 1.0f, 0.0f, 0.0f));
            break;

        case Direction::Forward:
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 1.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 1.0f, 0.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));

            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            addToVector(normals, glm::vec3( 0.0f, 0.0f, 1.0f));
            break;

        case Direction::Backward:
            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(1.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));

            addToVector(vertices, location + glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f,-1.0f, 0.0f, 0.0f));
            addToVector(vertices, location + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

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
    unsigned int _tileWidth;
    unsigned int _tileHeight;
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
        bottom = (y + 1) * _tileHeight;
        left = x * _tileWidth;
        right = (x + 1) * _tileWidth;

        float UVtop, UVbottom, UVleft, UVright;
        UVleft = 0.995 * (float) left / _tilemapImage->width + 0.002f;
        UVright = 0.995 * (float) right / _tilemapImage->width + 0.002f;
        UVtop = 0.995 * (float) top / _tilemapImage->height + 0.002f;
        UVbottom = 0.995 * (float) bottom / _tilemapImage->height + 0.002f;

        UVs.push_back(UVleft);
        UVs.push_back(UVtop);
        UVs.push_back(UVleft);
        UVs.push_back(UVbottom);
        UVs.push_back(UVright);
        UVs.push_back(UVbottom);

        UVs.push_back(UVleft);
        UVs.push_back(UVtop);
        UVs.push_back(UVright);
        UVs.push_back(UVbottom);
        UVs.push_back(UVright);
        UVs.push_back(UVtop);

        return UVs;
    }
};

class VoxelCollection
{
public:
    VoxelCollection(VoxelRepository *voxelRepository)
        : _repository(voxelRepository)
    {
        _voxels = new const Voxel*[VOXEL_SECTOR_ARRAY_SIZE];
        for (int i = 0; i < VOXEL_SECTOR_ARRAY_SIZE; ++i)
        {
            _voxels[i] = NULL;
        }
    }

    const Voxel *GetVoxel(IndexValue x, IndexValue y, IndexValue z) const
    {
        return _voxels[VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * y +
                       VOXEL_SECTOR_SIZE * z +
                       x];
    }

    void SetVoxel(IndexValue x, IndexValue y, IndexValue z, IndexValue type)
    {
        *getVoxelAtIndex(x, y, z) = _repository->GetVoxel(type);
    }
    
private:
    VoxelRepository *_repository;

    const Voxel **_voxels;

private:
    const Voxel **getVoxelAtIndex(IndexValue x, IndexValue y, IndexValue z)
    {
        return &_voxels[VOXEL_SECTOR_SIZE * VOXEL_SECTOR_SIZE * y +
                        VOXEL_SECTOR_SIZE * z +
                        x];
    }
};

class VoxelSectorGraphicsComponent
{
public:
    VoxelSectorGraphicsComponent(ADSRenderer *renderer, TileRenderer *tileRenderer)
        : _renderer(renderer), _tileRenderer(tileRenderer)
    {
    }
    
    void update(const VoxelCollection &collection)
    {
        _renderer->SetModelMatrix(glm::mat4(1.0));

        int x, y, z;
        for (x = 0; x < VOXEL_SECTOR_SIZE; ++x)
        {
            for (y = 0; y < VOXEL_SECTOR_SIZE; ++y)
            {
                for (z = 0; z < VOXEL_SECTOR_SIZE; ++z)
                {
                    const Voxel *voxel = collection.GetVoxel(x, y, z);
                    if (voxel != NULL)
                    {
                        renderVoxel(voxel->tileX, voxel->tileY,
                                    glm::vec3(x, y, z));
                    }
                }
            }
        }
        // renderVoxel(0, 0, glm::vec3(-1.0f,-1.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3( 0.0f,-1.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3( 1.0f,-1.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3(-1.0f, 0.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3( 0.0f, 0.0f, 0.0f));  
        // renderVoxel(0, 0, glm::vec3( 1.0f, 0.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3(-1.0f, 1.0f, 0.0f)); 
        // renderVoxel(0, 0, glm::vec3( 0.0f, 1.0f, 0.0f));
        // renderVoxel(0, 0, glm::vec3( 1.0f, 1.0f, 0.0f));
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
    VoxelSector(VoxelSectorGraphicsComponent *graphicsComponent,
                VoxelRepository *voxelRepository)
        : _graphicsComponent(graphicsComponent),
          _collection(voxelRepository)
    {
    }
    
    void update()
    {
        _graphicsComponent->update(_collection);
    }

    void SetVoxel(IndexValue x, IndexValue y, IndexValue z, IndexValue type)
    {
        _collection.SetVoxel(x, y, z, type);
    }
    
private:
    VoxelSectorGraphicsComponent *_graphicsComponent;

    VoxelCollection _collection;
};

VoxelSector *CreateVoxelSector(ADSRenderer *renderer, TileRenderer *tileRenderer,
                               VoxelRepository *voxelRepository)
{
    VoxelSectorGraphicsComponent *graphicsComponent = new VoxelSectorGraphicsComponent(renderer, tileRenderer);
    VoxelSector *sector = new VoxelSector(graphicsComponent, voxelRepository);

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
        _position = glm::vec3( 5.0f, 1.5f, 5.0f);
        _rotation = 0;
        _zoom = 5.0f;
        _orthoParamX = 4.0f;
        _orthoParamY = 3.0f;
        _renderer->SetViewMatrix(glm::lookAt(_position,
                                             glm::vec3( 0.5, 0.5, 0.5),
                                             glm::vec3( 0.0, 1.0, 0.0)));
        setProjectionMatrix();
    }

    void update()
    {
        if (_mouseState->GetMouseButtonState(System::MouseButton::Button1) == Framework::KeyState::Pressed)
        {
            int mouseDeltaX, mouseDeltaY;
            _mouseTracker->GetDeltaPosition(&mouseDeltaX, &mouseDeltaY);

            _rotation += ((float)mouseDeltaX / 10) * (PI / 180);
            if (_rotation > TAU)
                _rotation -= TAU;
            if (_rotation < 0)
                _rotation += TAU;
            
            glm::vec3 rotatedPosition = glm::rotate(_position,
                                                    _rotation,
                                                    glm::vec3( 0.0, 1.0, 0.0));
            _renderer->SetViewMatrix(glm::lookAt(rotatedPosition,
                                                 glm::vec3( 0.5, 0.5, 0.5),
                                                 glm::vec3( 0.0, 1.0, 0.0)));
        }

        int scrollDelta = _mouseState->GetScrollDelta();
        if (scrollDelta != 0)
        {
            _zoom += -0.1f * scrollDelta;
            setProjectionMatrix();
        }
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
    
    VoxelSector *sector = CreateVoxelSector(adsRenderer, &tileRenderer, &voxelRepository);
    sector->SetVoxel(0, 0, 0, 0);
    sector->SetVoxel(1, 0, 0, 0);
    sector->SetVoxel(0, 1, 0, 0);
    sector->SetVoxel(1, 2, 0, 1);
    sector->SetVoxel(1, 2, 1, 2);
    sector->SetVoxel(4, 2, 3, 3);
    
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
