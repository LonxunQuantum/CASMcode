#include "casm/app/casm_functions.hh"
#include "casm/CASM_classes.hh"

namespace CASM {

  int view_command(const CommandArgs &args) {

    fs::path selection;
    po::variables_map vm;
    bool force;
    std::vector<std::string> configname;



    try {

      // Set command line options using boost program_options
      po::options_description desc("'casm view' usage");
      desc.add_options()
      ("help,h", "Print help message")

      ("configname",
       po::value<std::vector<std::string> >(&configname)->multitoken(),
       "The name of 1 or more configurations to view. This is also a positional "
       "argument, so '--configname' is optional.")

      ("config,c",
       po::value<fs::path>(&selection),
       "Selected configurations to view.");


      // allow configname as positional options
      po::positional_options_description p;
      p.add("configname", -1);

      try {
        po::store(po::command_line_parser(args.argc - 1, args.argv + 1).options(desc).positional(p).run(), vm);
        //po::store(po::parse_command_line(argc, argv, desc), vm); // can throw

        /** --help option
        */
        if(vm.count("help")) {
          std::cout << "\n";
          std::cout << desc << std::endl;

          std::cout << "This allows opening visualization programs directly from \n"
                    "CASM. It iterates over all selected configurations and   \n"
                    "one by one writes a POSCAR and executes                  \n"
                    "   '$VIEW_COMMAND /path/to/POSCAR'                       \n"
                    "where $VIEW_COMMAND is set via 'casm settings --set-view-command'.\n"
                    "A script 'casm.view' is included with can be used to run \n"
                    "a command and then pause 1s, which is useful for opening \n"
                    "POSCARs with VESTA.  An example on Mac might look like:  \n"
                    "  casm settings --set-view-command 'casm.view \"open -a /Applications/VESTA/VESTA.app\"' \n\n";

          return 0;
        }

        po::notify(vm); // throws on error, so do after help in case
        // there are any problems

      }
      catch(po::error &e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return ERR_INVALID_ARG;
      }
    }
    catch(std::exception &e) {
      std::cerr << "Unhandled Exception reached the top of main: "
                << e.what() << ", application will now exit" << std::endl;
      return ERR_UNKNOWN;

    }

    const fs::path &root = args.root;
    if(root.empty()) {
      args.err_log.error("No casm project found");
      args.err_log << std::endl;
      return ERR_NO_PROJ;
    }

    DirectoryStructure dir(root);
    ProjectSettings set(root);
    if(set.view_command().empty()) {
      std::cerr << "Error in 'casm view': No command set. Use 'casm settings "
                "--set-view-command' to set the command to open visualization "
                "software. It should take one argument, the path to a POSCAR "
                "to be visualized. For example, to use VESTA on Mac: casm settings --set-view-command 'casm.view \"open -a /Applications/VESTA/VESTA.app\"'.\n";
      return ERR_MISSING_DEPENDS;
    }

    // If 'args.primclex', use that, else construct PrimClex in 'uniq_primclex'
    // Then whichever exists, store reference in 'primclex'
    std::unique_ptr<PrimClex> uniq_primclex;
    PrimClex &primclex = make_primclex_if_not(args, uniq_primclex);

    ConfigSelection<false> config_select;
    if(!vm.count("config")) {
      config_select = ConfigSelection<false>(primclex, "NONE");
    }
    else if(selection == "MASTER") {
      config_select = ConfigSelection<false>(primclex);
    }
    else {
      config_select = ConfigSelection<false>(primclex, selection);
    }

    // add --configname (or positional) input
    for(int i = 0; i < configname.size(); i++) {
      config_select.set_selected(configname[i], true);
    }

    fs::path tmp_dir = root / ".casm" / "tmp";
    fs::create_directory(tmp_dir);

    // execute the 'casm view' command for each selected configuration
    for(auto it = config_select.selected_config_cbegin(); it != config_select.selected_config_cend(); ++it) {
      // write '.casm/tmp/POSCAR'
      fs::ofstream file;
      fs::path POSCARpath = tmp_dir / "POSCAR";
      file.open(POSCARpath);
      VaspIO::PrintPOSCAR p(*it);
      p.sort();
      p.print(file);
      file.close();

      std::cout << it->name() << ":\n";
      Popen popen;
      popen.popen(set.view_command() + " " + POSCARpath.string());
      popen.print(std::cout);

    }

    return 0;
  };

}
