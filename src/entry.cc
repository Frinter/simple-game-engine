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

#include "camera.hh"
#include "conditionhandler.hh"
#include "conditions/mousescrollcondition.hh"
#include "entity.hh"
#include "imageloader.hh"
#include "moveable.hh"
#include "mousetracker.hh"
#include "objimporter/objimporter.hh"
#include "objimporter/objparser.hh"
#include "rendering/adsrenderer.hh"
#include "utility/sleepservice.hh"
#include "utility/systemtimer.hh"
#include "utility/ticker.hh"
#include "testinputhandler.hh"
#include "tilerenderer.hh"
#include "types.hh"
#include "voxels.hh"
#include "voxelsector.hh"

using std::cout;
using std::endl;

class SimpleObjectGraphicsComponent
{
private:
    typedef struct ExtendedRenderObject
    {
        RenderObject object;
        IndexValue materialId;
    } ExtendedRenderObject;

public:
    SimpleObjectGraphicsComponent(IRenderer *renderer)
        : _renderer(renderer)
    {
        ObjParser::ObjFileParser parser("assets/", "textured-things.obj");
        ObjParser::IParseResult *parseResult = parser.Parse();
        ObjImporter importer(parseResult);
        std::vector<RenderObject> renderObjects = importer.GetRenderObjects();
        for (int i = 0; i < renderObjects.size(); ++i)
        {
            RenderObject *renderObject = &renderObjects[i];
            ExtendedRenderObject object;
            object.object = *renderObject;
            object.materialId = _renderer->RegisterMaterial(importer.GetMaterial(renderObject->_materialName));
            _renderObjects.push_back(object);
        }
    }

    void update(const glm::vec3 &position)
    {
        _renderer->SetModelMatrix(glm::translate(glm::mat4(1.0), position));
        for (int i = 0; i < _renderObjects.size(); ++i)
        {
            ExtendedRenderObject *object = &_renderObjects[i];

            _renderer->Render(object->object._indices,
                              object->object._vertices,
                              object->object._normals,
                              object->object._uvCoords,
                              object->materialId);
        }
    }

private:
    IRenderer *_renderer;
    ObjImporter *_importer;
    std::vector<ExtendedRenderObject> _renderObjects;
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
    voxelRepository.AddVoxelType(VoxelType(0, 0));
    voxelRepository.AddVoxelType(VoxelType(0, 1));
    voxelRepository.AddVoxelType(VoxelType(1, 0));
    voxelRepository.AddVoxelType(VoxelType(1, 1));

    MouseTracker *mouseTracker = new MouseTracker(mouseState);
    Camera *camera = new Camera(adsRenderer);
    ticker.Start(17);

    ConditionHandler inputHandler;
    TestInputHandlerState testHandlerState(windowController,
                                           keyboardState, mouseState,
                                           mouseTracker,
                                           &inputHandler, simpleObject,
                                           camera);
    testHandlerState.Enter();

    std::vector<Entity*> entities;

    entities.push_back((Entity*)simpleObject);

    VoxelSector *sector = CreateVoxelSector(adsRenderer, &tileRenderer,
                                            &voxelRepository, Position(0, 0, 0));
    // sector->SetVoxel(Position(0, 0, 0), 0);
    // sector->SetVoxel(Position(1, 0, 0), 1);
    // sector->SetVoxel(Position(2, 0, 0), 1);
    // sector->SetVoxel(Position(3, 0, 0), 1);
    // sector->SetVoxel(Position(4, 0, 0), 1);
    // sector->SetVoxel(Position(5, 0, 0), 2);
    // sector->SetVoxel(Position(6, 0, 0), 3);

    // sector->Export("test.map");
    sector->Import("test.map");
    entities.push_back(sector);

    sector = CreateVoxelSector(adsRenderer, &tileRenderer,
                               &voxelRepository, Position(-1, 0, 0));
    sector->SetVoxel(Position(14, 0, 0), 0);
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
        inputHandler.Update();
        for (int i = 0; i < entities.size(); ++i)
        {
            entities[i]->update();
        }

        windowController->SwapBuffers();
        ticker.WaitUntilNextTick();
    }
}
