#include <stdlib.h>
#include "shader.h"
#include "file.h"
#include "log.h"
#include "SimpleIni.h"

#define SHADER_PATH "app0:assets/shaders/"
#define SHADER_SECTION "SHADERS"

Shaders *gShaders;

bool Params::Init(const SceGxmProgram *program)
{
    texture_size = sceGxmProgramFindParameterByName(program, "IN.texture_size");
    video_size = sceGxmProgramFindParameterByName(program, "IN.video_size");
    output_size = sceGxmProgramFindParameterByName(program, "IN.output_size");
    LogDebug("  %08x %08x %08x", texture_size, video_size, output_size);
    return texture_size != NULL && video_size != NULL && output_size != NULL;
};

Shader::Shader(const char *name)
    : _name(name),
      _shader(nullptr),
      _vertex_buf(nullptr),
      _fragment_buf(nullptr)
{
    _Load();
}

Shader::~Shader()
{
    LogFunctionName;
    _Clean();
}

void Shader::_Clean()
{
    LogFunctionName;

    if (_shader)
    {
        vita2d_free_shader(_shader);
        _shader = nullptr;
    }

    if (_vertex_buf)
    {
        delete[] _vertex_buf;
        _vertex_buf = nullptr;
    }

    if (_fragment_buf)
    {
        delete[] _fragment_buf;
        _fragment_buf = nullptr;
    }
}

void Shader::_Load()
{
    _Clean();

    LogDebug("  try to load shader: %s", _name.c_str());
    std::string name = std::string(SHADER_PATH) + _name + "_v.gxp";
    if (File::ReadFile(name.c_str(), (void **)&_vertex_buf) == 0)
    {
        goto FAILED;
    }

    name = std::string(SHADER_PATH) + _name + "_f.gxp";
    if (File::ReadFile(name.c_str(), (void **)&_fragment_buf) == 0)
    {
        goto FAILED;
    }

    _shader = vita2d_create_shader((const SceGxmProgram *)_vertex_buf, (const SceGxmProgram *)_fragment_buf);
    if (_shader)
    {
        const SceGxmProgram *program = sceGxmVertexProgramGetProgram(_shader->vertexProgram);
        _vertex_parms.Init(program);

        program = sceGxmFragmentProgramGetProgram(_shader->fragmentProgram);
        _fragment_parms.Init(program);

        LogDebug("  shader %s loaded: %08x", _name.c_str(), _shader);
        return;
    }

FAILED:
    LogError("  failed to load shader: %s", _name.c_str());
    _Clean();
}

const char *Shader::GetName() const
{
    return _name.c_str();
}

uint32_t Shader::GetVertexDefaultUniformBufferSize()
{
    return sceGxmProgramGetDefaultUniformBufferSize((const SceGxmProgram *)_vertex_buf);
}

uint32_t Shader::GetFragmentDefaultUniformBufferSize()
{
    return sceGxmProgramGetDefaultUniformBufferSize((const SceGxmProgram *)_fragment_buf);
}

Shaders::Shaders()
{
    Load(SHADER_PATH "shaders.ini");
}

Shaders::~Shaders()
{
}

bool Shaders::Load(const char *path)
{
    LogFunctionName;

    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        LogError("failed to load %s", path);
        return false;
    }

    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(SHADER_SECTION, keys);
    this->reserve(keys.size());
    for (auto const &key : keys)
    {
        const char *value = ini.GetValue(SHADER_SECTION, key.pItem, "NULL");
        LogDebug("%s %s", key.pItem, value);
        this->emplace_back(value);
    }

    return true;
}

std::vector<LanguageString> Shaders::GetConfigs()
{
    std::vector<LanguageString> configs;
    configs.emplace_back(LanguageString(LANG_NONE));
    for (auto const &shader : *this)
    {
        configs.emplace_back(shader.GetName());
    }
    return configs;
}