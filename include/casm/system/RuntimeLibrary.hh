#ifndef RuntimeLibrary_HH
#define RuntimeLibrary_HH

#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <dlfcn.h>
#include <cstdlib>
#include "casm/system/Popen.hh"
#include "casm/CASM_global_definitions.hh"

namespace CASM {

  /// \brief Write, compile, load and use code at runtime
  class RuntimeLibrary {

  public:

    /// \brief Construct a RuntimeLibrary object, with the options to be used for compile
    ///        the '.o' file and the '.so' file
    RuntimeLibrary(std::string _compile_options, std::string _so_options);

    ~RuntimeLibrary();

    /// \brief Compile a shared library
    void compile(std::string _filename_base,
                 std::string _source);

    /// \brief Compile a shared library
    void compile(std::string _filename_base);


    /// \brief Load a library with a given name
    void load(std::string _filename_base);

    /// \brief Obtain a function from the current library
    ///
    /// Must be a C-style function to enable symbol lookup, i.e your source code should use extern "C".
    /// This means no member functions or overloaded functions.
    ///
    template<typename Signature>
    std::function<Signature> get_function(std::string function_name) const {

      std::function<Signature> func = reinterpret_cast<Signature *>(dlsym(m_handle, function_name.c_str()));

      const char *dlsym_error = dlerror();
      if(dlsym_error) {
        throw std::runtime_error(std::string("Cannot load symbol " + function_name + " \n" + dlsym_error));
      }

      return func;
    }

    /// \brief Close the current library
    void close();

    /// \brief Remove the current library and source code
    void rm();

    /// \brief Default c++ compiler options
    static std::pair<std::string, std::string> default_cxxflags();

    /// \brief Default c++ compiler options
    static std::pair<std::string, std::string> default_soflags();

    /// \brief Return default compiler
    static std::pair<std::string, std::string> default_cxx();

    /// \brief Return default prefix for CASM
    static std::pair<fs::path, std::string> default_casm_prefix();

    /// \brief Return default prefix for boost
    static std::pair<fs::path, std::string> default_boost_prefix();


  private:

    std::string m_compile_options;
    std::string m_so_options;

    std::string m_filename_base;

    void *m_handle;

  };

  std::string include_path(const fs::path &prefix);

  std::string link_path(const fs::path &prefix);


}

#endif
