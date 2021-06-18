#ifndef TATOOINE_RENDERING_GL_FRAGMENTSHADER_H
#define TATOOINE_RENDERING_GL_FRAGMENTSHADER_H
//==============================================================================
#include <string>
#include "glincludes.h"

#include "shaderstage.h"
#include "dllexport.h"
//==============================================================================
namespace tatooine::rendering::gl {
//==============================================================================
class fragmentshader : public shaderstage {
 public:
  DLL_API fragmentshader(std::filesystem::path const& sourcepath);
  DLL_API fragmentshader(shadersource const& sourcepath);
  DLL_API fragmentshader(fragmentshader&& other);
};
//==============================================================================
}  // namespace tatooine::rendering::gl
//==============================================================================
#endif
