#pragma once
#include <vita2d.h>
#include <stdint.h>
#include "shader.h"

struct DrawDate
{
    uint8_t *data;
    SceGxmPrecomputedDraw draw;
    SceUID uid;
    DrawDate(Shader *shader, const void *index_data, uint32_t index_count);
    virtual ~DrawDate();
};

struct VertexState
{
    uint8_t *data;
    uint8_t *buf;
    SceUID uid;
    SceGxmPrecomputedVertexState state;
    VertexState(Shader *shader);
    virtual ~VertexState();
};

struct FragmentState
{
    uint8_t *data;
    uint8_t *buf;
    SceUID uid;
    SceGxmPrecomputedFragmentState state;
    FragmentState(Shader *shader);
    virtual ~FragmentState();
};

struct Precomp
{
    DrawDate *draw_data;
    VertexState *vertex_state;
    FragmentState *fragment_state;
    SceGxmNotification notification;

    Precomp(Shader *shader, SceGxmTexture *texture, const void *index_data, uint32_t index_count);
    virtual ~Precomp();
};