#include "TextureHolder.h"

#include <assert.h>

TextureHolder* TextureHolder::m_TextureHolder = nullptr;

TextureHolder::TextureHolder()
{
	assert(m_TextureHolder == nullptr);
	m_TextureHolder = this;
}

Texture& TextureHolder::GetTexture(std::string const& filename)
{
	auto& TexMapRef = m_TextureHolder->m_TextureMap;

	auto it = TexMapRef.find(filename);

	//if a texture found return  
	if (it != TexMapRef.end())
	{
		return it->second;
	}
	else //create new entry for map structure
	{
		auto& texture = TexMapRef[filename];
		texture.loadFromFile(filename);

		return texture;
	}
}
