#include "kfs_image.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cstring>
#include <filesystem>
#include <limits>
#include <set>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <unistd.h>

namespace fs = std::filesystem;
namespace p9::kfs {
namespace {
constexpr uint16_t kAllocated = 0x8000;
constexpr uint16_t kDirectory = 0x4000;
constexpr uint32_t kQidDir = 0x80000000U;
constexpr int16_t kSuper = 1, kDir = 2, kInd1 = 3, kFile = 5, kFree = 6;
constexpr uint32_t kRootQid = 1, kSuperQid = 2;
constexpr std::string_view kMagic = "kfs wren device\n";

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error("KFS: " + message);
}

uint16_t get16(const uint8_t* b) {
    return uint16_t(b[0]) | (uint16_t(b[1]) << 8);
}
uint32_t get32(const uint8_t* b) {
    return uint32_t(b[0]) | (uint32_t(b[1]) << 8) |
           (uint32_t(b[2]) << 16) | (uint32_t(b[3]) << 24);
}
void put16(uint8_t* b, uint16_t v) {
    b[0] = uint8_t(v); b[1] = uint8_t(v >> 8);
}
void put32(uint8_t* b, uint32_t v) {
    for (unsigned i = 0; i != 4; ++i) b[i] = uint8_t(v >> (i * 8));
}

std::vector<std::string> components(const std::string& path) {
    if (path.empty() || path[0] != '/') fail("absolute guest path required: " + path);
    if (path == "/") return {};
    if (path.back() == '/') fail("trailing '/' in guest path: " + path);
    std::vector<std::string> result;
    size_t start = 1;
    while (start < path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string::npos) end = path.size();
        std::string part = path.substr(start, end - start);
        if (part.empty() || part == "." || part == ".." || part.size() >= kNameLength)
            fail("invalid KFS component (max 27 bytes): " + part);
        result.push_back(std::move(part));
        start = end + 1;
    }
    return result;
}

}  // namespace

uint64_t parse_size(const std::string& text) {
    if (text.empty()) fail("empty size");
    std::string s = text;
    uint64_t mul = 1;
    if (s.back() == 'K' || s.back() == 'k') { mul = 1024; s.pop_back(); }
    else if (s.back() == 'M' || s.back() == 'm') { mul = 1024ULL*1024; s.pop_back(); }
    else if (s.back() == 'G' || s.back() == 'g') { mul = 1024ULL*1024*1024; s.pop_back(); }
    size_t used = 0;
    unsigned long long num;
    try { num = std::stoull(s, &used, 0); }
    catch (const std::exception&) { fail("invalid image size " + text); }
    if (used != s.size() || num == 0 || num > UINT64_MAX / mul)
        fail("invalid or overflowing image size " + text);
    return num * mul;
}

struct Builder::Node {
    std::string name;
    bool directory;
    uint16_t mode;
    uint32_t qid;
    std::vector<uint8_t> bytes;
    std::map<std::string, std::unique_ptr<Node>> children;
    std::vector<uint32_t> blocks;
    uint32_t indirect = 0;
};

Builder::Builder() : root_(std::make_unique<Node>(Node{"/", true, 0777, kRootQid, {}, {}, {}, 0})) {}
Builder::~Builder() = default;

void Builder::directory(const std::string& path) {
    Node* node = root_.get();
    for (const auto& comp : components(path)) {
        auto& child = node->children[comp];
        if (!child) child = std::make_unique<Node>(Node{comp, true, 0755, 0, {}, {}, {}, 0});
        if (!child->directory) fail("a file obstructs directory " + path);
        node = child.get();
    }
}

Builder::Node& Builder::parent(const std::string& path) {
    const auto parts = components(path);
    if (parts.empty()) fail("cannot overwrite KFS root");
    std::string parent_path;
    for (size_t i = 0; i + 1 < parts.size(); ++i) parent_path += "/" + parts[i];
    directory(parent_path.empty() ? "/" : parent_path);
    Node* node = root_.get();
    for (size_t i = 0; i + 1 < parts.size(); ++i) node = node->children.at(parts[i]).get();
    return *node;
}

void Builder::file(const std::string& path, std::vector<uint8_t> bytes, uint16_t mode) {
    const auto parts = components(path);
    if (parts.empty()) fail("root is not a file");
    Node& p = parent(path);
    if (p.children.count(parts.back())) fail("duplicate KFS destination " + path);
    if (bytes.size() > INT32_MAX) fail("file exceeds signed 32-bit size");
    p.children.emplace(parts.back(), std::make_unique<Node>(
        Node{parts.back(), false, uint16_t(mode & 0777), 0, std::move(bytes), {}, {}, 0}));
}

uint32_t Builder::new_block() {
    if (next_block_ >= total_blocks_) fail("image too small for files and metadata");
    return next_block_++;
}

void Builder::assign_qids(Node& node) {
    if (!node.qid) {
        if (next_qid_ >= INT32_MAX) fail("out of QID values");
        node.qid = ++next_qid_;
    }
    for (auto& [name, child] : node.children) assign_qids(*child);
}

void Builder::allocate(Node& node) {
    for (auto& [name, child] : node.children) allocate(*child);
    const uint32_t n = node.directory
        ? uint32_t((node.children.size() + payload_/kDentrySize - 1) / (payload_/kDentrySize))
        : uint32_t((node.bytes.size() + payload_ - 1) / payload_);
    if (n > kDirectBlocks + payload_/4)
        fail("file/directory needs double-indirect blocks: " + node.name);
    for (uint32_t i = 0; i < n; ++i) node.blocks.push_back(new_block());
    if (n > kDirectBlocks) node.indirect = new_block();
}

std::vector<uint8_t> Builder::dentry(const Node& node) const {
    std::vector<uint8_t> b(kDentrySize, 0);
    if (node.name.size() >= kNameLength) fail("name exceeds KFS dentry");
    std::memcpy(b.data(), node.name.data(), node.name.size());
    put16(b.data() + 28, 0xffff); // original KFS ream uses uid=-1
    put16(b.data() + 30, 0xffff);
    put16(b.data() + 32, uint16_t(kAllocated | (node.directory ? kDirectory : 0) | node.mode));
    put32(b.data() + 36, node.qid | (node.directory ? kQidDir : 0));
    put32(b.data() + 40, 0);
    put32(b.data() + 44, node.directory ? 0 : uint32_t(node.bytes.size()));
    for (size_t i = 0; i < std::min(size_t(kDirectBlocks), node.blocks.size()); ++i)
        put32(b.data() + 48 + 4*i, node.blocks[i]);
    put32(b.data() + 72, node.indirect);
    put32(b.data() + 80, timestamp_);
    put32(b.data() + 84, timestamp_);
    return b;
}

void Builder::put_block(uint32_t address, int16_t tag, uint32_t qid,
                        const std::vector<uint8_t>& contents) {
    if (!address || address >= total_blocks_ || contents.size() > payload_)
        fail("invalid block write");
    std::vector<uint8_t> block(block_size_, 0);
    std::copy(contents.begin(), contents.end(), block.begin());
    put16(block.data() + payload_, 0);
    put16(block.data() + payload_ + 2, uint16_t(tag));
    put32(block.data() + payload_ + 4, qid & ~kQidDir);
    disk_.seekp(uint64_t(address) * block_size_);
    disk_.write(reinterpret_cast<const char*>(block.data()), std::streamsize(block.size()));
    if (!disk_) fail("disk image write failed");
}

void Builder::emit(Node& node) {
    for (auto& [name, child] : node.children) emit(*child);
    if (node.directory) {
        const size_t per = payload_ / kDentrySize;
        std::vector<std::vector<uint8_t>> entries;
        for (auto& [name, child] : node.children) entries.push_back(dentry(*child));
        for (size_t i = 0; i < node.blocks.size(); ++i) {
            std::vector<uint8_t> contents;
            for (size_t j = i*per; j < std::min((i+1)*per, entries.size()); ++j)
                contents.insert(contents.end(), entries[j].begin(), entries[j].end());
            put_block(node.blocks[i], kDir, node.qid, contents);
        }
    } else {
        for (size_t i = 0; i < node.blocks.size(); ++i) {
            const size_t start = i * payload_;
            const size_t stop = std::min(start + payload_, node.bytes.size());
            put_block(node.blocks[i], kFile, node.qid,
                      std::vector<uint8_t>(node.bytes.begin()+start, node.bytes.begin()+stop));
        }
    }
    if (node.indirect) {
        std::vector<uint8_t> refs((node.blocks.size()-kDirectBlocks)*4, 0);
        for (size_t i = kDirectBlocks; i < node.blocks.size(); ++i)
            put32(refs.data() + (i-kDirectBlocks)*4, node.blocks[i]);
        put_block(node.indirect, kInd1, node.qid, refs);
    }
}

void Builder::write(const std::string& output, uint64_t image_bytes,
                    uint32_t block_size, bool force) {
    if (block_size < 1024 || block_size > 16384 || block_size % 512 != 0)
        fail("KFS block size must be a multiple of 512, 1024..16384");
    if (image_bytes < 4ULL*block_size || image_bytes % block_size)
        fail("image size must be block-aligned and at least four blocks");
    if (image_bytes / block_size > INT32_MAX) fail("too many 32-bit KFS blocks");
    if (fs::exists(output) && !force) fail("output exists (specify --force): " + output);
    block_size_ = block_size;
    payload_ = block_size_ - 8;
    total_blocks_ = uint32_t(image_bytes / block_size_);
    next_block_ = 3;
    next_qid_ = 10;
    timestamp_ = uint32_t(std::min<int64_t>(std::time(nullptr), INT32_MAX));
    assign_qids(*root_);
    allocate(*root_);
    fs::create_directories(fs::path(output).parent_path().empty() ? "." : fs::path(output).parent_path());
    fs::path temp = output + ".partial." + std::to_string(::getpid());
    if (fs::exists(temp)) fail("temporary file already exists: " + temp.string());
    try {
        { std::ofstream create(temp, std::ios::binary | std::ios::trunc);
          if (!create) fail("failed to create " + temp.string()); }
        fs::resize_file(temp, image_bytes); // sparse RAW image
        disk_.open(temp, std::ios::binary | std::ios::in | std::ios::out);
        if (!disk_) fail("failed to open writable sparse image");
        emit(*root_);
        const auto root_block = dentry(*root_);
        put_block(2, kDir, kRootQid, root_block);
        // Block 0 is a raw Wren header, unlike ordinary tagged KFS blocks.
        std::vector<uint8_t> header(block_size_, 0);
        const std::string label = std::string(kMagic) + std::to_string(block_size_) + "\n";
        if (256 + label.size() >= header.size()) fail("header too small");
        std::copy(label.begin(), label.end(), header.begin()+256);
        disk_.seekp(0);
        disk_.write(reinterpret_cast<const char*>(header.data()), std::streamsize(header.size()));
        if (!disk_) fail("KFS Wren header write failed");
        // Historical addfree() from kfs/sub.c: free[0] is the next popped
        // address; every FEPERBUF-th block stores the previous free stack.
        const uint32_t per = (payload_ - 32 - 4) / 4;
        std::vector<uint32_t> stack(per, 0);
        uint32_t nfree = 1;
        uint32_t tfree = 0;
        for (uint32_t a = total_blocks_-1; a >= next_block_; --a) {
            if (nfree >= per) {
                std::vector<uint8_t> chain((1+per)*4, 0);
                put32(chain.data(), nfree);
                for (size_t i=0; i<per; ++i) put32(chain.data()+4+4*i, stack[i]);
                put_block(a, kFree, 0, chain);
                nfree = 0;
            }
            stack.at(nfree++) = a;
            ++tfree;
        }
        std::vector<uint8_t> sb(payload_, 0);
        put32(sb.data()+0, 1);
        put32(sb.data()+4, total_blocks_);
        put32(sb.data()+8, tfree);
        put32(sb.data()+12, next_qid_);
        put32(sb.data()+16, 0); // fsok, as in superream()
        put32(sb.data()+32, nfree);
        for (size_t i=0; i<per; ++i) put32(sb.data()+36+4*i, stack[i]);
        put_block(1, kSuper, kSuperQid, sb);
        disk_.flush();
        if (!disk_) fail("image flush failed");
        disk_.close();
        fs::rename(temp, output);
    } catch (...) {
        if (disk_.is_open()) disk_.close();
        std::error_code ec;
        fs::remove(temp, ec);
        throw;
    }
}

Image::Image(const std::string& path) : disk_(path, std::ios::binary) {
    if (!disk_) fail("cannot read image " + path);
    disk_.seekg(0, std::ios::end);
    const auto bytes = disk_.tellg();
    if (bytes < 4096) fail("KFS disk image too small");
    disk_.seekg(256);
    std::array<char, 64> header{};
    disk_.read(header.data(), std::streamsize(header.size()));
    if (std::memcmp(header.data(), kMagic.data(), kMagic.size()))
        fail("KFS Wren magic mismatch");
    auto pos = std::string_view(header.data()+kMagic.size());
    const size_t newline = pos.find('\n');
    if (newline == std::string_view::npos) fail("invalid KFS block-size header");
    const std::string blocktext(pos.substr(0, newline));
    try { block_size_ = uint32_t(std::stoul(blocktext)); }
    catch (const std::exception&) { fail("bad KFS block size header"); }
    if (block_size_ < 1024 || block_size_ > 16384 || block_size_ % 512 || uint64_t(bytes)%block_size_)
        fail("invalid KFS block size or image length");
    payload_ = block_size_ - 8;
    blocks_ = uint32_t(uint64_t(bytes) / block_size_);
    super_ = block(1, kSuper, kSuperQid);
    const uint32_t start = get32(super_.data());
    fsize_ = get32(super_.data()+4);
    free_blocks_ = get32(super_.data()+8);
    if (start != 1 || fsize_ != blocks_ || free_blocks_ > blocks_-3)
        fail("KFS superblock size/free count invalid");
    const auto root_block = block(2, kDir, kRootQid);
    root_ = decode_dentry(root_block.data());
    if (root_.name != "/" || !root_.directory || root_.qid != kRootQid)
        fail("root directory invalid");
}

std::vector<uint8_t> Image::block(uint32_t address, int16_t tag, uint32_t qid) {
    if (address == 0 || address >= blocks_) fail("out-of-range KFS block");
    std::vector<uint8_t> b(block_size_, 0);
    disk_.seekg(uint64_t(address)*block_size_);
    if (!disk_.read(reinterpret_cast<char*>(b.data()), std::streamsize(b.size())))
        fail("short KFS block read");
    if (get16(b.data()+payload_+2) != uint16_t(tag) ||
        (qid && get32(b.data()+payload_+4) != (qid & ~kQidDir)))
        fail("KFS tag or QID mismatch at block " + std::to_string(address));
    return b;
}

Entry Image::decode_dentry(const uint8_t* d) {
    Entry e;
    const auto end = std::find(d, d+kNameLength, uint8_t(0));
    e.name.assign(reinterpret_cast<const char*>(d), reinterpret_cast<const char*>(end));
    e.mode = get16(d+32);
    e.directory = (e.mode & kDirectory) != 0;
    e.qid = get32(d+36) & ~kQidDir;
    e.size = get32(d+44);
    for (int i=0; i<6; ++i) if (get32(d+48+i*4)) e.direct.push_back(get32(d+48+i*4));
    e.indirect = get32(d+72);
    return e;
}

std::vector<uint32_t> Image::data_blocks(const Entry& e) {
    std::vector<uint32_t> result = e.direct;
    if (e.indirect) {
        auto b = block(e.indirect, kInd1, e.qid);
        for (size_t i=0; i<payload_/4; ++i) {
            const auto addr = get32(b.data()+i*4);
            if (addr) result.push_back(addr);
        }
    }
    return result;
}

std::vector<Entry> Image::children(const Entry& e) {
    if (!e.directory) fail("not a directory");
    std::vector<Entry> result;
    for (auto addr : data_blocks(e)) {
        const auto b = block(addr, kDir, e.qid);
        for (size_t i=0; i<payload_/kDentrySize; ++i) {
            const uint8_t* ptr = b.data()+i*kDentrySize;
            if (get16(ptr+32) & kAllocated) result.push_back(decode_dentry(ptr));
        }
    }
    return result;
}

Entry Image::lookup(const std::string& path) {
    Entry cur = root_;
    for (const auto& comp : components(path)) {
        bool found = false;
        for (auto& ch : children(cur)) if (ch.name == comp) {cur=std::move(ch); found=true;break;}
        if (!found) fail("KFS file not found: " + path);
    }
    return cur;
}

std::vector<uint8_t> Image::read_file(const std::string& path) {
    auto ent = lookup(path);
    if (ent.directory) fail("is a KFS directory: " + path);
    if (ent.size > blocks_ * uint64_t(payload_)) fail("oversized KFS entry");
    std::vector<uint8_t> data;
    for (auto addr : data_blocks(ent)) {
        const auto b = block(addr, kFile, ent.qid);
        data.insert(data.end(), b.begin(), b.begin()+payload_);
    }
    if (data.size() < ent.size) fail("file shorter than recorded length: " + path);
    data.resize(ent.size);
    return data;
}

void Image::traverse(const Entry& e, const std::string& path,
                     std::vector<std::pair<std::string, Entry>>& result) {
    if (result.size() > blocks_) fail("cyclic or corrupt KFS directory tree");
    result.emplace_back(path, e);
    if (e.directory) for (auto& ch : children(e))
        traverse(ch, path == "/" ? "/"+ch.name : path+"/"+ch.name, result);
}
std::vector<std::pair<std::string, Entry>> Image::tree() {
    std::vector<std::pair<std::string, Entry>> result;
    traverse(root_, "/", result);
    return result;
}

void Image::verify() {
    const auto entries = tree();
    std::set<uint32_t> used{1, 2};
    for (const auto& [path, e] : entries) {
        if (e.indirect && !used.insert(e.indirect).second)
            fail("duplicate indirect block: " + path);
        const auto data = data_blocks(e);
        for (const auto addr : data) {
            if (!used.insert(addr).second) fail("duplicate used data block: " + path);
            // Check each block's actual type and owner, not just its address.
            block(addr, e.directory ? kDir : kFile, e.qid);
        }
        if (!e.directory) {
            if (uint64_t(data.size())*payload_ < e.size)
                fail("missing file data blocks: " + path);
        }
    }
    const uint32_t per = (payload_-32-4)/4;
    uint32_t nfree = get32(super_.data()+32);
    if (nfree < 1 || nfree > per) fail("invalid KFS super free count");
    std::vector<uint32_t> free_stack(per);
    for (size_t i=0; i<per; ++i) free_stack[i]=get32(super_.data()+36+i*4);
    std::set<uint32_t> free_set;
    for (uint32_t n=0; n<free_blocks_; ++n) {
        if (!nfree) fail("free stack exhausted unexpectedly");
        const auto addr=free_stack[--nfree];
        if (addr < 3 || addr >= blocks_ || !free_set.insert(addr).second)
            fail("KFS free block out of range or duplicated");
        if (!nfree) {
            const auto chain=block(addr, kFree, 0);
            nfree = get32(chain.data());
            if (nfree < 1 || nfree > per) fail("invalid chained free stack");
            for (size_t i=0; i<per; ++i) free_stack[i]=get32(chain.data()+4+i*4);
        }
    }
    for (auto addr : free_set) if (used.count(addr)) fail("KFS free/used overlap");
    if (free_set.size()+used.size()!=blocks_-1)
        fail("KFS allocation accounting incorrect");
}

} // namespace p9::kfs
