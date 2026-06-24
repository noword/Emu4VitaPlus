#pragma once
#include <atomic>
#include <stdint.h>
#include <vector>
#include <psp2/touch.h>
#include "rect.h"
#include "locker.h"

namespace Emu4VitaPlus
{
    struct TouchAxis
    {
        int16_t x;
        int16_t y;

        bool operator==(const TouchAxis &axis) const { return x == axis.x && y == axis.y; };
        bool operator!=(const TouchAxis &axis) const { return !(*this == axis); };
        TouchAxis &operator=(const TouchAxis &axis)
        {
            x = axis.x;
            y = axis.y;
            return *this;
        };
    };

    struct TouchPoint : public TouchAxis
    {
        uint8_t id;
    };

    struct TouchState
    {
        uint8_t count;
        TouchPoint points[6];

        void Set(const SceTouchReport *report)
        {
            for (auto i = 0; i < count; i++)
            {
                if (points[i].id == report->id)
                {
                    points[i].x = report->x >> 1;
                    points[i].y = report->y >> 1;
                    return;
                }
            }

            points[count].id = report->id;
            points[count].x = report->x >> 1;
            points[count].y = report->y >> 1;
            count++;
        };

        void Reset() { count = 0; };
    };

    class Touch
    {
    public:
        Touch(SceTouchPortType port);
        virtual ~Touch();

        void Enable(bool enable);
        bool IsEnabled() const { return _enabled; };
        void Poll();
        bool IsTouched() const { return _touched; };
        const TouchState *Get() const { return _states + _current.load(std::memory_order_acquire); };
        const TouchAxis &GetCenter() const { return _center; };
        const SceTouchPanelInfo &GetInfo() const { return _info[_port]; };
        void InitMovingScale(float xscale, float yscale);
        const int16_t GetRelativeMovingX()
        {
            int current = _current.load(std::memory_order_acquire);
            int last = current ^ 1;
            return _GetRelativeMoving(&_scale_map_table_x, _states[last].points[0].x - _states[current].points[0].x);
        };

        const int16_t GetRelativeMovingY()
        {
            int current = _current.load(std::memory_order_acquire);
            int last = current ^ 1;
            return _GetRelativeMoving(&_scale_map_table_x, _states[last].points[0].y - _states[current].points[0].y);
        };

        template <typename T>
        void InitMapTable(const Rect<T> &rect)
        {
            T half_width = rect.width / 2;
            T half_height = rect.height / 2;
            float xscale = 32767.f / half_width; // 0x7fff == 32767
            float yscale = 32767.f / half_height;

            _map_table_x.clear();
            _map_table_x.reserve(rect.width);
            for (T x = 0; x < rect.width; x++)
            {
                _map_table_x.emplace_back((x - half_width) * xscale);
            }

            _map_table_y.clear();
            _map_table_y.reserve(rect.height);
            for (T y = 0; y < rect.height; y++)
            {
                _map_table_y.emplace_back((y - half_height) * yscale);
            }
        }

        template <typename T>
        int16_t GetMapedX(const Rect<T> &rect)
        {
            auto point = &Get()->points[0];
            size_t x = point->x - rect.left;
            int16_t mapx = rect.Contains(point->x, point->y) && x < _map_table_x.size() ? _map_table_x[x] : -0x8000;
            return mapx;
        }

        template <typename T>
        int16_t GetMapedY(const Rect<T> &rect)
        {
            auto point = &Get()->points[0];
            size_t y = point->y - rect.top;
            int16_t mapy = rect.Contains(point->x, point->y) && y < _map_table_y.size() ? _map_table_y[y] : -0x8000;
            return mapy;
        }

    private:
        const inline int16_t _GetRelativeMoving(std::vector<float> *table, int v)
        {
            int16_t result = 0;
            if (v != 0)
            {
                if (v > 0 && v < _scale_map_table_x.size())
                {
                    result = _scale_map_table_x[v];
                }
                else
                {
                    v = -v;
                    if (v < _scale_map_table_x.size())
                    {
                        result = -_scale_map_table_x[v];
                    }
                }
            }
            return result;
        }

        bool _enabled;
        static SceTouchPanelInfo _info[2];
        TouchState _states[2];
        TouchAxis _center;
        SceTouchPortType _port;
        float _x_scale;
        float _y_scale;

        std::atomic<int> _current{0};

        // map to retro's coordinate system
        // -0x7fff to 0x7fff
        std::vector<int16_t> _map_table_x;
        std::vector<int16_t> _map_table_y;

        // map to retro's mouse moving
        std::vector<float> _scale_map_table_x;
        std::vector<float> _scale_map_table_y;

        bool _touched;
    };
}