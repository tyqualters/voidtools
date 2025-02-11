#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <optional>

#ifdef _WIN32
#include "win_exports.hpp"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#else
#include "unix_exports.hpp"
#endif

#undef min // for VS2022 BuildTools (MSVC) for cxxopts
#undef max // for VS2022 BuildTools (MSVC) for cxxopts
#include <cxxopts.hpp>
#include <sol/sol.hpp>
#include <fmt/color.h>
#define RINI_IMPLEMENTATION
#include <rini.h>

namespace fs {
    std::filesystem::path StringToPath(const std::string& path) {
        return std::filesystem::path{path};
    }
    std::string PathToString(const std::filesystem::path& path) {
        return path.string();
    }
    std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) {
        return std::filesystem::absolute(path);
    }
    bool FileExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path);
    }
    std::string GetFileExtension(const std::filesystem::path& path) {
        return path.has_extension() ? path.extension().string() : std::string();
    }
    std::string GetFileNameWithExtension(const std::filesystem::path& path) {
        return path.filename().string();
    }
    std::string GetFileName(const std::filesystem::path& path) {
        if(!path.has_extension()) return path.filename().string();
        else return path.filename().string().substr(0, path.filename().string().length() - path.extension().string().length());
    }
    std::vector<std::filesystem::path> GetPATHDirectories() {
        const auto path = std::string(std::getenv("PATH"));
        std::vector<std::filesystem::path> dirs;
#ifdef _WIN32
        char delim = ';';
#else
        char delim = ':';
#endif
        std::stringstream ss(path);
        std::string _dir;
        while(std::getline(ss, _dir, delim)) {
            auto _path = fs::StringToPath(_dir);
            if(fs::FileExists(_path)) dirs.push_back(_path);
        }
        return dirs;
    }
}

namespace str {
    std::string TrimString(const std::string& str) {
        std::stringstream ss;
        for(char it : str) {
            if(std::isspace(it)) continue;
            ss << it;
        }
        return ss.str();
    }
}

class ConfigurationFile {
public:
    ConfigurationFile(std::string fileName) {
        this->config = rini_load_config(fileName.c_str());
    }
    [[no_discard]] std::string getValue(std::string key) const {
        return {rini_get_config_value_text(this->config, key.c_str())};
    }
    ~ConfigurationFile() {
        rini_unload_config(&this->config);
    }
private:
    rini_config config;
};

std::optional<std::pair<std::string, std::string>> GetPackagePaths(std::string iniFile = "lua.ini");

void BindFunctions(sol::state& lua);

void StartLua(const std::string& filePath, std::vector<std::string>&& args, bool noConfirm) {
    sol::state lua;
    lua.open_libraries();
    lua.set("arguments", args);
    if(!noConfirm) {
        fmt::print(fmt::fg(fmt::color::red)|fmt::emphasis::bold, "\n<<CAUTION>>\n");
        fmt::print("You are about to run {}.\nDo you trust this file and wish to continue? (Y/n) ", fs::PathToString(fs::GetAbsolutePath(fs::StringToPath(filePath))));
        std::string userInput;
        do {
            std::getline(std::cin, userInput);
            userInput = str::TrimString(userInput);
        } while(userInput.empty());
        if(userInput[0] != 'y' and userInput[0] != 'Y') {
            fmt::println("Canceled...");
            return;
        }
    }
    BindFunctions(lua);
    lua.unsafe_script_file(filePath);
}

std::string SatisfyLuaFile(std::string scriptFile) {
    // All this function does is validate the script file exists

    /***
     * TODO: Sanitation check. Run every script path through <filesystem> to get ONLY the file name!
     * TODO: For the scripts option, IF a script ends in .lua, do the following:
     *      * Check the parsed path for the location
     *      * Perform the sanitation check
     *      * Check the current working directory
     *      * Check the scripts directory
     *      * Check the PATH directories
     *      * Confirm the file path with the user before running (unless -y is specified)
     * TODO: For the scripts option, IF a script has no file extension, do the following:
     *      * Perform sanitation check
     *      * Check the scripts directory
     *      * If not present, DO NOT search anywhere else
     *      * NOTE: Do not need to confirm before running
     */

    std::string scriptsDir = "../scripts/";
    std::filesystem::path scriptsPath{scriptsDir};

    if(scriptFile.ends_with(".lua")) {
        std::filesystem::path scriptPath{scriptFile};
        if(scriptPath.is_absolute()) {
            if(!fs::FileExists(scriptPath)) throw std::runtime_error("File does not exist.");
            else return scriptPath.string();
        } else {
            // It is relative
            // 1. Check CWD
            if(fs::FileExists(std::filesystem::current_path() / scriptPath)) {
                return (std::filesystem::current_path() / scriptPath).string();
            }
            // 2. Check Scripts
            if(fs::FileExists(scriptsPath / scriptPath)) {
                 return (scriptsPath / scriptPath).string();
            }
            // 3. Check PATH
            for(const auto& dir : fs::GetPATHDirectories()) {
                if(fs::FileExists(dir / scriptPath)) {
                    return (dir / scriptPath).string();
                }
            }
            fmt::println("Could not locate script.");
            std::exit(1);
            // throw std::runtime_error("Could not locate script.");
        }
    } else {

        scriptFile += ".lua";

        std::filesystem::path currentPath = scriptsPath / fs::GetFileNameWithExtension(scriptFile);

        if(std::filesystem::exists(currentPath)) {
            return currentPath.string();
        } else {
            fmt::print(stderr, "Script path unaccepted: {}\n", currentPath.string());
            throw std::runtime_error("Script not available");
        }
    }
}

std::vector<std::string> GetLuaFilesInDirectory(const std::string& dir) {
    std::vector<std::string> files{};
    auto path = std::filesystem::path{dir};
    if(std::filesystem::is_directory(path)) {
        for(const auto& file : std::filesystem::recursive_directory_iterator(path)) {
            auto& file_path = file.path();
            if(file.is_regular_file() && file_path.has_extension() && file_path.extension() == ".lua") {
                files.push_back(fs::GetFileName(file_path));
            }
        }
    }
    return files;
}

int main(int argc, char* argv[]) {

    cxxopts::Options options("voidtools", "v01d_r34l1ty's hacking tool");

    options.add_options()
        ("l,list", "List available scripts")
        ("r,run", "Run a script", cxxopts::value<std::string>()->default_value(""))
        ("script", "Script to run", cxxopts::value<std::string>())
        ("script-args", "Script arguments", cxxopts::value<std::vector<std::string>>())
        ("y", "Skip script confirmation", cxxopts::value<bool>()->implicit_value("true"))
        ("h,help", "Print usage");

    options.parse_positional({"run",  "script", "script-args", "y"});

    const auto args = options.parse(argc, argv);

    if(args.count("help")) {
        fmt::println("{}", options.help());
        return EXIT_SUCCESS;
    }

    if(args.count("list")) {
        fmt::println("Scripts:");
        std::string scriptsPath = "../scripts/";
        auto scripts = GetLuaFilesInDirectory(scriptsPath);
        for(const auto& script : scripts) {
            fmt::print(fmt::fg(fmt::color::aqua), "{}\n", script);
        }
        return EXIT_SUCCESS;
    }

    if(args.count("run")) {
        const auto& script = args["script"].as<std::string>();
        auto scriptArgs = args["script-args"].as<std::vector<std::string>>();

        fmt::print("{} ", script);
        for(const auto& arg : scriptArgs) {
            fmt::print("{} ", arg);
        }
        fmt::println(stdout, "");

        StartLua(SatisfyLuaFile(script), std::move(scriptArgs), !script.ends_with(".lua") || args.count("y"));
        return EXIT_SUCCESS;
    }

    fmt::println("{}", options.help());

    return EXIT_SUCCESS;
}

void BindFunctions(sol::state& lua) {
    // Here is where all the Lua functions go

    // EXTERN_C void InitNetworking();
    lua.set_function("InitNetworking", &InitNetworking);
    // EXTERN_C SOCKET CreateTCPSocket(bool ipv6);
    lua.set_function("CreateTCPSocket", &CreateTCPSocket);
    // EXTERN_C SOCKET CreateUDPSocket(bool ipv6);
    lua.set_function("CreateUDPSocket", &CreateUDPSocket);
    // EXTERN_C sockaddr_in CreateSocketAddress(std::string addr, bool ipv6, int port);
    lua.set_function("CreateSocketAddress", &CreateSocketAddress);
    // EXTERN_C void ConnectToSocket(SOCKET sock, sockaddr_in& server);
    lua.set_function("ConnectToSocket", &ConnectToSocket);
    // EXTERN_C void WriteToSocket(SOCKET socket, std::string data);
    lua.set_function("WriteToSocket", &WriteToSocket);
    // EXTERN_C void CloseSocket(SOCKET socket);
    lua.set_function("CloseSocket", &CloseSocket);
    // EXTERN_C void DestroyNetworking();
    lua.set_function("DestroyNetworking", &DestroyNetworking);


}

std::optional<std::pair<std::string, std::string>> GetPackagePaths(std::string iniFile) {
    const ConfigurationFile config(std::move(iniFile));

    std::string s_packPath = config.getValue("package-path");
    std::string s_packCPath = config.getValue("package-cpath");

    if(s_packPath.empty() && s_packCPath.empty()) {
        return std::nullopt;
    } else return std::make_pair(s_packPath, s_packCPath);
    // TODO: Set the package.path and package.cpath in Lua
    // TODO: Verify everything exists. This will allow for 'require' to work.
}