#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

class COffsets
{
public:
	void New(const std::string& name, std::uint64_t offset);
	std::uint64_t Get(const std::string& name) const;
	bool Exists(const std::string& name) const;
	void Remove(const std::string& name);
	void Clear();
	void init();

private:
	std::unordered_map<std::string, std::uint64_t> m_Offsets;
};