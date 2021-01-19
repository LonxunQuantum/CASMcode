#include "ccasm/api.hh"

#include <wordexp.h>

#include <boost/filesystem.hpp>

#include "casm/app/DirectoryStructure.hh"
#include "casm/app/casm_functions.hh"
#include "casm/casm_io/Log.hh"
#include "casm/casm_io/container/json_io.hh"
#include "casm/casm_io/json/jsonParser.hh"
#include "casm/clex/PrimClex.hh"

using namespace CASM;

extern "C" {

costream *casm_STDOUT() { return reinterpret_cast<costream *>(&cout_log()); }

costream *casm_STDERR() { return reinterpret_cast<costream *>(&cerr_log()); }

costream *casm_nullstream() {
  return reinterpret_cast<costream *>(&null_log());
}

costream *casm_ostringstream_new() {
  return reinterpret_cast<costream *>(new OStringStreamLog());
}

void casm_ostringstream_delete(costream *ptr) {
  delete reinterpret_cast<OStringStreamLog *>(ptr);
}

unsigned long casm_ostringstream_size(costream *ptr) {
  typedef std::char_traits<char>::pos_type pos_type;
  return reinterpret_cast<OStringStreamLog *>(ptr)->ss().tellp() + pos_type(1);
}

char *casm_ostringstream_strcpy(costream *ptr, char *c_str) {
  auto str = reinterpret_cast<OStringStreamLog *>(ptr)->ss().str();
  std::strcpy(c_str, str.c_str());
  return c_str;
}

cPrimClex *casm_primclex_null() { return nullptr; }

cPrimClex *casm_primclex_new(char *path, costream *log, costream *err_log) {
  Log &_log(*reinterpret_cast<Log *>(log));
  Log &_err_log(*reinterpret_cast<Log *>(err_log));
  ScopedLogging logging{_log, _err_log};
  PrimClex *ptr = new PrimClex(fs::path(path));
  return reinterpret_cast<cPrimClex *>(ptr);
}

void casm_primclex_delete(cPrimClex *ptr) {
  delete reinterpret_cast<PrimClex *>(ptr);
}

void casm_primclex_refresh(cPrimClex *ptr, costream *log, costream *err_log,
                           bool read_settings, bool read_composition,
                           bool read_chem_ref, bool read_configs,
                           bool clear_clex) {
  Log &_log(*reinterpret_cast<Log *>(log));
  Log &_err_log(*reinterpret_cast<Log *>(err_log));
  ScopedLogging logging{_log, _err_log};
  PrimClex *_primclex = reinterpret_cast<PrimClex *>(ptr);
  _primclex->refresh(read_settings, read_composition, read_chem_ref,
                     read_configs, clear_clex);
}

/// Print to log the recognized casm commands as a JSON array
void casm_command_list(costream *ostringstream_log) {
  Log &_log(*reinterpret_cast<OStringStreamLog *>(ostringstream_log));
  std::vector<std::string> command_list;
  for (const auto &val : command_map()) {
    command_list.push_back(val.first);
  }
  jsonParser json;
  json = command_list;
  _log << json;
}

/// Construct a CommandArgs instance and call casm_api
///
/// - Will construct temporary PrimClex if necessary
/// - If primclex is not null, then root is ignored.
int casm_capi(char *args, cPrimClex *primclex, char *root, costream *log,
              costream *err_log) {
  PrimClex *_primclex = reinterpret_cast<PrimClex *>(primclex);
  Log &_log(*reinterpret_cast<Log *>(log));
  Log &_err_log(*reinterpret_cast<Log *>(err_log));
  ScopedLogging logging{_log, _err_log};

  std::string s("casm ");
  s += std::string(args);

  fs::path _root(root);

  CommandArgs command_args(s, _primclex, _root);
  if (command_args.parse_result()) {
    return command_args.parse_result();
  }

  return casm_api(command_args);
}
}
