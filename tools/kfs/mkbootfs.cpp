#include "kfs_image.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Mapping { std::string host; std::string guest; };

[[noreturn]] void usage(const std::string& error) {
    if (!error.empty()) std::cerr << "mkbootfs.kfs: " << error << "\n";
    std::cerr << "usage:\n"
              << "  mkbootfs.kfs create --output FILE --size 64M [--block-size 1024]"
              << " [--root DIR] [--dir /PATH] [--file HOST:/PATH]"
              << " [--executable /PATH] [--force]\n"
              << "  mkbootfs.kfs verify --image FILE [--file HOST:/PATH] [--list]\n";
    throw std::invalid_argument(error.empty() ? "usage" : error);
}

std::vector<uint8_t> read_host(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("cannot open host input " + path);
    auto count = in.tellg();
    if (count < 0 || uint64_t(count) > INT32_MAX)
        throw std::runtime_error("host input is too large " + path);
    std::vector<uint8_t> bytes(size_t(count), 0);
    in.seekg(0);
    if (!bytes.empty() && !in.read(reinterpret_cast<char*>(bytes.data()), bytes.size()))
        throw std::runtime_error("cannot read host input " + path);
    return bytes;
}

Mapping mapping(const std::string& arg) {
    auto sep = arg.find(':');
    if (sep == std::string::npos || sep == 0 || sep + 1 == arg.size() || arg[sep+1] != '/')
        usage("--file expects HOST_PATH:/absolute/guest/path");
    return {arg.substr(0, sep), arg.substr(sep+1)};
}

// Walk an already-staged root filesystem. Import paths are always relative
// to --root, never to the host filesystem root. Deliberately reject symlinks
// and special files: original KFS has no matching portable host semantics.
std::vector<Mapping> import_root(p9::kfs::Builder& builder,
                                 const std::filesystem::path& root) {
    namespace fs = std::filesystem;
    if (fs::is_symlink(fs::symlink_status(root)) || !fs::is_directory(root))
        throw std::runtime_error("--root must be a real directory: " + root.string());

    std::vector<fs::path> paths;
    for (const auto& item : fs::recursive_directory_iterator(root))
        paths.push_back(item.path());
    std::sort(paths.begin(), paths.end(), [&](const auto& a, const auto& b) {
        return a.lexically_relative(root).generic_string() <
               b.lexically_relative(root).generic_string();
    });

    std::vector<Mapping> imported;
    for (const auto& path : paths) {
        const auto relative = path.lexically_relative(root);
        const auto guest = "/" + relative.generic_string();
        const auto status = fs::symlink_status(path);
        if (fs::is_symlink(status))
            throw std::runtime_error("KFS cannot import symlink: " + path.string());
        if (fs::is_directory(status)) {
            builder.directory(guest);
        } else if (fs::is_regular_file(status)) {
            const auto mode = static_cast<uint16_t>(
                static_cast<unsigned>(status.permissions()) & 0777U);
            builder.file(guest, read_host(path.string()), mode);
            imported.push_back({path.string(), guest});
        } else {
            throw std::runtime_error("KFS cannot import special file: " + path.string());
        }
    }
    return imported;
}
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) usage("missing command");
        const std::string command(argv[1]);
        if (command != "create" && command != "verify") usage("unknown command " + command);
        std::string output;
        std::string root;
        std::string size = "64M";
        uint32_t blocksize = p9::kfs::kDefaultBlockSize;
        std::vector<Mapping> files;
        std::vector<std::string> dirs;
        std::set<std::string> executables;
        bool force = false, list = false;
        for (int i=2; i<argc; ++i) {
            std::string flag = argv[i];
            if (flag == "--force") force = true;
            else if (flag == "--list") list = true;
            else {
                if (i+1 >= argc) usage("missing value for " + flag);
                const std::string val(argv[++i]);
                if (flag == "--output" || flag == "--image") {
                    if (!output.empty()) usage("output/image specified more than once");
                    output=val;
                } else if (flag == "--root") root=val;
                else if (flag == "--size") size=val;
                else if (flag == "--block-size") blocksize=uint32_t(std::stoul(val));
                else if (flag == "--file") files.push_back(mapping(val));
                else if (flag == "--dir") dirs.push_back(val);
                else if (flag == "--executable") executables.insert(val);
                else usage("unknown argument " + flag);
            }
        }
        if (output.empty()) usage("missing --output/--image");
        if (command == "verify" && (force || !root.empty() || !dirs.empty() || !executables.empty()))
            usage("--root, --force, --dir, --executable are create-only options");
        if (command == "create") {
            p9::kfs::Builder builder;
            for (const auto& d : dirs) builder.directory(d);
            for (const auto& f : files)
                builder.file(f.guest, read_host(f.host),
                             executables.count(f.guest) ? 0755 : 0644);
            if (!root.empty()) {
                auto imported = import_root(builder, root);
                files.insert(files.end(), imported.begin(), imported.end());
            }
            builder.write(output, p9::kfs::parse_size(size), blocksize, force);
            std::cout << "KFS image created: " << output << "\n";
        }
        p9::kfs::Image image(output);
        image.verify();
        for (const auto& f : files) {
            const auto actual = image.read_file(f.guest);
            const auto expected = read_host(f.host);
            if (actual != expected)
                throw std::runtime_error("KFS image content mismatch for " + f.guest);
            std::cout << "PASS: " << f.guest << " (" << actual.size() << " bytes identical)\n";
        }
        if (list) {
            for (const auto& [path, e] : image.tree()) {
                std::cout << (e.directory ? 'd' : '-') << ' '
                          << std::oct << std::setw(3) << std::setfill('0') << (e.mode & 0777)
                          << std::dec << std::setfill(' ') << ' '
                          << std::setw(9) << e.size << "  " << path << "\n";
            }
        }
        std::cout << "KFS verified: " << image.blocks() << " blocks x "
                  << image.block_size() << " bytes, " << image.free_blocks() << " free\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "mkbootfs.kfs: " << e.what() << "\n";
        return 1;
    }
}
