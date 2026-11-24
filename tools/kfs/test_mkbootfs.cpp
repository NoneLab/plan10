#include "kfs_image.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace p9::kfs;

int main() {
    const auto image = fs::temp_directory_path() / ("plan9-kfs-selftest-" + std::to_string(getpid()) + ".raw");
    try {
        Builder builder;
        std::vector<uint8_t> program(200'091);
        for (size_t i=0; i<program.size(); ++i) program[i]=uint8_t(i*31);
        builder.file("/amd64/init", program, 0755);
        builder.directory("/tmp");
        for (unsigned i=0; i<70; ++i)
            builder.file("/bin/f" + std::to_string(i), std::vector<uint8_t>(i+1, uint8_t(i)), 0755);
        builder.write(image.string(), 8*1024*1024);
        {
            Image reader(image.string());
            reader.verify();
            assert(reader.read_file("/amd64/init") == program);
            assert(reader.read_file("/bin/f69") == std::vector<uint8_t>(70, 69));
            assert(reader.lookup("/bin").indirect != 0);
            assert(reader.lookup("/amd64/init").indirect != 0);
            assert(reader.lookup("/tmp").directory);
            assert(reader.free_blocks() > 0);
        }
        // Corrupt the trailing KFS Tag of an allocated file block.
        {
            Image r(image.string());
            const auto d = r.lookup("/amd64/init");
            std::fstream f(image, std::ios::binary | std::ios::in | std::ios::out);
            f.seekp(uint64_t(d.direct.front()) * 1024 + 1018);
            const char invalid[2]={0,0};
            f.write(invalid, sizeof invalid);
        }
        bool rejected=false;
        try { Image broken(image.string()); broken.verify(); }
        catch (const std::exception&) { rejected=true; }
        assert(rejected);
        fs::remove(image);
        std::cout << "mkbootfs.kfs: PASS direct/indirect, 70-entry directory, free-list, corruption\n";
        return 0;
    } catch (const std::exception& e) {
        fs::remove(image);
        std::cerr << "mkbootfs.kfs: FAIL " << e.what() << "\n";
        return 1;
    }
}
