#pragma once
#include <vita2d.h>
#include <stdint.h>
#include "shader.h"

struct DrawDate
{
    uint8_t *data;
    SceGxmPrecomputedDraw *draw;
    DrawDate(Shader *shader, const void *index_data, uint32_t index_count);
    virtual ~DrawDate();
};

struct VertexState
{
    uint8_t *data;
    uint8_t *buf;
    SceGxmPrecomputedVertexState *state;
    VertexState(Shader *shader);
    virtual ~VertexState();
};

struct FragmentState
{
    uint8_t *data;
    uint8_t *buf;
    SceGxmPrecomputedFragmentState *state;
    FragmentState(Shader *shader);
    virtual ~FragmentState();
};

class Precomp
{
public:
    Precomp(Shader *shader);
    virtual ~Precomp();

private:
    DrawDate *_draw_data;
    VertexState *_vertex_state;
    FragmentState *_fragment_state;
};