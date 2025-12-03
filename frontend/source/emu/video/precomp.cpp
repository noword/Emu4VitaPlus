#include "precomp.h"
#include "log.h"

#define SCE_GXM_PRECOMPUTED_ALIGNMENT 16
#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#endif

DrawDate::DrawDate(Shader *shader, const void *index_data, uint32_t index_count)
{
    LogFunctionName;
    const uint32_t draw_size = sceGxmGetPrecomputedDrawSize(shader->Get()->vertexProgram);
    data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[draw_size];
    int result = sceGxmPrecomputedDrawInit(draw, shader->Get()->vertexProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedDrawInit failed: %08x", result);
        return;
    }

    void *vertex_streams[] = {shader->Get()->vertexProgram};

    sceGxmPrecomputedDrawSetAllVertexStreams(draw, vertex_streams);
    sceGxmPrecomputedDrawSetParams(draw,
                                   SCE_GXM_PRIMITIVE_TRIANGLE_STRIP,
                                   SCE_GXM_INDEX_FORMAT_U16,
                                   index_data,
                                   index_count);
}

DrawDate::~DrawDate()
{
    delete[] data;
}

VertexState::VertexState(Shader *shader)
{
    const uint32_t state_size = sceGxmGetPrecomputedVertexStateSize(shader->Get()->vertexProgram);
    const uint32_t default_buffer_size = shader->GetVertexDefaultUniformBufferSize();
    const uint32_t total_size = ALIGN(state_size + default_buffer_size, SCE_GXM_PRECOMPUTED_ALIGNMENT);
    state = new SceGxmPrecomputedVertexState;
    data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[total_size];

    int result = sceGxmPrecomputedVertexStateInit(state, shader->Get()->vertexProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedVertexStateInit failed: %08x", result);
        return;
    }

    buf = data + state_size;
    sceGxmPrecomputedVertexStateSetDefaultUniformBuffer(state, buf);
}

VertexState::~VertexState()
{
    delete state;
    delete[] data;
}

FragmentState::FragmentState(Shader *shader)
{
    const uint32_t state_size = sceGxmGetPrecomputedFragmentStateSize(shader->Get()->fragmentProgram);
    const uint32_t default_buffer_size = shader->GetFragmentDefaultUniformBufferSize();
    const uint32_t total_size = ALIGN(state_size + default_buffer_size, SCE_GXM_PRECOMPUTED_ALIGNMENT);
    state = new SceGxmPrecomputedFragmentState;
    data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[total_size];

    int result = sceGxmPrecomputedFragmentStateInit(state, shader->Get()->fragmentProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedFragmentStateInit failed: %08x", result);
        return;
    }

    buf = data + state_size;
    sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer(state, buf);
}

FragmentState::~FragmentState()
{
    delete state;
    delete[] data;
}

Precomp::Precomp(Shader *shader)
{
    _draw_data = new DrawDate(shader, vita2d_get_linear_indices(), 4);
    _vertex_state = new VertexState(shader);
    _fragment_state = new FragmentState(shader);
}

Precomp::~Precomp()
{
    delete _draw_data;
    delete _vertex_state;
    delete _fragment_state;
}