#include "offsets.h"
#include <iostream>
#include "../include.h"

void COffsets::New(const std::string& name, std::uint64_t offset)
{
	m_Offsets[name] = offset;
}

std::uint64_t COffsets::Get(const std::string& name) const
{
	auto it = m_Offsets.find(name);
	if (it != m_Offsets.end())
	{
		return it->second;
	}
	return 0;
}

bool COffsets::Exists(const std::string& name) const
{
	return m_Offsets.find(name) != m_Offsets.end();
}

void COffsets::Remove(const std::string& name)
{
	auto it = m_Offsets.find(name);
	if (it != m_Offsets.end())
	{
		m_Offsets.erase(it);
	}
}

void COffsets::Clear()
{
	m_Offsets.clear();
}

void COffsets::init()
{
	
}