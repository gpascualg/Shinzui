/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/intrusive.hpp"
#include <boost/intrusive_ptr.hpp>
#include <glm/glm.hpp>


class Bezier;
class MapAwareEntity;
class Packet;

class MovementGenerator
{
public:
    virtual ~MovementGenerator();

    virtual boost::intrusive_ptr<Packet> packet() = 0;
    virtual glm::vec3 update(MapAwareEntity* owner, float elapsed) = 0;
    virtual bool hasNext() = 0;
};


class RandomMovement : public MovementGenerator
{
public:
    RandomMovement();
    virtual ~RandomMovement();

    boost::intrusive_ptr<Packet> packet() override;
    glm::vec3 update(MapAwareEntity* owner, float elapsed) override;
    bool hasNext() override;

private:
    bool _hasPoint;

    Bezier* _bezier;

    float _t;
    glm::vec2 _previous;
};

class Bezier
{
public:
    Bezier(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) :
        _a(a), _b(b), _c(c), _d(d)
    {}

    virtual ~Bezier();

    virtual glm::vec2 next(float t) = 0;
    virtual float increase(float t, float speed) = 0;

    inline glm::vec2 start() { return _a; }
    inline glm::vec2 startOffset() { return _b; }
    inline glm::vec2 endOffset() { return _c; }
    inline glm::vec2 end() { return _d; }

protected:
    glm::vec2 calc(float t)
    {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        glm::vec2 p = uuu * _a;     // first term
        p += 3 * uu * t * _b;       // second term
        p += 3 * u * tt * _c;       // third term
        p += ttt * _d;              // fourth term

        return p;
    }

protected:
    glm::vec2 _a;
    glm::vec2 _b;
    glm::vec2 _c;
    glm::vec2 _d;
};


template <int len>
class SegmentedBezier : public Bezier
{
public:
    SegmentedBezier(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) :
        Bezier(a, b, c, d),
        _length(0)
    {
        _segments[0] = 0;

        glm::vec2 origin = calc(0);
        for (int i = 1; i <= len; i += 1)
        {
            glm::vec2 p = calc(i * (1.0f / len));
            _length += glm::distance(p, origin);
            _segments[i] = _length;
            origin = p;
        }
    }

    float increase(float t, float speed)
    {
        return speed / _length;
    }

    glm::vec2 next(float t)
    {
        return calc(map(t));
    }

protected:
    float map(float t)
    {
        float targetLength = t * _segments[len];
        int low = 0;
        int high = len;
        int index = 0;

        while (low < high)
        {
            index = low + (high - low) / 2;
            if (_segments[index] < targetLength)
            {
                low = index + 1;
            }
            else
            {
                high = index;
            }
        }

        if (_segments[index] > targetLength)
        {
            index--;
        }

        float lengthBefore = _segments[index];
        if (lengthBefore == targetLength)
        {
            return index / static_cast<float>(len);
        }


        return (index + (targetLength - lengthBefore) /
            static_cast<float>(_segments[index + 1] - lengthBefore)) / static_cast<float>(len);
    }

protected:
    float _segments[len + 1];
    float _length;
};


class DerivativeBezier : public Bezier
{
public:
    DerivativeBezier(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) :
        Bezier(a, b, c, d)
    {
        _dv1 = -3.0f * a + 9.0f * b - 9.0f * c + 3.0f * d;
        _dv2 = 6.0f * a - 12.0f * b + 6.0f * c;
        _dv3 = -3.0f * a + 3.0f * b;
    }

    float increase(float t, float speed)
    {
        return speed / glm::length(t * t * _dv1 + t * _dv2 + _dv3);
    }

    glm::vec2 next(float t)
    {
        return calc(t);
    }

protected:
    glm::vec2 _dv1;
    glm::vec2 _dv2;
    glm::vec2 _dv3;
};
