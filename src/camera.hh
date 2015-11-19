#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "constants.hh"
#include "moveable.hh"
#include "rendering/adsrenderer.hh"

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

    glm::vec4 WorldToCamera(const glm::vec4 &vec)
    {
        return _projectionMatrix * _viewMatrix * vec;
    }

    glm::vec4 CameraToWorld(const glm::vec4 &vec)
    {
        return glm::inverse(_projectionMatrix * _viewMatrix) * vec;
    }

private:
    ADSRenderer *_renderer;

    glm::vec3 _position;
    float _rotation;
    float _zoom;
    float _orthoParamX;
    float _orthoParamY;

    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;

private:
    void setPosition()
    {
        glm::vec3 adjustedPosition = _zoom * _position;
        glm::vec3 rotatedPosition = glm::rotate(adjustedPosition,
                                                _rotation,
                                                glm::vec3( 0.0, 1.0, 0.0));
        _viewMatrix = glm::lookAt(rotatedPosition,
                                  glm::vec3( 0.0, 0.0, 0.0),
                                  glm::vec3( 0.0, 1.0, 0.0));

        _renderer->SetViewMatrix(_viewMatrix);
    }

    void setProjectionMatrix()
    {
        float xParam = _zoom * _orthoParamX;
        float yParam = _zoom * _orthoParamY;

        _projectionMatrix = glm::ortho(-xParam, xParam,
                                       -yParam, yParam,
                                       1.0f, 100.0f);

        _renderer->SetProjectionMatrix(_projectionMatrix);
    }
};
