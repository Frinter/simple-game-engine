#pragma once

#include "camera.hh"
#include "command.hh"
#include "mousetracker.hh"

class ZoomFromMouseCommand : public Command
{
public:
    ZoomFromMouseCommand(Camera *camera, MouseTracker *tracker)
        : _camera(camera), _tracker(tracker)
    {
    }

    void Execute()
    {
        _camera->Zoom(-0.1f * _tracker->GetScrollDelta());
    }

private:
    Camera *_camera;
    MouseTracker *_tracker;
};
