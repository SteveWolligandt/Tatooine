#ifndef TATOOINE_GL_TEXTURE_H
#define TATOOINE_GL_TEXTURE_H

#include <tatooine/concepts.h>
#include <tatooine/gl/errorcheck.h>
#include <tatooine/gl/glfunctions.h>
#include <tatooine/gl/glwrapper.h>
#include <tatooine/gl/idholder.h>
#include <tatooine/gl/pixelunpackbuffer.h>
#include <tatooine/gl/shader.h>
#include <tatooine/gl/texcomponents.h>
#include <tatooine/gl/texpng.h>
#include <tatooine/gl/texsettings.h>
#include <tatooine/gl/textarget.h>
#include <tatooine/gl/type.h>

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <iostream>
#include <type_traits>
#include <utility>
//==============================================================================
namespace tatooine::gl {
//==============================================================================
template <typename T>
concept texture_value = arithmetic<T> || same_as<T, tex::depth24>;
//==============================================================================
enum WrapMode {
  CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
  CLAMP_TO_EDGE   = GL_CLAMP_TO_EDGE,
  REPEAT          = GL_REPEAT,
  MIRRORED_REPEAT = GL_MIRRORED_REPEAT
};
//==============================================================================
enum InterpolationMode {
  NEAREST                = GL_NEAREST,
  LINEAR                 = GL_LINEAR,
  NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
  LINEAR_MIPMAP_NEAREST  = GL_LINEAR_MIPMAP_NEAREST,
  NEAREST_MIPMAP_LINEAR  = GL_NEAREST_MIPMAP_LINEAR,
  LINEAR_MIPMAP_LINEAR   = GL_LINEAR_MIPMAP_LINEAR
};
//==============================================================================
enum CompareFunc {
  NEVER    = GL_NEVER,
  LESS     = GL_LESS,
  LEQUAL   = GL_LEQUAL,
  GREATER  = GL_GREATER,
  NOTEQUAL = GL_NOTEQUAL,
  GEQUAL   = GL_GEQUAL,
  ALWAYS   = GL_ALWAYS
};
//==============================================================================
enum CompareMode {
  COMPARE_R_TO_TEXTURE = GL_COMPARE_R_TO_TEXTURE,
  NONE                 = GL_NONE
};
//==============================================================================
template <unsigned int D, texture_value T, texture_component C>
class texture : public id_holder<GLuint> {
  static_assert(D >= 1 && D <= 3,
                "number of dimensions must be between 1 and 3");

 public:
  //============================================================================
  using type                                  = T;
  using components                            = C;
  static constexpr auto target                = tex::target_v<D>;
  static constexpr auto target_binding        = tex::target_binding<D>;
  static constexpr auto default_interpolation = LINEAR;
  static constexpr auto default_wrap_mode     = REPEAT;
  static constexpr auto num_components        = components::num_components;
  static constexpr auto num_dimensions        = D;

  static constexpr auto gl_internal_format =
      tex::settings<T, C>::internal_format;
  static constexpr auto gl_format = tex::settings<T, C>::format;
  static constexpr auto gl_type   = tex::settings<T, C>::type;
  static constexpr std::array<GLenum, 3> wrapmode_indices{
      GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R};

  static constexpr bool is_readable =
      (D == 2 && std::is_same_v<C, R>) || (D == 2 && std::is_same_v<C, RGB>) ||
      (D == 2 && std::is_same_v<C, RGBA>) ||
      (D == 2 && std::is_same_v<C, BGR>) || (D == 2 && std::is_same_v<C, BGRA>);
  static constexpr bool is_writable =
      (D == 2 && std::is_same_v<C, R>) || (D == 2 && std::is_same_v<C, RG>) ||
      (D == 2 && std::is_same_v<C, RGB>) ||
      (D == 2 && std::is_same_v<C, RGBA>) ||
      (D == 2 && std::is_same_v<C, BGR>) || (D == 2 && std::is_same_v<C, BGRA>);

 protected:
  //============================================================================
  std::array<size_t, D> m_size;

 public:
  //============================================================================
  texture() : texture{std::make_index_sequence<D>{}} {}
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template <size_t... Is>
  texture(std::index_sequence<Is...>) : m_size{((void)Is, 0)...} {
    create_id();
    set_wrap_mode(default_wrap_mode);
    set_interpolation_mode(default_interpolation);
  }
  //----------------------------------------------------------------------------
  /// TODO: copy wrap and interpolation modes
  texture(const texture& other) : texture{} { copy_data(other); }
  //----------------------------------------------------------------------------
  texture(texture&& other)
      : id_holder{std::move(other)}, m_size{std::move(other.m_size)} {}
  //----------------------------------------------------------------------------
  auto& operator=(const texture& other) {
    copy_data(other);
    return *this;
  }
  //----------------------------------------------------------------------------
  auto& operator=(texture&& other) {
    id_holder::operator=(std::move(other));
    m_size             = std::move(other.m_size);
    return *this;
  }
  //----------------------------------------------------------------------------
  ~texture() {
    if constexpr (D == 1) {
      if (bound_texture1d() == id()) {
        unbind();
      }
    } else if constexpr (D == 2) {
      if (bound_texture2d() == id()) {
        unbind();
      }
    } else if constexpr (D == 3) {
      if (bound_texture3d() == id()) {
        unbind();
      }
    }
    if (id()) {
      gl::delete_textures(1, &id_ref());
    }
  }
  //----------------------------------------------------------------------------
  template <typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(Sizes... sizes)
      : m_size{static_cast<size_t>(sizes)...} {
    create_id();
    set_wrap_mode(default_wrap_mode);
    set_interpolation_mode(default_interpolation);
    resize(sizes...);
  }
  //----------------------------------------------------------------------------
  template <typename S, typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(S const* const data, Sizes... sizes)
      : m_size{static_cast<size_t>(sizes)...} {
    static_assert(sizeof...(Sizes) == D,
                  "number of sizes does not match number of dimensions");
    static_assert((std::is_integral_v<Sizes> && ...),
                  "types of sizes must be integral types");
    upload_data(std::vector<T>(data, data + num_components * (sizes * ...)));
  }
  //----------------------------------------------------------------------------
  template <typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(T const* const data, Sizes... sizes)
      : m_size{static_cast<size_t>(sizes)...} {
    static_assert(sizeof...(Sizes) == D,
                  "number of sizes does not match number of dimensions");
    static_assert((std::is_integral_v<Sizes> && ...),
                  "types of sizes must be integral types");
    create_id();
    set_interpolation_mode(default_interpolation);
    set_wrap_mode(default_wrap_mode);
    m_size = std::array<size_t, D>{static_cast<size_t>(sizes)...};
    upload_data(data);
  }
  //----------------------------------------------------------------------------
  template <typename S, typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(const std::vector<S>& data, Sizes... sizes)
      : m_size{static_cast<size_t>(sizes)...} {
    static_assert(sizeof...(Sizes) == D,
                  "number of sizes does not match number of dimensions");
    static_assert((std::is_integral_v<Sizes> && ...),
                  "types of sizes must be integral types");
    create_id();
    set_interpolation_mode(default_interpolation);
    set_wrap_mode(default_wrap_mode);
    upload_data(data);
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  texture(InterpolationMode interp_mode, WrapMode wrap_mode,
          std::index_sequence<Is...>)
      : m_size{((void)Is, 0)...} {
    create_id();
    set_interpolation_mode(interp_mode);
    set_wrap_mode(wrap_mode);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  texture(InterpolationMode interp_mode, WrapMode wrap_mode)
      : texture{interp_mode, wrap_mode, std::make_index_sequence<D>{}} {}

  //----------------------------------------------------------------------------
  template <typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(InterpolationMode interp_mode, WrapMode wrap_mode,
                  Sizes... sizes)
      : m_size{sizes...} {
    static_assert(sizeof...(Sizes) == D,
                  "number of sizes does not match number of dimensions");
    static_assert((std::is_integral_v<Sizes> && ...),
                  "types of sizes must be integral types");
    create_id();
    set_interpolation_mode(interp_mode);
    set_wrap_mode(wrap_mode);
    resize(sizes...);
  }
  //----------------------------------------------------------------------------
  template <typename S, typename... Sizes>
  requires(sizeof...(Sizes) == D) &&
      (std::is_integral_v<typename std::decay_t<Sizes>> && ...)
          texture(InterpolationMode interp_mode, WrapMode wrap_mode,
                  const std::vector<S>& data, Sizes... sizes)
      : m_size{sizes...} {
    static_assert(sizeof...(Sizes) == D,
                  "number of sizes does not match number of dimensions");
    static_assert((std::is_integral_v<Sizes> && ...),
                  "types of sizes must be integral types");
    create_id();
    set_interpolation_mode(interp_mode);
    set_wrap_mode(wrap_mode);
    upload_data(data);
  }
  //----------------------------------------------------------------------------
  texture(const std::string& filepath) : texture{} { read(filepath); }
  //----------------------------------------------------------------------------
  texture(InterpolationMode interp_mode, WrapMode wrap_mode,
          const std::string& filepath)
      : texture{interp_mode, wrap_mode} {
    read(filepath);
  }

 private:
  //----------------------------------------------------------------------------
  void create_id() { gl::create_textures(target, 1, &id_ref()); }

 public:
  //----------------------------------------------------------------------------
  auto bind(GLuint unit = 0) const {
    assert(unit < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    gl::active_texture(GL_TEXTURE0 + unit);
    auto last_tex = bound_texture(target_binding);
    gl::bind_texture(target, id());
    return last_tex;
  }

  //----------------------------------------------------------------------------
  static void unbind(GLuint unit = 0) {
    gl::active_texture(GL_TEXTURE0 + unit);
    gl::bind_texture(target, 0);
  }

  //------------------------------------------------------------------------------
  void bind_image_texture(GLuint unit) const {
    gl::bind_image_texture(unit, id(), 0, GL_FALSE, 0, GL_READ_ONLY,
                           gl_internal_format);
  }

  //------------------------------------------------------------------------------
  void bind_image_texture(GLuint unit) {
    gl::bind_image_texture(unit, id(), 0, GL_FALSE, 0, GL_READ_WRITE,
                           gl_internal_format);
  }
  //------------------------------------------------------------------------------
  void bind_image_texture_read_write(GLuint unit) { bind_image_texture(unit); }

  //------------------------------------------------------------------------------
  void bind_image_texture_write(GLuint unit) {
    gl::bind_image_texture(unit, id(), 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           gl_internal_format);
  }

  //------------------------------------------------------------------------------
  void unbind_image_texture(GLuint unit) {
    gl::bind_image_texture(unit, 0, 0, GL_FALSE, 0, GL_READ_WRITE,
                           gl_internal_format);
  }

  //------------------------------------------------------------------------------

  void bind_image_texture_layer(GLuint unit, GLint layer) const
      requires(D == 3) {
    gl::bind_image_texture(unit, id(), 0, GL_TRUE, layer, GL_READ_ONLY,
                           gl_internal_format);
  }

  //------------------------------------------------------------------------------

  void bind_image_texture_layer(GLuint unit, GLint layer) requires(D == 3) {
    gl::bind_image_texture(unit, id(), 0, GL_TRUE, layer, GL_READ_WRITE,
                           gl_internal_format);
  }

  //------------------------------------------------------------------------------

  static void unbind_image_texture_layer(GLuint unit,
                                         GLint  layer) requires(D == 3) {
    gl::bind_image_texture(unit, 0, 0, GL_TRUE, layer, GL_READ_WRITE,
                           gl_internal_format);
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  size_t num_texels(std::index_sequence<Is...>) const {
    return (m_size[Is] * ...);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t num_texels() const {
    return num_texels(std::make_index_sequence<D>{});
  }

  //----------------------------------------------------------------------------
  void copy_data(const texture& other) {
    resize(other.m_size);
    if constexpr (D == 1) {
      gl::copy_image_sub_data(other.id(), target, 0, 0, 0, 0, id(), target, 0,
                              0, 0, 0, m_size[0], 1, 1);
    } else if (D == 2) {
      gl::copy_image_sub_data(other.id(), target, 0, 0, 0, 0, id(), target, 0,
                              0, 0, 0, m_size[0], m_size[1], 1);

    } else {
      gl::copy_image_sub_data(other.id(), target, 0, 0, 0, 0, id(), target, 0,
                              0, 0, 0, m_size[0], m_size[1], m_size[2]);
    }
  }
  //------------------------------------------------------------------------------
  template <typename Size>
  void resize(const std::array<Size, D>& size) {
    static_assert(std::is_integral_v<Size>);
    auto last_tex = bind();
    if constexpr (std::is_same_v<size_t, Size>) {
      m_size = size;
    } else {
      m_size = std::array<size_t, D>(begin(size), end(size));
    }
    if constexpr (D == 1) {
      gl::tex_image_1d(target, 0, gl_internal_format, width(), 0, gl_format,
                       gl_type, nullptr);
    } else if constexpr (D == 2) {
      gl::tex_image_2d(target, 0, gl_internal_format, width(), height(), 0,
                       gl_format, gl_type, nullptr);
    } else if constexpr (D == 3) {
      gl::tex_image_3d(target, 0, gl_internal_format, width(), height(),
                       depth(), 0, gl_format, gl_type, nullptr);
    }
    if (last_tex > 0) {
      gl::bind_texture(target, last_tex);
    }
  }
  //------------------------------------------------------------------------------
  template <typename... Sizes>
  void resize(Sizes... sizes) {
    static_assert(sizeof...(Sizes) == D);
    static_assert((std::is_integral_v<Sizes> && ...));
    resize(std::array<size_t, D>{static_cast<size_t>(sizes)...});
  }

 private:
  //------------------------------------------------------------------------------
  template <typename S>
  void upload_data(const std::vector<S>& data) {
    upload_data(std::vector<type>(begin(data), end(data)));
  }
  //------------------------------------------------------------------------------
  void upload_data(type const* const data) {
    auto last_tex = bind();
    if constexpr (D == 1) {
      gl::tex_image_1d(target, 0, gl_internal_format, width(), 0, gl_format,
                       gl_type, data);
    } else if constexpr (D == 2) {
      gl::tex_image_2d(target, 0, gl_internal_format, width(), height(), 0,
                       gl_format, gl_type, data);
    } else if constexpr (D == 3) {
      gl::tex_image_3d(target, 0, gl_internal_format, width(), height(),
                       depth(), 0, gl_format, gl_type, data);
    }
    if (last_tex > 0) {
      gl::bind_texture(target, last_tex);
    }
  }
  //------------------------------------------------------------------------------
  void upload_data(const std::vector<type>& data) {
    assert(data.size() == num_texels() * num_components);
    upload_data(data.data());
  }

 public:
  //------------------------------------------------------------------------------
  template <typename... Sizes>
  void upload_data(const std::vector<type>& data, Sizes... sizes) {
    static_assert(sizeof...(Sizes) == D);
    static_assert((std::is_integral_v<Sizes> && ...));
    m_size = std::array<size_t, D>{static_cast<size_t>(sizes)...};
    upload_data(data);
  }
  //------------------------------------------------------------------------------
  template <typename... Sizes>
  void upload_data(type const* const data, Sizes... sizes) {
    static_assert(sizeof...(Sizes) == D);
    static_assert((std::is_integral_v<Sizes> && ...));
    m_size = std::array<size_t, D>{static_cast<size_t>(sizes)...};
    upload_data(data);
  }
  //------------------------------------------------------------------------------
  template <typename S, typename... Sizes>
  void upload_data(const std::vector<S>& data, Sizes... sizes) {
    static_assert(sizeof...(Sizes) == D);
    static_assert((std::is_integral_v<Sizes> && ...));
    m_size = std::array<size_t, D>{static_cast<size_t>(sizes)...};
    upload_data(data);
  }
  //------------------------------------------------------------------------------
  template <typename S, typename... Sizes>
  void upload_data(const S* data, Sizes... sizes) {
    static_assert(sizeof...(Sizes) == D);
    static_assert((std::is_integral_v<Sizes> && ...));
    m_size = std::array<size_t, D>{static_cast<size_t>(sizes)...};
    upload_data(data);
  }
  //------------------------------------------------------------------------------
  auto download_data() const {
    std::vector<type> data(num_components * num_texels());
    gl::get_texture_image(id(), 0, gl_format, gl_type,
                          data.size() * sizeof(type), data.data());
    return data;
  }
  //------------------------------------------------------------------------------
  void download_data(std::vector<type>& data) const {
    const auto n = num_components * num_texels();
    if (data.size() != n) {
      data.resize(n);
    }
    gl::get_texture_image(id(), 0, gl_format, gl_type, n * sizeof(type),
                          data.data());
  }
  //------------------------------------------------------------------------------
  void download_data(type* data) const {
    gl::get_texture_image(id(), 0, gl_format, gl_type,
                          num_texels() * num_components * sizeof(type), data);
  }
  //------------------------------------------------------------------------------

  auto& download_sub_data(GLint xoffset, GLsizei width, std::vector<T>& data,
                          GLint level = 0) const requires(D == 1) {
    if (data.size() != width * num_components) {
      data.resize(width * num_components);
    }
    gl::get_texture_sub_image(id(), level, xoffset, 0, 0, width, 1, 1,
                              gl_format, gl_type, data.size() * sizeof(type),
                              data.data());
    return data;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  auto download_sub_data(GLint xoffset, GLsizei width, GLint level = 0) const
      requires(D == 1) {
    std::vector<T> data(width * num_components);
    gl::get_texture_sub_image(id(), level, xoffset, 0, 0, width, 1, 1,
                              gl_format, gl_type, data.size() * sizeof(type),
                              data.data());
    return data;
  }
  //------------------------------------------------------------------------------

  auto& download_sub_data(GLint xoffset, GLint yoffset, GLsizei width,
                          GLsizei height, std::vector<T>& data,
                          GLint level = 0) const requires(D == 2) {
    if (data.size() != width * height * num_components) {
      data.resize(width * height * num_components);
    }
    gl::get_texture_sub_image(id(), level, xoffset, yoffset, 0, width, height,
                              1, gl_format, gl_type, data.size() * sizeof(type),
                              data.data());
    return data;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  auto download_sub_data(GLint xoffset, GLint yoffset, GLsizei width,
                         GLsizei height, GLint level = 0) const
      requires(D == 2) {
    std::vector<T> data(width * height * num_components);
    gl::get_texture_sub_image(id(), level, xoffset, yoffset, 0, width, height,
                              1, gl_format, gl_type, data.size() * sizeof(type),
                              data.data());
    return data;
  }
  //------------------------------------------------------------------------------

  auto& download_sub_data(GLint xoffset, GLint yoffset, GLint zoffset,
                          GLsizei width, GLsizei height, GLsizei depth,
                          std::vector<T>& data, GLint level = 0) const
      requires(D == 3) {
    if (data.size() != width * height * depth * num_components) {
      data.resize(width * height * depth * num_components);
    }
    gl::get_texture_sub_image(id(), level, xoffset, yoffset, zoffset, width,
                              height, depth, gl_format, gl_type,
                              data.size() * sizeof(type), data.data());
    return data;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  auto download_sub_data(GLint xoffset, GLint yoffset, GLint zoffset,
                         GLsizei width, GLsizei height, GLsizei depth,
                         GLint level = 0) const requires(D == 3) {
    std::vector<T> data(width * height * depth * num_components);
    gl::get_texture_sub_image(id(), level, xoffset, yoffset, zoffset, width,
                              height, depth, gl_format, gl_type,
                              data.size() * sizeof(type), data.data());
    return data;
  }
  //----------------------------------------------------------------------------
  template <typename... Indices>
  requires(sizeof...(Indices) == D) &&
      (std::is_integral_v<Indices> && ...) auto operator()(
          Indices... indices) const {
    return download_sub_data(indices..., ((void)indices, 1)..., 0).front();
  }
  //----------------------------------------------------------------------------
  auto width() const { return m_size[0]; }
  //----------------------------------------------------------------------------
  auto height() const requires(D > 1) { return m_size[1]; }
  //----------------------------------------------------------------------------
  auto depth() const requires(D > 2) { return m_size[2]; }
  //----------------------------------------------------------------------------
  /// setting all wrapmodes to same mode
  void set_wrap_mode(WrapMode mode) {
    set_wrap_mode_s(mode);
    if constexpr (D > 1) {
      set_wrap_mode_t(mode);
    }
    if constexpr (D > 2) {
      set_wrap_mode_r(mode);
    }
  }

  //----------------------------------------------------------------------------
  /// setting all wrapmodes individual modes
  template <size_t... Is, typename... Modes>
  requires(std::is_same_v<Modes, WrapMode>&&...) void set_wrap_mode(
      std::index_sequence<Is...>, Modes... modes) {
    static_assert(sizeof...(Modes) == D);
    static_assert((std::is_same_v<Modes, WrapMode> && ...));
    using discard = int[];
    (void)discard{((void)set_wrap_mode(Is, modes), 0)...};
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// setting all wrapmodes individual modes
  template <typename... Modes>
  requires(std::is_same_v<Modes, WrapMode>&&...) &&
      (sizeof...(Modes) == D) void set_wrap_mode(Modes... modes) {
    set_wrap_mode(std::make_index_sequence<D>{}, modes...);
  }

  //----------------------------------------------------------------------------
  /// setting wrapmode with index i to modes
  void set_wrap_mode(size_t i, WrapMode mode) {
    gl::texture_parameter_i(id(), wrapmode_indices[i], mode);
  }

  //----------------------------------------------------------------------------
  /// setting all wrapmodes to repeat
  void set_repeat() { set_wrap_mode(REPEAT); }
  //----------------------------------------------------------------------------
  void set_wrap_mode_s(WrapMode mode) { set_wrap_mode(0, mode); }

  //----------------------------------------------------------------------------
  void set_wrap_mode_t(WrapMode mode) requires(D > 1) {
    set_wrap_mode(1, mode);
  }

  //----------------------------------------------------------------------------
  template <unsigned int D_ = D>
  requires(D > 2) void set_wrap_mode_r(WrapMode mode) {
    set_wrap_mode(2, mode);
  }

  //------------------------------------------------------------------------------
  void set_interpolation_mode(InterpolationMode mode) {
    set_interpolation_mode_min(mode);
    set_interpolation_mode_mag(mode);
  }

  //------------------------------------------------------------------------------
  void set_interpolation_mode_min(InterpolationMode mode) {
    gl::texture_parameter_i(id(), GL_TEXTURE_MIN_FILTER, mode);
  }

  //------------------------------------------------------------------------------
  void set_interpolation_mode_mag(InterpolationMode mode) {
    gl::texture_parameter_i(id(), GL_TEXTURE_MAG_FILTER, mode);
  }

  //----------------------------------------------------------------------------

  void set_compare_func(
      CompareFunc f) requires std::is_same_v<components, Depth> {
    gl::texture_parameter_i(id(), GL_TEXTURE_COMPARE_FUNC, f);
  }

  //----------------------------------------------------------------------------

  void set_compare_mode(
      CompareMode m) requires std::is_same_v<components, Depth> {
    gl::texture_parameter_i(id(), GL_TEXTURE_COMPARE_MODE, m);
  }

  //----------------------------------------------------------------------------
  template <typename... Components>
  requires(sizeof...(Components) == num_components) &&
      (std::is_arithmetic_v<Components> &&
       ...) void clear(Components... components) {
    clear(std::array<type, num_components>{static_cast<type>(components)...});
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void clear(const std::array<type, num_components>& col) {
    gl::clear_tex_image(id(), 0, gl_format, gl_type, col.data());
  }
  //------------------------------------------------------------------------------

  void set_data(const pixelunpackbuffer<type>& pbo) requires(D == 2) {
    pbo.bind();
    auto last_tex = bind();
    gl::tex_sub_image_2d(GL_TEXTURE_2D, 0, 0, 0, width(), height(), gl_format,
                         gl_type, 0);
    if (last_tex > 0) {
      gl::bind_texture(target, last_tex);
    }
  }
  //------------------------------------------------------------------------------
  void read(const std::string filepath) {
    auto ext = filepath.substr(filepath.find_last_of(".") + 1);
    if constexpr (D == 2 && is_readable) {
      if (ext == "png") {
        read_png(filepath);
        return;
      }
    }

    throw std::runtime_error("could not read fileformat ." + ext);
  }

  //------------------------------------------------------------------------------
  void write(const std::string filepath) const {
    auto ext = filepath.substr(filepath.find_last_of('.') + 1);
    if constexpr (D == 2 && is_writable) {
      if constexpr (has_png_support()) {
        if (ext == "png") {
          write_png(filepath);
          return;
        }
      }
    }
    throw std::runtime_error("could not write fileformat ." + ext);
  }
  //----------------------------------------------------------------------------

  void read_png(const std::string& filepath) requires(has_png_support()) &&
      (D == 2) && is_readable {
    using tex_png_t = tex_png<type, components>;
    typename tex_png_t::png_t image;
    image.read(filepath);
    m_size[0] = image.get_width();
    m_size[1] = image.get_height();
    std::vector<type> data;
    data.reserve(num_texels() * num_components);
    for (png::uint_32 y = 0; y < height(); ++y) {
      for (png::uint_32 x = 0; x < width(); ++x) {
        tex_png_t::load_pixel(data, image, x, y);
      }
    }
    if constexpr (std::is_same_v<type, float>) {
      auto normalize = [](auto d) { return d / 255.0f; };
      boost::transform(data, begin(data), normalize);
    }

    upload_data(data);
  }
  //------------------------------------------------------------------------------
  void write_png(const std::string& filepath) const
      requires(has_png_support()) &&
      (D == 2) && is_writable {
    using tex_png_t = tex_png<type, components>;
    typename tex_png_t::png_t image(width(), height());
    auto                      data = download_data();

    for (unsigned int y = 0; y < image.get_height(); ++y)
      for (png::uint_32 x = 0; x < image.get_width(); ++x) {
        unsigned int idx = x + width() * y;
        tex_png_t::save_pixel(data, image, x, y, idx);
      }
    image.write(filepath);
  }
};
//==============================================================================

template <texture_value T, texture_component C>
using tex1 = texture<1, T, C>;
template <texture_value T, texture_component C>
using tex2 = texture<2, T, C>;
template <texture_value T, texture_component C>
using tex3 = texture<3, T, C>;

template <texture_value T>
using tex1r = tex1<T, R>;
template <texture_value T>
using tex1rg = tex1<T, RG>;
template <texture_value T>
using tex1rgb = tex1<T, RGB>;
template <texture_value T>
using tex1rgba = tex1<T, RGBA>;
template <texture_value T>
using tex1bgr = tex1<T, BGR>;
template <texture_value T>
using tex1bgra = tex1<T, BGRA>;
template <texture_value T>
using tex2r = tex2<T, R>;
template <texture_value T>
using tex2rg = tex2<T, RG>;
template <texture_value T>
using tex2rgb = tex2<T, RGB>;
template <texture_value T>
using tex2rgba = tex2<T, RGBA>;
template <texture_value T>
using tex2bgr = tex2<T, BGR>;
template <texture_value T>
using tex2bgra = tex2<T, BGRA>;
template <texture_value T>
using tex2depth = tex2<T, Depth>;
template <texture_value T>
using tex3r = tex3<T, R>;
template <texture_value T>
using tex3rg = tex3<T, RG>;
template <texture_value T>
using tex3rgb = tex3<T, RGB>;
template <texture_value T>
using tex3rgba = tex3<T, RGBA>;
template <texture_value T>
using tex3bgr = tex3<T, BGR>;
template <texture_value T>
using tex3bgra = tex3<T, BGRA>;

using tex1r8ui  = tex1r<GLubyte>;
using tex1r16ui = tex1r<GLushort>;
using tex1r32ui = tex1r<GLuint>;
using tex1r8i   = tex1r<GLbyte>;
using tex1r16i  = tex1r<GLshort>;
using tex1r32i  = tex1r<GLint>;
using tex1r16f  = tex1r<GLhalf>;
using tex1r32f  = tex1r<GLfloat>;

using tex1rg8ui  = tex1rg<GLubyte>;
using tex1rg16ui = tex1rg<GLushort>;
using tex1rg32ui = tex1rg<GLuint>;
using tex1rg8i   = tex1rg<GLbyte>;
using tex1rg16i  = tex1rg<GLshort>;
using tex1rg32i  = tex1rg<GLint>;
using tex1rg16f  = tex1rg<GLhalf>;
using tex1rg32f  = tex1rg<GLfloat>;

using tex1rgb8ui  = tex1rgb<GLubyte>;
using tex1rgb16ui = tex1rgb<GLushort>;
using tex1rgb32ui = tex1rgb<GLuint>;
using tex1rgb8i   = tex1rgb<GLbyte>;
using tex1rgb16i  = tex1rgb<GLshort>;
using tex1rgb32i  = tex1rgb<GLint>;
using tex1rgb16f  = tex1rgb<GLhalf>;
using tex1rgb32f  = tex1rgb<GLfloat>;

using tex1rgba8ui  = tex1rgba<GLubyte>;
using tex1rgba16ui = tex1rgba<GLushort>;
using tex1rgba32ui = tex1rgba<GLuint>;
using tex1rgba8i   = tex1rgba<GLbyte>;
using tex1rgba16i  = tex1rgba<GLshort>;
using tex1rgba32i  = tex1rgba<GLint>;
using tex1rgba16f  = tex1rgba<GLhalf>;
using tex1rgba32f  = tex1rgba<GLfloat>;

using tex2r8ui  = tex2r<GLubyte>;
using tex2r16ui = tex2r<GLushort>;
using tex2r32ui = tex2r<GLuint>;
using tex2r8i   = tex2r<GLbyte>;
using tex2r16i  = tex2r<GLshort>;
using tex2r32i  = tex2r<GLint>;
using tex2r16f  = tex2r<GLhalf>;
using tex2r32f  = tex2r<GLfloat>;

using tex2rg8ui  = tex2rg<GLubyte>;
using tex2rg16ui = tex2rg<GLushort>;
using tex2rg32ui = tex2rg<GLuint>;
using tex2rg8i   = tex2rg<GLbyte>;
using tex2rg16i  = tex2rg<GLshort>;
using tex2rg32i  = tex2rg<GLint>;
using tex2rg16f  = tex2rg<GLhalf>;
using tex2rg32f  = tex2rg<GLfloat>;

using tex2rgb8ui  = tex2rgb<GLubyte>;
using tex2rgb16ui = tex2rgb<GLushort>;
using tex2rgb32ui = tex2rgb<GLuint>;
using tex2rgb8i   = tex2rgb<GLbyte>;
using tex2rgb16i  = tex2rgb<GLshort>;
using tex2rgb32i  = tex2rgb<GLint>;
using tex2rgb16f  = tex2rgb<GLhalf>;
using tex2rgb32f  = tex2rgb<GLfloat>;

using tex2rgba8ui  = tex2rgba<GLubyte>;
using tex2rgba16ui = tex2rgba<GLushort>;
using tex2rgba32ui = tex2rgba<GLuint>;
using tex2rgba8i   = tex2rgba<GLbyte>;
using tex2rgba16i  = tex2rgba<GLshort>;
using tex2rgba32i  = tex2rgba<GLint>;
using tex2rgba16f  = tex2rgba<GLhalf>;
using tex2rgba32f  = tex2rgba<GLfloat>;

using texdepth16ui = tex2depth<GLushort>;
using texdepth24ui = tex2depth<tex::depth24>;
using texdepth32ui = tex2depth<GLuint>;
using texdepth32f  = tex2depth<GLfloat>;

template <texture_value T, texture_component C>
struct texture_format_qualifier_impl;
template <>
struct texture_format_qualifier_impl<float, R> {
  static constexpr std::string_view value = "r32f";
};
template <>
struct texture_format_qualifier_impl<float, RG> {
  static constexpr std::string_view value = "rg32f";
};
template <>
struct texture_format_qualifier_impl<float, RGB> {
  static constexpr std::string_view value = "rgba32f";
};
template <>
struct texture_format_qualifier_impl<float, RGBA> {
  static constexpr std::string_view value = "rgba32f";
};
template <>
struct texture_format_qualifier_impl<std::uint8_t, RGBA> {
  static constexpr std::string_view value = "rgba8";
};
template <texture_value T, texture_component C>
static auto constexpr texture_format_qualifier =
    texture_format_qualifier_impl<T, C>::value;
//------------------------------------------------------------------------------
template <either_of<R, RG, RGBA> C>
auto to_2d(tex1<float, C>const & t1, std::size_t const height,
           std::size_t const local_size_x = 32,
           std::size_t const local_size_y = 32) {
  auto t2 = tex2<float, C>{t1.width(), height};
  auto s  = shader{};
  auto ss = std::stringstream{};
  ss << "#version 430\n"
     << "uniform int width;\n"
     << "uniform int height;\n"
     << "layout(local_size_x = " << local_size_x
     << ", local_size_y = " << local_size_y << ") in;\n"
     << "layout("
     << texture_format_qualifier<float,
                                 C> << ", binding = 0) uniform image1D t1;\n"
     << "layout("
     << texture_format_qualifier<float,
                                 C> << ", binding = 1) uniform image2D t2;\n"
     << "void main() {\n"
     << "  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);\n"
     << "  if(pixel_coords.x < width && pixel_coords.y < height) {\n"
     << "    imageStore(t2, pixel_coords, imageLoad(t1, pixel_coords.x));\n"
     << "  }\n"
     << "}\n";
  std::cout << ss.str();
  s.add_stage<computeshader>(gl::shadersource{ss.str()});
  s.create();
  s.bind();
  s.set_uniform("width", static_cast<int>(t1.width()));
  s.set_uniform("height", static_cast<int>(height));
  t1.bind_image_texture(0);
  t2.bind_image_texture(1);
  dispatch_compute(
      static_cast<int>(
          std::ceil(t1.width() / static_cast<double>(local_size_x))),
      static_cast<int>(std::ceil(height / static_cast<double>(local_size_y))),
      1);
  return t2;
}
//==============================================================================
}  // namespace tatooine::gl
//==============================================================================
#endif
