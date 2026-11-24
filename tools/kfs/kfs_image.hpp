#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace p9::kfs {

// All on-disk integers are explicitly serialized little-endian. No native
// C++ struct is ever written to disk: the historical KFS Dentry is 88 bytes.
constexpr uint32_t kDefaultBlockSize = 1024;
constexpr uint32_t kDentrySize = 88;
constexpr uint32_t kNameLength = 28;
constexpr uint32_t kDirectBlocks = 6;

uint64_t parse_size(const std::string& size);

struct Entry {
    std::string name;
    bool directory = false;
    uint16_t mode = 0;
    uint32_t qid = 0;
    uint32_t size = 0;
    std::vector<uint32_t> direct;
    uint32_t indirect = 0;
};

class Builder {
public:
    Builder();
    ~Builder();
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    void directory(const std::string& path);
    void file(const std::string& path, std::vector<uint8_t> bytes,
              uint16_t mode = 0644);
    void write(const std::string& output, uint64_t image_bytes,
               uint32_t block_size = kDefaultBlockSize, bool force = false);

private:
    struct Node;
    std::unique_ptr<Node> root_;
    uint32_t next_qid_ = 10;
    uint32_t next_block_ = 3;
    uint32_t total_blocks_ = 0;
    uint32_t block_size_ = 0;
    uint32_t payload_ = 0;
    uint32_t timestamp_ = 0;
    std::fstream disk_;

    Node& parent(const std::string& path);
    void assign_qids(Node& node);
    void allocate(Node& node);
    std::vector<uint8_t> dentry(const Node& node) const;
    void emit(Node& node);
    uint32_t new_block();
    void put_block(uint32_t address, int16_t tag, uint32_t qid,
                   const std::vector<uint8_t>& payload);
};

class Image {
public:
    explicit Image(const std::string& path);
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Entry lookup(const std::string& path);
    std::vector<uint8_t> read_file(const std::string& path);
    std::vector<std::pair<std::string, Entry>> tree();
    void verify();
    uint32_t block_size() const { return block_size_; }
    uint32_t blocks() const { return blocks_; }
    uint32_t free_blocks() const { return free_blocks_; }

private:
    std::ifstream disk_;
    uint32_t block_size_ = 0;
    uint32_t payload_ = 0;
    uint32_t blocks_ = 0;
    uint32_t free_blocks_ = 0;
    uint32_t fsize_ = 0;
    std::vector<uint8_t> super_;
    Entry root_;

    std::vector<uint8_t> block(uint32_t address, int16_t tag, uint32_t qid);
    Entry decode_dentry(const uint8_t* data);
    std::vector<uint32_t> data_blocks(const Entry& entry);
    std::vector<Entry> children(const Entry& directory);
    void traverse(const Entry& entry, const std::string& path,
                  std::vector<std::pair<std::string, Entry>>& result);
};

}  // namespace p9::kfs
