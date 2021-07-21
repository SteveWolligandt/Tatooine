#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <tatooine/multidim_array.h>
#include <tatooine/make_array.h>
#include <tatooine/rectilinear_grid.h>
#include <tatooine/vtk_legacy.h>
#include <tatooine/tensor.h>
#include <tatooine/interpolation.h>
#include <tatooine/grid_sampler.h>
#include <tatooine/filesystem.h>
//==============================================================================
static const std::array<std::string_view, 4> filenames{
    "texture_case1.txt", "texture_case2.txt", "texture_case3.txt",
    "texture_case4.txt"};
//==============================================================================
int main() {
  using namespace tatooine;
  for (const auto& filename : filenames) {

    std::map<char, dynamic_multidim_array<double>> data{
        {'x', dynamic_multidim_array<double>{}},
        {'y', dynamic_multidim_array<double>{}},
        {'u', dynamic_multidim_array<double>{}},
        {'v', dynamic_multidim_array<double>{}}};
    std::ifstream file_stream{std::string{filename}};
    char          cur_data = ' ';

    if (file_stream.is_open()) {
      // read data
      std::string line;
      size_t      x = 0, y = 0;
      size_t      resx = 0, resy = 0;
      double      d = 0;
      while (std::getline(file_stream, line)) {
        std::istringstream line_stream{line};
        if (line[1] == 'x' || line[1] == 'y' || line[1] == 'u' ||
            line[1] == 'v') {
          line_stream >> cur_data >> resx >> resy;
          data[cur_data].resize(resx, resy);
          x = y = 0;
        } else {
          while (line_stream >> d) { data.at(cur_data)(x++, y) = d; }
          x = 0;
          ++y;
        }
      }
      // build discrete domain
      rectilinear_grid domain{linspace{data['x'](0, 0), data['x'](data['x'].size(0) - 1, 0),
                    data['x'].size(0)},
           linspace{data['y'](0, 0), data['y'](0, data['y'].size(1) - 1),
                    data['y'].size(1)}};
      //check if really linear in x
      size_t cnt = 0;
      for (auto x : domain.dimension(0)) {
        if (std::abs(data['x'](cnt++, 0) - x) > 1e-7) {
          std::cerr << data['x'](cnt - 1, 0) << " - " << x << '\n';
          throw std::runtime_error{"not linear"};
        }
      }
      //check if really linear in y
      cnt = 0;
      for (auto y : domain.dimension(1)) {
        if (std::abs(data['y'](0, cnt++) - y) > 1e-7) {
          std::cerr << data['y'](0, cnt - 1) << " - " << y << '\n';
          throw std::runtime_error{"not linear"};
        }
      }
      // build vtk file
      std::string out_filename{filename};
      auto        dotpos       = out_filename.find_last_of(".");
      out_filename.replace(dotpos + 1, 3, "vtk");
      vtk::legacy_file_writer writer(out_filename, vtk::STRUCTURED_POINTS);
      if (writer.is_open()) {
        writer.set_title("material science fields");
        writer.write_header();

        writer.write_dimensions(domain.size(0), domain.size(1), 1);
        writer.write_origin(domain.front(0), domain.front(1), 0);
        writer.write_spacing(domain.spacing(0), domain.spacing(1), 0);
        writer.write_point_data(domain.size(0) * domain.size(1));

        // write data
        std::vector<vec<double, 2>> vector_field;
        std::vector<vec<double, 3>> vector_field3;
        vector_field.reserve(domain.size(0) * domain.size(1));
        for (auto v : domain.vertices()) {
          vector_field.push_back(
              vec{data['u'](v.indices().data()),
                  data['v'](v.indices().data())});
          vector_field3.push_back(
              vec{data['u'](v.indices().data()),
                  data['v'](v.indices().data()),
                  0});
        }
        writer.write_scalars("vector_field", vector_field);
        writer.write_scalars("three_dimensional_vector_field", vector_field3);
        writer.close();
      }
      // read data back and check if everything is correct
      using namespace interpolation;
      grid_sampler<double, 2, vec<double, 2>, linear, linear> sampler;
      sampler.read_vtk_scalars(out_filename, "vector_field");
      assert(sampler.size(0) == resx);
      assert(sampler.size(1) == resy);
      assert(std::abs(sampler.front(0) - data['x'](0, 0)) < 1e-5);
      assert(std::abs(sampler[0][0](0) - data['u'](0, 0)) < 1e-5);
    }
  }
}
