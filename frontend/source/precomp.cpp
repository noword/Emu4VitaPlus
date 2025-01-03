#include "precomp.h"
#include "log.h"

#define SCE_GXM_PRECOMPUTED_ALIGNMENT 16
#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#endif

extern "C" void *gpu_alloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID *uid);
extern "C" void gpu_free(SceUID uid);

DrawDate::DrawDate(Shader *shader, const void *index_data, uint32_t index_count)
{
    LogFunctionName;
    const uint32_t draw_size = sceGxmGetPrecomputedDrawSize(shader->Get()->vertexProgram);
    // data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[draw_size];
    data = (uint8_t *)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, draw_size, SCE_GXM_PRECOMPUTED_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ, &uid);

    int result = sceGxmPrecomputedDrawInit(&draw, shader->Get()->vertexProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedDrawInit failed: %08x", result);
        return;
    }

    const void *const vertex_streams[] = {index_data};
    sceGxmPrecomputedDrawSetAllVertexStreams(&draw, vertex_streams);
    sceGxmPrecomputedDrawSetParams(&draw,
                                   SCE_GXM_PRIMITIVE_TRIANGLE_STRIP,
                                   SCE_GXM_INDEX_FORMAT_U16,
                                   index_data,
                                   index_count);
}

DrawDate::~DrawDate()
{
    gpu_free(uid);
}

VertexState::VertexState(Shader *shader)
{
    LogFunctionName;
    const uint32_t state_size = sceGxmGetPrecomputedVertexStateSize(shader->Get()->vertexProgram);
    const uint32_t default_buffer_size = shader->GetVertexDefaultUniformBufferSize();
    const uint32_t total_size = ALIGN(state_size + default_buffer_size, SCE_GXM_PRECOMPUTED_ALIGNMENT);
    // data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[total_size];
    data = (uint8_t *)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, total_size, SCE_GXM_PRECOMPUTED_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ, &uid);

    int result = sceGxmPrecomputedVertexStateInit(&state, shader->Get()->vertexProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedVertexStateInit failed: %08x", result);
        return;
    }

    buf = data + state_size;
    sceGxmPrecomputedVertexStateSetDefaultUniformBuffer(&state, buf);
}

VertexState::~VertexState()
{
    LogFunctionName;
    gpu_free(uid);
}

FragmentState::FragmentState(Shader *shader)
{
    LogFunctionName;
    const uint32_t state_size = sceGxmGetPrecomputedFragmentStateSize(shader->Get()->fragmentProgram);
    const uint32_t default_buffer_size = shader->GetFragmentDefaultUniformBufferSize();
    const uint32_t total_size = ALIGN(state_size + default_buffer_size, SCE_GXM_PRECOMPUTED_ALIGNMENT);
    // data = new (std::align_val_t(SCE_GXM_PRECOMPUTED_ALIGNMENT)) uint8_t[total_size];
    data = (uint8_t *)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, total_size, SCE_GXM_PRECOMPUTED_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ, &uid);

    int result = sceGxmPrecomputedFragmentStateInit(&state, shader->Get()->fragmentProgram, data);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedFragmentStateInit failed: %08x", result);
        return;
    }

    buf = data + state_size;
    sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer(&state, buf);
    LogDebug("%08x %08x", state_size, default_buffer_size);
}

FragmentState::~FragmentState()
{
    LogFunctionName;
    gpu_free(uid);
}

Precomp::Precomp(Shader *shader, SceGxmTexture *texture, const void *index_data, uint32_t index_count)
{
    LogFunctionName;
    draw_data = new DrawDate(shader, index_data, index_count);
    vertex_state = new VertexState(shader);
    fragment_state = new FragmentState(shader);

    int result = sceGxmPrecomputedFragmentStateSetTexture(&fragment_state->state, 0, texture);
    if (result != SCE_OK)
    {
        LogError("sceGxmPrecomputedFragmentStateSetTexture failed: %08x", result);
    }
}

Precomp::~Precomp()
{
    LogFunctionName;
    delete draw_data;
    delete vertex_state;
    delete fragment_state;
}