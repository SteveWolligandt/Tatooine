#ifndef TATOOINE_VTK_LEGACY_H
#define TATOOINE_VTK_LEGACY_H

#include <stdlib.h>
#include <cassert>
#include <exception>
#include <fstream>
#include <future>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <vector>
#include "string_conversion.h"
#include "swap_endianess.h"
#include "type_to_str.h"

//=============================================================================
namespace tatooine::vtk {
//=============================================================================

//! reads until terminator_char was found. buffer will containg the
//! terminator_char
inline auto read_until(std::istream &stream, const char terminator_char,
                       char *buffer) {
  size_t idx = 0;
  do {
    if (!stream.eof()) stream.read(&buffer[idx], sizeof(char));
    idx++;
  } while (buffer[idx - 1] != terminator_char && !stream.eof());
  buffer[idx] = '\0';
  return std::string(buffer);
}

//-----------------------------------------------------------------------------

//! reads stream until a linebreak was found. buffer will not contain the break
inline auto read_binaryline(std::istream &stream, char *buffer) {
  auto str = read_until(stream, '\n', buffer);
  return str.erase(str.size() - 1, 1);
}

//-----------------------------------------------------------------------------

//! reads stream until a whitespace was found and consumes any whitespace until
//! another character was found. buffer will not contain any whitespace
inline auto read_word(std::istream &stream, char *buffer) {
  size_t idx = 0;
  do {
    if (!stream.eof()) stream.read(&buffer[idx], sizeof(char));

    idx++;
  } while ((buffer[idx - 1] != ' ') && (buffer[idx - 1] != '\n') &&
           (buffer[idx - 1] != '\t') && !stream.eof());
  buffer[idx - 1] = '\0';
  std::string word(buffer);

  do {
    if (!stream.eof()) stream.read(buffer, sizeof(char));
  } while ((buffer[0] == ' ' || buffer[0] == '\n' || buffer[0] == '\t') &&
           !stream.eof());
  if (!stream.eof())
    stream.seekg(stream.tellg() - std::streampos(1), stream.beg);
  return word;
}

//-----------------------------------------------------------------------------

inline void write_binary(std::ostream &stream, const std::string &str) {
  stream.write(str.c_str(), long(sizeof(char)) * long(str.size()));
}

//-----------------------------------------------------------------------------

inline void write_binary(std::ostream &stream, const char c) {
  stream.write(&c, long(sizeof(char)));
}

enum Format { UNKNOWN_FORMAT, ASCII, BINARY };

enum ReaderData { POINT_DATA, CELL_DATA, UNSPECIFIED_DATA };

enum DatasetType {
  UNKNOWN_TYPE,
  STRUCTURED_POINTS,
  STRUCTURED_GRID,
  UNSTRUCTURED_GRID,
  POLYDATA,
  RECTILINEAR_GRID,
  FIELD
};

enum DataType {
  UNKNOWN_DATA_TYPE,
  UNSIGNED_CHAR,
  CHAR,
  UNSIGNED_SHORT,
  SHORT,
  UNSIGNED_INT,
  INT,
  UNSIGNED_LONG,
  LONG,
  FLOAT,
  DOUBLE
};

enum CellType {
  UNKNOWN_CELL_TYPE = 0,
  VERTEX            = 1,
  POLY_VERTEX       = 2,
  LINE              = 3,
  POLY_LINE         = 4,
  TRIANGLE          = 5,
  TRIANGLE_STRIP    = 6,
  POLYGON           = 7,
  PIXEL             = 8,
  QUAD              = 9,
  TETRA             = 10,
  VOXEL             = 11,
  HEXAHEDRON        = 12,
  WEDGE             = 13,
  PYRAMID           = 14
};

inline std::string type_to_str(DatasetType type) {
  switch (type) {
    default:
    case UNKNOWN_TYPE: return "UNKNOWN_TYPE";
    case STRUCTURED_POINTS: return "STRUCTURED_POINTS";
    case STRUCTURED_GRID: return "STRUCTURED_GRID";
    case UNSTRUCTURED_GRID: return "UNSTRUCTURED_GRID";
    case POLYDATA: return "POLYDATA";
    case RECTILINEAR_GRID: return "RECTILINEAR_GRID";
    case FIELD: return "FIELD";
  }
}

//-----------------------------------------------------------------------------

inline DatasetType str_to_type(const std::string &type) {
  if (type == "STRUCTURED_POINTS" || type == "structured_points")
    return STRUCTURED_POINTS;
  else if (type == "STRUCTURED_GRID" || type == "structured_grid")
    return STRUCTURED_GRID;
  else if (type == "UNSTRUCTURED_GRID" || type == "unstructured_grid")
    return UNSTRUCTURED_GRID;
  else if (type == "POLYDATA" || type == "polydata")
    return POLYDATA;
  else if (type == "RECTILINEAR_GRID" || type == "rectilinear_grid")
    return RECTILINEAR_GRID;
  else if (type == "FIELD" || type == "field")
    return FIELD;
  else
    return UNKNOWN_TYPE;
}

//-----------------------------------------------------------------------------

inline std::string format_to_str(Format format) {
  switch (format) {
    default:
    case UNKNOWN_FORMAT: return "UNKNOWN_FORMAT";
    case ASCII: return "ASCII";
    case BINARY: return "BINARY";
  }
}

//-----------------------------------------------------------------------------

inline Format str_to_format(const std::string &format) {
  if (format == "ASCII" || format == "ascii")
    return ASCII;
  else if (format == "BINARY" || format == "binary")
    return BINARY;
  else
    return UNKNOWN_FORMAT;
}

//-----------------------------------------------------------------------------

inline std::string cell_type_to_str(CellType cell_type) {
  switch (cell_type) {
    default:
    case UNKNOWN_CELL_TYPE: return "UNKNOWN_CELL_TYPE";
    case VERTEX: return "VERTEX";
    case POLY_VERTEX: return "POLY_VERTEX";
    case LINE: return "LINE";
    case POLY_LINE: return "POLY_LINE";
    case TRIANGLE: return "TRIANGLE";
    case TRIANGLE_STRIP: return "TRIANGLE_STRIP";
    case POLYGON: return "POLYGON";
    case PIXEL: return "PIXEL";
    case QUAD: return "QUAD";
    case TETRA: return "TETRA";
    case VOXEL: return "VOXEL";
    case HEXAHEDRON: return "HEXAHEDRON";
    case WEDGE: return "WEDGE";
    case PYRAMID: return "PYRAMID";
  }
}

//-----------------------------------------------------------------------------

inline CellType str_to_cell_type(const std::string &cell_type) {
  if (cell_type == "VERTEX" || cell_type == "vertex")
    return VERTEX;
  else if (cell_type == "POLY_VERTEX" || cell_type == "poly_vertex")
    return POLY_VERTEX;
  else if (cell_type == "LINE" || cell_type == "line")
    return LINE;
  else if (cell_type == "POLY_LINE" || cell_type == "poly_line")
    return POLY_LINE;
  else if (cell_type == "TRIANGLE" || cell_type == "triangle")
    return TRIANGLE;
  else if (cell_type == "TRIANGLE_STRIP" || cell_type == "triangle_strip")
    return TRIANGLE_STRIP;
  else if (cell_type == "POLYGON" || cell_type == "polygon")
    return POLYGON;
  else if (cell_type == "PIXEL" || cell_type == "pixel")
    return PIXEL;
  else if (cell_type == "QUAD" || cell_type == "quad")
    return QUAD;
  else if (cell_type == "TETRA" || cell_type == "tetra")
    return TETRA;
  else if (cell_type == "VOXEL" || cell_type == "voxel")
    return VOXEL;
  else if (cell_type == "HEXAHEDRON" || cell_type == "hexahedron")
    return HEXAHEDRON;
  else if (cell_type == "WEDGE" || cell_type == "wedge")
    return WEDGE;
  else if (cell_type == "PYRAMID" || cell_type == "pyramid")
    return PYRAMID;
  else
    return UNKNOWN_CELL_TYPE;
}

struct LegacyFileListener {
  // header data
  virtual void on_version(unsigned short /*major*/, unsigned short /*minor*/) {}
  virtual void on_title(const std::string &) {}
  virtual void on_format(Format) {}
  virtual void on_dataset_type(DatasetType) {}

  // coordinate data
  virtual void on_points(const std::vector<std::array<float, 3>> &) {}
  virtual void on_points(const std::vector<std::array<double, 3>> &) {}
  virtual void on_origin(double /*x*/, double /*y*/, double /*z*/) {}
  virtual void on_spacing(double /*x*/, double /*y*/, double /*z*/) {}
  virtual void on_dimensions(size_t /*x*/, size_t /*y*/, size_t /*z*/) {}
  virtual void on_x_coordinates(const std::vector<float> & /*xs*/) {}
  virtual void on_x_coordinates(const std::vector<double> & /*xs*/) {}
  virtual void on_y_coordinates(const std::vector<float> & /*ys*/) {}
  virtual void on_y_coordinates(const std::vector<double> & /*ys*/) {}
  virtual void on_z_coordinates(const std::vector<float> & /*zs*/) {}
  virtual void on_z_coordinates(const std::vector<double> & /*zs*/) {}

  // index data
  virtual void on_cells(const std::vector<int> &) {}
  virtual void on_cell_types(const std::vector<CellType> &) {}
  virtual void on_vertices(const std::vector<int> &) {}
  virtual void on_lines(const std::vector<int> &) {}
  virtual void on_polygons(const std::vector<int> &) {}
  virtual void on_triangle_strips(const std::vector<int> &) {}

  // cell- / pointdata
  virtual void on_vectors(const std::string & /*name*/,
                          const std::vector<std::array<float, 3>> & /*vectors*/, ReaderData) {}
  virtual void on_vectors(const std::string & /*name*/,
                          const std::vector<std::array<double, 3>> & /*vectors*/, ReaderData) {
  }
  virtual void on_normals(const std::string & /*name*/,
                          const std::vector<std::array<float, 3>> & /*normals*/, ReaderData) {}
  virtual void on_normals(const std::string & /*name*/,
                          const std::vector<std::array<double, 3>> & /*normals*/, ReaderData) {
  }
  virtual void on_texture_coordinates(
      const std::string & /*name*/,
      const std::vector<std::array<float, 2>> & /*texture_coordinates*/, ReaderData) {}
  virtual void on_texture_coordinates(
      const std::string & /*name*/,
      const std::vector<std::array<double, 2>> & /*texture_coordinates*/, ReaderData) {}
  virtual void on_tensors(const std::string & /*name*/,
                          const std::vector<std::array<float, 9>> & /*tensors*/, ReaderData) {}
  virtual void on_tensors(const std::string & /*name*/,
                          const std::vector<std::array<double, 9>> & /*tensors*/, ReaderData) {
  }

  virtual void on_scalars(const std::string & /*data_name*/,
                          const std::string & /*lookup_table_name*/,
                          size_t /*num_comps*/,
                          const std::vector<float> & /*scalars*/, ReaderData) {}
  virtual void on_scalars(const std::string & /*data_name*/,
                          const std::string & /*lookup_table_name*/,
                          size_t /*num_comps*/,
                          const std::vector<double> & /*scalars*/, ReaderData) {
  }
  virtual void on_point_data(size_t) {}
  virtual void on_cell_data(size_t) {}
  virtual void on_field_array(const std::string /*field_name*/,
                              const std::string /*field_array_name*/,
                              const std::vector<int> & /*data*/,
                              size_t /*num_comps*/, size_t /*num_tuples*/) {}
  virtual void on_field_array(const std::string /*field_name*/,
                              const std::string /*field_array_name*/,
                              const std::vector<float> & /*data*/,
                              size_t /*num_comps*/, size_t /*num_tuples*/) {}
  virtual void on_field_array(const std::string /*field_name*/,
                              const std::string /*field_array_name*/,
                              const std::vector<double> & /*data*/,
                              size_t /*num_comps*/, size_t /*num_tuples*/
  ) {}
};

class LegacyFile {
 public:
  std::vector<LegacyFileListener *> m_listeners;

  void add_listener(LegacyFileListener &listener) {
    m_listeners.push_back(&listener);
  }

  //---------------------------------------------------------------------------

  LegacyFile(const std::string &path) : m_path(path) {}

  //---------------------------------------------------------------------------

  void read() {
    read_header();
    read_data();
  }

  //---------------------------------------------------------------------------

  void        set_path(const std::string &path) { m_path = path; }
  void        set_path(std::string &&path) { m_path = std::move(path); }
  const auto &path() const { return m_path; }

  //---------------------------------------------------------------------------

 private:
  inline void read_header();
  inline void read_data();

  inline void read_spacing(std::ifstream &file);
  inline void read_dimensions(std::ifstream &file);
  inline void read_origin(std::ifstream &file);

  inline void read_points(std::ifstream &file);
  template <typename real_t>
  inline void read_points_ascii(std::ifstream &file, const size_t &n);
  template <typename real_t>
  inline void read_points_binary(std::ifstream &file, const size_t &n);

  inline void read_cell_types(std::ifstream &file);
  inline void read_cell_types_ascii(std::ifstream &file, const size_t &n);
  inline void read_cell_types_binary(std::ifstream &file, const size_t &n);

  inline std::vector<int> read_indices(std::ifstream &file);
  inline std::vector<int> read_indices_ascii(std::ifstream &file,
                                             const size_t n, const size_t size);
  inline std::vector<int> read_indices_binary(std::ifstream &file,
                                              const size_t   n,
                                              const size_t   size);

  auto read_scalars_header(std::ifstream &file) {
    std::string       scalar_params = vtk::read_binaryline(file, buffer);
    std::stringstream scalar_params_stream(scalar_params);

    auto data_name    = vtk::read_word(scalar_params_stream, buffer);
    auto data_type    = vtk::read_word(scalar_params_stream, buffer);
    auto num_comp_str = vtk::read_word(scalar_params_stream, buffer);
    // number of components is optional
    size_t num_comps = 1;
    if (num_comp_str.empty()) {
      vtk::read_word(file, buffer);  // consume empty space
    } else if (num_comp_str != "LOOKUP_TABLE") {
      num_comps = parse<size_t>(num_comp_str);
      vtk::read_word(file, buffer);  // consume LOOKUP_TABLE keyword
    }
    auto lookup_table_name = vtk::read_word(file, buffer);
    return std::tuple{data_name, data_type, num_comps, lookup_table_name};
  }

  inline void read_scalars(std::ifstream &file);
  template <typename real_t>
  inline void read_scalars_ascii(std::ifstream &file, const std::string &name,
                                 const std::string &lookup_table,
                                 const size_t       num_comps);
  template <typename real_t>
  inline void read_scalars_binary(std::ifstream &file, const std::string &name,
                                  const std::string &lookup_table,
                                  const size_t       num_comps);

  auto read_data_header(std::ifstream &file) {
    return std::pair{vtk::read_word(file, buffer),
                     vtk::read_word(file, buffer)};
  }
  template <typename real_t, unsigned int n>
  inline std::vector<std::array<real_t, n>> read_data(std::ifstream &file);
  template <typename real_t, unsigned int n>
  inline std::vector<std::array<real_t, n>> read_data_ascii(std::ifstream &file);
  template <typename real_t, unsigned int n>
  inline std::vector<std::array<real_t, n>> read_data_binary(std::ifstream &file);

  //----------------------------------------------------------------------------
  // coordinates
  auto read_coordinates_header(std::ifstream &file) {
    return std::pair{parse<size_t>(vtk::read_word(file, buffer)),
                     vtk::read_word(file, buffer)};
  }
  template <typename real_t>
  auto read_coordinates(std::ifstream &file, size_t n) {
    if (m_format == ASCII)
      return read_coordinates_ascii<real_t>(file, n);

    else /*if (m_format == BINARY)*/
      return read_coordinates_binary<real_t>(file, n);
  }
  template <typename real_t>
  inline std::vector<real_t> read_coordinates_ascii(std::ifstream &file,
                                                    size_t         n);
  template <typename real_t>
  inline std::vector<real_t> read_coordinates_binary(std::ifstream &file,
                                                     size_t         n);

  void read_x_coordinates(std::ifstream &file) {
    auto [n, type] = read_coordinates_header(file);
    if (type == "float") {
      auto c = read_coordinates<float>(file, n);
      for (auto l : m_listeners) l->on_x_coordinates(c);
    } else if (type == "double") {
      auto c = read_coordinates<double>(file, n);
      for (auto l : m_listeners) l->on_x_coordinates(c);
    }
  }

  void read_y_coordinates(std::ifstream &file) {
    auto [n, type] = read_coordinates_header(file);
    if (type == "float") {
      auto c = read_coordinates<float>(file, n);
      for (auto l : m_listeners) l->on_y_coordinates(c);
    } else if (type == "double") {
      auto c = read_coordinates<double>(file, n);
      for (auto l : m_listeners) l->on_y_coordinates(c);
    }
  }

  void read_z_coordinates(std::ifstream &file) {
    auto [n, type] = read_coordinates_header(file);
    if (type == "float") {
      auto c = read_coordinates<float>(file, n);
      for (auto l : m_listeners) l->on_z_coordinates(c);
    } else if (type == "double") {
      auto c = read_coordinates<double>(file, n);
      for (auto l : m_listeners) l->on_z_coordinates(c);
    }
  }

  //----------------------------------------------------------------------------
  // index data
  void read_cells(std::ifstream &file) {
    auto i = read_indices(file);
    for (auto l : m_listeners) l->on_cells(i);
  }
  void read_vertices(std::ifstream &file) {
    auto i = read_indices(file);
    for (auto l : m_listeners) l->on_vertices(i);
  }
  void read_lines(std::ifstream &file) {
    auto i = read_indices(file);
    for (auto l : m_listeners) l->on_lines(i);
  }
  void read_polygons(std::ifstream &file) {
    auto i = read_indices(file);
    for (auto l : m_listeners) l->on_polygons(i);
  }
  void read_triangle_strips(std::ifstream &file) {
    auto i = read_indices(file);
    for (auto l : m_listeners) l->on_triangle_strips(i);
  }

  //----------------------------------------------------------------------------
  // fixed size data
  void read_vectors(std::ifstream &file) {
    auto [name, type] = read_data_header(file);
    if (type == "float") {
      auto data = read_data<float, 3>(file);
      for (auto l : m_listeners) l->on_vectors(name, data, m_data);
    } else if (type == "double") {
      auto data = read_data<double, 3>(file);
      for (auto l : m_listeners) l->on_vectors(name, data, m_data);
    }
  }

  void read_normals(std::ifstream &file) {
    auto [name, type] = read_data_header(file);
    if (type == "float") {
      auto data = read_data<float, 3>(file);
      for (auto l : m_listeners) l->on_normals(name, data, m_data);
    } else if (type == "double") {
      auto data = read_data<double, 3>(file);
      for (auto l : m_listeners) l->on_normals(name, data, m_data);
    }
  }

  void read_texture_coordinates(std::ifstream &file) {
    auto [name, type] = read_data_header(file);
    if (type == "float") {
      auto data = read_data<float, 2>(file);
      for (auto l : m_listeners) l->on_texture_coordinates(name, data, m_data);
    } else if (type == "double") {
      auto data = read_data<double, 2>(file);
      for (auto l : m_listeners) l->on_texture_coordinates(name, data, m_data);
    }
  }

  void read_tensors(std::ifstream &file) {
    auto [name, type] = read_data_header(file);
    if (type == "float") {
      auto data = read_data<float, 9>(file);
      for (auto l : m_listeners) l->on_tensors(name, data, m_data);
    } else if (type == "double") {
      auto data = read_data<double, 9>(file);
      for (auto l : m_listeners) l->on_tensors(name, data, m_data);
    }
  }

  auto read_field_header(std::ifstream &file) {
    std::string       field_params = vtk::read_binaryline(file, buffer);
    std::stringstream field_params_stream(field_params);

    return std::pair{
        vtk::read_word(field_params_stream, buffer),
        parse<size_t>(vtk::read_word(field_params_stream, buffer))};
  }

  //----------------------------------------------------------------------------
  // field data
  auto read_field_array_header(std::ifstream &file) {
    std::string       field_array_params = vtk::read_binaryline(file, buffer);
    std::stringstream field_array_params_stream(field_array_params);

    // {array_name, num_components, num_tuples, datatype_str}
    return std::tuple{
        vtk::read_word(field_array_params_stream, buffer),
        parse<size_t>(vtk::read_word(field_array_params_stream, buffer)),
        parse<size_t>(vtk::read_word(field_array_params_stream, buffer)),
        vtk::read_word(field_array_params_stream, buffer)};
  }

  void read_field(std::ifstream &file) {
    auto [field_name, num_arrays] = read_field_header(file);
    for (size_t i = 0; i < num_arrays; ++i) {
      auto [field_array_name, num_comps, num_tuples, datatype_str] =
          read_field_array_header(file);

      if (m_format == ASCII) {
        if (datatype_str == "int") {
          auto data = read_field_array_ascii<int>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        } else if (datatype_str == "float") {
          auto data =
              read_field_array_ascii<float>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        } else if (datatype_str == "double") {
          auto data =
              read_field_array_ascii<double>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        }

      } else if (m_format == BINARY) {
        if (datatype_str == "int") {
          auto data = read_field_array_binary<int>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        } else if (datatype_str == "float") {
          auto data =
              read_field_array_binary<float>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        } else if (datatype_str == "double") {
          auto data =
              read_field_array_binary<double>(file, num_comps, num_tuples);
          for (auto l : m_listeners)
            l->on_field_array(field_name, field_array_name, data, num_comps,
                              num_tuples);
        }
      }
    }
  }

  template <typename real_t>
  std::vector<real_t> read_field_array_binary(std::ifstream &file,
                                              size_t         num_comps,
                                              size_t         num_tuples) {
    std::vector<real_t> data(num_comps * num_tuples);
    file.read((char *)data.data(), sizeof(real_t) * num_comps * num_tuples);

    // consume trailing \n
    char consumer;
    file.read(&consumer, sizeof(char));

    return data;
  }
  template <typename real_t>
  std::vector<real_t> read_field_array_ascii(std::ifstream &file,
                                             size_t         num_comps,
                                             size_t         num_tuples) {
    std::vector<real_t> data;
    data.reserve(num_comps * num_tuples);
    for (size_t i = 0; i < num_comps * num_tuples; i++)
      data.push_back(parse<real_t>(vtk::read_word(file, buffer)));

    return data;
  }

  void consume_trailing_break(std::ifstream &file) {
    char consumer;
    file.read(&consumer, sizeof(char));
  }

  std::string m_path;
  Format      m_format;
  ReaderData  m_data = UNSPECIFIED_DATA;
  size_t      m_data_size;  // cell_data or point_data size
  long        m_begin_of_data;
  char        buffer[256];
};

//-----------------------------------------------------------------------------------------------

void LegacyFile::read_header() {
  std::ifstream file(m_path, std::ifstream::binary);
  if (file.is_open()) {
    // read part1 # vtk DataFile Version x.x
    std::string part1 = vtk::read_binaryline(file, buffer);
    for (auto listener : m_listeners)
      listener->on_version((unsigned short)(atoi(&buffer[23])),
                           (unsigned short)(atoi(&buffer[25])));

    // read part2 maximal 256 characters
    std::string part2 = vtk::read_binaryline(file, buffer);
    for (auto listener : m_listeners) listener->on_title(part2);

    // read part3 ASCII | BINARY
    std::string part3 = vtk::read_binaryline(file, buffer);
    if (part3 == "ASCII" || part3 == "ascii")
      m_format = ASCII;
    else if (part3 == "BINARY" || part3 == "binary")
      m_format = BINARY;
    else
      m_format = UNKNOWN_FORMAT;

    for (auto listener : m_listeners) listener->on_format(m_format);

    // read part4 STRUCTURED_POINTS | STRUCTURED_GRID | UNSTRUCTURED_GRID |
    // POLYDATA | RECTILINEAR_GRID | FIELD
    file.read(buffer, sizeof(char) * 8);  // consume "DATASET "
    auto part4 = str_to_type(vtk::read_binaryline(file, buffer));
    for (auto listener : m_listeners) listener->on_dataset_type(part4);

    m_begin_of_data = file.tellg();
    file.close();
  } else
    throw std::runtime_error("[vtk::LegacyFile] could not open file " + m_path);
}

//-----------------------------------------------------------------------------

void LegacyFile::read_data() {
  std::ifstream file(m_path, std::ifstream::binary);
  if (file.is_open()) {
    file.seekg(m_begin_of_data, file.beg);
    std::string keyword;

    while (!file.eof()) {
      keyword = vtk::read_word(file, buffer);
      if (!keyword.empty()) {
        if (keyword == "POINTS")
          read_points(file);

        else if (keyword == "LINES")
          read_lines(file);

        else if (keyword == "VERTICES")
          read_vertices(file);

        else if (keyword == "POLYGONS")
          read_polygons(file);

        else if (keyword == "CELLS")
          read_cells(file);

        else if (keyword == "CELL_TYPES")
          read_cell_types(file);

        else if (keyword == "DIMENSIONS")
          read_dimensions(file);

        else if (keyword == "ORIGIN")
          read_origin(file);

        else if (keyword == "SPACING")
          read_spacing(file);

        else if (keyword == "X_COORDINATES")
          read_x_coordinates(file);

        else if (keyword == "Y_COORDINATES")
          read_y_coordinates(file);

        else if (keyword == "Z_COORDINATES")
          read_z_coordinates(file);

        else if (keyword == "POINT_DATA") {
          m_data_size = size_t(parse<int>(vtk::read_word(file, buffer)));
          m_data      = POINT_DATA;
          for (auto l : m_listeners) l->on_point_data(m_data_size);

        } else if (keyword == "CELL_DATA") {
          m_data_size = size_t(parse<int>(vtk::read_word(file, buffer)));
          m_data      = POINT_DATA;
          for (auto l : m_listeners) l->on_cell_data(m_data_size);
        }

        else if (keyword == "SCALARS")
          read_scalars(file);

        else if (keyword == "VECTORS")
          read_vectors(file);

        else if (keyword == "NORMALS")
          read_normals(file);

        else if (keyword == "TEXTURE_COORDINATES")
          read_texture_coordinates(file);

        else if (keyword == "FIELD")
          read_field(file);

        else
          std::cerr << "[tatooine::vtk::LegacyFile] unknown keyword: "
                    << keyword << '\n';
      }
    }
  } else
    throw std::runtime_error(
        "[tatooine::vtk::LegacyFile] could not open file " + m_path);
}

//------------------------------------------------------------------------------

void LegacyFile::read_spacing(std::ifstream &file) {
  std::array spacing{parse<double>(vtk::read_word(file, buffer)),
                     parse<double>(vtk::read_word(file, buffer)),
                     parse<double>(vtk::read_word(file, buffer))};
  for (auto l : m_listeners) l->on_spacing(spacing[0], spacing[1], spacing[2]);
}

//------------------------------------------------------------------------------

void LegacyFile::read_dimensions(std::ifstream &file) {
  std::array dims{parse<size_t>(vtk::read_word(file, buffer)),
                  parse<size_t>(vtk::read_word(file, buffer)),
                  parse<size_t>(vtk::read_word(file, buffer))};
  for (auto l : m_listeners) l->on_dimensions(dims[0], dims[1], dims[2]);
}

//------------------------------------------------------------------------------

void LegacyFile::read_origin(std::ifstream &file) {
  std::array origin{parse<double>(vtk::read_word(file, buffer)),
                    parse<double>(vtk::read_word(file, buffer)),
                    parse<double>(vtk::read_word(file, buffer))};
  for (auto l : m_listeners) l->on_origin(origin[0], origin[1], origin[2]);
}

//-----------------------------------------------------------------------------------------------

void LegacyFile::read_points(std::ifstream &file) {
  auto num_points_str = vtk::read_word(file, buffer);
  auto n              = parse<size_t>(num_points_str);
  auto datatype_str   = vtk::read_word(file, buffer);

  if (m_format == ASCII) {
    if (datatype_str == "float")
      read_points_ascii<float>(file, n);
    else if (datatype_str == "double")
      read_points_ascii<double>(file, n);
  } else if (m_format == BINARY) {
    if (datatype_str == "float")
      read_points_binary<float>(file, n);
    else if (datatype_str == "double")
      read_points_binary<double>(file, n);
  }
}

//------------------------------------------------------------------------------

template <typename real_t>
void LegacyFile::read_points_ascii(std::ifstream &file, const size_t &n) {
  std::vector<std::array<real_t, 3>> points;
  for (size_t i = 0; i < n; i++)
    points.push_back(
        {{static_cast<real_t>(parse<real_t>(vtk::read_word(file, buffer))),
          static_cast<real_t>(parse<real_t>(vtk::read_word(file, buffer))),
          static_cast<real_t>(parse<real_t>(vtk::read_word(file, buffer)))}});

  for (auto l : m_listeners) l->on_points(points);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFile::read_points_binary(std::ifstream &file, const size_t &n) {
  // std::vector<std::array<real_t, 3>> points(n);
  // file.read((char *)points.data(), sizeof(real_t) * 3 * n);
  // swap_endianess(reinterpret_cast<real_t *>(points.data()), n * 3);
  // for (auto l : m_listeners) l->on_points(points);
  // consume_trailing_break(file);
  file.ignore(sizeof(real_t) * 3 * n + 1);
}

//-----------------------------------------------------------------------------------------------

void LegacyFile::read_cell_types(std::ifstream &file) {
  auto num_cell_types_str = vtk::read_word(file, buffer);
  auto num_cell_types     = parse<size_t>(num_cell_types_str);

  if (m_format == ASCII)
    read_cell_types_ascii(file, num_cell_types);
  else if (m_format == BINARY)
    read_cell_types_binary(file, num_cell_types);
}

//-----------------------------------------------------------------------------------------------

void LegacyFile::read_cell_types_ascii(std::ifstream &file,
                                       const size_t & num_cell_types) {
  std::vector<CellType> cell_types;
  std::string           cell_type_str;
  for (size_t i = 0; i < num_cell_types; i++) {
    cell_type_str = vtk::read_word(file, buffer);
    cell_types.push_back((CellType)parse<int>(cell_type_str));
  }
  for (auto listener : m_listeners) listener->on_cell_types(cell_types);
}

//-----------------------------------------------------------------------------

void LegacyFile::read_cell_types_binary(std::ifstream &file,
                                        const size_t & num_cell_types) {
  std::vector<CellType> cell_types(num_cell_types);
  file.read((char *)cell_types.data(), sizeof(int) * num_cell_types);
  swap_endianess(cell_types);
  for (auto listener : m_listeners) listener->on_cell_types(cell_types);
  consume_trailing_break(file);
}

//-----------------------------------------------------------------------------------------------

std::vector<int> LegacyFile::read_indices(std::ifstream &file) {
  auto num_indices_str = vtk::read_word(file, buffer);
  auto size_str        = vtk::read_word(file, buffer);

  auto num_indices = parse<size_t>(num_indices_str);
  auto size        = parse<size_t>(size_str);

  if (m_format == ASCII)
    return read_indices_ascii(file, num_indices, size);
  else /*if (m_format == BINARY)*/
    return read_indices_binary(file, num_indices, size);
}

//-----------------------------------------------------------------------------

std::vector<int> LegacyFile::read_indices_ascii(std::ifstream &file,
                                                const size_t /* num_indices*/,
                                                const size_t size) {
  std::vector<int> indices;
  indices.reserve(size);
  std::string val_str;
  for (size_t i = 0; i < size; i++)
    indices.push_back(parse<size_t>(vtk::read_word(file, buffer)));
  return indices;
}

//-----------------------------------------------------------------------------

std::vector<int> LegacyFile::read_indices_binary(std::ifstream &file,
                                                 const size_t /*num_indices*/,
                                                 const size_t size) {
  std::vector<int> indices(size);
  file.read((char *)indices.data(), sizeof(int) * size);
  swap_endianess(indices);
  consume_trailing_break(file);
  return indices;
}

//-----------------------------------------------------------------------------------------------

template <typename real_t, unsigned int n>
std::vector<std::array<real_t, n>> LegacyFile::read_data(std::ifstream &file) {
  if (m_format == ASCII)
    return read_data_ascii<real_t, n>(file);
  else
    return read_data_binary<real_t, n>(file);
}

//-----------------------------------------------------------------------------

template <typename real_t, unsigned int n>
std::vector<std::array<real_t, n>> LegacyFile::read_data_ascii(std::ifstream &file) {
  std::vector<std::array<real_t, n>> data(m_data_size);
  for (size_t i = 0; i < m_data_size; i++)
    for (size_t j = 0; j < n; j++)
      data[i][n] = parse<real_t>(vtk::read_word(file, buffer));
  return data;
}

//-----------------------------------------------------------------------------

template <typename real_t, unsigned int n>
std::vector<std::array<real_t, n>> LegacyFile::read_data_binary(std::ifstream &file) {
  std::vector<std::array<real_t, n>> data(m_data_size);
  file.read((char *)data.data(), sizeof(real_t) * m_data_size * n);
  swap_endianess(reinterpret_cast<real_t *>(data.data()), n * m_data_size);
  consume_trailing_break(file);
  return data;
}
//-----------------------------------------------------------------------------------------------

template <typename real_t>
std::vector<real_t> LegacyFile::read_coordinates_ascii(std::ifstream &file,
                                                       const size_t   n) {
  std::vector<real_t> coordinates(n);
  for (size_t i = 0; i < n; i++)
    coordinates[i] = parse<real_t>(vtk::read_word(file, buffer));
  return coordinates;
}

//-----------------------------------------------------------------------------

template <typename real_t>
std::vector<real_t> LegacyFile::read_coordinates_binary(std::ifstream &file,
                                                        const size_t   n) {
  std::vector<real_t> coordinates(n);
  file.read((char *)coordinates.data(), sizeof(real_t) * n);
  swap_endianess(coordinates);
  consume_trailing_break(file);
  return coordinates;
}

//-----------------------------------------------------------------------------

void LegacyFile::read_scalars(std::ifstream &file) {
  auto [name, type, num_comps, lookup_table] = read_scalars_header(file);
  if (m_format == ASCII) {
    if (type == "float")
      read_scalars_ascii<float>(file, name, lookup_table, num_comps);
    else if (type == "double")
      read_scalars_ascii<double>(file, name, lookup_table, num_comps);
  } else if (m_format == BINARY) {
    if (type == "float")
      read_scalars_binary<float>(file, name, lookup_table, num_comps);
    else if (type == "double")
      read_scalars_binary<double>(file, name, lookup_table, num_comps);
  }
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFile::read_scalars_ascii(std::ifstream &    file,
                                    const std::string &name,
                                    const std::string &lookup_table,
                                    const size_t       num_comps) {
  std::vector<real_t> scalars;
  scalars.reserve(m_data_size * num_comps);
  std::string val_str;
  for (size_t i = 0; i < m_data_size * num_comps; i++)
    scalars.push_back(parse<real_t>(vtk::read_word(file, buffer)));
  for (auto l : m_listeners)
    l->on_scalars(name, lookup_table, num_comps, scalars, m_data);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFile::read_scalars_binary(std::ifstream &    file,
                                     const std::string &name,
                                     const std::string &lookup_table,
                                     const size_t       num_comps) {
  std::vector<real_t> data(m_data_size * num_comps);

  file.read((char *)data.data(), sizeof(real_t) * m_data_size * num_comps);
  swap_endianess(data);

  consume_trailing_break(file);
  for (auto l : m_listeners)
    l->on_scalars(name, lookup_table, num_comps, data, m_data);
}

//-----------------------------------------------------------------------------------------------

template <typename real_t>
class LegacyFileWriter {
 public:
  LegacyFileWriter(const std::string &path, DatasetType type,
                   unsigned short major = 2, unsigned short minor = 0,
                   const std::string &title = "")
      : m_file(path, std::ofstream::binary),
        m_major_version(major),
        m_minor_version(minor),
        m_dataset_type(type),
        m_title(title) {}

  auto is_open() { return m_file.is_open(); }
  auto close() { m_file.close(); }

  //---------------------------------------------------------------------------

 private:
  void write_indices(const std::string &                     keyword,
                     const std::vector<std::vector<size_t>> &indices);
  template <unsigned int n>
  void write_data(const std::string &keyword, const std::string &name,
                  const std::vector<std::array<real_t, n>> &data);

 public:
  void write_header();
  void write_points(const std::vector<std::array<real_t, 3>> &points);
  void write_cells(const std::vector<std::vector<size_t>> &cells);
  void write_cell_types(const std::vector<CellType> &cell_types);

  void write_vertices(const std::vector<std::vector<size_t>> &vertices);
  void write_lines(const std::vector<std::vector<size_t>> &lines);
  void write_polygons(const std::vector<std::vector<size_t>> &polygons);
  void write_triangle_strips(
      const std::vector<std::vector<size_t>> &triangle_strips);

  void write_point_data(size_t i);
  void write_cell_data(size_t i);
  void write_normals(const std::string &                 name,
                     std::vector<std::array<real_t, 3>> &normals);
  void write_vectors(const std::string &                 name,
                     std::vector<std::array<real_t, 3>> &vectors);
  void write_texture_coordinates(
      const std::string &                 name,
      std::vector<std::array<real_t, 2>> &texture_coordinates);
  void write_tensors(const std::string &                 name,
                     std::vector<std::array<real_t, 9>> &tensors);

  template <typename data_t,
            typename = std::enable_if_t<(std::is_same_v<data_t, double> ||
                                         std::is_same_v<data_t, float> ||
                                         std::is_same_v<data_t, int>)>>
  void write_scalars(const std::string &                     name,
                     const std::vector<std::vector<data_t>> &data,
                     const std::string &lookup_table_name = "default");

  //---------------------------------------------------------------------------

  void set_version(const unsigned short major_version,
                   const unsigned short minor_version) {
    set_major_version(major_version);
    set_minor_version(minor_version);
  }

  //---------------------------------------------------------------------------

  const auto &major_version() const { return m_major_version; }
  void        set_major_version(const unsigned short major_version) {
    m_major_version = major_version;
  }

  //---------------------------------------------------------------------------

  const auto &minor_version() const { return m_minor_version; }
  void        set_minor_version(const unsigned short minor_version) {
    m_minor_version = minor_version;
  }

  //---------------------------------------------------------------------------

  const auto &type() const { return m_dataset_type; }
  auto        type_str() const { return type_to_str(m_dataset_type); }
  void        set_type(const DatasetType type) { m_dataset_type = type; }
  void        set_type(const std::string &type_str) {
    m_dataset_type = str_to_type(type_str);
  }

  //---------------------------------------------------------------------------

  const auto &title() const { return m_title; }
  void        set_title(const std::string &title) { m_title = title; }
  void        set_title(std::string &&title) { m_title = std::move(title); }

  //---------------------------------------------------------------------------
 private:
  std::ofstream  m_file;
  unsigned short m_major_version;
  unsigned short m_minor_version;
  DatasetType    m_dataset_type;
  std::string    m_title;
};

//=============================================================================

template <typename real_t>
void LegacyFileWriter<real_t>::write_header() {
  // write opener
  vtk::write_binary(m_file, "# vtk DataFile Version " +
                                std::to_string(m_major_version) + "." +
                                std::to_string(m_minor_version) + '\n');

  // write title
  vtk::write_binary(m_file, m_title + '\n');

  // write part3 ASCII | BINARY
  vtk::write_binary(m_file, format_to_str(BINARY) + '\n');

  // write part4 STRUCTURED_POINTS | STRUCTURED_GRID | UNSTRUCTURED_GRID|
  // POLYDATA | RECTILINEAR_GRID | FIELD
  vtk::write_binary(m_file, "DATASET " + type_to_str(m_dataset_type));
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_points(
    const std::vector<std::array<real_t, 3>> &points) {
  vtk::write_binary(m_file, "\nPOINTS " + std::to_string(points.size()) + ' ' +
                                tatooine::type_to_str<real_t>() + '\n');
  std::vector<std::array<real_t, 3>> points_swapped(points);
  swap_endianess(reinterpret_cast<real_t *>(points_swapped.data()),
                 3 * points.size());
  for (const auto &p : points_swapped)
    for (auto c : p) m_file.write((char *)(&c), sizeof(real_t));
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_indices(
    const std::string &                     keyword,
    const std::vector<std::vector<size_t>> &indices) {
  size_t total_number = 0;
  for (const auto &is : indices) total_number += is.size() + 1;
  vtk::write_binary(m_file, "\n" + keyword + " " +
                                std::to_string(indices.size()) + ' ' +
                                std::to_string(total_number) + '\n');
  for (const auto &p : indices) {
    int size = (int)p.size();
    size     = swap_endianess(size);
    m_file.write((char *)(&size), sizeof(int));
    for (int i : p) {
      i = swap_endianess(i);
      m_file.write((char *)(&i), sizeof(int));
    }
  }
}

//-----------------------------------------------------------------------------

template <typename real_t>
template <unsigned int n>
void LegacyFileWriter<real_t>::write_data(
    const std::string &keyword, const std::string &name,
    const std::vector<std::array<real_t, n>> &data) {
  vtk::write_binary(m_file, "\n" + keyword + " " + name + ' ' +
                                tatooine::type_to_str<real_t>() + '\n');
  for (const auto &vec : data)
    for (auto comp : vec) {
      comp = swap_endianess(comp);
      m_file.write((char *)(&comp), sizeof(real_t));
    }
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_cells(
    const std::vector<std::vector<size_t>> &cells) {
  write_indices("CELLS", cells);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_cell_types(
    const std::vector<CellType> &cell_types) {
  vtk::write_binary(m_file,
                    "\nCELL_TYPES " + std::to_string(cell_types.size()) + '\n');
  for (int type : cell_types) {
    type = swap_endianess(int(type));
    m_file.write((char *)(&type), sizeof(int));
  }
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_point_data(size_t i) {
  vtk::write_binary(m_file, "\nPOINT_DATA " + std::to_string(i));
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_cell_data(size_t i) {
  vtk::write_binary(m_file, "\nCELL_DATA " + std::to_string(i));
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_vertices(
    const std::vector<std::vector<size_t>> &vertices) {
  write_indices("VERTICES", vertices);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_lines(
    const std::vector<std::vector<size_t>> &lines) {
  write_indices("LINES", lines);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_polygons(
    const std::vector<std::vector<size_t>> &lines) {
  write_indices("POLYGONS", lines);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_triangle_strips(
    const std::vector<std::vector<size_t>> &lines) {
  write_indices("TRIANGLE_STRIPS", lines);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_normals(
    const std::string &name, std::vector<std::array<real_t, 3>> &normals) {
  write_data<3>("NORMALS", name, normals);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_vectors(
    const std::string &name, std::vector<std::array<real_t, 3>> &vectors) {
  write_data<3>("VECTORS", name, vectors);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_texture_coordinates(
    const std::string &                 name,
    std::vector<std::array<real_t, 2>> &texture_coordinates) {
  write_data<2>("TEXTURE_COORDINATES", name, texture_coordinates);
}

//-----------------------------------------------------------------------------

template <typename real_t>
void LegacyFileWriter<real_t>::write_tensors(
    const std::string &name, std::vector<std::array<real_t, 9>> &tensors) {
  write_data<9>("TENSORS", name, tensors);
}
template <typename real_t>
template <typename data_t, typename>
void LegacyFileWriter<real_t>::write_scalars(
    const std::string &name, const std::vector<std::vector<data_t>> &data,
    const std::string &lookup_table_name) {
  vtk::write_binary(m_file, "\nSCALARS " + name + ' ' +
                                tatooine::type_to_str<data_t>() + ' ' +
                                std::to_string(data.front().size()) + '\n');
  vtk::write_binary(m_file, "\nLOOKUP_TABLE " + lookup_table_name + '\n');
  for (const auto &vec : data)
    for (auto comp : vec) {
      comp = swap_endianess(comp);
      m_file.write((char *)(&comp), sizeof(data_t));
    }
}

//=============================================================================
}  // namespace tatooine::vtk
//=============================================================================

#endif