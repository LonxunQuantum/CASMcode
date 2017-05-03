#ifndef CASM_rm
#define CASM_rm

#include "casm/completer/Handlers.hh"
#include "casm/app/APICommand.hh"

namespace CASM {
  namespace Completer {

    class RmOption : public OptionHandlerBase {

    public:

      using OptionHandlerBase::selection_path;
      using OptionHandlerBase::name_strs;
      using OptionHandlerBase::db_type;
      using OptionHandlerBase::db_type_opts;

      RmOption();

      bool force() const;

      bool data() const;

      bool dry_run() const;

    private:

      void initialize() override;

    };
  }

  class RmCommandImplBase;
  template<typename T> class RmCommandImpl;

  /// 'casm rm' command
  ///
  class RmCommand :
    public APICommand<Completer::RmOption> {

  public:

    using ImplBase = RmCommandImplBase;
    template<typename ConfigType> using Impl = RmCommandImpl<ConfigType>;

    static const std::string name;

    RmCommand(const CommandArgs &_args, Completer::RmOption &_opt);

    int vm_count_check() const override;

    int help() const override;

    int desc() const override;

    int run() const override;

    // -- custom --

    RmCommandImplBase &impl() const;

    void print_names(std::ostream &sout) const;

    void print_config_names(std::ostream &sout) const;

  private:
    mutable std::unique_ptr<RmCommandImplBase> m_impl;
  };

}

#endif
